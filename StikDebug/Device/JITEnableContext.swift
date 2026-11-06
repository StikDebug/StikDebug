//
//  JITEnableContext.swift
//  StikDebug
//
//  Created by Stephen on 2026/3/30.
//

import Foundation
import idevice
import Darwin
import Network

final class RemotePairingPortResolver {
    static let fallback: UInt16 = 49152
    private static let overrideKey = "TunnelRemotePairingPort"
    private static let stateLock = NSLock()
    private static var selectedEndpoint = "Not attempted yet"

    /// The precise port and discovery method used for the latest connection attempt.
    static var lastAttemptDescription: String {
        stateLock.lock()
        defer { stateLock.unlock() }
        return selectedEndpoint
    }

    private static func record(targetIPAddress: String, port: UInt16, source: String) {
        let endpoint = "\(targetIPAddress):\(port) (\(source))"
        stateLock.lock()
        selectedEndpoint = endpoint
        stateLock.unlock()
        NSLog("[StikDebug] Remote Pairing selected endpoint: %@", endpoint)
    }

    private let discoveryQueue = DispatchQueue(label: "com.stik.remotePairingDiscovery")
    private let completion = DispatchSemaphore(value: 0)
    private var browser: NWBrowser?
    private var connection: NWConnection?
    private var selectedService: NWEndpoint?
    private var selectedLoopback = false
    private var resolvedPort: UInt16?
    private var finished = false

    static func resolve(targetIPAddress: String = DeviceConnectionContext.targetIPAddress, timeout: TimeInterval = 5) -> UInt16 {
        let override = (UserDefaults.standard.string(forKey: overrideKey) ?? "")
            .trimmingCharacters(in: .whitespacesAndNewlines)
        if !override.isEmpty, let port = UInt16(override), port != 0 {
            record(targetIPAddress: targetIPAddress, port: port, source: "Manual")
            return port
        }
        if !override.isEmpty {
            NSLog("[StikDebug] Ignoring invalid manual Remote Pairing port: %@", override)
        }

        let result = discover(targetIPAddress: targetIPAddress, timeout: timeout)
        record(targetIPAddress: targetIPAddress, port: result.port, source: result.source)
        return result.port
    }

    static func discover(targetIPAddress: String = DeviceConnectionContext.targetIPAddress, timeout: TimeInterval = 5) -> (port: UInt16, source: String) {
        if targetIPAddress != DeviceConnectionContext.defaultTargetIPAddress {
            var peer = sockaddr_in()
            peer.sin_len = UInt8(MemoryLayout<sockaddr_in>.size)
            peer.sin_family = UInt8(AF_INET)
            guard targetIPAddress.withCString({ inet_pton(AF_INET, $0, &peer.sin_addr) }) == 1 else { return (fallback, "Fallback") }
            let address = withUnsafeBytes(of: &peer) { Data($0) }
            guard let resolved = try? PairedDeviceDiscovery(address: address).resolve(timeout: timeout),
                  resolved.count == MemoryLayout<sockaddr_in>.size else { return (fallback, "Fallback") }
            withUnsafeMutableBytes(of: &peer) { _ = resolved.copyBytes(to: $0) }
            let port = UInt16(bigEndian: peer.sin_port)
            return port == 0 ? (fallback, "Fallback") : (port, "Bonjour")
        }
        let worker = RemotePairingPortResolver()
        worker.discoveryQueue.async {
            worker.startDiscovery()
        }
        _ = worker.completion.wait(timeout: .now() + timeout)
        return worker.discoveryQueue.sync {
            worker.finishDiscovery()
            if let port = worker.resolvedPort {
                return (port, "Bonjour")
            }
            return (fallback, "Fallback")
        }
    }

    private static func preferredCandidate(
        from candidates: [(endpoint: NWEndpoint, isLoopback: Bool)]
    ) -> (endpoint: NWEndpoint, isLoopback: Bool)? {
        candidates.first(where: { $0.isLoopback }) ?? candidates.first
    }

