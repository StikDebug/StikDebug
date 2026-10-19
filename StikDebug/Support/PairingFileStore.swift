//
//  PairingFileStore.swift
//  StikDebug
//

import Foundation
import UniformTypeIdentifiers

enum PairingFileStore {
    static let fileName = "pairingFile.plist"
    static let supportedContentTypes: [UTType] = [
        UTType(filenameExtension: "mobiledevicepairing", conformingTo: .data)!,
        UTType(filenameExtension: "mobiledevicepair", conformingTo: .data)!,
        .propertyList
    ]

    private static let legacyFileName = "rp_pairing_file.plist"

    static var url: URL {
        directoryURL.appendingPathComponent(fileName)
    }

    @discardableResult
    static func prepareURL(fileManager: FileManager = .default) -> URL {
        let destination = url
        try? fileManager.createDirectory(at: directoryURL, withIntermediateDirectories: true)

        if let documentURL = documentSourceURL(fileManager: fileManager) {
            if !fileManager.fileExists(atPath: destination.path) ||
                !fileManager.contentsEqual(atPath: documentURL.path, andPath: destination.path) {
                try? replaceItem(at: destination, with: documentURL, fileManager: fileManager)
                protectPairingFile(at: destination, fileManager: fileManager)
            }
            return destination
        }

        if !fileManager.fileExists(atPath: destination.path) {
            migrateLegacyCopy(to: destination, fileManager: fileManager)
        }
        return destination
    }

    static func replace(with sourceURL: URL, fileManager: FileManager = .default) throws {
        try fileManager.createDirectory(at: directoryURL, withIntermediateDirectories: true)

        let documentsDestination = documentsURL
        if sourceURL.standardizedFileURL != documentsDestination.standardizedFileURL {
            try replaceItem(at: documentsDestination, with: sourceURL, fileManager: fileManager)
        }

        try replaceItem(at: url, with: documentsDestination, fileManager: fileManager)
        protectPairingFile(at: url, fileManager: fileManager)
    }

    static func importFromPicker(_ sourceURL: URL, fileManager: FileManager = .default) throws {
        let accessing = sourceURL.startAccessingSecurityScopedResource()
        defer {
            if accessing {
                sourceURL.stopAccessingSecurityScopedResource()
            }
        }

        guard fileManager.fileExists(atPath: sourceURL.path) else {
            throw CocoaError(.fileNoSuchFile)
        }

        try replace(with: sourceURL, fileManager: fileManager)
    }

    static func remove(fileManager: FileManager = .default) throws {
        let destination = prepareURL(fileManager: fileManager)
        if fileManager.fileExists(atPath: destination.path) {
            try fileManager.removeItem(at: destination)
        }
        removeLegacyCopies(fileManager: fileManager)
    }

    private static var directoryURL: URL {
        FileManager.default
            .urls(for: .applicationSupportDirectory, in: .userDomainMask)[0]
            .appendingPathComponent("Pairing", isDirectory: true)
    }

    private static var legacyURLs: [URL] {
        [
            directoryURL.appendingPathComponent(legacyFileName),
            documentsURL,
            URL.documentsDirectory.appendingPathComponent(legacyFileName)
        ]
    }

    private static var documentsURL: URL {
        URL.documentsDirectory.appendingPathComponent(fileName)
    }

    private static func documentSourceURL(fileManager: FileManager) -> URL? {
        [documentsURL, URL.documentsDirectory.appendingPathComponent(legacyFileName)]
            .filter { fileManager.fileExists(atPath: $0.path) }
            .max {
                let firstDate = (try? $0.resourceValues(forKeys: [.contentModificationDateKey]))?.contentModificationDate ?? .distantPast
                let secondDate = (try? $1.resourceValues(forKeys: [.contentModificationDateKey]))?.contentModificationDate ?? .distantPast
                return firstDate < secondDate
            }
    }

    private static func migrateLegacyCopy(to destination: URL, fileManager: FileManager) {
        for legacyURL in legacyURLs where fileManager.fileExists(atPath: legacyURL.path) {
            do {
                try replaceItem(at: destination, with: legacyURL, fileManager: fileManager)
            } catch {
                if let data = try? Data(contentsOf: legacyURL) {
                    try? data.write(to: destination, options: .atomic)
                }
            }

            protectPairingFile(at: destination, fileManager: fileManager)
            break
        }
    }

    private static func replaceItem(at destination: URL, with source: URL, fileManager: FileManager) throws {
        let temporary = destination
            .deletingLastPathComponent()
            .appendingPathComponent(UUID().uuidString + ".tmp")
        try? fileManager.removeItem(at: temporary)
        try fileManager.copyItem(at: source, to: temporary)
        defer { try? fileManager.removeItem(at: temporary) }

        if fileManager.fileExists(atPath: destination.path) {
            _ = try fileManager.replaceItemAt(destination, withItemAt: temporary)
        } else {
            try fileManager.moveItem(at: temporary, to: destination)
        }
    }

    private static func removeLegacyCopies(fileManager: FileManager) {
        for legacyURL in legacyURLs where fileManager.fileExists(atPath: legacyURL.path) {
            try? fileManager.removeItem(at: legacyURL)
        }
    }

    private static func protectPairingFile(at url: URL, fileManager: FileManager) {
        try? fileManager.setAttributes([.posixPermissions: 0o600], ofItemAtPath: url.path)
    }
}
