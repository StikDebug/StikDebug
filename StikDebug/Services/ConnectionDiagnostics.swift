import Darwin
import Foundation
import Network

struct ConnectionDiagnosis: Equatable {
    enum Problem {
        case pairingFile, invalidAddress, pairingRejected, deviceLocked, developerMode
        case developerService, cellular, networkUnavailable, noVPN, vpnUnresponsive, serviceUnreachable, unknown
    }

    let problem: Problem
    let isLocal: Bool

    var title: String {
        switch problem {
        case .pairingFile: return "Pairing file unavailable"
        case .invalidAddress: return "Invalid target address"
        case .pairingRejected: return "Pairing file rejected"
        case .deviceLocked: return "Device is locked"
        case .developerMode: return "Developer Mode is off"
        case .developerService: return "Developer service unavailable"
        case .cellular: return "Enable Airplane Mode"
        case .networkUnavailable: return "Turn off Airplane Mode"
        case .noVPN: return "Connect LocalDevVPN"
        case .vpnUnresponsive: return "Local VPN isn’t responding"
        case .serviceUnreachable: return "Device isn’t reachable"
        case .unknown: return "Couldn’t connect to the device"
        }
    }

    var systemImage: String {
        switch problem {
        case .pairingFile, .pairingRejected: return "doc.badge.xmark"
        case .invalidAddress: return "network"
        case .deviceLocked: return "lock.fill"
        case .developerMode: return "hammer.fill"
        case .developerService: return "externaldrive.badge.exclamationmark"
        case .cellular, .networkUnavailable: return "airplane"
        case .noVPN, .vpnUnresponsive: return "shield.slash"
        case .serviceUnreachable: return "antenna.radiowaves.left.and.right.slash"
        case .unknown: return "exclamationmark.triangle"
        }
    }

    var hasSimpleError: Bool { problem != .unknown }

    var canConnectVPN: Bool { isLocal && (problem == .noVPN || problem == .vpnUnresponsive) }

    var detectedSummary: String? {
        switch problem {
        case .cellular: return "Cellular"
        case .networkUnavailable: return "No Wi-Fi or cellular connection"
        case .noVPN: return "LocalDevVPN disconnected"
        case .unknown: return nil
        default: return title
        }
    }

    var message: String {
        switch problem {
        case .pairingFile, .pairingRejected:
            if #available(iOS 27.0, *) {
                return "Generate and place a new pairing file using Settings → Pair a Device, or iloader."
            }
            return "Generate and place a new pairing file using iloader."
        case .invalidAddress:
            return "Check Target Device IP in Settings. Enter an IPv4 address such as 10.7.0.1."
        case .deviceLocked:
            return "Unlock the target device and try again."
        case .developerMode:
            return "Enable Developer Mode in the target device’s Settings → Privacy & Security, then follow its restart instructions."
        case .developerService:
            return "Check that Developer Mode is enabled. Try reconnecting; if the service is still unavailable, redownload the developer disk image in Settings."
        case .cellular:
            return "Keep LocalDevVPN connected. Enable Airplane Mode (or just disable Cellular Data), then try again."
        case .networkUnavailable:
            return "Enable Wi-Fi or Cellular Data, then connect LocalDevVPN."
        case .noVPN:
            return "Connect LocalDevVPN, then return to StikDebug and try again."
        case .vpnUnresponsive:
            return "Reconnect LocalDevVPN, then return to StikDebug and try again."
        case .serviceUnreachable:
            return isLocal
                ? "The device’s pairing service isn’t reachable. Check Remote Pairing Port in Settings. If it still fails, restart the device and reconnect LocalDevVPN."
                : "Check Target Device IP and Remote Pairing Port in Settings, and make sure the target device is reachable on your network."
        case .unknown:
            return "Unlock the target device and try again. If the error continues, restart the target device."
        }
    }
}

enum ConnectionDiagnostics {
    struct NetworkConditions {
        let wifi: Bool
        let cellular: Bool
    }

    enum Stage: String {
        case pairingFile, targetAddress, tunnel
    }