    private func startDiscovery() {
        guard !finished else { return }
        let parameters = NWParameters.tcp
        parameters.includePeerToPeer = true
        let browser = NWBrowser(
            for: .bonjour(type: "_remotepairing._tcp", domain: "local."),
            using: parameters
        )
        self.browser = browser
        browser.browseResultsChangedHandler = { [weak self] results, _ in
            guard let self, !self.finished else { return }
            let candidates = results.compactMap { result -> (endpoint: NWEndpoint, isLoopback: Bool)? in
                guard case .service = result.endpoint else { return nil }
                let isLoopback = result.interfaces.contains { $0.name.hasPrefix("lo") }
                return (result.endpoint, isLoopback)
            }
            guard let selected = Self.preferredCandidate(from: candidates),
                  self.selectedService == nil || (!self.selectedLoopback && selected.isLoopback) else {
                return
            }
            self.selectedService = selected.endpoint
            self.selectedLoopback = selected.isLoopback
            self.connection?.cancel()
            let connection = NWConnection(to: selected.endpoint, using: parameters)
            self.connection = connection
            connection.pathUpdateHandler = { [weak self, weak connection] path in
                guard let self, let connection, self.connection === connection, !self.finished else { return }
                if path.status == .satisfied {
                    self.recordResolvedPort(from: connection)
                }
            }
            connection.stateUpdateHandler = { [weak self, weak connection] state in
                guard let self, let connection, self.connection === connection, !self.finished else { return }
                switch state {
                case .ready, .waiting:
                    self.recordResolvedPort(from: connection)
                case .failed:
                    self.finishDiscovery()
                default:
                    break
                }
            }
            connection.start(queue: self.discoveryQueue)
        }
        browser.stateUpdateHandler = { [weak self] state in
            if case .failed = state {
                self?.finishDiscovery()
            }
        }
        browser.start(queue: discoveryQueue)
    }

    private func recordResolvedPort(from connection: NWConnection) {
        guard case .hostPort(_, let port) = connection.currentPath?.remoteEndpoint,
              port.rawValue != 0 else { return }
        resolvedPort = port.rawValue
        finishDiscovery()
    }

    private func finishDiscovery() {
        guard !finished else { return }
        finished = true
        browser?.cancel()
        connection?.cancel()
        completion.signal()
    }

}

struct PairingPlacementTarget: Sendable {
    let pairingFileURL: URL
    let address: Data
    let hostname: String
}

final class PairedDeviceDiscovery: NSObject, NetServiceBrowserDelegate, NetServiceDelegate {
    private let browser = NetServiceBrowser()
    private var services: [NetService] = []
    private let peerHost: String?
    private var resolvedAddress: Data?

    init(address: Data) {
        peerHost = Self.numericHost(address)
        super.init()
        browser.delegate = self
        browser.includesPeerToPeer = true
    }

    static func numericHost(_ address: Data) -> String? {
        guard address.count >= MemoryLayout<sockaddr>.size else { return nil }
        return address.withUnsafeBytes { bytes in
            let socketAddress = bytes.baseAddress!.assumingMemoryBound(to: sockaddr.self)
            let family = Int32(socketAddress.pointee.sa_family)
            let length: Int
            switch family {
            case AF_INET: length = MemoryLayout<sockaddr_in>.size
            case AF_INET6: length = MemoryLayout<sockaddr_in6>.size
            default: return nil
            }
            guard address.count >= length else { return nil }
            var host = [CChar](repeating: 0, count: Int(NI_MAXHOST))
            guard getnameinfo(socketAddress, socklen_t(length), &host, socklen_t(host.count), nil, 0, NI_NUMERICHOST) == 0 else { return nil }
            return String(cString: host)
        }
    }

    func resolve(timeout: TimeInterval = 8) throws -> Data {
        guard peerHost != nil else {
            throw NSError(domain: "StikDebug", code: -18, userInfo: [NSLocalizedDescriptionKey: "The paired device has an invalid network address."])
        }
        browser.searchForServices(ofType: "_remotepairing._tcp.", inDomain: "local.")
        defer {
            browser.stop()
            services.forEach { $0.stop() }
        }
        let deadline = Date().addingTimeInterval(timeout)
        while resolvedAddress == nil && Date() < deadline {
            RunLoop.current.run(until: min(deadline, Date().addingTimeInterval(0.05)))
        }
        guard let resolvedAddress else {
            throw NSError(domain: "StikDebug", code: -19, userInfo: [NSLocalizedDescriptionKey: "Could not discover the paired device. Keep both devices on the same Wi-Fi network and try again."])
        }
        return resolvedAddress
    }

    func netServiceBrowser(_ browser: NetServiceBrowser, didFind service: NetService, moreComing: Bool) {
        services.append(service)
        service.includesPeerToPeer = true
        service.delegate = self
        service.resolve(withTimeout: 6)
    }

    func netServiceDidResolveAddress(_ sender: NetService) {
        if let address = sender.addresses?.first(where: { Self.numericHost($0) == peerHost }) {
            resolvedAddress = address
        }
    }
}

