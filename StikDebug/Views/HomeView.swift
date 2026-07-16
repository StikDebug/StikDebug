//
//  HomeView.swift
//  StikDebug
//
//  Created by Stephen on 3/26/25.
//

import SwiftUI

struct HomeView: View {
    @AppStorage("autoQuitAfterEnablingJIT") private var doAutoQuitAfterEnablingJIT = false
    @AppStorage("bundleID") private var bundleID: String = ""
    @AppStorage(UserDefaults.Keys.confirmExternalJITRequests) private var confirmExternalJITRequests = true
    @AppStorage("keepAppAliveBackground") private var keepAppAliveBackground = false

    @ObservedObject private var mounting = MountingProgress.shared
    @ObservedObject private var aliveManager = BackgroundAliveManager.shared

    @State private var hasAppeared = false
    @State private var pendingJITEnableConfiguration: JITEnableConfiguration?
    @State private var isShowingPairingFilePicker = false
    @State private var debugFeedback: DebugFeedback?
    @State private var pendingExternalURLAction: HomeExternalAction?
    @State private var scriptRunModel: RunJSViewModel?

    private let timer = Timer.publish(every: 1, on: .main, in: .common).autoconnect()

    private struct DebugFeedback: Identifiable {
        let id = UUID()
        let message: String
        let isError: Bool
        let isWorking: Bool
    }

    var body: some View {
        InstalledAppsListView(onSelectApp: { selectedBundle, selectedName in
            bundleID = selectedBundle
            Haptics.medium()
            if keepAppAliveBackground {
                startKeepAlive(bundleID: selectedBundle, displayName: selectedName)
            } else {
                startJITInBackground(bundleID: selectedBundle, displayName: selectedName)
            }
        }, showDoneButton: false, onImportPairingFile: { isShowingPairingFilePicker = true }, onHoldApp: { selectedBundle, selectedName in
            // Selecting an app from the "Other" tab while keep-alive is enabled
            // holds it in the background (and shows the banner) instead of just
            // launching it — the same behavior as the JIT tab.
            bundleID = selectedBundle
            Haptics.medium()
            startKeepAlive(bundleID: selectedBundle, displayName: selectedName)
        })
        .overlay(alignment: .top) {
            if let activeApp = aliveManager.activeAppName {
                keepAliveBanner(appName: activeApp)
                    .padding(.top, 8)
                    .transition(.move(edge: .top).combined(with: .opacity))
            }
        }
        .overlay(alignment: .bottom) {
            if let debugFeedback {
                debugFeedbackView(debugFeedback)
                    .padding(.bottom, 40)
                    .transition(.move(edge: .bottom).combined(with: .opacity))
            }
        }
        .animation(.default, value: aliveManager.activeAppName)
        .onAppear(perform: handleAppear)
        .onReceive(NotificationCenter.default.publisher(for: .intentJSScriptReady), perform: handleScriptReadyNotification)
        .onReceive(timer) { _ in
            refreshMountStatusIfNeeded()
        }
        .onOpenURL { url in
            handleExternalURL(url)
        }
        .confirmationDialog(
            pendingExternalURLAction?.title ?? "External Request",
            isPresented: Binding(
                get: { pendingExternalURLAction != nil },
                set: { isPresented in
                    if !isPresented {
                        pendingExternalURLAction = nil
                    }
                }
            ),
            titleVisibility: .visible,
            presenting: pendingExternalURLAction
        ) { action in
            Button(action.confirmationTitle, role: action.role) {
                performExternalURLAction(action)
                pendingExternalURLAction = nil
            }
            Button("Cancel", role: .cancel) {
                pendingExternalURLAction = nil
            }
        } message: { action in
            Text(action.message)
        }
        .fileImporter(
            isPresented: $isShowingPairingFilePicker,
            allowedContentTypes: PairingFileStore.supportedContentTypes,
            onCompletion: importPairingFile
        )
        .sheet(item: $scriptRunModel) { model in
            NavigationStack {
                RunJSView(model: model)
                    .toolbar {
                        ToolbarItem(placement: .topBarTrailing) {
                            Button("Done") { scriptRunModel = nil }
                        }
                    }
                    .navigationBarTitleDisplayMode(.inline)
            }
        }
    }

