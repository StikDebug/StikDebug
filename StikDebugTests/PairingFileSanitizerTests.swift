//
//  PairingFileSanitizerTests.swift
//  StikDebugTests
//

import Foundation
import Testing
@testable import StikDebug

struct PairingFileSanitizerTests {

    private static func pairingRecord() -> [String: Any] {
        [
            "HostID": "0C9F3E0E-4C6B-4D1A-9E1C-9E9D1D0E1F20",
            "SystemBUID": "11111111-2222-3333-4444-555555555555",
            "HostCertificate": Data("certificate".utf8),
            "HostPrivateKey": Data("private-key".utf8)
        ]
    }

    /// An XML pairing file ending exactly at `</plist>`, so the fixtures can be
    /// compared byte for byte without the newline exporters append.
    private static func xmlPairingFile() throws -> Data {
        let encoded = try PropertyListSerialization.data(
            fromPropertyList: pairingRecord(),
            format: .xml,
            options: 0
        )
        guard let closingTag = encoded.range(of: Data("</plist>".utf8)) else {
            throw CocoaError(.propertyListWriteInvalid)
        }
        return Data(encoded[..<closingTag.upperBound])
    }

    private static func binaryPairingFile() throws -> Data {
        try PropertyListSerialization.data(
            fromPropertyList: pairingRecord(),
            format: .binary,
            options: 0
        )
    }

    @Test func cleanPairingFileIsLeftAlone() throws {
        let clean = try Self.xmlPairingFile()
        #expect(try PairingFileSanitizer.sanitized(clean) == clean)
    }

    @Test func trailingNewlineIsPreserved() throws {
        let clean = try Self.xmlPairingFile()
        let withNewline = clean + Data("\n".utf8)
        #expect(try PairingFileSanitizer.sanitized(withNewline) == withNewline)
    }

    @Test func trailingJunkIsRemoved() throws {
        let clean = try Self.xmlPairingFile()
        let damaged = clean + Data("\n<!-- leftovers from a longer file -->\n".utf8)
        #expect(try PairingFileSanitizer.sanitized(damaged) == clean)
    }

    @Test func trailingPaddingIsRemoved() throws {
        let clean = try Self.xmlPairingFile()
        let damaged = clean + Data(repeating: 0x00, count: 64)
        #expect(try PairingFileSanitizer.sanitized(damaged) == clean)
    }

    @Test func leadingByteOrderMarkAndWhitespaceAreRemoved() throws {
        let clean = try Self.xmlPairingFile()
        let damaged = Data([0xEF, 0xBB, 0xBF]) + Data("\n  ".utf8) + clean
        #expect(try PairingFileSanitizer.sanitized(damaged) == clean)
    }

    @Test func appendedSecondPropertyListIsDropped() throws {
        let clean = try Self.xmlPairingFile()
        let damaged = clean + clean
        #expect(try PairingFileSanitizer.sanitized(damaged) == clean)
    }

    @Test func binaryPairingFileIsAccepted() throws {
        let binary = try Self.binaryPairingFile()
        #expect(try PairingFileSanitizer.sanitized(binary) == binary)
    }

    @Test func emptyFileIsRejected() throws {
        #expect(throws: PairingFileSanitizer.SanitizeError.empty) {
            try PairingFileSanitizer.sanitized(Data())
        }
    }

    @Test func unrelatedFileIsRejected() throws {
        #expect(throws: PairingFileSanitizer.SanitizeError.notAPropertyList) {
            try PairingFileSanitizer.sanitized(Data("not a pairing file at all".utf8))
        }
    }

    @Test func truncatedPropertyListIsRejected() throws {
        let clean = try Self.xmlPairingFile()
        let truncated = clean.prefix(clean.count / 2) + Data("</plist>".utf8)
        #expect(throws: PairingFileSanitizer.SanitizeError.notAPropertyList) {
            try PairingFileSanitizer.sanitized(Data(truncated))
        }
    }

    @Test func canonicalFormKeepsEveryValue() throws {
        let clean = try Self.xmlPairingFile()
        let canonical = try PairingFileSanitizer.canonicalized(clean)
        let object = try PropertyListSerialization.propertyList(from: canonical, options: [], format: nil)
        let decoded = try #require(object as? [String: Any])

        #expect(decoded.count == Self.pairingRecord().count)
        #expect(decoded["HostID"] as? String == Self.pairingRecord()["HostID"] as? String)
        #expect(decoded["HostPrivateKey"] as? Data == Self.pairingRecord()["HostPrivateKey"] as? Data)
    }

    @Test func canonicalFormStripsSurroundingJunk() throws {
        let clean = try Self.xmlPairingFile()
        let damaged = clean + Data(repeating: 0x00, count: 8)
        let canonical = try PairingFileSanitizer.canonicalized(damaged)

        #expect(try PairingFileSanitizer.sanitized(canonical) == canonical)
    }

    @Test func repairRewritesDamagedFileInPlace() throws {
        let directory = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent(UUID().uuidString, isDirectory: true)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }

        let fileURL = directory.appendingPathComponent("pairingFile.plist")
        let clean = try Self.xmlPairingFile()
        try (clean + Data(repeating: 0x00, count: 16)).write(to: fileURL)

        #expect(PairingFileSanitizer.repairIfNeeded(at: fileURL) == true)
        #expect(try Data(contentsOf: fileURL) == clean)
    }

    @Test func repairLeavesUnsalvageableFileUntouched() throws {
        let directory = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent(UUID().uuidString, isDirectory: true)
        try FileManager.default.createDirectory(at: directory, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: directory) }

        let fileURL = directory.appendingPathComponent("pairingFile.plist")
        let garbage = Data("definitely not a property list".utf8)
        try garbage.write(to: fileURL)

        #expect(PairingFileSanitizer.repairIfNeeded(at: fileURL) == false)
        #expect(try Data(contentsOf: fileURL) == garbage)
    }
}