typealias LogFunc = (String?) -> Void
typealias DebugAppCallback = (_ pid: Int32, _ debugProxy: OpaquePointer?, _ remoteServer: OpaquePointer?, _ semaphore: DispatchSemaphore) -> Void
typealias SyslogLineHandler = (String) -> Void
typealias SyslogErrorHandler = (NSError?) -> Void

final class JITEnableContext {
    static let shared = JITEnableContext()

    private static func withCStringArray<R>(
        _ strings: [String], _ body: (UnsafePointer<UnsafePointer<CChar>?>?, UInt) -> R
    ) -> R {
        if strings.isEmpty {
            return body(nil, 0)
        }
        var cStrings: [UnsafeMutablePointer<CChar>?] = strings.map { strdup($0) }
        defer { cStrings.forEach { free($0) } }
        return cStrings.withUnsafeBufferPointer { buffer in
            buffer.baseAddress!.withMemoryRebound(to: UnsafePointer<CChar>?.self, capacity: buffer.count) { rebound in
                body(rebound, UInt(buffer.count))
            }
        }
    }

    private static func mallocDebugEnvVars() -> [String] {
        let defaults = UserDefaults.standard
        let enableAll = defaults.bool(forKey: UserDefaults.Keys.mallocDebug)
        let guardEdges = enableAll || defaults.bool(forKey: UserDefaults.Keys.mallocGuardEdges)
        let scribble = enableAll || defaults.bool(forKey: UserDefaults.Keys.mallocScribble)

        var envVars: [String] = []
        if guardEdges {
            envVars.append("MallocGuardEdges=1")
        }
        if scribble {
            envVars.append("MallocScribble=1")
        }
        return envVars
    }

    private struct TunnelHandles {
        var adapter: OpaquePointer?
        var handshake: OpaquePointer?

        mutating func free() {
            if let handshake {
                rsd_handshake_free(handshake)
                self.handshake = nil
            }
            if let adapter {
                adapter_free(adapter)
                self.adapter = nil
            }
        }
    }

    private var adapter: OpaquePointer?
    private var handshake: OpaquePointer?

    private let tunnelLock = NSLock()
    private var tunnelConnecting = false
    private var tunnelSemaphore: DispatchSemaphore?
    private var lastTunnelError: NSError?

    private let syslogQueue = DispatchQueue(label: "com.stik.syslogrelay.queue")
    private var syslogStreaming = false
    private var syslogClient: OpaquePointer?
    private var syslogLineHandler: SyslogLineHandler?
    private var syslogErrorHandler: SyslogErrorHandler?

    var adapterHandle: OpaquePointer? { adapter }
    var handshakeHandle: OpaquePointer? { handshake }

    private init() {
        var path = Array(IdeviceLogFileManager.logURL.path.utf8CString)
        path.withUnsafeMutableBufferPointer { buffer in
            // Debug logging includes pairing-file credentials and protocol payloads.
            _ = idevice_init_logger(Info, Info, buffer.baseAddress)
        }
    }

    deinit {
        stopSyslogRelay()
        if let handshake {
            rsd_handshake_free(handshake)
        }
        if let adapter {
            adapter_free(adapter)
        }
    }

    private func makeError(_ message: String, code: Int = -1) -> NSError {
        NSError(
            domain: "StikDebug",
            code: code,
            userInfo: [NSLocalizedDescriptionKey: message]
        )
    }

    private func nsString(from cString: UnsafePointer<CChar>?, fallback: String) -> String {
        guard let cString, let string = String(validatingUTF8: cString) else {
            return fallback
        }
        return string
    }

    private func error(from ffiError: UnsafeMutablePointer<IdeviceFfiError>?, fallback: String) -> NSError {
        guard let ffiError else {
            return makeError(fallback)
        }
        let detail = nsString(from: ffiError.pointee.message, fallback: fallback)
        let message = detail == fallback ? fallback : "\(fallback): \(detail)"
        let error = makeError(message, code: Int(ffiError.pointee.code))
        idevice_error_free(ffiError)
        return error
    }

    private func routeLog(_ message: String) {
        if message.localizedCaseInsensitiveContains("error") {
            LogManager.shared.addErrorLog(message)
        } else if message.localizedCaseInsensitiveContains("warning") {
            LogManager.shared.addWarningLog(message)
        } else if message.localizedCaseInsensitiveContains("debug") {
            LogManager.shared.addDebugLog(message)
        } else {
            LogManager.shared.addInfoLog(message)
        }
    }