    private func handleAppear() {
        startTunnelInBackground()
        MountingProgress.shared.checkforMounted()
        hasAppeared = true

        if let config = pendingJITEnableConfiguration {
            startJITInBackground(
                bundleID: config.bundleID,
                pid: config.pid,
                scriptData: config.scriptData,
                scriptName: config.scriptName,
                triggeredByURLScheme: true
            )
            pendingJITEnableConfiguration = nil
        }
    }

    private func handleScriptReadyNotification(_ notification: Notification) {
        guard let model = notification.userInfo?["model"] as? RunJSViewModel else {
            return
        }

        scriptRunModel = model
    }

    private func refreshMountStatusIfNeeded() {
        guard mounting.mountingThread == nil, !mounting.coolisMounted else {
            return
        }
        MountingProgress.shared.checkforMounted()
    }

    private func importPairingFile(_ result: Result<URL, Error>) {
        switch result {
        case .success(let url):
            do {
                try PairingFileStore.importFromPicker(url)
                markTunnelDisconnected()
                startTunnelInBackground()
                NotificationCenter.default.post(name: .pairingFileImported, object: nil)
                AlertPresenter.dismissPresentedAlert()
            } catch {
                LogManager.shared.addErrorLog("Failed to import pairing file: \(error.localizedDescription)")
            }
        case .failure(let error):
            LogManager.shared.addErrorLog("Pairing file picker failed: \(error.localizedDescription)")
        }
    }

    private func queryValue(_ names: [String], in components: URLComponents?) -> String? {
        guard let queryItems = components?.queryItems else { return nil }
        for name in names {
            if let rawValue = queryItems.first(where: { $0.name.caseInsensitiveCompare(name) == .orderedSame })?.value {
                let value = rawValue.trimmingCharacters(in: .whitespacesAndNewlines)
                if !value.isEmpty {
                    return value
                }
            }
        }
        return nil
    }

    private func handleExternalURL(_ url: URL) {
        guard let host = url.host()?.lowercased() else { return }
        let components = URLComponents(url: url, resolvingAgainstBaseURL: false)

        switch host {
        case "enable-jit":
            var config = JITEnableConfiguration()
            if let pidStr = queryValue(["pid"], in: components), let pid = Int(pidStr) {
                config.pid = pid
            }
            if let bundleId = queryValue(["bundle-id", "bundleID", "bundle_id", "bundleId"], in: components) {
                config.bundleID = bundleId
            }
            if let scriptBase64URL = queryValue(["script-data", "scriptData", "script_data"], in: components)?.removingPercentEncoding {
                let base64 = base64URLToBase64(scriptBase64URL)
                if let scriptData = Data(base64Encoded: base64) {
                    config.scriptData = scriptData
                }
            }
            if let scriptName = queryValue(["script-name", "scriptName", "script_name"], in: components) {
                config.scriptName = scriptName
            }
            if config.scriptData == nil, let bundleID = config.bundleID,
               let scriptInfo = ScriptStore.preferredScript(for: bundleID) {
                config.scriptData = scriptInfo.data
                config.scriptName = scriptInfo.name
            }
            let action = HomeExternalAction.enableJIT(config)
            if confirmExternalJITRequests {
                pendingExternalURLAction = action
            } else {
                performExternalURLAction(action)
            }
        case "kill-process":
            if let pidStr = queryValue(["pid"], in: components), let pid = Int(pidStr) {
                pendingExternalURLAction = .killProcess(pid)
            }
        case "launch-app":
            if let bundleId = queryValue(["bundle-id", "bundleID", "bundle_id", "bundleId"], in: components) {
                pendingExternalURLAction = .launchApp(bundleId)
            }
        default:
            break
        }
    }

