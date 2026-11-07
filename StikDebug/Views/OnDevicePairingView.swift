import SwiftUI

struct OnDevicePairingView: View {
    @ObservedObject var service: OnDevicePairingService
    @State private var appleTVPIN = ""
    @State private var pairingAppleTV = false
    @State private var showingAppPlacement = false

    var body: some View {
        Form {
            content
        }
        .navigationTitle("Create Pairing File")
        .navigationBarTitleDisplayMode(.inline)
        .animation(.default, value: service.phase)
        .sheet(isPresented: $showingAppPlacement) {
            if let url = service.exportURL {
                PairingFilePlacementView(pairingFileURL: url, target: service.placementTarget)
            }
        }
        .onDisappear {
            service.cancel()
        }
    }

    @ViewBuilder
    private var content: some View {
        switch service.phase {
        case .idle:
            Section {
                Button {
                    pairingAppleTV = false
                    service.start(discoverAppleTVs: false)
                } label: {
                    Label("Pair iPhone or iPad", systemImage: "iphone")
                }
                Button {
                    pairingAppleTV = true
                    service.start(advertisePhone: false)
                } label: {
                    Label("Pair Apple TV", systemImage: "appletv")
                }
            } footer: {
                Text("Create a pairing file without a computer. The iPhone or iPad being paired must run iOS or iPadOS 27 or later. Apple TV pairing is also supported.")
            }

        case .discovering:
            if pairingAppleTV {
                Section {
                    if service.appleTVDevices.isEmpty {
                        HStack(spacing: 12) {
                            ProgressView()
                            Text("Looking for Apple TVs…")
                                .foregroundStyle(.secondary)
                        }
                    } else {
                        ForEach(service.appleTVDevices) { device in
                            Button {
                                appleTVPIN = ""
                                service.pairAppleTV(device)
                            } label: {
                                Label(device.name, systemImage: "appletv")
                            }
                        }
                    }
                } header: {
                    Text("Choose an Apple TV")
                } footer: {
                    Text("On Apple TV, open Settings › Remotes and Devices › Remote App and Devices.")
                }
            } else {
                Section {
                    HStack(spacing: 12) {
                        ProgressView()
                        Text("Waiting for a device to connect…")
                            .foregroundStyle(.secondary)
                    }
                }
                Section {
                    guideStep(1, "Open Settings on the device being paired.")
                    guideStep(2, "Go to Privacy & Security › Developer Mode.")
                    guideStep(3, "Tap Pair with \(service.hostname).")
                    guideStep(4, "Enter the code shown by StikDebug or in its notification.")
                } header: {
                    Text("On the iPhone or iPad")
                } footer: {
                    Text("For this device, StikDebug stays active while you open Settings. For another device, keep this screen open to see its code.")
                }
            }
            cancelSection

        case .waiting:
            Section {
                HStack(spacing: 12) {
                    ProgressView()
                    Text(service.mode == .appleTV
                         ? "Pairing with \(service.selectedAppleTVName ?? "Apple TV")…"
                         : "Confirming the iPhone or iPad pairing…")
                }
            }
            cancelSection

        case .pin(let pin):
            Section {
                VStack(spacing: 14) {
                    Text(service.mode == .localDevice
                         ? "Enter this code in Settings on this device"
                         : "Enter this code on the other device")
                        .font(.headline)
                        .multilineTextAlignment(.center)
                    Text(pin)
                        .font(.system(size: 44, weight: .bold, design: .rounded))
                        .monospacedDigit()
                        .tracking(6)
                        .textSelection(.enabled)
                    ProgressView()
                }
                .frame(maxWidth: .infinity)
                .padding(.vertical, 8)
            }
            cancelSection

        case .appleTVPIN:
            Section {
                TextField("Six-digit code", text: $appleTVPIN)
                    .keyboardType(.numberPad)
                    .textContentType(.oneTimeCode)
                    .multilineTextAlignment(.center)
                    .font(.system(size: 32, weight: .semibold, design: .rounded))
                    .onChange(of: appleTVPIN) { _, newValue in
                        appleTVPIN = String(newValue.filter { $0 >= "0" && $0 <= "9" }.prefix(6))
                    }
                Button("Pair") {
                    service.submitAppleTVPIN(appleTVPIN)
                    appleTVPIN = ""
                }
                .disabled(appleTVPIN.count != 6)
            } header: {
                Text("Pair with \(service.selectedAppleTVName ?? "Apple TV")")
            } footer: {
                Text("Enter the six-digit code shown on your Apple TV.")
            }
            cancelSection

        case .success(let name, let model, let isLocal):
            Section {
                Label("Pairing Complete", systemImage: "checkmark.seal.fill")
                    .foregroundStyle(.green)
                if !name.isEmpty || !model.isEmpty {
                    Text([name, model].filter { !$0.isEmpty }.joined(separator: " · "))
                        .foregroundStyle(.secondary)
                }
                if isLocal {
                    Label("Now used by StikDebug", systemImage: "checkmark.circle.fill")
                        .foregroundStyle(.green)
                }
            }
            Section {
                if service.exportURL != nil {
                    Button {
                        showingAppPlacement = true
                    } label: {
                        Label("Place in Another App", systemImage: "square.and.arrow.down.on.square")
                    }
                }
                if let exportURL = service.exportURL {
                    ShareLink(item: exportURL) {
                        Label("Export Pairing File", systemImage: "square.and.arrow.up")
                    }
                }
            } footer: {
                Text(isLocal
                     ? "The pairing file is saved in StikDebug. You can also place it in a supported app or export it."
                     : "You can place this pairing file in a supported app on the paired device or export it.")
            }

        case .failed(let message):
            Section {
                Label("Pairing Failed", systemImage: "xmark.octagon.fill")
                    .foregroundStyle(.red)
                Text(message)
                    .font(.footnote)
                    .foregroundStyle(.secondary)
                    .textSelection(.enabled)
            }
            Section {
                Button("Try Again") {
                    service.reset()
                }
            }
        }
    }