    private func emitLog(_ message: String, logger: LogFunc?) {
        routeLog(message)
        logger?(message)
    }

    private func getPairingFile(at url: URL? = nil) throws -> OpaquePointer {
        let pairingFileURL = url ?? PairingFileStore.prepareURL()

        guard FileManager.default.fileExists(atPath: pairingFileURL.path) else {
            throw makeError("Pairing file not found!", code: -17)
        }

        var pairingFile: OpaquePointer?
        let ffiError = pairingFileURL.path.withCString { path in
            rp_pairing_file_read(path, &pairingFile)
        }

        if let ffiError {
            throw error(from: ffiError, fallback: "Failed to read pairing file!")
        }

        guard let pairingFile else {
            throw makeError("Failed to read pairing file!", code: -17)
        }

        return pairingFile
    }

    private func createTunnel(hostname: String, target: PairingPlacementTarget? = nil) throws -> TunnelHandles {
        let deviceIP = target.flatMap { PairedDeviceDiscovery.numericHost($0.address) } ?? DeviceConnectionContext.targetIPAddress
        let pairingFile: OpaquePointer
        do {
            pairingFile = try getPairingFile(at: target?.pairingFileURL)
        } catch {
            throw ConnectionDiagnostics.annotate(error as NSError, stage: .pairingFile, targetIP: deviceIP)
        }
        defer { rp_pairing_file_free(pairingFile) }

        var remotePairingPort: UInt16?
        let address: Data
        if let target {
            address = try PairedDeviceDiscovery(address: target.address).resolve()
        } else {
            var addr = sockaddr_in()
            addr.sin_len = UInt8(MemoryLayout<sockaddr_in>.size)
            addr.sin_family = sa_family_t(AF_INET)
            let parseResult = deviceIP.withCString { inet_pton(AF_INET, $0, &addr.sin_addr) }
            guard parseResult == 1 else {
                throw ConnectionDiagnostics.annotate(makeError("Failed to parse target IP address.", code: -18), stage: .targetAddress, targetIP: deviceIP)
            }
            let port = RemotePairingPortResolver.resolve(targetIPAddress: deviceIP)
            remotePairingPort = port
            addr.sin_port = port.bigEndian
            address = withUnsafeBytes(of: &addr) { Data($0) }
        }

        var socketAddress = sockaddr_storage()
        guard address.count <= MemoryLayout<sockaddr_storage>.size else {
            throw makeError("Invalid target network address.", code: -18)
        }
        withUnsafeMutableBytes(of: &socketAddress) { _ = address.copyBytes(to: $0) }
        var tunnel = TunnelHandles()
        let ffiError = (target?.hostname ?? hostname).withCString { hostname in
            withUnsafePointer(to: &socketAddress) { pointer in
                pointer.withMemoryRebound(to: sockaddr.self, capacity: 1) {
                    tunnel_create_rppairing(
                        $0,
                        socklen_t(address.count),
                        hostname,
                        pairingFile,
                        nil,
                        nil,
                        &tunnel.adapter,
                        &tunnel.handshake
                    )
                }
            }
        }

        if let ffiError {
            tunnel.free()
            throw ConnectionDiagnostics.annotate(error(from: ffiError, fallback: "Failed to create tunnel"), stage: .tunnel, targetIP: deviceIP, port: remotePairingPort)
        }

        guard tunnel.adapter != nil, tunnel.handshake != nil else {
            var incompleteTunnel = tunnel
            incompleteTunnel.free()
            throw makeError("Tunnel was created without valid handles")
        }

        return tunnel
    }

    func startTunnel() throws {
        tunnelLock.lock()
        if tunnelConnecting {
            let waitSemaphore = tunnelSemaphore
            tunnelLock.unlock()

            if let waitSemaphore {
                guard waitSemaphore.wait(timeout: .now() + .seconds(15)) == .success else {
                    throw makeError("Timed out waiting for the tunnel connection", code: -19)
                }
                waitSemaphore.signal()
            }

            if let lastTunnelError {
                throw lastTunnelError
            }
            return
        }

        tunnelConnecting = true
        let completionSemaphore = DispatchSemaphore(value: 0)
        tunnelSemaphore = completionSemaphore
        tunnelLock.unlock()

        var newAdapter: OpaquePointer?
        var newHandshake: OpaquePointer?
        var finalError: NSError?

        defer {
            tunnelLock.lock()
            tunnelConnecting = false
            tunnelSemaphore = nil
            lastTunnelError = finalError
            tunnelLock.unlock()
            completionSemaphore.signal()
        }

        do {
            let newTunnel = try createTunnel(hostname: "StikDebug")
            newAdapter = newTunnel.adapter
            newHandshake = newTunnel.handshake
        } catch let tunnelError as NSError {
            finalError = tunnelError
            throw tunnelError
        }

        if let handshake {
            rsd_handshake_free(handshake)
        }
        if let adapter {
            adapter_free(adapter)
        }

        adapter = newAdapter
        handshake = newHandshake
    }

