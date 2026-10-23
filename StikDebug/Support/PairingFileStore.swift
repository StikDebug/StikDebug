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
            let documentIsUsable = PairingFileSanitizer.repairIfNeeded(at: documentURL, fileManager: fileManager)
            let destinationExists = fileManager.fileExists(atPath: destination.path)

            // A damaged copy in Documents must never replace a working one.
            if (documentIsUsable || !destinationExists),
               !destinationExists ||
                !fileManager.contentsEqual(atPath: documentURL.path, andPath: destination.path) {
                do {
                    try replaceItem(at: destination, with: documentURL, fileManager: fileManager)
                } catch {
                    return destination
                }
            }
            protectPairingFile(at: destination, fileManager: fileManager)
            removeLegacyCopies(fileManager: fileManager)
            return destination
        }

        if !fileManager.fileExists(atPath: destination.path) {
            migrateLegacyCopy(fileManager: fileManager)
        } else {
            if PairingFileSanitizer.repairIfNeeded(at: destination, fileManager: fileManager) {
                protectPairingFile(at: destination, fileManager: fileManager)
            }
            removeLegacyCopies(fileManager: fileManager)
        }
        return destination
    }

    static func replace(with sourceURL: URL, fileManager: FileManager = .default) throws {
        // Store the property list itself rather than the picked file verbatim:
        // stray bytes around it make the parser in idevice fail later on with
        // an error nobody can act on.
        try store(PairingFileSanitizer.sanitized(Data(contentsOf: sourceURL)), fileManager: fileManager)
    }

    /// Writes `data` over both copies of the pairing file.
    static func store(_ data: Data, fileManager: FileManager = .default) throws {
        try fileManager.createDirectory(at: directoryURL, withIntermediateDirectories: true)
        try write(data, to: documentsURL, fileManager: fileManager)
        try write(data, to: url, fileManager: fileManager)
        protectPairingFile(at: url, fileManager: fileManager)
        removeLegacyCopies(fileManager: fileManager)
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
        if fileManager.fileExists(atPath: documentsURL.path) {
            try fileManager.removeItem(at: documentsURL)
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
            URL.documentsDirectory.appendingPathComponent(legacyFileName),
            directoryURL.appendingPathComponent(legacyFileName)
        ]
    }

    private static var documentsURL: URL {
        URL.documentsDirectory.appendingPathComponent(fileName)
    }

    private static func documentSourceURL(fileManager: FileManager) -> URL? {
        fileManager.fileExists(atPath: documentsURL.path) ? documentsURL : nil
    }

    private static func migrateLegacyCopy(fileManager: FileManager) {
        for legacyURL in legacyURLs where fileManager.fileExists(atPath: legacyURL.path) {
            try? replace(with: legacyURL, fileManager: fileManager)
            break
        }
    }

    private static func write(_ data: Data, to destination: URL, fileManager: FileManager) throws {
        try fileManager.createDirectory(
            at: destination.deletingLastPathComponent(),
            withIntermediateDirectories: true
        )
        try data.write(to: destination, options: .atomic)
    }

    private static func replaceItem(at destination: URL, with source: URL, fileManager: FileManager) throws {
        let temporary = destination
            .deletingLastPathComponent()
            .appendingPathComponent(UUID().uuidString + ".tmp")
        try? fileManager.removeItem(at: temporary)
        defer { try? fileManager.removeItem(at: temporary) }
        try fileManager.copyItem(at: source, to: temporary)

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
