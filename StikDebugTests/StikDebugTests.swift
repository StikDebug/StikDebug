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

    @Test func processNameResolvesExactExecutableName() throws {
        let entries = [
            ["pid": 101, "path": "file:///Applications/Example.app/Example", "bundleID": "com.example.app"] as NSDictionary,
            ["pid": 102, "path": "file:///Applications/Example.app/ExampleHelper"] as NSDictionary,
            ["pid": 103, "path": "file:///Applications/Other.app/My%20Process"] as NSDictionary
        ].compactMap(ProcessInfoEntry.init(dictionary:))

        #expect(try ProcessInfoEntry.pid(named: "Example", in: entries) == 101)
        #expect(try ProcessInfoEntry.pid(named: "My Process", in: entries) == 103)
        #expect(throws: NSError.self) { try ProcessInfoEntry.pid(named: "example", in: entries) }
        #expect(throws: NSError.self) { try ProcessInfoEntry.pid(named: "Exam", in: entries) }
        #expect(throws: NSError.self) { try ProcessInfoEntry.pid(named: "com.example.app", in: entries) }
    }

    @Test func processNameRejectsAmbiguousAndMissingMatches() throws {
        let entries = [
            ["pid": 101, "path": "file:///Applications/First.app/Example"] as NSDictionary,
            ["pid": 102, "path": "file:///Applications/Second.app/Example"] as NSDictionary
        ].compactMap(ProcessInfoEntry.init(dictionary:))

        #expect(throws: NSError.self) { try ProcessInfoEntry.pid(named: "Example", in: entries) }
        #expect(throws: NSError.self) { try ProcessInfoEntry.pid(named: "Missing", in: entries) }
        #expect(throws: NSError.self) { try ProcessInfoEntry.pid(named: "", in: entries) }
    }

    @Test func processNameRejectsInvalidPIDs() throws {
        for pid in [0, -1, Int(Int32.max) + 1] {
            let entry = try #require(ProcessInfoEntry(dictionary: ["pid": pid, "path": "/Applications/Example.app/Example"] as NSDictionary))
            #expect(throws: NSError.self) { try ProcessInfoEntry.pid(named: "Example", in: [entry]) }
        }
    }

    @Test func deviceVersionParsesSupportedIdentifiers() async throws {
        #expect(ProcessInfo.processInfo.deviceVersion(from: "iPhone14,2") == 14.2)
        #expect(ProcessInfo.processInfo.deviceVersion(from: "iPad14,5") == 14.5)
        #expect(ProcessInfo.processInfo.deviceVersion(from: "Mac14,2") == nil)
    }

}