    func ensureTunnel() throws {
        if adapter == nil || handshake == nil {
            try startTunnel()
        }
    }

    func withFreshDebugTunnel<T>(
        hostname: String,
        target: PairingPlacementTarget? = nil,
        _ body: (OpaquePointer, OpaquePointer) throws -> T
    ) throws -> T {
        var tunnel = try createTunnel(hostname: hostname, target: target)
        defer { tunnel.free() }

        guard let adapter = tunnel.adapter, let handshake = tunnel.handshake else {
            throw makeError("Tunnel is not connected")
        }

        return try body(adapter, handshake)
    }

    private struct DebugSession {
        var remoteServer: OpaquePointer?
        var debugProxy: OpaquePointer?

        mutating func free() {
            if let debugProxy {
                debug_proxy_free(debugProxy)
                self.debugProxy = nil
            }
            if let remoteServer {
                remote_server_free(remoteServer)
                self.remoteServer = nil
            }
        }
    }

    private final class DebugHeartbeatKeepAlive {
        private static let defaultInterval: UInt64 = 2
        private static let maxInterval: UInt64 = 3

        private let queue = DispatchQueue(label: "com.stikdebug.debug-heartbeat", qos: .utility)
        private let stateLock = NSLock()
        private let startupSemaphore = DispatchSemaphore(value: 0)
        private let stoppedSemaphore = DispatchSemaphore(value: 0)
        private let logger: LogFunc?
        private let makeClient: () throws -> (client: OpaquePointer, tunnel: TunnelHandles)
        private let errorBuilder: (UnsafeMutablePointer<IdeviceFfiError>?, String) -> NSError
        private var startupError: NSError?
        private var client: OpaquePointer?
        private var tunnel: TunnelHandles?
        private var stopRequested = false

        init(
            logger: LogFunc?,
            makeClient: @escaping () throws -> (client: OpaquePointer, tunnel: TunnelHandles),
            errorBuilder: @escaping (UnsafeMutablePointer<IdeviceFfiError>?, String) -> NSError
        ) {
            self.logger = logger
            self.makeClient = makeClient
            self.errorBuilder = errorBuilder
        }

        func start() throws {
            startupError = nil
            queue.async { [weak self] in
                self?.run()
            }

            startupSemaphore.wait()
            if let startupError {
                throw startupError
            }
        }

        func stop() {
            stateLock.lock()
            stopRequested = true
            stateLock.unlock()
            _ = stoppedSemaphore.wait(timeout: .now() + .seconds(Int(Self.maxInterval + 1)))
        }

        private func shouldStop() -> Bool {
            stateLock.lock()
            defer { stateLock.unlock() }
            return stopRequested
        }

        private func log(_ message: String) {
            logger?(message)
        }

        private func run() {
            do {
                let resources = try makeClient()
                client = resources.client
                tunnel = resources.tunnel
                startupError = nil
            } catch let error as NSError {
                startupError = error
                startupSemaphore.signal()
                stoppedSemaphore.signal()
                return
            }

            defer {
                if let client {
                    heartbeat_client_free(client)
                    self.client = nil
                }
                if var tunnel = tunnel {
                    tunnel.free()
                    self.tunnel = nil
                }
                stoppedSemaphore.signal()
            }

            startupSemaphore.signal()

            var interval = Self.defaultInterval

            while !shouldStop(), let client {
                var suggestedInterval: UInt64 = 0
                let ffiError = heartbeat_get_marco(client, interval, &suggestedInterval)

                if shouldStop() {
                    break
                }

                if let ffiError {
                    let heartbeatError = errorBuilder(ffiError, "Debug heartbeat failed")
                    let description = heartbeatError.localizedDescription

                    if description.contains("HeartbeatTimeout") {
                        interval = Self.defaultInterval
                        continue
                    }

                    if description.contains("HeartbeatSleepyTime") {
                        log("Debug heartbeat stopped: device entered SleepyTime")
                        break
                    }

                    log("Debug heartbeat warning: \(description)")
                    interval = Self.defaultInterval
                    continue
                }

                interval = min(max(suggestedInterval, 1), Self.maxInterval)

                if let ffiError = heartbeat_send_polo(client) {
                    let heartbeatError = errorBuilder(ffiError, "Failed to reply to heartbeat")
                    log("Debug heartbeat warning: \(heartbeatError.localizedDescription)")
                    interval = Self.defaultInterval
                }
            }
        }
    }