    private func performExternalURLAction(_ action: HomeExternalAction) {
        switch action {
        case .enableJIT(let config):
            if hasAppeared {
                startJITInBackground(
                    bundleID: config.bundleID,
                    pid: config.pid,
                    scriptData: config.scriptData,
                    scriptName: config.scriptName,
                    triggeredByURLScheme: true
                )
            } else {
                pendingJITEnableConfiguration = config
            }
        case .killProcess(let pid):
            markTunnelDisconnected()
            startTunnelInBackground(showErrorUI: false)
            DispatchQueue.global(qos: .userInitiated).async {
                sleep(1)
                do {
                    try JITEnableContext.shared.killProcess(withPID: Int32(pid))
                    DispatchQueue.main.async {
                        LogManager.shared.addInfoLog("Killed process \(pid) via URL scheme")
                    }
                } catch {
                    DispatchQueue.main.async {
                        LogManager.shared.addErrorLog("Failed to kill process \(pid): \(error.localizedDescription)")
                    }
                }
            }
        case .launchApp(let bundleID):
            Haptics.medium()
            DispatchQueue.global(qos: .userInitiated).async {
                let _ = JITEnableContext.shared.launchAppWithoutDebug(bundleID, logger: nil)
            }
        }
    }

    private func keepAliveBanner(appName: String) -> some View {
        HStack(spacing: 10) {
            Image(systemName: "bolt.circle.fill")
                .foregroundStyle(.green)
            VStack(alignment: .leading, spacing: 1) {
                Text(String(format: "Keeping %@ alive".localized, appName))
                    .font(.subheadline.weight(.semibold))
                Text("Held in the background".localized)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            Spacer(minLength: 8)
            Button("Stop".localized) {
                Haptics.medium()
                BackgroundAliveManager.shared.stop()
            }
            .buttonStyle(.borderedProminent)
            .controlSize(.small)
            .tint(.red)
        }
        .padding(.horizontal, 14)
        .padding(.vertical, 10)
        .background(Capsule().fill(.ultraThinMaterial))
        .shadow(radius: 4)
        .padding(.horizontal, 20)
        .accessibilityElement(children: .combine)
        .accessibilityLabel(String(format: "Keeping %@ alive in the background".localized, appName))
    }

    private func startKeepAlive(bundleID: String, displayName: String?) {
        let name = displayName ?? bundleID
        guard !aliveManager.isActive else {
            showAlert(
                title: "Keep-Alive Active".localized,
                message: "StikDebug is already holding an app alive in the background. Stop it before starting another.".localized,
                showOk: true
            )
            return
        }
        // Share one cancellation token between the hold and the JIT script so
        // that stopping the session also stops the script's loop.
        let token = HoldToken()

        // Resolve the JIT script to run in hold mode. On TXM devices this falls
        // back to the bundled universal script for apps without an assigned or
        // name-matched script, so keep-alive services JIT (and actively holds
        // the debug connection) for *any* app instead of only known ones.
        var script: DebugAppCallback? = nil
        if let holdScript = ScriptStore.keepAliveScript(for: bundleID) {
            script = getJsCallback(holdScript.data, name: holdScript.name, cancellation: token)
        }

        let startingMessage = String(format: "Keeping %@ alive in the background".localized, name)
        AccessibilityAnnouncer.announce(startingMessage)
        BackgroundAliveManager.shared.start(bundleID: bundleID, displayName: displayName, script: script, token: token)
    }

    private func debugFeedbackView(_ feedback: DebugFeedback) -> some View {
        HStack(spacing: 10) {
            if feedback.isWorking {
                ProgressView()
                    .controlSize(.small)
            }

            Text(feedback.message)
                .font(.subheadline.weight(.semibold))
                .multilineTextAlignment(.center)
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 10)
        .background(Capsule().fill(.ultraThinMaterial))
        .foregroundStyle(feedback.isError ? .red : .primary)
        .shadow(radius: 4)
        .padding(.horizontal, 20)
        .accessibilityElement(children: .ignore)
        .accessibilityLabel(feedback.message)
    }

    private func getJsCallback(_ script: Data, name: String? = nil, cancellation: HoldToken? = nil) -> DebugAppCallback {
        return { pid, debugProxyHandle, remoteServerHandle, semaphore in
            let model = RunJSViewModel(pid: Int(pid),
                                       debugProxy: debugProxyHandle,
                                       remoteServer: remoteServerHandle,
                                       semaphore: semaphore,
                                       cancellation: cancellation)

            DispatchQueue.main.async {
                scriptRunModel = model
            }

            do {
                try model.runScript(data: script, name: name)
            } catch {
                semaphore.signal()
                DispatchQueue.main.async {
                    showAlert(title: "Error Occurred While Executing Script.".localized, message: error.localizedDescription, showOk: true)
                }
            }
        }
    }

    private func startJITInBackground(bundleID: String? = nil, pid: Int? = nil, scriptData: Data? = nil, scriptName: String? = nil, triggeredByURLScheme: Bool = false, displayName: String? = nil) {
        let targetName = displayName ?? bundleID ?? pid.map { String(format: "process %d".localized, $0) } ?? "app".localized
        let startingMessage = String(format: "Starting JIT for %@".localized, targetName)
        LogManager.shared.addInfoLog("Starting Debug for \(bundleID ?? String(pid ?? 0))")
        withAnimation {
            debugFeedback = DebugFeedback(message: startingMessage, isError: false, isWorking: true)
        }
        AccessibilityAnnouncer.announce(startingMessage)

        if triggeredByURLScheme {
            markTunnelDisconnected()
            startTunnelInBackground(showErrorUI: false)
        }

        DispatchQueue.global(qos: .background).async {
            let keepAliveLease = DebugKeepAliveLease()
            defer { keepAliveLease.invalidate() }

            if triggeredByURLScheme {
                sleep(1)
            }

            let finishProcessing: (Bool, String?) -> Void = { success, detail in
                DispatchQueue.main.async {
                    let message = success
                        ? String(format: "JIT request completed for %@".localized, targetName)
                        : String(format: "JIT failed for %@".localized, targetName)
                    let feedback = DebugFeedback(message: message, isError: !success, isWorking: false)
                    withAnimation {
                        debugFeedback = feedback
                    }
                    AccessibilityAnnouncer.announce(message)

                    DispatchQueue.main.asyncAfter(deadline: .now() + 3.0) {
                        if debugFeedback?.id == feedback.id {
                            withAnimation {
                                debugFeedback = nil
                            }
                        }
                    }

                    if !success {
                        let failureMessage = detail ?? "StikDebug could not launch or attach to the selected app. Check that the VPN is enabled, the pairing file is current, and the app is still installed.".localized
                        showAlert(title: "Failed to Enable JIT".localized, message: failureMessage, showOk: true)
                    }
                }
            }

            var scriptData = scriptData
            var scriptName = scriptName
            if scriptData == nil,
               let bundleID,
               let preferred = ScriptStore.preferredScript(for: bundleID) {
                scriptName = preferred.name
                scriptData = preferred.data
            }

            var callback: DebugAppCallback? = nil
            if ProcessInfo.processInfo.hasTXM, let sd = scriptData {
                callback = getJsCallback(sd, name: scriptName ?? bundleID ?? "Script")
            }

            var lastDebugMessage: String?
            let logger: LogFunc = { message in
                if let message {
                    lastDebugMessage = message
                    LogManager.shared.addInfoLog(message)
                }
            }
            var success: Bool
            if let pid {
                success = JITEnableContext.shared.debugApp(withPID: Int32(pid), logger: logger, jsCallback: callback)
            } else if let bundleID {
                success = JITEnableContext.shared.debugApp(withBundleID: bundleID, logger: logger, jsCallback: callback)
            } else {
                lastDebugMessage = "Either bundle ID or PID should be specified.".localized
                success = false
            }

            if success {
                DispatchQueue.main.async {
                    LogManager.shared.addInfoLog("Debug process completed for \(bundleID ?? String(pid ?? 0))")

                    if doAutoQuitAfterEnablingJIT {
                        exit(0)
                    }
                }
            }
            finishProcessing(success, success ? nil : lastDebugMessage)
        }
    }

    private func base64URLToBase64(_ base64url: String) -> String {
        var base64 = base64url.replacingOccurrences(of: "-", with: "+").replacingOccurrences(of: "_", with: "/")
        let pad = 4 - (base64.count % 4)
        if pad < 4 { base64 += String(repeating: "=", count: pad) }
        return base64
    }
}

#Preview {
    HomeView()
}
