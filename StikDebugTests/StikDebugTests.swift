//
//  StikDebugTests.swift
//  StikDebugTests
//
//  Created by Stephen on 3/26/25.
//

import Foundation
import Testing
@testable import StikDebug

struct StikDebugTests {

    @Test func pairingImportSurvivesNewerLegacyFile() throws {
        let fileManager = try PairingFileManager()
        defer { try? fileManager.removeItem(at: fileManager.root) }
        let canonical = fileManager.documents.appendingPathComponent(PairingFileStore.fileName)
        let legacy = fileManager.documents.appendingPathComponent("rp_pairing_file.plist")
        let source = fileManager.root.appendingPathComponent("import.plist")
        let imported = Data("imported pairing".utf8)
        try Data("stale legacy pairing".utf8).write(to: legacy)
        try fileManager.setAttributes([.modificationDate: Date(timeIntervalSince1970: 200)], ofItemAtPath: legacy.path)
        try imported.write(to: source)
        try fileManager.setAttributes([.modificationDate: Date(timeIntervalSince1970: 100)], ofItemAtPath: source.path)

        try PairingFileStore.importFromPicker(source, fileManager: fileManager)
        for _ in 0..<2 {
            let active = PairingFileStore.prepareURL(fileManager: fileManager)
            #expect(try Data(contentsOf: active) == imported)
            #expect(try fileManager.attributesOfItem(atPath: active.path)[.posixPermissions] as? Int == 0o600)
        }
        try PairingFileStore.importFromPicker(canonical, fileManager: fileManager)
        #expect(throws: (any Error).self) {
            try PairingFileStore.importFromPicker(fileManager.root.appendingPathComponent("missing.plist"), fileManager: fileManager)
        }
        #expect(throws: (any Error).self) {
            try PairingFileStore.replace(with: fileManager.root.appendingPathComponent("missing.plist"), fileManager: fileManager)
        }
        #expect(try Data(contentsOf: canonical) == imported)
        let active = fileManager.support.appendingPathComponent("Pairing").appendingPathComponent(PairingFileStore.fileName)
        #expect(try Data(contentsOf: active) == imported)
        for directory in [fileManager.documents, active.deletingLastPathComponent()] {
            #expect(try fileManager.contentsOfDirectory(atPath: directory.path).allSatisfy { !$0.hasSuffix(".tmp") })
        }
        try PairingFileStore.remove(fileManager: fileManager)
        #expect(!fileManager.fileExists(atPath: active.path))
        #expect(!fileManager.fileExists(atPath: canonical.path))
        #expect(!fileManager.fileExists(atPath: legacy.path))
    }

    @Test(arguments: [100.0, 200.0, 300.0])
    func canonicalPairingTakesPrecedence(modificationTime: TimeInterval) throws {
        let fileManager = try PairingFileManager()
        defer { try? fileManager.removeItem(at: fileManager.root) }
        let canonical = fileManager.documents.appendingPathComponent(PairingFileStore.fileName)
        let legacy = fileManager.documents.appendingPathComponent("rp_pairing_file.plist")
        let active = PairingFileStore.prepareURL(fileManager: fileManager)
        try Data("stale active pairing".utf8).write(to: active)
        try Data("legacy pairing".utf8).write(to: legacy)
        try fileManager.setAttributes([.modificationDate: Date(timeIntervalSince1970: 200)], ofItemAtPath: legacy.path)
        let canonicalData = Data("canonical pairing".utf8)
        try canonicalData.write(to: canonical)
        try fileManager.setAttributes([.modificationDate: Date(timeIntervalSince1970: modificationTime)], ofItemAtPath: canonical.path)
        #expect(try Data(contentsOf: PairingFileStore.prepareURL(fileManager: fileManager)) == canonicalData)

        let edited = Data("edited through Files".utf8)
        try edited.write(to: canonical)
        try fileManager.setAttributes([.modificationDate: Date(timeIntervalSince1970: 50)], ofItemAtPath: canonical.path)
        #expect(try Data(contentsOf: PairingFileStore.prepareURL(fileManager: fileManager)) == edited)
    }

    @Test(arguments: ["documents", "support", "none"])
    func pairingLegacyMigration(source: String) throws {
        let fileManager = try PairingFileManager()
        defer { try? fileManager.removeItem(at: fileManager.root) }
        let pairingDirectory = fileManager.support.appendingPathComponent("Pairing")
        let expected = pairingDirectory.appendingPathComponent(PairingFileStore.fileName)
        let data = Data("legacy pairing".utf8)
        if source != "none" {
            let directory = source == "documents" ? fileManager.documents : pairingDirectory
            try fileManager.createDirectory(at: directory, withIntermediateDirectories: true)
            try data.write(to: directory.appendingPathComponent("rp_pairing_file.plist"))
        }
        let active = PairingFileStore.prepareURL(fileManager: fileManager)
        #expect(active == expected)
        if source == "none" {
            #expect(!fileManager.fileExists(atPath: active.path))
        } else {
            #expect(try Data(contentsOf: active) == data)
            #expect(try fileManager.attributesOfItem(atPath: active.path)[.posixPermissions] as? Int == 0o600)
        }
    }

    @Test func txmDetectionUsesClassicTXMBeforeIOS266() async throws {
        #expect(
            ProcessInfo.hasTXMSupport(
                isIOS266OrNewer: false,
                hasTXMClassic: false,
                hardwareIdentifier: "iPhone15,2"
            ) == false
        )
        #expect(
            ProcessInfo.hasTXMSupport(
                isIOS266OrNewer: false,
                hasTXMClassic: true,
                hardwareIdentifier: "iPhone1,1"
            ) == true
        )
    }

    @Test func txmDetectionUsesClassicTXMWhenAvailableOnIOS266() async throws {
        #expect(
            ProcessInfo.hasTXMSupport(
                isIOS266OrNewer: true,
                hasTXMClassic: true,
                hardwareIdentifier: "iPhone1,1"
            ) == true
        )
    }

    @Test func txmDetectionFallsBackToIPhoneThresholdOnIOS266() async throws {
        #expect(
            ProcessInfo.hasTXMSupport(
                isIOS266OrNewer: true,
                hasTXMClassic: false,
                hardwareIdentifier: "iPhone14,1"
            ) == false
        )
        #expect(
            ProcessInfo.hasTXMSupport(
                isIOS266OrNewer: true,
                hasTXMClassic: false,
                hardwareIdentifier: "iPhone14,2"
            ) == true
        )
    }

    @Test func txmDetectionFallsBackToIPadThresholdOnIOS266() async throws {
        #expect(
            ProcessInfo.hasTXMSupport(
                isIOS266OrNewer: true,
                hasTXMClassic: false,
                hardwareIdentifier: "iPad14,4"
            ) == false
        )
        #expect(
            ProcessInfo.hasTXMSupport(
                isIOS266OrNewer: true,
                hasTXMClassic: false,
                hardwareIdentifier: "iPad14,5"
            ) == true
        )
    }

    @Test func deviceVersionParsesSupportedIdentifiers() async throws {
        #expect(ProcessInfo.processInfo.deviceVersion(from: "iPhone14,2") == 14.2)
        #expect(ProcessInfo.processInfo.deviceVersion(from: "iPad14,5") == 14.5)
        #expect(ProcessInfo.processInfo.deviceVersion(from: "Mac14,2") == nil)
    }

}

private final class PairingFileManager: FileManager, @unchecked Sendable {
    let root: URL
    var documents: URL { root.appendingPathComponent("Documents") }
    var support: URL { root.appendingPathComponent("Application Support") }

    init(root: URL = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)) throws {
        self.root = root
        super.init()
        try createDirectory(at: documents, withIntermediateDirectories: true)
        try createDirectory(at: support, withIntermediateDirectories: true)
    }

    override func urls(for directory: FileManager.SearchPathDirectory, in domainMask: FileManager.SearchPathDomainMask) -> [URL] {
        switch directory {
        case .documentDirectory: return [documents]
        case .applicationSupportDirectory: return [support]
        default: return super.urls(for: directory, in: domainMask)
        }
    }
}
