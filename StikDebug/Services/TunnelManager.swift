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
    private var restartPending = false

    private init() {}

    func markDisconnected() {
        runOnMain {
            self.isConnected = false
        }
    }

    func restart() {
        runOnMain {
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

        let pairingFileURL = PairingFileStore.prepareURL()
        guard FileManager.default.fileExists(atPath: pairingFileURL.path) else {
            isConnected = false
            lastError = NSError(domain: "StikDebug", code: -17, userInfo: [NSLocalizedDescriptionKey: "Pairing file not found."])
            return
        }

        guard !isStarting else {
            return
        }

        lastError = nil
        isConnected = false
        isStarting = true

        DispatchQueue.global(qos: .userInteractive).async { [showErrorUI] in
            let result: Result<Void, NSError>
            do {
                try JITEnableContext.shared.startTunnel()
                result = .success(())
            } catch {
                result = .failure(error as NSError)
            }

            DispatchQueue.main.async {
                self.finishStart(result, showErrorUI: showErrorUI)
            }
        }
    }

    private func finishStart(_ result: Result<Void, NSError>, showErrorUI: Bool) {
        isStarting = false
        if restartPending {
            restartPending = false
            start()
            return
        }

        switch result {
        case .success:
            lastError = nil
            isConnected = true
            LogManager.shared.addInfoLog("Tunnel connected successfully")
            mountDeveloperDiskImageIfNeeded()
        case .failure(let error):
            lastError = error
            isConnected = false
            handleStartFailure(error, showErrorUI: showErrorUI)
        }
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

        if error.code == -9 {
            handleInvalidPairingFile()
            return
        }

        showAlert(
            title: "Connection Error",
            message: tunnelConnectionAlertMessage(for: error),
            showOk: false,
            showTryAgain: true
        ) { shouldTryAgain in
            if shouldTryAgain {
                startTunnelInBackground()
            }
        }
    }

    private func handleInvalidPairingFile() {
        LogManager.shared.addInfoLog("Pairing file reported invalid; keeping existing file")

        showAlert(
            title: "Invalid Pairing File",
            message: "The pairing file may be invalid or expired. You can import a new pairing file to replace it.\n\n\(connectionDiagnosticDetails())",
            showOk: true,
            showTryAgain: false,
            primaryButtonText: "Select New File"
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

private func tunnelConnectionAlertMessage(for error: NSError) -> String {
    let pairingInstructions: String
    if #available(iOS 27.0, *) {
        pairingInstructions = "Generate and place a fresh pairing file using the built-in pairing menu in Settings or iloader."
    } else {
        pairingInstructions = "Generate and place a fresh pairing file using iloader."
    }

    return """
    Code \(error.code): \(error.localizedDescription)

    Troubleshooting:
    1. Make sure Wi-Fi is connected or Cellular Data is disabled, and LocalDevVPN is enabled.
    2. Restart your iPhone or iPad, then re-enable LocalDevVPN and try again.
    3. \(pairingInstructions)

    Target attempted: \(RemotePairingPortResolver.lastAttemptDescription)

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
