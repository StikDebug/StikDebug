//
//  MountingProgress.swift
//  StikDebug
//

import Foundation
import idevice

final class MountingProgress: ObservableObject {
    static let shared = MountingProgress()

    @Published private(set) var mountingThread: Thread?
    @Published private(set) var coolisMounted: Bool = false

    private let mountCheckLock = NSLock()
    private var mountCheckInProgress = false
    private let mountLock = NSLock()
    private var mountInProgress = false

    private init() {}

    func checkforMounted() {
        guard TunnelManager.shared.isConnected else { return }

        mountCheckLock.lock()
        guard !mountCheckInProgress else {
            mountCheckLock.unlock()
            return
        }
        mountCheckInProgress = true
        mountCheckLock.unlock()

        DispatchQueue.global(qos: .utility).async {
            let status = checkMountStatus()

            self.mountCheckLock.lock()
            self.mountCheckInProgress = false
            self.mountCheckLock.unlock()

            DispatchQueue.main.async {
                if status != .unreachable {
                    self.coolisMounted = status == .mounted
                }
            }
        }
    }

    func pubMount() {
        guard TunnelManager.shared.isConnected, DeveloperDiskImageService.filesAreReady else { return }

        mountLock.lock()
        guard !mountInProgress else {
            mountLock.unlock()
            return
        }
        mountInProgress = true
        mountLock.unlock()

        let thread = Thread { [weak self] in
            self?.mount()
        }
        thread.qualityOfService = .background
        thread.name = "mounting"
        DispatchQueue.main.async {
            self.mountingThread = thread
        }
        thread.start()
    }

    private func mount() {
        switch checkMountStatus() {
        case .mounted:
            finishMount {
                self.coolisMounted = true
            }
            return
        case .unreachable:
            finishMount()
            return
        case .notMounted:
            break
        }

        guard isPairing() else {
            finishMount()
            return
        }

        let mountError = mountDeveloperDiskImage(from: DeveloperDiskImageService.directoryURL.path)
        let mounted = mountError == nil || checkMountStatus() == .mounted
        finishMount {
            if mounted {
                self.coolisMounted = true
                self.checkforMounted()
            } else if let mountError {
                LogManager.shared.addErrorLog("Failed to mount DDI: \(mountError)")
                showAlert(title: "DDI Mount Failed".localized, message: mountError, showOk: true, showTryAgain: true) { shouldTryAgain in
                    if shouldTryAgain {
                        self.pubMount()
                    }
                }
            }
        }
    }

    private func finishMount(_ completion: @escaping () -> Void = {}) {
        DispatchQueue.main.async {
            self.mountingThread = nil
            self.mountLock.lock()
            self.mountInProgress = false
            self.mountLock.unlock()
            completion()
        }
    }
}

func isPairing() -> Bool {
    let pairingPath = PairingFileStore.prepareURL().path
    var pairingFile: RpPairingFileHandle?
    let error = rp_pairing_file_read(pairingPath, &pairingFile)
    if error != nil {
        return false
    }
    rp_pairing_file_free(pairingFile)
    return true
}
