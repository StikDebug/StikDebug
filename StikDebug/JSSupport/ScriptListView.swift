//
//  ScriptListView.swift
//  StikDebug
//
//  Created by s s on 2025/7/4.
//

import SwiftUI
import UniformTypeIdentifiers
import UIKit

struct ScriptListView: View {
    @State private var scripts: [URL] = []
    @State private var showNewFileAlert = false
    @State private var newFileName = ""
    @State private var showImporter = false
    @AppStorage(UserDefaults.Keys.defaultScriptName) private var defaultScriptName = UserDefaults.Keys.defaultScriptNameValue

    @State private var isBusy = false
    @State private var alertVisible = false
    @State private var alertTitle = ""
    @State private var alertMessage = ""
    @State private var alertIsSuccess = false
    @State private var justCopied = false
    @State private var searchText = ""

    @State private var showDeleteConfirmation = false
    @State private var pendingDelete: URL? = nil

    var allowsNoScript = true
    var onSelectScript: ((URL?) -> Void)? = nil

    private var isPickerMode: Bool { onSelectScript != nil }

    private var filteredScripts: [URL] {
        guard !searchText.isEmpty else { return scripts }
        return scripts.filter { $0.lastPathComponent.localizedCaseInsensitiveContains(searchText) }
    }


    var body: some View {
        NavigationStack {
            List {
                if isPickerMode && allowsNoScript {
                    Section {
                        Button {
                            onSelectScript?(nil)
                        } label: {
                            Label("No Script", systemImage: "nosign")
                        }
                    }
                }

                if filteredScripts.isEmpty {
                    Section {
                        VStack(alignment: .leading, spacing: 4) {
                            Label(
                                isPickerMode ? "No scripts available" : "No scripts found",
                                systemImage: "doc.text.magnifyingglass"
                            )
                            .foregroundStyle(.secondary)
                            Text(isPickerMode ? (allowsNoScript ? "Import a file or choose None." : "Import a script file.") : "Tap New or Import to get started.")
                                .font(.caption)
                                .foregroundStyle(.secondary)
                        }
                        .padding(.vertical, 4)
                    }
                } else {
                    Section {
                        ForEach(filteredScripts, id: \.self) { script in
                            let canDelete = !isPickerMode && !ScriptStore.isBuiltInScript(script)
                            scriptRow(script)
                                .swipeActions(edge: .trailing, allowsFullSwipe: true) {
                                    if canDelete {
                                        Button(role: .destructive) {
                                            pendingDelete = script
                                            showDeleteConfirmation = true
                                        } label: { Label("Delete", systemImage: "trash") }
                                    }
                                }
                                .contextMenu {
                                    Button { copyName(script) } label: {
                                        Label("Copy Filename", systemImage: "doc.on.doc")
                                    }
                                    Button { copyPath(script) } label: {
                                        Label("Copy Path", systemImage: "folder")
                                    }
                                    if !isPickerMode {
                                        ShareLink(item: script) {
                                            Label("Export", systemImage: "square.and.arrow.up")
                                        }
                                        Button { saveDefaultScript(script) } label: {
                                            Label("Set Default", systemImage: "star")
                                        }
                                    }
                                    if canDelete {
                                        Divider()
                                        Button(role: .destructive) {
                                            pendingDelete = script
                                            showDeleteConfirmation = true
                                        } label: { Label("Delete", systemImage: "trash") }
                                    }
                                }
                        }
                    }
                }
            }
            .listStyle(.insetGrouped)
            .searchable(
                text: $searchText,
                placement: .navigationBarDrawer(displayMode: .always),
                prompt: "Search scripts…"
            )
            .navigationTitle(isPickerMode ? "Choose Script" : "Scripts")
            .toolbar {
                ToolbarItemGroup(placement: .navigationBarTrailing) {
                    if !isPickerMode {
                        Button { showNewFileAlert = true } label: {
                            Label("New", systemImage: "doc.badge.plus")
                        }
                        Button { showImporter = true } label: {
                            Label("Import", systemImage: "tray.and.arrow.down")
                        }
                    }
                }
            }
            .onAppear(perform: loadScripts)
            .alert("New Script", isPresented: $showNewFileAlert) {
                TextField("Filename", text: $newFileName)
                Button("Create", action: createNewScript)
                Button("Cancel", role: .cancel) { }
            }
            .alert("Delete Script?", isPresented: $showDeleteConfirmation, presenting: pendingDelete) { script in
                Button("Delete", role: .destructive) { deleteScript(script) }
                Button("Cancel", role: .cancel) { pendingDelete = nil }
            } message: { script in
                Text("Delete \(script.lastPathComponent)? This cannot be undone.")
            }
            .fileImporter(
                isPresented: $showImporter,
                allowedContentTypes: [UTType(filenameExtension: "js") ?? .plainText]
            ) { result in
                switch result {
                case .success(let fileURL): importScript(from: fileURL)
                case .failure(let error): presentError(title: "Import Failed", message: error.localizedDescription)
                }
            }
        }
                .overlay {
            if isBusy {
                Color.black.opacity(0.35).ignoresSafeArea()
                ProgressView("Working…")
                    .padding(16)
                    .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 16, style: .continuous))
            }
            if justCopied {
                VStack {
                    Spacer()
                    Text("Copied")
                        .font(.footnote.weight(.semibold))
                        .padding(.horizontal, 14).padding(.vertical, 10)
                        .background(.ultraThinMaterial, in: Capsule())
                        .transition(.move(edge: .bottom).combined(with: .opacity))
                        .padding(.bottom, 30)
                }
                .animation(.easeInOut(duration: 0.25), value: justCopied)
            }
        }
        .alert(alertTitle, isPresented: $alertVisible) {
            Button("OK", role: .cancel) { }
        } message: {
            Text(alertMessage)
        }
    }

    // MARK: - Row

    @ViewBuilder
    private func scriptRow(_ script: URL) -> some View {
        let isDefault = defaultScriptName == script.lastPathComponent
        if isPickerMode {
            Button {
                onSelectScript?(script)
            } label: {
                HStack {
                    Label(script.lastPathComponent, systemImage: "doc.text.fill")
                    Spacer()
                    if isDefault {
                        Image(systemName: "star.fill").foregroundStyle(.yellow).imageScale(.small)
                    }
                }
            }
        } else {
            NavigationLink {
                ScriptEditorView(scriptURL: script)
            } label: {
                HStack {
                    Label(script.lastPathComponent, systemImage: "doc.text.fill")
                    Spacer()
                    if isDefault {
                        Image(systemName: "star.fill").foregroundStyle(.yellow).imageScale(.small)
                    }
                }
            }
        }
    }

    // MARK: - File Ops

    private func scriptsDirectory() throws -> URL {
        try ScriptStore.prepareDirectory()
    }

    private func loadScripts() {
        do {
            let directory = try scriptsDirectory()
            scripts = try FileManager.default.contentsOfDirectory(at: directory, includingPropertiesForKeys: nil)
                .filter { $0.pathExtension.lowercased() == "js" }
                .sorted { $0.lastPathComponent.localizedCaseInsensitiveCompare($1.lastPathComponent) == .orderedAscending }
        } catch {
            scripts = []
            presentError(title: "Unable to Load Scripts", message: error.localizedDescription)
        }
    }

    private func saveDefaultScript(_ url: URL) {
        defaultScriptName = url.lastPathComponent
        presentSuccess(title: "Default Script Set", message: url.lastPathComponent)
    }

    private func createNewScript() {
        guard !newFileName.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty else { return }
        var filename = newFileName.trimmingCharacters(in: .whitespacesAndNewlines)
        if !filename.hasSuffix(".js") { filename += ".js" }
        guard let filename = ScriptStore.normalizedScriptFileName(filename) else {
            presentError(title: "Failed to Create New Script", message: "Use a simple .js filename without folders.")
            return
        }
        do {
            let newURL = try ScriptStore.scriptURL(named: filename)
            guard !FileManager.default.fileExists(atPath: newURL.path) else {
                presentError(title: "Failed to Create New Script", message: "A script with the same name already exists.")
                return
            }
            try "".write(to: newURL, atomically: true, encoding: .utf8)
            newFileName = ""
            loadScripts()
            presentSuccess(title: "Created", message: filename)
        } catch {
            presentError(title: "Error Creating File", message: error.localizedDescription)
        }
    }

    private func deleteScript(_ url: URL) {
        do {
            try FileManager.default.removeItem(at: url)
            if url.lastPathComponent == defaultScriptName {
                UserDefaults.standard.removeObject(forKey: UserDefaults.Keys.defaultScriptName)
            }
            loadScripts()
        } catch {
            presentError(title: "Delete Failed", message: error.localizedDescription)
        }
    }

    private func importScript(from fileURL: URL) {
        isBusy = true
        DispatchQueue.global(qos: .userInitiated).async {
            defer { DispatchQueue.main.async { self.isBusy = false } }
            let accessing = fileURL.startAccessingSecurityScopedResource()
            defer {
                if accessing {
                    fileURL.stopAccessingSecurityScopedResource()
                }
            }
            do {
                let destination = try ScriptStore.importScript(from: fileURL)
                DispatchQueue.main.async {
                    self.loadScripts()
                    self.presentSuccess(title: "Imported", message: destination.lastPathComponent)
                }
            } catch {
                DispatchQueue.main.async {
                    self.presentError(title: "Import Failed", message: error.localizedDescription)
                }
            }
        }
    }

    // MARK: - Feedback

    private func presentError(title: String, message: String) {
        alertTitle = title; alertMessage = message
        alertIsSuccess = false; alertVisible = true
    }

    private func presentSuccess(title: String, message: String) {
        alertTitle = title; alertMessage = message
        alertIsSuccess = true; alertVisible = true
    }

    private func copyName(_ url: URL) {
        UIPasteboard.general.string = url.lastPathComponent
        showCopiedToast()
    }

    private func copyPath(_ url: URL) {
        UIPasteboard.general.string = url.path
        showCopiedToast()
    }

    private func showCopiedToast() {
        withAnimation { justCopied = true }
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
            withAnimation { justCopied = false }
        }
    }
}
