//
//  TunnelManager.swift
//  StikDebug
//

import Foundation
import UIKit

final class TunnelManager: ObservableObject {
    static let shared = TunnelManager()

    @Published private(set) var isConnected = false

    @Published private(set) var isStarting = false
    @Published private(set) var lastError: NSError?
    @Published private(set) var lastDiagnosis: ConnectionDiagnosis?
    @Published private(set) var connectionNotice: String?
    private var noticeDismissal: DispatchWorkItem?
    private var restartPending = false
    private var isRefreshingDiagnosis = false

    private init() {}

    func markDisconnected() {
        runOnMain {
            self.setConnectionNotice(nil)
            self.isConnected = false
        }
    }

    func restart() {
        runOnMain {
            self.setConnectionNotice(nil)
            self.isConnected = false
            if self.isStarting {
                self.restartPending = true
            } else {
                self.start()
            }
        }
    }

    func start(showErrorUI: Bool = true) {
        guard Thread.isMainThread else {
            DispatchQueue.main.async {
                self.start(showErrorUI: showErrorUI)
            }
            return
        }

        guard !isStarting else { return }
        let targetIP = DeviceConnectionContext.targetIPAddress
        let pairingFileURL = PairingFileStore.prepareURL()
        guard FileManager.default.fileExists(atPath: pairingFileURL.path) else {
            let error = ConnectionDiagnostics.annotate(NSError(domain: "StikDebug", code: -17, userInfo: [NSLocalizedDescriptionKey: "Pairing file not found."]), stage: .pairingFile, targetIP: targetIP)
            finishStart(.failure(error), diagnosis: ConnectionDiagnosis(problem: .pairingFile, isLocal: targetIP == DeviceConnectionContext.defaultTargetIPAddress), showErrorUI: showErrorUI)
            return
        }

        lastError = nil
        lastDiagnosis = nil
        setConnectionNotice(nil)
        isConnected = false
        isStarting = true

        DispatchQueue.global(qos: .userInteractive).async { [showErrorUI] in
            let result: Result<Void, NSError>
            let diagnosis: ConnectionDiagnosis?
            let notice: String?
            do {
                try JITEnableContext.shared.startTunnel()
                result = .success(())
                diagnosis = nil
                let conditions = targetIP == DeviceConnectionContext.defaultTargetIPAddress ? ConnectionDiagnostics.networkConditions() : nil
                notice = ConnectionDiagnostics.connectedNotice(targetIP: targetIP, conditions: conditions)
            } catch {
                result = .failure(error as NSError)
                diagnosis = ConnectionDiagnostics.diagnose(error as NSError, targetIP: targetIP)
                notice = nil
            }

            DispatchQueue.main.async {
                self.finishStart(result, diagnosis: diagnosis, notice: notice, showErrorUI: showErrorUI)
            }
        }
    }

    func refreshDiagnosis() {
        guard !isStarting, !isConnected, !isRefreshingDiagnosis,
              let error = lastError, lastDiagnosis?.isLocal == true else { return }
        let targetIP = DeviceConnectionContext.targetIPAddress
        guard targetIP == DeviceConnectionContext.defaultTargetIPAddress else { return }
        isRefreshingDiagnosis = true
        DispatchQueue.global(qos: .userInitiated).async {
            let diagnosis = ConnectionDiagnostics.diagnose(error, targetIP: targetIP)
            DispatchQueue.main.async {
                self.isRefreshingDiagnosis = false
                guard !self.isStarting, !self.isConnected, self.lastError === error,
                      DeviceConnectionContext.targetIPAddress == targetIP else { return }
                self.lastDiagnosis = diagnosis
            }
        }
    }