    private func connectDebugSession(adapter: OpaquePointer, handshake: OpaquePointer) throws -> DebugSession {
        var session = DebugSession()

        if let ffiError = remote_server_connect_rsd(adapter, handshake, &session.remoteServer) {
            throw error(from: ffiError, fallback: "Failed to connect remote server")
        }

        if let ffiError = debug_proxy_connect_rsd(adapter, handshake, &session.debugProxy) {
            session.free()
            throw error(from: ffiError, fallback: "Failed to connect debug proxy")
        }

        return session
    }

    private func withConnectedDebugSession<T>(
        _ body: (OpaquePointer, OpaquePointer) throws -> T
    ) throws -> T {
        try withFreshDebugTunnel(hostname: "StikDebugDebug") { adapter, handshake in
            var session = try connectDebugSession(adapter: adapter, handshake: handshake)
            defer { session.free() }

            guard let remoteServer = session.remoteServer,
                  let debugProxy = session.debugProxy else {
                throw makeError("Debug session was not created")
            }

            return try body(remoteServer, debugProxy)
        }
    }

    private func withConnectedRemoteServer<T>(
        _ body: (OpaquePointer) throws -> T
    ) throws -> T {
        try withFreshDebugTunnel(hostname: "StikDebugRemoteServer") { adapter, handshake in
            var remoteServer: OpaquePointer?
            if let ffiError = remote_server_connect_rsd(adapter, handshake, &remoteServer) {
                throw error(from: ffiError, fallback: "Failed to connect remote server")
            }

            guard let remoteServer else {
                throw makeError("Remote server handle was not created")
            }

            defer { remote_server_free(remoteServer) }
            return try body(remoteServer)
        }
    }

    private func withProcessControl<T>(
        remoteServer: OpaquePointer,
        _ body: (OpaquePointer) throws -> T
    ) throws -> T {
        var processControl: OpaquePointer?
        if let ffiError = process_control_new(remoteServer, &processControl) {
            throw error(from: ffiError, fallback: "Failed to open process control")
        }

        guard let processControl else {
            throw makeError("Process control handle was not created")
        }

        defer { process_control_free(processControl) }
        return try body(processControl)
    }

    private func connectHeartbeatKeepAlive(logger: LogFunc?) throws -> DebugHeartbeatKeepAlive {
        return DebugHeartbeatKeepAlive(
            logger: logger,
            makeClient: { [weak self] in
                guard let self else {
                    throw NSError(
                        domain: "StikDebug",
                        code: -1,
                        userInfo: [NSLocalizedDescriptionKey: "Debug heartbeat context is unavailable"]
                    )
                }

                var tunnel = try self.createTunnel(hostname: "StikDebugHeartbeat")
                guard let adapter = tunnel.adapter, let handshake = tunnel.handshake else {
                    tunnel.free()
                    throw self.makeError("Tunnel is not connected")
                }

                var heartbeatClient: OpaquePointer?
                if let ffiError = heartbeat_connect_rsd(adapter, handshake, &heartbeatClient) {
                    tunnel.free()
                    throw self.error(from: ffiError, fallback: "Failed to connect debug heartbeat")
                }

                guard let heartbeatClient else {
                    tunnel.free()
                    throw self.makeError("Heartbeat client was not created")
                }

                return (client: heartbeatClient, tunnel: tunnel)
            },
            errorBuilder: { [weak self] ffiError, fallback in
                self?.error(from: ffiError, fallback: fallback) ?? NSError(
                    domain: "StikDebug",
                    code: -1,
                    userInfo: [NSLocalizedDescriptionKey: fallback]
                )
            }
        )
    }

    private func sendDebugCommand(_ command: String, debugProxy: OpaquePointer) throws -> String? {
        guard let commandHandle = debugserver_command_new(command, nil, 0) else {
            throw makeError("Failed to create debugserver command: \(command)")
        }

        var response: UnsafeMutablePointer<CChar>?
        let ffiError = debug_proxy_send_command(debugProxy, commandHandle, &response)
        debugserver_command_free(commandHandle)

        if let ffiError {
            if let response {
                idevice_string_free(response)
            }
            throw error(from: ffiError, fallback: "Debugserver command failed: \(command)")
        }

        defer {
            if let response {
                idevice_string_free(response)
            }
        }

        guard let response else { return nil }
        return String(cString: response)
    }

