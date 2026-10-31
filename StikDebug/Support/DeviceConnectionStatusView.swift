import SwiftUI
import UIKit

struct DeviceConnectionStatusView: View {
    @ObservedObject private var tunnel = TunnelManager.shared
    @State private var didCopy = false
    var isReadingApps = false
    var hasConfirmedApps = true
    var errorMessage: String?

    private var isWorking: Bool { tunnel.isStarting || isReadingApps }

    private var errorDetail: String? {
        tunnel.lastError.map { "Code \($0.code): \($0.localizedDescription)" } ?? errorMessage
    }

    private var title: String {
        if tunnel.isStarting { return "Connecting…" }
        if !tunnel.isConnected { return "Device not connected" }
        if isReadingApps { return "Connecting…" }
        if errorMessage != nil { return "Connection error" }
        if !hasConfirmedApps { return "App list unavailable" }
        return "Device connected"
    }

    var body: some View {
        HStack(alignment: .top, spacing: 12) {
            if isWorking {
                ProgressView()
            } else {
                Image(systemName: tunnel.isConnected && errorMessage == nil ? "checkmark.circle" : "exclamationmark.circle")
                    .foregroundStyle(tunnel.isConnected && errorMessage == nil ? .green : .orange)
            }
            VStack(alignment: .leading, spacing: 4) {
                Text(title)
                if !isWorking, let detail = errorDetail {
                    Text(detail)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                        .textSelection(.enabled)
                }
            }
            Spacer(minLength: 0)
            if !isWorking, let detail = errorDetail {
                Button {
                    let diagnostics = connectionDiagnosticDetails().components(separatedBy: "\n").joined(separator: ", ")
                    UIPasteboard.general.string = "StikDebug error:\n\(detail)\nv\(diagnostics)"
                    didCopy = true
                } label: {
                    Image(systemName: didCopy ? "checkmark" : "doc.on.doc")
                        .frame(minWidth: 44, minHeight: 44)
                }
                .buttonStyle(.borderless)
                .accessibilityLabel(didCopy ? "Error copied" : "Copy Error")
                .task(id: didCopy) {
                    guard didCopy else { return }
                    try? await Task.sleep(for: .seconds(1.2))
                    guard !Task.isCancelled else { return }
                    didCopy = false
                }
            }
        }
        .onChange(of: errorDetail) { _, _ in didCopy = false }
    }
}
