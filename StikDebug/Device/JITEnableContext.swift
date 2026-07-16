//
//  JITEnableContext.swift
//  StikDebug
//
//  Created by Stephen on 2026/3/30.
//

import Foundation
import idevice
import Darwin

typealias LogFunc = (String?) -> Void
typealias DebugAppCallback = (_ pid: Int32, _ debugProxy: OpaquePointer?, _ remoteServer: OpaquePointer?, _ semaphore: DispatchSemaphore) -> Void
typealias SyslogLineHandler = (String) -> Void
typealias SyslogErrorHandler = (NSError?) -> Void

final class JITEnableContext {
    static let shared = JITEnableContext()

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
        let logURL = FileManager.default
            .urls(for: .documentDirectory, in: .userDomainMask)[0]
            .appendingPathComponent("idevice_log.txt")

        var path = Array(logURL.path.utf8CString)
        path.withUnsafeMutableBufferPointer { buffer in
            _ = idevice_init_logger(Info, Debug, buffer.baseAddress)
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
        let message = nsString(from: ffiError.pointee.message, fallback: fallback)
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

    private func getPairingFile() throws -> OpaquePointer {
        let pairingFileURL = PairingFileStore.prepareURL()

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

    private func createTunnel(hostname: String) throws -> TunnelHandles {
        let pairingFile = try getPairingFile()
        defer { rp_pairing_file_free(pairingFile) }

        var addr = sockaddr_in()
        addr.sin_family = sa_family_t(AF_INET)
        addr.sin_port = in_port_t(49152).bigEndian

        let deviceIP = DeviceConnectionContext.targetIPAddress
        let parseResult = deviceIP.withCString { inet_pton(AF_INET, $0, &addr.sin_addr) }
        guard parseResult == 1 else {
            throw makeError("Failed to parse target IP address.", code: -18)
        }

        var tunnel = TunnelHandles()
        let ffiError = hostname.withCString { hostname in
            withUnsafePointer(to: &addr) { pointer in
                pointer.withMemoryRebound(to: sockaddr.self, capacity: 1) {
                    tunnel_create_rppairing(
                        $0,
                        socklen_t(MemoryLayout<sockaddr_in>.stride),
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
            throw error(from: ffiError, fallback: "Failed to create tunnel")
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
                waitSemaphore.wait()
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

    private func withFreshDebugTunnel<T>(
        hostname: String,
        _ body: (OpaquePointer, OpaquePointer) throws -> T
    ) throws -> T {
        var tunnel = try createTunnel(hostname: hostname)
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
        try ensureTunnel()
        guard let adapter, let handshake else {
            throw makeError("Tunnel is not connected")
        }

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
                    process_control_launch_app(processControl, bundleID, nil, 0, nil, 0, true, false, &pid)
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

    /// Launches an app, attaches the debugger, runs the app's assigned JIT
    /// script (if any), and *holds* the session open so the app keeps running
    /// in the background.
    ///
    /// A debugger-owned process is not suspended by iOS when it moves to the
    /// background (the same reason an app run from Xcode keeps executing while
    /// backgrounded). Attaching also sets `CS_DEBUGGED`, so JIT is enabled too.
    /// We additionally lift the process memory limit to reduce the chance of a
    /// background jetsam kill. The hold runs until `cancellation` is set, then we
    /// interrupt and detach so the app continues running on its own.
    ///
    /// - Parameter script: The same JIT-script callback the normal debug path
    ///   uses. When present it runs once (doing its own attach + JIT enable +
    ///   continue) before the hold begins, so scripts work in hold mode too;
    ///   when `nil` the app is simply attached and continued.
    /// - Note: This keeps StikDebug's connection to *this* app alive; it cannot
    ///   revive an app iOS has already killed. Requires StikDebug itself to stay
    ///   running (see `DebugKeepAliveLease`).
    func keepAppAlive(withBundleID bundleID: String, script: DebugAppCallback?, cancellation: HoldToken, logger: LogFunc?) -> Bool {
        do {
            try withConnectedDebugSession { remoteServer, debugProxy in
                let pid = try withProcessControl(remoteServer: remoteServer) { processControl in
                    var pid: UInt64 = 0
                    let ffiError = bundleID.withCString { bundleID in
                        process_control_launch_app(processControl, bundleID, nil, 0, nil, 0, true, false, &pid)
                    }
                    if let ffiError {
                        throw error(from: ffiError, fallback: "Failed to launch app")
                    }

                    // Best-effort: lift the memory limit so iOS is less likely to
                    // jetsam-kill the app while it is in the background.
                    if let limitError = process_control_disable_memory_limit(processControl, pid) {
                        let disableError = error(from: limitError, fallback: "Failed to disable memory limit")
                        emitLog("Keep-alive: could not disable memory limit: \(disableError.localizedDescription)", logger: logger)
                    }

                    return Int32(truncatingIfNeeded: pid)
                }

                try holdDebugSession(
                    pid: pid,
                    debugProxy: debugProxy,
                    remoteServer: remoteServer,
                    script: script,
                    logger: logger,
                    cancellation: cancellation
                )
            }

            emitLog("Keep-alive session ended for \(bundleID)", logger: logger)
            return true
        } catch {
            emitLog("Keep-alive session failed: \(error.localizedDescription)", logger: logger)
            return false
        }
    }

    private func holdDebugSession(
        pid: Int32,
        debugProxy: OpaquePointer,
        remoteServer: OpaquePointer,
        script: DebugAppCallback?,
        logger: LogFunc?,
        cancellation: HoldToken
    ) throws {
        debug_proxy_send_ack(debugProxy)
        debug_proxy_send_ack(debugProxy)

        do {
            let response = try sendDebugCommand("QStartNoAckMode", debugProxy: debugProxy) ?? "<nil>"
            emitLog("Keep-alive QStartNoAckMode = \(response)", logger: logger)
        } catch {
            emitLog(error.localizedDescription, logger: logger)
        }

        debug_proxy_set_ack_mode(debugProxy, 0)

        // Keep the RSD/lockdown tunnel trusted for the whole hold.
        let heartbeat = try? connectHeartbeatKeepAlive(logger: logger)
        try? heartbeat?.start()
        defer { heartbeat?.stop() }

        if let script {
            // TXM / iOS 26+ JIT: the app requests executable memory at runtime via
            // `brk #0xf00d` syscalls that the debugger must service. That is exactly
            // what the JIT script does — it attaches (`vAttach`, enabling JIT),
            // then loops handling those breakpoints for as long as the app uses
            // JIT, which also keeps the app alive under the debugger the whole
            // time. Merely attaching without running the script (the old hold
            // behavior) left those syscalls unhandled, so the app trapped and hung
            // on its first JIT call — the bug this fixes.
            //
            // The script issues its own attach/continue via `send_command`, so we
            // must NOT send `vAttach` here (that would double-attach). It runs
            // until the app detaches or exits; the semaphore blocks until then.
            // StikDebug itself is held alive across this by the caller's
            // `DebugKeepAliveLease`.
            emitLog("Keep-alive: running JIT script (services JIT and holds the app)", logger: logger)
            let semaphore = DispatchSemaphore(value: 0)
            script(pid, debugProxy, remoteServer, semaphore)
            semaphore.wait()
            emitLog("Keep-alive: JIT script ended (app detached or exited)", logger: logger)

            // If the user stopped the hold (rather than the app detaching on its
            // own), detach cleanly so the app keeps running instead of being
            // torn down when the debug connection closes.
            if cancellation.isCancelled {
                var interrupt: UInt8 = 0x03
                _ = debug_proxy_send_raw(debugProxy, &interrupt, 1)
                usleep(100_000)
                do {
                    if let response = try sendDebugCommand("D", debugProxy: debugProxy) {
                        emitLog("Keep-alive detach response: \(response)", logger: logger)
                    }
                } catch {
                    emitLog(error.localizedDescription, logger: logger)
                }
            }
            return
        }

        // No script: hold the app by attaching and *actively servicing* the
        // debug connection. A debugger-owned process is not suspended by iOS in
        // the background — but a debugged process also stops on every signal or
        // exception, and if we never answer those stops it stays halted on the
        // first one, can't service its own run loop, and iOS's watchdog kills it
        // (which is what looked like the app "crashing" / keep-alive not
        // working). So we continue on each stop, forwarding real signals so the
        // app behaves exactly as if it were not being debugged, and keep going
        // until the app exits or the user stops the hold.
        let attachCommand = "vAttach;\(String(UInt32(bitPattern: pid), radix: 16))"
        var stopReply = try sendDebugCommand(attachCommand, debugProxy: debugProxy) ?? ""
        emitLog("Keep-alive attach response: \(stopReply)", logger: logger)
        emitLog("Keep-alive: app is running and held under the debugger", logger: logger)

        // Cancellation is re-checked after each stop. While the app runs, the
        // continue blocks reading the socket — single-threaded, so there is no
        // concurrent access to the (non-thread-safe) debug proxy. The hold also
        // ends by itself when the app exits (the natural end when the user
        // closes it).
        while !cancellation.isCancelled {
            // A process-exit reply ("W"/"X") means the app is gone — nothing
            // left to hold or detach.
            if stopReply.hasPrefix("W") || stopReply.hasPrefix("X") {
                emitLog("Keep-alive: app exited (\(stopReply)); ending hold", logger: logger)
                return
            }

            do {
                stopReply = try sendDebugCommand(holdContinueCommand(forStopReply: stopReply), debugProxy: debugProxy) ?? ""
            } catch {
                // The connection dropped — the app was killed or the tunnel
                // died. Stop holding; there is nothing to detach from.
                emitLog("Keep-alive: debug connection ended (\(error.localizedDescription)); ending hold", logger: logger)
                return
            }

            if stopReply.isEmpty {
                emitLog("Keep-alive: empty stop reply; ending hold", logger: logger)
                return
            }
        }

        // Cancelled by the user: interrupt, then detach, so the app keeps
        // running on its own afterward.
        var interrupt: UInt8 = 0x03
        _ = debug_proxy_send_raw(debugProxy, &interrupt, 1)
        usleep(100_000)
        do {
            if let response = try sendDebugCommand("D", debugProxy: debugProxy) {
                emitLog("Keep-alive detach response: \(response)", logger: logger)
            }
        } catch {
            emitLog(error.localizedDescription, logger: logger)
        }
    }

    /// The continue command used to resume a held app after it stops. Real
    /// signals are forwarded (`C<sig>`) so the app behaves as if it were not
    /// under a debugger; debugger artifacts — the SIGSTOP from attaching and
    /// SIGTRAP traps — are swallowed with a plain continue so they don't
    /// immediately re-stop or re-trap the app.
    private func holdContinueCommand(forStopReply reply: String) -> String {
        guard reply.hasPrefix("T"), reply.count >= 3,
              let signal = UInt8(reply.dropFirst().prefix(2), radix: 16) else {
            return "c"
        }

        switch Int32(signal) {
        case 0, SIGSTOP, SIGTRAP:
            return "c"
        default:
            return String(format: "C%02x", signal)
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
