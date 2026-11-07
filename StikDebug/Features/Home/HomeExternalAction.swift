//
//  HomeExternalAction.swift
//  StikDebug
//

import Foundation
import SwiftUI

struct JITEnableConfiguration {
    var bundleID: String?
    var pid: Int?
    var scriptData: Data?
    var scriptName: String?
}

enum ProcessKillTarget {
    case pid(Int32)
    case name(String)

    var description: String {
        switch self {
        case .pid(let pid): return String(pid)
        case .name(let name): return name
        }
    }
}

enum HomeExternalAction: Identifiable {
    case enableJIT(JITEnableConfiguration)
    case killProcess(ProcessKillTarget)
    case launchApp(String)

    var id: String {
        switch self {
        case .enableJIT(let configuration):
            return "enable-\(configuration.bundleID ?? "")-\(configuration.pid ?? 0)-\(configuration.scriptName ?? "")"
        case .killProcess(let target):
            return "kill-\(target)"
        case .launchApp(let bundleID):
            return "launch-\(bundleID)"
        }
    }

    var title: LocalizedStringKey {
        switch self {
        case .enableJIT:
            return "Enable JIT?"
        case .killProcess:
            return "Kill Process?"
        case .launchApp:
            return "Launch App?"
        }
    }

    var message: String {
        switch self {
        case .enableJIT(let configuration):
            let scriptText = configuration.scriptData == nil ? "" : String(localized: " and run a script")
            return String(localized: "An external link wants to enable JIT\(scriptText) for \(targetDescription(for: configuration)).")
        case .killProcess(let target):
            return String(localized: "An external link wants to kill process \(target.description).")
        case .launchApp(let bundleID):
            return String(localized: "An external link wants to launch \(bundleID).")
        }
    }

    var confirmationTitle: String {
        switch self {
        case .enableJIT(let configuration):
            return configuration.scriptData == nil ? String(localized: "Enable JIT") : String(localized: "Enable and Run Script")
        case .killProcess:
            return String(localized: "Kill Process")
        case .launchApp:
            return String(localized: "Launch App")
        }
    }

    var role: ButtonRole? {
        switch self {
        case .enableJIT(let configuration):
            return configuration.scriptData == nil ? nil : .destructive
        case .killProcess:
            return .destructive
        case .launchApp:
            return nil
        }
    }

    private func targetDescription(for configuration: JITEnableConfiguration) -> String {
        if let bundleID = configuration.bundleID {
            return bundleID
        }
        if let pid = configuration.pid {
            return String(localized: "process \(pid)")
        }
        return String(localized: "the requested app")
    }
}