    static func annotate(_ error: NSError, stage: Stage, targetIP: String, port: UInt16? = nil) -> NSError {
        var info = error.userInfo
        info["ConnectionStage"] = stage.rawValue
        info["ConnectionTargetIP"] = targetIP
        if let port { info["ConnectionPort"] = Int(port) }
        return NSError(domain: error.domain, code: error.code, userInfo: info)
    }

    static func diagnose(_ error: NSError, targetIP: String) -> ConnectionDiagnosis {
        let target = error.userInfo["ConnectionTargetIP"] as? String ?? targetIP
        let isLocal = target == DeviceConnectionContext.defaultTargetIPAddress
        if let problem = knownProblem(error) {
            return ConnectionDiagnosis(problem: problem, isLocal: isLocal)
        }
        let port = (error.userInfo["ConnectionPort"] as? Int).flatMap(UInt16.init(exactly:))
        let conditions = isLocal ? networkConditions() : nil
        let usesCellular = conditions.map { $0.cellular && !$0.wifi } ?? false
        let interfaces = isLocal ? activeTunnelAddresses() : nil
        let loopback = interfaces.map { !$0.isEmpty && testLoopback(targetIP: target, interfaces: $0) }
        let reachable = port.map { testService(targetIP: target, port: $0) }
        return assessment(isLocal: isLocal, cellular: usesCellular, hasVPN: interfaces.map { !$0.isEmpty }, loopback: loopback, reachable: reachable, networkUnavailable: conditions.map { !$0.wifi && !$0.cellular } ?? false)
    }

    static func knownProblem(_ error: NSError) -> ConnectionDiagnosis.Problem? {
        if error.userInfo["ConnectionStage"] as? String == Stage.pairingFile.rawValue { return .pairingFile }
        if error.userInfo["ConnectionStage"] as? String == Stage.targetAddress.rawValue { return .invalidAddress }
        let detail = error.localizedDescription.lowercased()
        if detail.contains("invalidhostid") || detail.contains("invalid host id") || detail.contains("pairverifyfailed") { return .pairingRejected }
        if detail.contains("devicelocked") || detail.contains("device locked") || detail.contains("passwordprotected") { return .deviceLocked }
        if detail.contains("developermodenotenabled") || detail.contains("developer mode is not enabled") { return .developerMode }
        if detail.contains("servicenotfound") || detail.contains("service not found") || detail.contains("imagenotmounted") || detail.contains("image not mounted") { return .developerService }
        return nil
    }

    static func assessment(isLocal: Bool, cellular: Bool, hasVPN: Bool?, loopback: Bool?, reachable: Bool?, networkUnavailable: Bool = false) -> ConnectionDiagnosis {
        let problem: ConnectionDiagnosis.Problem
        if reachable == true {
            problem = .unknown
        } else if !isLocal {
            problem = reachable == false ? .serviceUnreachable : .unknown
        } else if hasVPN == false || (hasVPN == true && loopback == false) {
            problem = networkUnavailable ? .networkUnavailable : (hasVPN == false ? .noVPN : .vpnUnresponsive)
        } else if cellular {
            problem = .cellular
        } else if loopback == true && reachable == false {
            problem = .serviceUnreachable
        } else {
            problem = .unknown
        }
        return ConnectionDiagnosis(problem: problem, isLocal: isLocal)
    }

    static func connectedNotice(targetIP: String, conditions: NetworkConditions?) -> String? {
        guard targetIP == DeviceConnectionContext.defaultTargetIPAddress,
              let conditions, !conditions.wifi, !conditions.cellular else { return nil }
        return "Connected without Wi-Fi or cellular. You can now enable Cellular Data, and the tunnel will remain connected."
    }

    static func networkConditions() -> NetworkConditions? {
        let monitor = NWPathMonitor()
        let queue = DispatchQueue(label: "com.stik.connectionNetworkCheck")
        let completion = DispatchSemaphore(value: 0)
        var conditions: NetworkConditions?
        monitor.pathUpdateHandler = { path in
            conditions = NetworkConditions(wifi: path.usesInterfaceType(.wifi), cellular: path.usesInterfaceType(.cellular))
            completion.signal()
        }
        monitor.start(queue: queue)
        _ = completion.wait(timeout: .now() + 0.5)
        monitor.cancel()
        return queue.sync { conditions }
    }

