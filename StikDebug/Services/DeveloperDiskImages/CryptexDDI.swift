import CryptoKit
import Foundation

/// Cryptex DDIs use a generic identity and four signed payloads, rather than an
/// ApChipID/ApBoardID identity. Never mix these with the personalized image files.
enum CryptexDDI {
    static let payloads = [
        "Cryptex1,GenericDmg": "Image.dmg",
        "Cryptex1,GenericTrustCache": "Image.dmg.trustcache",
        "Cryptex1,CryptexInfoPlist": "Image.dmg.cryptex_info",
        "Cryptex1,GenericVolume": "Image.dmg.root_hash"
    ]

    static func validate(in directory: URL) throws {
        let data = try Data(contentsOf: directory.appendingPathComponent("BuildManifest.plist"))
        guard let manifest = try PropertyListSerialization.propertyList(from: data, format: nil) as? [String: Any],
              let identities = manifest["BuildIdentities"] as? [[String: Any]],
              let identity = identities.first(where: {
                  (($0["Info"] as? [String: Any])?["Variant"] as? String)?
                      .hasSuffix("Developer Disk Image Cryptex") == true
              }),
              let entries = identity["Manifest"] as? [String: [String: Any]] else {
            throw ValidationError.invalidManifest
        }
        // The native loader follows manifest paths. Require the normalized names
        // we verified so it cannot read an unverified file or follow a symlink out.
        let root = directory.resolvingSymlinksInPath().standardizedFileURL.path + "/"
        for (key, name) in payloads {
            guard let entry = entries[key],
                  (entry["Info"] as? [String: Any])?["Path"] as? String == name,
                  let digest = entry["Digest"] as? Data else {
                throw ValidationError.invalidManifest
            }
            let url = directory.appendingPathComponent(name).resolvingSymlinksInPath().standardizedFileURL
            guard url.path.hasPrefix(root) else { throw ValidationError.invalidManifest }
            let bytes = try Data(contentsOf: url, options: .mappedIfSafe)
            guard !bytes.isEmpty, Data(SHA384.hash(data: bytes)) == digest else {
                throw ValidationError.invalidPayload(name)
            }
        }
    }

    enum ValidationError: LocalizedError {
        case invalidManifest
        case invalidPayload(String)

        var errorDescription: String? {
            switch self {
            case .invalidManifest: return "The bundled Cryptex DDI manifest is invalid. Reinstall a complete build of this fork."
            case .invalidPayload(let name): return "The bundled Cryptex DDI file \(name) failed verification. Reinstall a complete build of this fork."
            }
        }
    }
}
