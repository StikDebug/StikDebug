// swift-tools-version: 5.9
import PackageDescription

// Validate Cryptex payloads without linking the device-only idevice library.
let package = Package(
    name: "StikDebugDDI",
    platforms: [.macOS(.v14)],
    targets: [
        .target(name: "DDIKit", path: "StikDebug/Services/DeveloperDiskImages"),
        .testTarget(name: "DDIKitTests", dependencies: ["DDIKit"], path: "Tests/DDI")
    ]
)