    private func runDebugServerCommand(
        pid: Int32,
        debugProxy: OpaquePointer,
        remoteServer: OpaquePointer,
        logger: LogFunc?,
        callback: DebugAppCallback?
    ) {
        debug_proxy_send_ack(debugProxy)
        debug_proxy_send_ack(debugProxy)

        do {
            let response = try sendDebugCommand("QStartNoAckMode", debugProxy: debugProxy) ?? "<nil>"
            emitLog("QStartNoAckMode result = \(response)", logger: logger)
        } catch {
            emitLog(error.localizedDescription, logger: logger)
        }

        debug_proxy_set_ack_mode(debugProxy, 0)

        if let callback {
            let keepAlive: DebugHeartbeatKeepAlive?
            do {
                let heartbeat = try connectHeartbeatKeepAlive(logger: logger)
                try heartbeat.start()
                keepAlive = heartbeat
                emitLog("Debug heartbeat keepalive started", logger: logger)
            } catch {
                keepAlive = nil
                emitLog("Warning: failed to start debug heartbeat keepalive: \(error.localizedDescription)", logger: logger)
            }
            defer {
                keepAlive?.stop()
                if keepAlive != nil {
                    emitLog("Debug heartbeat keepalive stopped", logger: logger)
                }
            }

            let semaphore = DispatchSemaphore(value: 0)
            callback(pid, debugProxy, remoteServer, semaphore)
            semaphore.wait()

            var breakByte: UInt8 = 0x03
            if let ffiError = debug_proxy_send_raw(debugProxy, &breakByte, 1) {
                emitLog(error(from: ffiError, fallback: "Failed to interrupt target").localizedDescription, logger: logger)
            }
            usleep(500)
        } else {
            let attachCommand = "vAttach;\(String(UInt32(pid), radix: 16))"
            do {
                let response = try sendDebugCommand(attachCommand, debugProxy: debugProxy) ?? "<nil>"
                emitLog("Attach response: \(response)", logger: logger)
            } catch {
                emitLog(error.localizedDescription, logger: logger)
            }
        }

        do {
            let response = try sendDebugCommand("D", debugProxy: debugProxy)
            if let response {
                emitLog("Detach response: \(response)", logger: logger)
            }
        } catch {
            emitLog(error.localizedDescription, logger: logger)
        }
    }

    func debugApp(withBundleID bundleID: String, logger: LogFunc?, jsCallback: DebugAppCallback?) -> Bool {
        runDebugSession(logger: logger, callback: jsCallback) { remoteServer in
            try withProcessControl(remoteServer: remoteServer) { processControl in
                var pid: UInt64 = 0
                let ffiError = bundleID.withCString { bundleID in
                    Self.withCStringArray(Self.mallocDebugEnvVars()) { envPtr, envCount in
                        process_control_launch_app(processControl, bundleID, envPtr, envCount, nil, 0, true, false, &pid)
                    }
                }

                if let ffiError {
                    throw error(from: ffiError, fallback: "Failed to launch app")
                }

                return Int32(pid)
            }
        }
    }

    func debugApp(withPID pid: Int32, logger: LogFunc?, jsCallback: DebugAppCallback?) -> Bool {
        runDebugSession(logger: logger, callback: jsCallback) { _ in pid }
    }

    func launchAppWithoutDebug(_ bundleID: String, logger: LogFunc?) -> Bool {
        do {
            let pid = try withConnectedRemoteServer { remoteServer in
                try withProcessControl(remoteServer: remoteServer) { processControl in
                    var pid: UInt64 = 0
                    let ffiError = bundleID.withCString { bundleID in
                        process_control_launch_app(processControl, bundleID, nil, 0, nil, 0, false, true, &pid)
                    }

                    if let ffiError {
                        throw error(from: ffiError, fallback: "Failed to launch app")
                    }

                    return pid
                }
            }

            emitLog("Launched app (PID \(pid))", logger: logger)
            return true
        } catch {
            emitLog(error.localizedDescription, logger: logger)
            return false
        }
    }

