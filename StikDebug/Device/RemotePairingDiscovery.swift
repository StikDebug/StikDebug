//
//  RemotePairingDiscovery.swift
//  StikDebug
//

import Foundation
import Network

/// Discovers the device's RemotePairing service port.
///
/// iOS advertises `_remotepairing._tcp` with a port that is randomized per boot,
/// so a fixed port only works until the next reboot.
final class RemotePairingDiscovery {
    static let shared = RemotePairingDiscovery()

    private static let serviceType = "_remotepairing._tcp"
    private static let queue = DispatchQueue(label: "com.stikdebug.remotepairing-discovery", qos: .utility)

    private let stateLock = NSLock()
    private var cachedPort: UInt16?
    private var discoveryInFlight = false
    private var nextRetryAt: Date?

    private init() {}

    var currentPort: UInt16? {
        stateLock.lock()
        defer { stateLock.unlock() }
        return cachedPort
    }

    func invalidate() {
        stateLock.lock()
        cachedPort = nil
        nextRetryAt = nil
        stateLock.unlock()
    }

    func resolvePortIfNeeded(timeout: TimeInterval = 4) -> UInt16? {
        stateLock.lock()
        if let cachedPort {
            stateLock.unlock()
            return cachedPort
        }
        if let nextRetryAt, Date() < nextRetryAt {
            stateLock.unlock()
            return nil
        }
        if discoveryInFlight {
            stateLock.unlock()
            return waitForInFlightDiscovery(timeout: timeout)
        }
        discoveryInFlight = true
        stateLock.unlock()

        let port = discover(timeout: timeout)

        stateLock.lock()
        if let port {
            cachedPort = port
            nextRetryAt = nil
        } else {
            nextRetryAt = Date().addingTimeInterval(60)
        }
        discoveryInFlight = false
        stateLock.unlock()

        if let port {
            LogManager.shared.addInfoLog("Discovered RemotePairing port \(port)")
        }
        return port
    }

    private func waitForInFlightDiscovery(timeout: TimeInterval) -> UInt16? {
        let deadline = Date().addingTimeInterval(timeout)
        while Date() < deadline {
            usleep(100_000)
            stateLock.lock()
            let port = cachedPort
            let stillDiscovering = discoveryInFlight
            stateLock.unlock()
            if let port {
                return port
            }
            if !stillDiscovering {
                return nil
            }
        }
        return currentPort
    }

    private func discover(timeout: TimeInterval) -> UInt16? {
        let parameters = NWParameters.tcp
        parameters.includePeerToPeer = true
        let browser = NWBrowser(for: .bonjour(type: Self.serviceType, domain: nil), using: parameters)

        let semaphore = DispatchSemaphore(value: 0)
        let resultLock = NSLock()
        var finished = false
        var resolvedPort: UInt16?
        var pendingConnection: NWConnection?

        func finish(_ port: UInt16?) {
            resultLock.lock()
            if !finished {
                finished = true
                resolvedPort = port
                semaphore.signal()
            }
            resultLock.unlock()
        }

        func hasFinished() -> Bool {
            resultLock.lock()
            defer { resultLock.unlock() }
            return finished
        }

        browser.browseResultsChangedHandler = { results, _ in
            guard !hasFinished() else { return }

            let preferred = results.first { result in
                result.interfaces.contains { $0.name == "lo0" }
            } ?? results.first
            guard let result = preferred, case .service = result.endpoint else { return }

            resultLock.lock()
            let alreadyConnecting = pendingConnection != nil
            if !alreadyConnecting {
                pendingConnection = NWConnection(to: result.endpoint, using: parameters)
            }
            let connection = pendingConnection
            resultLock.unlock()

            guard !alreadyConnecting, let connection else { return }

            connection.stateUpdateHandler = { state in
                switch state {
                case .ready:
                    if let remoteEndpoint = connection.currentPath?.remoteEndpoint,
                       case .hostPort(_, let port) = remoteEndpoint {
                        finish(port.rawValue)
                    } else {
                        finish(nil)
                    }
                case .failed, .cancelled:
                    finish(nil)
                default:
                    break
                }
            }
            connection.start(queue: Self.queue)
        }

        browser.stateUpdateHandler = { state in
            switch state {
            case .failed, .cancelled:
                finish(nil)
            default:
                break
            }
        }

        browser.start(queue: Self.queue)
        _ = semaphore.wait(timeout: .now() + timeout)
        browser.cancel()

        resultLock.lock()
        let connectionToCancel = pendingConnection
        resultLock.unlock()
        connectionToCancel?.cancel()

        resultLock.lock()
        defer { resultLock.unlock() }
        return resolvedPort
    }
}
