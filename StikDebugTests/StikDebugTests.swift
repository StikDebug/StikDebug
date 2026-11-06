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

    @Test func customTargetDiagnosticsDoNotRequireLocalVPN() {
        let result = ConnectionDiagnostics.assessment(isLocal: false, cellular: true, hasVPN: false, loopback: false, reachable: false)
        #expect(result.problem == .serviceUnreachable)
        #expect(result.isLocal == false)
    }

    @Test func reachableServiceOverridesInconclusiveVPNChecks() {
        let result = ConnectionDiagnostics.assessment(isLocal: true, cellular: true, hasVPN: false, loopback: false, reachable: true)
        #expect(result.problem == .unknown)
    }

    @Test func cellularWithoutWiFiTakesPriorityOverUnreachableService() {
        let result = ConnectionDiagnostics.assessment(isLocal: true, cellular: true, hasVPN: true, loopback: true, reachable: false)
        #expect(result.problem == .cellular)
    }

    @Test func vpnSetupPrecedesCellularGuidance() {
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: true, hasVPN: false, loopback: false, reachable: false).problem == .noVPN)
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: false, hasVPN: false, loopback: false, reachable: false, networkUnavailable: true).problem == .networkUnavailable)
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: false, hasVPN: true, loopback: false, reachable: false, networkUnavailable: true).problem == .networkUnavailable)
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: false, hasVPN: true, loopback: true, reachable: false, networkUnavailable: true).problem == .serviceUnreachable)
        #expect(ConnectionDiagnostics.assessment(isLocal: false, cellular: false, hasVPN: false, loopback: false, reachable: false, networkUnavailable: true).problem == .serviceUnreachable)
        #expect(ConnectionDiagnosis(problem: .noVPN, isLocal: true).canConnectVPN)
        #expect(!ConnectionDiagnosis(problem: .networkUnavailable, isLocal: true).canConnectVPN)
        #expect(!ConnectionDiagnosis(problem: .noVPN, isLocal: false).canConnectVPN)
    }

    @Test func offlineConnectionNoticeRequiresConfirmedLocalNetworkConditions() {
        let offline = ConnectionDiagnostics.NetworkConditions(wifi: false, cellular: false)
        #expect(ConnectionDiagnostics.connectedNotice(targetIP: "10.7.0.1", conditions: offline) != nil)
        #expect(ConnectionDiagnostics.connectedNotice(targetIP: "192.0.2.1", conditions: offline) == nil)
        #expect(ConnectionDiagnostics.connectedNotice(targetIP: "10.7.0.1", conditions: nil) == nil)
        #expect(ConnectionDiagnostics.connectedNotice(targetIP: "10.7.0.1", conditions: .init(wifi: true, cellular: false)) == nil)
        #expect(ConnectionDiagnostics.connectedNotice(targetIP: "10.7.0.1", conditions: .init(wifi: false, cellular: true)) == nil)
    }

    @Test func diagnosticsDistinguishVPNAndServiceFailures() {
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: false, hasVPN: false, loopback: false, reachable: false).problem == .noVPN)
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: false, hasVPN: true, loopback: false, reachable: false).problem == .vpnUnresponsive)
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: true, hasVPN: true, loopback: false, reachable: false).problem == .vpnUnresponsive)
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: false, hasVPN: true, loopback: true, reachable: false).problem == .serviceUnreachable)
        #expect(ConnectionDiagnostics.assessment(isLocal: true, cellular: false, hasVPN: nil, loopback: nil, reachable: nil).problem == .unknown)
    }

    @Test func genericConnectionErrorsDoNotImplyInvalidPairing() {
        let error = NSError(domain: "StikDebug", code: -9, userInfo: [NSLocalizedDescriptionKey: "Connection timed out"])
        #expect(ConnectionDiagnostics.knownProblem(error) == nil)
        let rejected = NSError(domain: "StikDebug", code: 103, userInfo: [NSLocalizedDescriptionKey: "RemotePairing(PairVerifyFailed)"])
        #expect(ConnectionDiagnostics.knownProblem(rejected) == .pairingRejected)
    }

    @Test func diagnosticContextPreservesOriginalError() {
        let error = NSError(domain: "Example", code: -18, userInfo: [NSLocalizedDescriptionKey: "Invalid address"])
        let annotated = ConnectionDiagnostics.annotate(error, stage: .targetAddress, targetIP: "invalid")
        #expect(annotated.domain == error.domain)
        #expect(annotated.code == error.code)
        #expect(annotated.localizedDescription == error.localizedDescription)
        #expect(ConnectionDiagnostics.knownProblem(annotated) == .invalidAddress)
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
