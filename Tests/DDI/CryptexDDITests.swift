import CryptoKit
import Foundation
import Testing
@testable import DDIKit

struct CryptexDDITests {
    private func fixture(_ body: (URL) throws -> Void) throws {
        let directory = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }
        var entries: [String: Any] = [:]
        for (key, name) in CryptexDDI.payloads {
            let data = Data(name.utf8)
            try data.write(to: directory.appendingPathComponent(name))
            entries[key] = ["Digest": Data(SHA384.hash(data: data)), "Info": ["Path": name]]
        }
        let manifest: [String: Any] = ["BuildIdentities": [[
            "Info": ["Variant": "iOS Developer Disk Image Cryptex"], "Manifest": entries
        ]]]
        try PropertyListSerialization.data(fromPropertyList: manifest, format: .xml, options: 0)
            .write(to: directory.appendingPathComponent("BuildManifest.plist"))
        try body(directory)
    }

    @Test func genericIdentityDoesNotRequireLegacyHardwareIDs() throws {
        try fixture { try CryptexDDI.validate(in: $0) }
    }

    @Test(arguments: Array(CryptexDDI.payloads.values))
    func everyPayloadMustMatchItsSignedDigest(_ name: String) throws {
        try fixture { directory in
            try Data("corrupt".utf8).write(to: directory.appendingPathComponent(name))
            #expect(throws: (any Error).self) { try CryptexDDI.validate(in: directory) }
        }
    }

    @Test func missingRootHashIsRejected() throws {
        try fixture { directory in
            try FileManager.default.removeItem(at: directory.appendingPathComponent("Image.dmg.root_hash"))
            #expect(throws: (any Error).self) { try CryptexDDI.validate(in: directory) }
        }
    }

    @Test func nativeLoaderCannotFollowUnverifiedManifestPaths() throws {
        try fixture { directory in
            let url = directory.appendingPathComponent("BuildManifest.plist")
            let xml = try String(contentsOf: url, encoding: .utf8)
                .replacingOccurrences(of: "<string>Image.dmg</string>", with: "<string>../Image.dmg</string>")
            try Data(xml.utf8).write(to: url)
            #expect(throws: (any Error).self) { try CryptexDDI.validate(in: directory) }
        }
    }
}
