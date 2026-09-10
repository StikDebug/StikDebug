//
//  ToolsView.swift
//  StikDebug
//
//  Created by Stephen on 2/23/26.
//

import SwiftUI

struct ToolsView: View {
    @State private var selectedTool: AppFeature?

    var body: some View {
        // Let the system expand/collapse the same navigation hierarchy as the
        // window resizes, preserving the selected tool and its local state.
        NavigationSplitView {
            List(AppFeature.toolList, selection: $selectedTool) { tool in
                NavigationLink(value: tool) {
                    Label {
                        VStack(alignment: .leading, spacing: 2) {
                            Text(tool.toolTitle)
                            Text(tool.detail)
                                .font(.caption)
                                .foregroundStyle(.secondary)
                        }
                    } icon: {
                        Image(systemName: tool.systemImage)
                    }
                }
                .accessibilityIdentifier("tool-\(tool.id)")
            }
            .navigationTitle("Tools")
        } detail: {
            if let selectedTool {
                selectedTool.destination
            } else {
                ContentUnavailableView(
                    "Select a Tool",
                    systemImage: "wrench.and.screwdriver",
                    description: Text("Choose a tool to get started.")
                )
            }
        }
        .navigationSplitViewStyle(.balanced)
    }
}