    private static func activeTunnelAddresses() -> Set<in_addr_t>? {
        var first: UnsafeMutablePointer<ifaddrs>?
        guard getifaddrs(&first) == 0 else { return nil }
        defer { freeifaddrs(first) }
        var addresses: Set<in_addr_t> = []
        var current = first
        while let interface = current {
            defer { current = interface.pointee.ifa_next }
            guard String(cString: interface.pointee.ifa_name).hasPrefix("utun"),
                  interface.pointee.ifa_flags & UInt32(IFF_UP) != 0,
                  let address = interface.pointee.ifa_addr,
                  address.pointee.sa_family == UInt8(AF_INET) else { continue }
            addresses.insert(UnsafeRawPointer(address).assumingMemoryBound(to: sockaddr_in.self).pointee.sin_addr.s_addr)
        }
        return addresses
    }

    private static func testLoopback(targetIP: String, interfaces: Set<in_addr_t>) -> Bool {
        let fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)
        guard fd >= 0 else { return false }
        defer { close(fd) }
        var local = sockaddr_in()
        local.sin_len = UInt8(MemoryLayout<sockaddr_in>.size)
        local.sin_family = UInt8(AF_INET)
        let bindResult = withUnsafePointer(to: &local) {
            $0.withMemoryRebound(to: sockaddr.self, capacity: 1) {
                bind(fd, $0, socklen_t(MemoryLayout<sockaddr_in>.size))
            }
        }
        guard bindResult == 0 else { return false }
        var length = socklen_t(MemoryLayout<sockaddr_in>.size)
        let portResult = withUnsafeMutablePointer(to: &local) {
            $0.withMemoryRebound(to: sockaddr.self, capacity: 1) { getsockname(fd, $0, &length) }
        }
        guard portResult == 0 else { return false }
        var peer = sockaddr_in()
        peer.sin_len = local.sin_len
        peer.sin_family = local.sin_family
        peer.sin_port = local.sin_port
        guard targetIP.withCString({ inet_pton(AF_INET, $0, &peer.sin_addr) }) == 1 else { return false }
        let connectResult = withUnsafePointer(to: &peer) {
            $0.withMemoryRebound(to: sockaddr.self, capacity: 1) { connect(fd, $0, length) }
        }
        guard connectResult == 0 else { return false }
        let addressResult = withUnsafeMutablePointer(to: &local) {
            $0.withMemoryRebound(to: sockaddr.self, capacity: 1) { getsockname(fd, $0, &length) }
        }
        guard addressResult == 0, interfaces.contains(local.sin_addr.s_addr) else { return false }
        // LocalDevVPN swaps IP addresses without swapping ports; equal ports return the packet to this socket.
        let nonce = Data(UUID().uuidString.utf8)
        guard nonce.withUnsafeBytes({ send(fd, $0.baseAddress, $0.count, MSG_DONTWAIT) }) == nonce.count else { return false }
        var descriptor = pollfd(fd: fd, events: Int16(POLLIN), revents: 0)
        guard poll(&descriptor, 1, 1000) > 0, descriptor.revents & Int16(POLLIN) != 0 else { return false }
        var buffer = [UInt8](repeating: 0, count: 128)
        let count = buffer.withUnsafeMutableBytes { recv(fd, $0.baseAddress, $0.count, MSG_DONTWAIT) }
        guard count == nonce.count else { return false }
        return Data(buffer.prefix(count)) == nonce
    }

    private static func testService(targetIP: String, port: UInt16) -> Bool {
        guard let endpointPort = NWEndpoint.Port(rawValue: port) else { return false }
        let queue = DispatchQueue(label: "com.stik.connectionServiceCheck")
        let completion = DispatchSemaphore(value: 0)
        let connection = NWConnection(host: NWEndpoint.Host(targetIP), port: endpointPort, using: .tcp)
        var reachable = false
        connection.stateUpdateHandler = { state in
            switch state {
            case .ready:
                reachable = true
                completion.signal()
            case .failed:
                completion.signal()
            default: break
            }
        }
        connection.start(queue: queue)
        _ = completion.wait(timeout: .now() + 1.5)
        connection.cancel()
        return queue.sync { reachable }
    }
}
