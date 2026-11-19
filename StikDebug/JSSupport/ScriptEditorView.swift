//
//  ScriptEditorView.swift
//  StikDebug
//
//  Created by s s on 2025/7/4.
//

import SwiftUI
import CodeEditorView
import LanguageSupport
import UIKit

struct ScriptEditorView: View {
    let scriptURL: URL

    @State private var scriptContent: String = ""
    @State private var position: CodeEditor.Position = .init()
    @State private var messages: Set<TextLocated<Message>> = []

    @Environment(\.colorScheme) private var colorScheme
    @Environment(\.dismiss) private var dismiss
    private var isReadOnly: Bool { ScriptStore.isBuiltInScript(scriptURL) }

    private var editorTheme: Theme {
        colorScheme == .dark ? Theme.defaultDark : Theme.defaultLight
    }

    var body: some View {
        ZStack {
            Color(UIColor.systemBackground)
                .ignoresSafeArea()

            VStack(spacing: 0) {
                if isReadOnly {
                    ReadOnlyCodeEditor(content: editor)
                } else {
                    editor
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .ignoresSafeArea(edges: .bottom)
        }
        .navigationTitle(scriptURL.lastPathComponent)
        .navigationBarTitleDisplayMode(.inline)
        .onAppear(perform: loadScript)
        .toolbar {
            if !isReadOnly {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        saveScript()
                        dismiss()
                    }
                }
            }
        }
                .tint(colorScheme == .dark ? .white : .black)
        .toolbar(.hidden, for: .tabBar)
    }

    private var editor: some View {
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

    private func loadScript() {
        scriptContent = (try? String(contentsOf: scriptURL)) ?? ""
    }

    private func saveScript() {
        try? scriptContent.write(to: scriptURL, atomically: true, encoding: .utf8)
    }
}

private struct ReadOnlyCodeEditor<Content: View>: UIViewControllerRepresentable {
    let content: Content

    func makeUIViewController(context: Context) -> ReadOnlyEditorHost<Content> {
        ReadOnlyEditorHost(rootView: content)
    }

    func updateUIViewController(_ controller: ReadOnlyEditorHost<Content>, context: Context) {
        controller.rootView = content
    }
}

private final class ReadOnlyEditorHost<Content: View>: UIHostingController<Content> {
    override func viewDidLayoutSubviews() {
        super.viewDidLayoutSubviews()
        lockEditing(in: view)
    }

    private func lockEditing(in view: UIView) {
        if let textView = view as? UITextView {
            // CodeEditorView does not expose an editability setting.
            textView.isEditable = false
            return
        }
        for subview in view.subviews {
            lockEditing(in: subview)
        }
    }
}