    private func finishStart(_ result: Result<Void, NSError>, diagnosis: ConnectionDiagnosis?, notice: String? = nil, showErrorUI: Bool) {
        isStarting = false
        if restartPending {
            restartPending = false
            start()
            return
        }

        switch result {
        case .success:
            lastError = nil
            lastDiagnosis = nil
            isConnected = true
            setConnectionNotice(notice)
            LogManager.shared.addInfoLog("Tunnel connected successfully")
            mountDeveloperDiskImageIfNeeded()
        case .failure(let error):
            setConnectionNotice(nil)
            lastDiagnosis = diagnosis
            lastError = error
            isConnected = false
            handleStartFailure(error, showErrorUI: showErrorUI)
        }
    }

    private func setConnectionNotice(_ notice: String?) {
        noticeDismissal?.cancel()
        noticeDismissal = nil
        connectionNotice = notice
        guard notice != nil else { return }
        let dismissal = DispatchWorkItem { [weak self] in
            self?.connectionNotice = nil
        }
        noticeDismissal = dismissal
        DispatchQueue.main.asyncAfter(deadline: .now() + 8, execute: dismissal)
    }

    private func mountDeveloperDiskImageIfNeeded() {
        guard DeveloperDiskImageService.filesAreReady else { return }
        MountingProgress.shared.pubMount()
    }

    private func handleStartFailure(_ error: NSError, showErrorUI: Bool) {
        LogManager.shared.addErrorLog(tunnelConnectionLogMessage(for: error))
        guard showErrorUI else {
            return
        }

        if lastDiagnosis?.problem == .pairingRejected {
            handleInvalidPairingFile(error)
            return
        }

        showAlert(
            title: "Connection Error".localized,
            message: tunnelConnectionAlertMessage(for: error, diagnosis: lastDiagnosis),
            showOk: false,
            showTryAgain: true
        ) { shouldTryAgain in
            if shouldTryAgain {
                startTunnelInBackground()
            }
        }
    }

    private func handleInvalidPairingFile(_ error: NSError) {
        LogManager.shared.addInfoLog("Pairing file reported invalid; keeping existing file")

        showAlert(
            title: "Invalid Pairing File".localized,
            message: tunnelConnectionAlertMessage(for: error, diagnosis: lastDiagnosis),
            showOk: true,
            showTryAgain: false,
            primaryButtonText: String(localized: "Select New File")
        ) { _ in
            NotificationCenter.default.post(name: NSNotification.Name("ShowPairingFilePicker"), object: nil)
        }
    }

    private func runOnMain(_ work: @escaping () -> Void) {
        if Thread.isMainThread {
            work()
        } else {
            DispatchQueue.main.async(execute: work)
        }
    }
}

func startTunnelInBackground(showErrorUI: Bool = true) {
    TunnelManager.shared.start(showErrorUI: showErrorUI)
}

func markTunnelDisconnected() {
    TunnelManager.shared.markDisconnected()
}

private func tunnelConnectionLogMessage(for error: NSError) -> String {
    let target = RemotePairingPortResolver.lastAttemptDescription
    return "Tunnel connection failed for \(target): \(error.localizedDescription) (Domain: \(error.domain), Code: \(error.code), Raw: \(String(describing: error)))"
}

private func tunnelConnectionAlertMessage(for error: NSError, diagnosis: ConnectionDiagnosis?) -> String {
    let summary = diagnosis.map { "\($0.title)\n\($0.message)" } ?? "Unlock the target device and try again."
    let detail = diagnosis?.hasSimpleError == true ? "" : "\n\nCode \(error.code): \(error.localizedDescription)"
    return """
    \(summary)\(detail)

    \(connectionDiagnosticDetails())
    """
}

func connectionDiagnosticDetails() -> String {
    let version = Bundle.main.infoDictionary?["CFBundleShortVersionString"] as? String ?? "Unknown"
    var systemInfo = utsname()
    uname(&systemInfo)
    let model = withUnsafePointer(to: &systemInfo.machine) {
        $0.withMemoryRebound(to: CChar.self, capacity: 1) { String(cString: $0) }
    }

    return """
    \(version)
    \(UIDevice.current.systemName) \(UIDevice.current.systemVersion)
    \(DeviceModelName.name(for: model))
    """
}