    private var cancelSection: some View {
        Section {
            Button("Cancel", role: .cancel) {
                service.cancel()
            }
        }
    }

    private func guideStep(_ number: Int, _ text: LocalizedStringKey) -> some View {
        HStack(alignment: .firstTextBaseline, spacing: 10) {
            Text("\(number)")
                .font(.caption.bold())
                .foregroundStyle(.white)
                .frame(width: 20, height: 20)
                .background(.tint, in: Circle())
            Text(text)
                .font(.subheadline)
                .fixedSize(horizontal: false, vertical: true)
        }
    }
}

private struct PairingFilePlacementView: View {
    @Environment(\.dismiss) private var dismiss
    let pairingFileURL: URL
    let target: PairingPlacementTarget?
    @State private var apps: [PairingDestinationApp] = []
    @State private var isLoading = true
    @State private var placingApp: PairingDestinationApp?
    @State private var pendingApp: PairingDestinationApp?
    @State private var message: String?
    @State private var isError = false

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    if isLoading {
                        ProgressView("Loading installed apps…")
                            .frame(maxWidth: .infinity)
                    } else if apps.isEmpty {
                        Text(target == nil
                             ? "No supported apps were found on this device."
                             : "No supported apps were found on the paired device.")
                            .foregroundStyle(.secondary)
                    } else {
                        ForEach(apps) { app in
                            Button {
                                pendingApp = app
                            } label: {
                                VStack(alignment: .leading, spacing: 4) {
                                    Text(app.name)
                                    Text(app.bundleID)
                                        .font(.caption)
                                        .foregroundStyle(.secondary)
                                }
                            }
                            .disabled(placingApp != nil)
                        }
                    }
                } header: {
                    Text("Choose an App")
                } footer: {
                    Text(target == nil
                         ? "LocalDevVPN must be enabled. Only apps with known Remote Pairing file destinations are listed. The selected app must allow access to its Documents folder."
                         : "Keep both devices on the same Wi-Fi network. Apps are read from the paired device. The selected app must allow access to its Documents folder.")
                }
                if let placingApp {
                    Section {
                        ProgressView("Placing pairing file in \(placingApp.name)…")
                            .frame(maxWidth: .infinity)
                    }
                }
                if let message {
                    Section {
                        Label(message, systemImage: isError ? "exclamationmark.triangle" : "checkmark.circle")
                            .foregroundStyle(isError ? .red : .green)
                    }
                }
            }
            .navigationTitle("Place Pairing File")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Done") { dismiss() }
                        .disabled(placingApp != nil)
                }
            }
            .confirmationDialog(
                "Replace the pairing file in \(pendingApp?.name ?? "this app")?",
                isPresented: Binding(
                    get: { pendingApp != nil },
                    set: { if !$0 { pendingApp = nil } }
                ),
                titleVisibility: .visible,
                presenting: pendingApp
            ) { app in
                Button("Place Pairing File", role: .destructive) {
                    pendingApp = nil
                    place(in: app)
                }
                Button("Cancel", role: .cancel) { pendingApp = nil }
            } message: { app in
                Text("Writes Documents/\(app.path), replacing any existing pairing file.")
            }
        }
        .interactiveDismissDisabled(placingApp != nil)
        .task {
            do {
                let loaded = try await Task.detached {
                    try JITEnableContext.shared.pairingDestinationApps(target: target)
                }.value
                guard !Task.isCancelled else { return }
                apps = loaded
            } catch {
                guard !Task.isCancelled else { return }
                isError = true
                message = error.localizedDescription
            }
            isLoading = false
        }
    }

    private func place(in app: PairingDestinationApp) {
        placingApp = app
        message = nil
        Task {
            do {
                let data = try Data(contentsOf: pairingFileURL)
                try await Task.detached {
                    try JITEnableContext.shared.placePairingFile(data, in: app, target: target)
                }.value
                isError = false
                message = "Pairing file placed in \(app.name)."
            } catch {
                isError = true
                message = error.localizedDescription
            }
            placingApp = nil
        }
    }
}
