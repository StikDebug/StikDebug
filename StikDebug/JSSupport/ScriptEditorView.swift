//
//  ScriptEditorView.swift
//  StikDebug
//
//  Created by s s on 2025/7/4.
//

import SwiftUI
import CodeEditorView
import LanguageSupport

struct ScriptEditorView: View {
    let scriptURL: URL

    @State private var scriptContent: String = ""
    @State private var hasLoadedScript = false
    @State private var position: CodeEditor.Position = .init()
    @State private var messages: Set<TextLocated<Message>> = []

    @Environment(\.colorScheme) private var colorScheme
    @Environment(\.dismiss) private var dismiss
    private var editorTheme: Theme {
        colorScheme == .dark ? Theme.defaultDark : Theme.defaultLight
    }

    var body: some View {
        ZStack {
            Color(UIColor.systemBackground)
                .ignoresSafeArea()

            VStack(spacing: 0) {
                CodeEditor(
                    text:     $scriptContent,
                    position: $position,
                    messages: $messages,
                    language: .none
                )
                .font(.system(.footnote, design: .monospaced))
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .environment(\.codeEditorTheme, editorTheme)
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
        .navigationTitle(scriptURL.lastPathComponent)
        .navigationBarTitleDisplayMode(.inline)
        .onAppear(perform: loadScript)
        .toolbar {
            ToolbarItem(placement: .confirmationAction) {
                Button("Save") {
                    saveScript()
                    dismiss()
                }
            }
        }
                .tint(colorScheme == .dark ? .white : .black)
        .toolbar(.hidden, for: .tabBar)
    }

    private func loadScript() {
        // A display transition may make the editor appear again. Preserve drafts.
        guard !hasLoadedScript else { return }
        hasLoadedScript = true
        scriptContent = (try? String(contentsOf: scriptURL)) ?? ""
    }

    private func saveScript() {
        try? scriptContent.write(to: scriptURL, atomically: true, encoding: .utf8)
    }
}
