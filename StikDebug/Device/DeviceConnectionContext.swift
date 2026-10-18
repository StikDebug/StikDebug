//
//  DeviceConnectionContext.swift
//  StikDebug
//
//  Created by Stephen.
//

import Foundation

enum DeviceConnectionContext {
    static let defaultTargetIPAddress = "10.7.0.1"
    static let defaultTargetPort: UInt16 = 49152

    /// Port typed by the user in Settings. `nil` means auto-discovery.
    static var configuredTargetPort: UInt16? {
        let stored = UserDefaults.standard
            .string(forKey: UserDefaults.Keys.targetDevicePort)?
            .trimmingCharacters(in: .whitespacesAndNewlines)
        guard let stored, let port = UInt16(stored), port > 0 else {
            return nil
        }
        return port
    }

    /// Best-known port without running discovery.
    static var targetPort: UInt16 {
        configuredTargetPort ?? RemotePairingDiscovery.shared.currentPort ?? defaultTargetPort
    }

    /// Port to connect with, discovering the RemotePairing service if needed.
    static func resolvedTargetPort() -> UInt16 {
        if let configured = configuredTargetPort {
            return configured
        }
        if let discovered = RemotePairingDiscovery.shared.resolvePortIfNeeded() {
            return discovered
        }
        return defaultTargetPort
    }

    static var targetIPAddress: String {
        let stored = UserDefaults.standard
            .string(forKey: UserDefaults.Keys.targetDeviceIP)?
            .trimmingCharacters(in: .whitespacesAndNewlines)
        guard let stored, !stored.isEmpty else {
            return defaultTargetIPAddress
        }
        return stored
    }
}