    func relaunchApp(_ bundleID: String, logger: LogFunc? = nil) -> Bool {
        do {
            let pid = try withConnectedRemoteServer { remoteServer in
                try relaunchApp(bundleID, remoteServer: remoteServer)
            }

            emitLog("Returned to \(bundleID) (PID \(pid))", logger: logger)
            return true
        } catch {
            emitLog("Failed to return to \(bundleID): \(error.localizedDescription)", logger: logger)
            return false
        }
    }

    func relaunchApp(_ bundleID: String, usingRemoteServer remoteServer: OpaquePointer, logger: LogFunc? = nil) -> Bool {
        do {
            let pid = try relaunchApp(bundleID, remoteServer: remoteServer)
            emitLog("Returned to \(bundleID) (PID \(pid))", logger: logger)
            return true
        } catch {
            emitLog("Failed to return to \(bundleID): \(error.localizedDescription)", logger: logger)
            return false
        }
    }

    private func relaunchApp(_ bundleID: String, remoteServer: OpaquePointer) throws -> UInt64 {
        try withProcessControl(remoteServer: remoteServer) { processControl in
            var pid: UInt64 = 0
            let ffiError = bundleID.withCString { bundleID in
                process_control_launch_app(processControl, bundleID, nil, 0, nil, 0, false, false, &pid)
            }

            if let ffiError {
                throw error(from: ffiError, fallback: "Failed to return to app")
            }

            return pid
        }
    }

    func startSyslogRelay(handler: @escaping SyslogLineHandler, onError: @escaping SyslogErrorHandler) {
        do {
            try ensureTunnel()
        } catch let nsError as NSError {
            onError(nsError)
            return
        } catch {
            onError(makeError(error.localizedDescription))
            return
        }

        guard !syslogStreaming else { return }

        syslogStreaming = true
        syslogLineHandler = handler
        syslogErrorHandler = onError

        syslogQueue.async { [weak self] in
            guard let self else { return }
            guard let adapter = self.adapter, let handshake = self.handshake else {
                self.handleSyslogFailure(self.makeError("Tunnel is not connected"))
                return
            }

            var client: OpaquePointer?
            if let ffiError = syslog_relay_connect_rsd(adapter, handshake, &client) {
                self.handleSyslogFailure(self.error(from: ffiError, fallback: "Failed to connect to syslog relay"))
                return
            }

            self.syslogClient = client

            while self.syslogStreaming, let client = self.syslogClient {
                var message: UnsafeMutablePointer<CChar>?
                let ffiError = syslog_relay_next(client, &message)
                if let ffiError {
                    if let message {
                        idevice_string_free(message)
                    }
                    self.handleSyslogFailure(self.error(from: ffiError, fallback: "Syslog relay read failed"))
                    break
                }

                guard let message else {
                    continue
                }

                let line = String(cString: message)
                idevice_string_free(message)

                if let handler = self.syslogLineHandler {
                    DispatchQueue.main.async {
                        handler(line)
                    }
                }
            }

            if let client = self.syslogClient {
                syslog_relay_client_free(client)
            }

            self.syslogClient = nil
            self.syslogStreaming = false
            self.syslogLineHandler = nil
            self.syslogErrorHandler = nil
        }
    }

    func stopSyslogRelay() {
        guard syslogStreaming else { return }

        syslogStreaming = false
        syslogLineHandler = nil
        syslogErrorHandler = nil

        syslogQueue.async { [weak self] in
            guard let self else { return }
            if let syslogClient = self.syslogClient {
                syslog_relay_client_free(syslogClient)
                self.syslogClient = nil
            }
        }
    }

    private func runDebugSession(
        logger: LogFunc?,
        callback: DebugAppCallback?,
        pidProvider: (OpaquePointer) throws -> Int32
    ) -> Bool {
        do {
            try withConnectedDebugSession { remoteServer, debugProxy in
                let pid = try pidProvider(remoteServer)
                runDebugServerCommand(
                    pid: pid,
                    debugProxy: debugProxy,
                    remoteServer: remoteServer,
                    logger: logger,
                    callback: callback
                )
            }

            emitLog("Debug session completed", logger: logger)
            return true
        } catch {
            emitLog(error.localizedDescription, logger: logger)
            return false
        }
    }

    private func handleSyslogFailure(_ error: NSError) {
        syslogStreaming = false
        if let syslogClient {
            syslog_relay_client_free(syslogClient)
            self.syslogClient = nil
        }

        let errorHandler = syslogErrorHandler
        syslogLineHandler = nil
        syslogErrorHandler = nil

        if let errorHandler {
            DispatchQueue.main.async {
                errorHandler(error)
            }
        }
    }
}
