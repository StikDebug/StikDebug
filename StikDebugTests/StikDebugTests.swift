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

    @Test func deviceVersionParsesSupportedIdentifiers() async throws {
        #expect(ProcessInfo.processInfo.deviceVersion(from: "iPhone14,2") == 14.2)
        #expect(ProcessInfo.processInfo.deviceVersion(from: "iPad14,5") == 14.5)
        #expect(ProcessInfo.processInfo.deviceVersion(from: "Mac14,2") == nil)
    }

}
