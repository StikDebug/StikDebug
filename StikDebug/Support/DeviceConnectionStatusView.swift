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

    private var diagnosis: ConnectionDiagnosis? {
        tunnel.lastError == nil ? nil : tunnel.lastDiagnosis
    }

    private var title: String {
        if tunnel.isStarting { return String(localized: "Connecting…") }
        if let diagnosis { return diagnosis.title }
        if !tunnel.isConnected { return String(localized: "Device not connected") }
        if isReadingApps { return String(localized: "Connecting…") }
        if errorMessage != nil { return String(localized: "Connection error") }
        if !hasConfirmedApps { return String(localized: "App list unavailable") }
        return String(localized: "Device connected")
    }

    private var diagnosisSummary: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(title)
            if !isWorking, let diagnosis {
                Text(diagnosis.message)
                    .font(.subheadline)
            }
        }
        .frame(maxWidth: .infinity, alignment: .leading)
    }

    var body: some View {
        HStack(alignment: .top, spacing: 12) {
            if isWorking {
                ProgressView()
            } else {
                Image(systemName: diagnosis?.systemImage ?? (tunnel.isConnected && errorMessage == nil ? "checkmark.circle" : "exclamationmark.circle"))
                    .foregroundStyle(tunnel.isConnected && errorMessage == nil ? .green : .orange)
            }
            VStack(alignment: .leading, spacing: 4) {
                diagnosisSummary
                if !isWorking, diagnosis?.canConnectVPN == true {
                    Button(action: connectLocalDevVPN) {
                        Text("Tap here to connect the VPN")
                            .font(.subheadline)
                            .foregroundStyle(.tint)
                            .multilineTextAlignment(.leading)
                            .fixedSize(horizontal: false, vertical: true)
                    }
                    .buttonStyle(.borderless)
                    .accessibilityHint("Opens LocalDevVPN")
                }
                if !isWorking, tunnel.isConnected, errorMessage == nil, let notice = tunnel.connectionNotice {
                    Text(notice)
                        .font(.subheadline)
                }
                if !isWorking, diagnosis?.hasSimpleError != true, let detail = errorDetail {
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
                    let detected = diagnosis?.detectedSummary.map { "Detected: \($0)\n" } ?? ""
                    UIPasteboard.general.string = "StikDebug error:\n\(detail)\n\(detected)v\(diagnostics)"
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
        .onReceive(NotificationCenter.default.publisher(for: UIApplication.didBecomeActiveNotification)) { _ in
            tunnel.refreshDiagnosis()
        }
        .onChange(of: errorDetail) { _, _ in didCopy = false }
    }
}
