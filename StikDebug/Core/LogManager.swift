//
//  LogManager.swift
//  StikDebug
//
//  Created by neoarz on 3/29/25.
//

import Foundation

final class LogManager: ObservableObject {
    static let shared = LogManager()

    @Published var logs: [LogEntry] = []
    @Published var errorCount: Int = 0

    struct LogEntry: Identifiable, Sendable {
        let id: UUID
        let timestamp: Date
        let type: LogType
        let message: String
        let origin: Origin

        enum LogType: String, Sendable {
            case info    = "INFO"
            case error   = "ERROR"
            case debug   = "DEBUG"
            case warning = "WARNING"
        }

        /// Where the entry came from. Reloading the idevice trace replaces only
        /// its own entries, so what StikDebug reported stays put.
        enum Origin: Sendable {
            case app
            case idevice
        }

        init(timestamp: Date, type: LogType, message: String, origin: Origin = .app) {
            self.id = UUID()
            self.timestamp = timestamp
            self.type = type
            self.message = message
            self.origin = origin
        }
    }

    private static let redundantPrefixes: [String] = [
        "Info: ", "INFO: ", "Information: ",
        "Error: ", "ERROR: ", "ERR: ",
        "Debug: ", "DEBUG: ", "DBG: ",
        "Warning: ", "WARN: ", "WARNING: "
    ]

    private init() {
        addInfoLog("StikDebug starting up")
        addInfoLog("Initializing environment")
    }

    func addLog(message: String, type: LogEntry.LogType) {
        let clean = Self.redundantPrefixes
            .first(where: { message.hasPrefix($0) })
            .map { String(message.dropFirst($0.count)) } ?? message

        DispatchQueue.main.async {
            self.logs.append(LogEntry(timestamp: Date(), type: type, message: clean))
            if type == .error { self.errorCount += 1 }
            if self.logs.count > 1000 { self.logs.removeFirst(100) }
        }
    }

    func addInfoLog(_ message: String)    { addLog(message: message, type: .info) }
    func addErrorLog(_ message: String)   { addLog(message: message, type: .error) }
    func addDebugLog(_ message: String)   { addLog(message: message, type: .debug) }
    func addWarningLog(_ message: String) { addLog(message: message, type: .warning) }

    /// Replaces the entries read from `idevice_log.txt`, keeping everything
    /// StikDebug logged itself.
    func setIdeviceLogs(_ entries: [LogEntry]) {
        DispatchQueue.main.async {
            let ownEntries = self.logs.filter { $0.origin == .app }
            self.logs = (ownEntries + entries).sorted { $0.timestamp < $1.timestamp }
            self.errorCount = self.logs.filter { $0.type == .error }.count
        }
    }

    func appendLogs(_ entries: [LogEntry], maxTotal: Int = 1000) {
        DispatchQueue.main.async {
            self.logs.append(contentsOf: entries)

            guard self.logs.count > maxTotal else { return }

            // Trim the trace, never what StikDebug reported: the trace runs to
            // hundreds of lines a second and would otherwise push out the few
            // messages that say what went wrong.
            var excess = self.logs.count - maxTotal
            var kept: [LogEntry] = []
            kept.reserveCapacity(maxTotal)

            for entry in self.logs {
                if excess > 0, entry.origin == .idevice {
                    excess -= 1
                    continue
                }
                kept.append(entry)
            }

            self.logs = kept
            self.errorCount = kept.filter { $0.type == .error }.count
        }
    }

    func clearLogs() {
        DispatchQueue.main.async {
            self.logs.removeAll()
            self.errorCount = 0
        }
    }
}
