//
//  BackgroundAliveManager.swift
//  StikDebug
//

import Foundation
import UIKit

/// Thread-safe cancellation flag for a held debug session.
final class HoldToken {
    private let lock = NSLock()
    private var cancelled = false

    var isCancelled: Bool {
        lock.lock()
        defer { lock.unlock() }
        return cancelled
    }

    func cancel() {
        lock.lock()
        cancelled = true
        lock.unlock()
    }
}

/// Owns an active "keep this app alive in the background" session.
///
/// It launches the target app, holds the debugger attached to it (which stops
/// iOS from suspending it in the background), and keeps StikDebug itself alive
/// (`DebugKeepAliveLease`) for as long as the hold is running. Only one app can
/// be held at a time. This is experimental — see `JITEnableContext.keepAppAlive`.
final class BackgroundAliveManager: ObservableObject {
    static let shared = BackgroundAliveManager()

    @Published private(set) var activeAppName: String?

    private let lock = NSLock()
    private var token: HoldToken?
    private var lease: DebugKeepAliveLease?
    private var activeBundleID: String?

    private init() {}

    var isActive: Bool {
        lock.lock()
        defer { lock.unlock() }
        return activeBundleID != nil
    }

    /// - Parameter script: Optional JIT-script callback to run once (after
    ///   attach, before the hold begins) so the app's assigned script executes
    ///   in hold mode just like it does for a normal JIT run.
    /// - Parameter token: Cancellation token for the hold. Pass the same token
    ///   the `script` callback was built with so that stopping the session also
    ///   stops the script's loop; defaults to a fresh token when there is no
    ///   script.
    func start(bundleID: String, displayName: String?, script: DebugAppCallback? = nil, token: HoldToken = HoldToken()) {
        lock.lock()
        guard activeBundleID == nil else {
            lock.unlock()
            return
        }
        activeBundleID = bundleID
        self.token = token
        lock.unlock()

        // Create the keep-alive lease outside the lock (it touches the main thread).
        let lease = DebugKeepAliveLease()
        lock.lock()
        self.lease = lease
        lock.unlock()

        let name = displayName ?? bundleID
        DispatchQueue.main.async { self.activeAppName = name }
        LogManager.shared.addInfoLog("Starting background keep-alive for \(name)")

        DispatchQueue.global(qos: .userInitiated).async {
            let logger: LogFunc = { message in
                if let message { LogManager.shared.addInfoLog(message) }
            }

            let succeeded = JITEnableContext.shared.keepAppAlive(
                withBundleID: bundleID,
                script: script,
                cancellation: token,
                logger: logger
            )

            self.lock.lock()
            let stillCurrent = self.activeBundleID == bundleID && self.token === token
            if stillCurrent {
                self.activeBundleID = nil
                self.token = nil
                self.lease = nil
            }
            self.lock.unlock()

            lease.invalidate()

            DispatchQueue.main.async {
                if stillCurrent {
                    self.activeAppName = nil
                }
                if !succeeded {
                    showAlert(
                        title: "Keep-Alive Ended",
                        message: "The background keep-alive session for \(name) could not start or ended early. Make sure the app is installed and that LocalDevVPN and the pairing file are active.",
                        showOk: true
                    )
                }
            }
        }
    }

    func stop() {
        lock.lock()
        let token = self.token
        lock.unlock()
        token?.cancel()
    }
}
