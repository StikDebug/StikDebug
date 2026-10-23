//
//  PairingFileSanitizer.swift
//  StikDebug
//

import Foundation

/// Trims pairing files back to the property list they contain.
///
/// Pairing files handed to the app sometimes carry extra bytes around the
/// property list itself: a byte order mark, NUL padding, or the tail of a
/// longer file that was overwritten in place. The parser inside `idevice`
/// rejects those with an opaque XML error such as
/// `UnexpectedXmlCharactersExpectedElement`, so the payload is isolated and
/// validated here before anything else gets to read it.
enum PairingFileSanitizer {
    /// `NSError` code used whenever a pairing file cannot be read. `TunnelManager`
    /// turns it into a prompt to import a new one instead of surfacing the
    /// parser error.
    static let errorCode = -9

    enum SanitizeError: Error, Equatable, LocalizedError {
        case empty
        case notAPropertyList
        case unexpectedStructure

        var errorDescription: String? {
            switch self {
            case .empty:
                return "The pairing file is empty."
            case .notAPropertyList:
                return "The pairing file is damaged and could not be read. Export it again and import the new copy."
            case .unexpectedStructure:
                return "The file does not contain pairing information. Select the pairing file exported for this device."
            }
        }
    }

    private static let xmlDeclaration = Data("<?xml".utf8)
    private static let plistOpenTag = Data("<plist".utf8)
    private static let plistCloseTag = Data("</plist>".utf8)
    private static let binaryMagic = Data("bplist00".utf8)
    private static let byteOrderMark = Data([0xEF, 0xBB, 0xBF])
    private static let whitespaceBytes: Set<UInt8> = [0x09, 0x0A, 0x0D, 0x20]
    private static let paddingBytes: Set<UInt8> = whitespaceBytes.union([0x00])

    /// Returns the property list contained in `raw`, byte for byte, with any
    /// surrounding junk removed. Throws when the contents cannot be read as a
    /// pairing file.
    static func sanitized(_ raw: Data) throws -> Data {
        let trimmed = try payload(in: raw)
        _ = try pairingRecord(in: trimmed)
        return trimmed
    }

    /// Re-encodes the pairing record as a canonical XML property list.
    ///
    /// Foundation tolerates a handful of malformed documents that the parser in
    /// `idevice` turns down, so a record it still refuses is written out again
    /// from the values Foundation managed to read.
    static func canonicalized(_ raw: Data) throws -> Data {
        let record = try pairingRecord(in: payload(in: raw))
        return try PropertyListSerialization.data(
            fromPropertyList: record,
            format: .xml,
            options: 0
        )
    }

    /// Rewrites the file at `url` when it carries junk around the property
    /// list. Returns `false` when the contents cannot be salvaged, leaving the
    /// file untouched so the user can still export it for inspection.
    @discardableResult
    static func repairIfNeeded(at url: URL, fileManager: FileManager = .default) -> Bool {
        guard fileManager.fileExists(atPath: url.path),
              let raw = try? Data(contentsOf: url) else {
            return false
        }

        guard let trimmed = try? sanitized(raw) else {
            return false
        }

        guard trimmed != raw else {
            return true
        }

        do {
            try trimmed.write(to: url, options: .atomic)
            LogManager.shared.addWarningLog(
                "Repaired pairing file at \(url.lastPathComponent): removed \(raw.count - trimmed.count) stray byte(s)"
            )
        } catch {
            return false
        }

        return true
    }

    /// Reads the pairing record out of an already isolated property list.
    private static func pairingRecord(in trimmed: Data) throws -> [String: Any] {
        let object: Any
        do {
            object = try PropertyListSerialization.propertyList(from: trimmed, options: [], format: nil)
        } catch {
            throw SanitizeError.notAPropertyList
        }

        guard let dictionary = object as? [String: Any], !dictionary.isEmpty else {
            throw SanitizeError.unexpectedStructure
        }

        return dictionary
    }

    /// Isolates the property list inside `raw` without parsing it.
    private static func payload(in raw: Data) throws -> Data {
        var stripped = Data(trimmingLeadingPadding(raw))
        if stripped.starts(with: byteOrderMark) {
            stripped = Data(trimmingLeadingPadding(stripped.dropFirst(byteOrderMark.count)))
        }

        guard !stripped.isEmpty else {
            throw SanitizeError.empty
        }

        // Binary property lists keep their trailer in the final bytes, so they
        // are only ever trimmed from the front.
        if stripped.starts(with: binaryMagic) {
            return stripped
        }

        if let xmlStart = stripped.range(of: xmlDeclaration) ?? stripped.range(of: plistOpenTag) {
            // Stop at the first closing tag: anything past it is either padding
            // or a second property list appended to the first. Plain whitespace
            // is legal there and every exporter writes a trailing newline, so
            // it is kept to leave untouched files byte identical.
            guard let xmlEnd = stripped.range(
                of: plistCloseTag,
                options: [],
                in: xmlStart.lowerBound..<stripped.endIndex
            ) else {
                throw SanitizeError.notAPropertyList
            }

            let remainder = stripped[xmlEnd.upperBound...]
            let end = remainder.allSatisfy { whitespaceBytes.contains($0) }
                ? stripped.endIndex
                : xmlEnd.upperBound
            return Data(stripped[xmlStart.lowerBound..<end])
        }

        if let binaryStart = stripped.range(of: binaryMagic) {
            return Data(stripped[binaryStart.lowerBound...])
        }

        throw SanitizeError.notAPropertyList
    }

    private static func trimmingLeadingPadding(_ raw: Data) -> Data {
        var slice = raw
        while let first = slice.first, paddingBytes.contains(first) {
            slice = slice.dropFirst()
        }
        return slice
    }
}
