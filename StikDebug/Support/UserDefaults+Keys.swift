//
//  UserDefaults+Keys.swift
//  StikDebug
//

import Foundation

extension UserDefaults {
    enum Keys {
        /// Forces the app to treat the current device as TXM-capable so scripts always run.
        static let txmOverride = "overrideTXMForScripts"
        /// Requires confirmation before external links can enable JIT.
        static let confirmExternalJITRequests = "confirmExternalJITRequests"
        static let bundleScriptMap = "BundleScriptMap"
        static let defaultScriptName = "DefaultScriptName"
        static let defaultScriptNameValue = ""
        static let targetDeviceIP = "TunnelDeviceIP"
        /// Attaches processes with MallocGuardEdges and MallocScribble.
        static let mallocDebug = "enableMallocDebug"
        static let mallocGuardEdges = "enableMallocGuardEdges"
        static let mallocScribble = "enableMallocScribble"
    }
}
