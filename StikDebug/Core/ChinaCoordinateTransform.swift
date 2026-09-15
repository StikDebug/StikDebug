//
//  ChinaCoordinateTransform.swift
//  StikDebug
//
//  Inside mainland China, map data is legally required to use the GCJ-02
//  ("Mars") datum, so MapKit renders AutoNavi tiles and hands back GCJ-02
//  coordinates. The location simulation service expects WGS-84. Feeding a
//  GCJ-02 coordinate straight through therefore lands 100-700 m away from
//  the point the user tapped.
//
//  This converts at the single boundary where map coordinates leave the
//  picker. Coordinates outside the mainland bounding box pass through
//  untouched, so nothing changes for users elsewhere.
//

import CoreLocation
import Foundation

enum ChinaCoordinateTransform {

    /// Krasovsky 1940 semi-major axis used by the GCJ-02 obfuscation.
    private static let semiMajorAxis = 6_378_245.0
    /// Square of the first eccentricity for the same ellipsoid.
    private static let eccentricitySquared = 0.006_693_421_622_965_943

    /// Rough bounding box of mainland China. Deliberately generous: the
    /// GCJ-02 offset is at most ~700 m, so a coordinate that is genuinely
    /// outside cannot be pushed inside by the transform.
    static func isOutsideMainlandChina(latitude: Double, longitude: Double) -> Bool {
        !(longitude > 73.66 && longitude < 135.05 && latitude > 3.86 && latitude < 53.55)
    }

    /// WGS-84 -> GCJ-02. Closed form.
    static func gcj02(fromWGS84 coordinate: CLLocationCoordinate2D) -> CLLocationCoordinate2D {
        guard !isOutsideMainlandChina(latitude: coordinate.latitude, longitude: coordinate.longitude) else {
            return coordinate
        }

        let x = coordinate.longitude - 105.0
        let y = coordinate.latitude - 35.0

        var deltaLatitude = latitudeOffset(x: x, y: y)
        var deltaLongitude = longitudeOffset(x: x, y: y)

        let radianLatitude = coordinate.latitude / 180.0 * .pi
        var magic = sin(radianLatitude)
        magic = 1 - eccentricitySquared * magic * magic
        let sqrtMagic = sqrt(magic)

        deltaLatitude = (deltaLatitude * 180.0)
            / ((semiMajorAxis * (1 - eccentricitySquared)) / (magic * sqrtMagic) * .pi)
        deltaLongitude = (deltaLongitude * 180.0)
            / (semiMajorAxis / sqrtMagic * cos(radianLatitude) * .pi)

        return CLLocationCoordinate2D(
            latitude: coordinate.latitude + deltaLatitude,
            longitude: coordinate.longitude + deltaLongitude
        )
    }

    /// GCJ-02 -> WGS-84. The forward transform has no analytic inverse, so
    /// this iterates on the residual; it converges well below a metre in a
    /// handful of passes.
    static func wgs84(fromGCJ02 coordinate: CLLocationCoordinate2D) -> CLLocationCoordinate2D {
        guard !isOutsideMainlandChina(latitude: coordinate.latitude, longitude: coordinate.longitude) else {
            return coordinate
        }

        var estimate = coordinate
        // ~1e-9 degrees is well under a millimetre; the loop normally exits
        // after three or four passes.
        let tolerance = 1e-9

        for _ in 0..<10 {
            let roundTrip = gcj02(fromWGS84: estimate)
            let latitudeError = roundTrip.latitude - coordinate.latitude
            let longitudeError = roundTrip.longitude - coordinate.longitude

            if abs(latitudeError) < tolerance && abs(longitudeError) < tolerance {
                break
            }

            estimate = CLLocationCoordinate2D(
                latitude: estimate.latitude - latitudeError,
                longitude: estimate.longitude - longitudeError
            )
        }

        return estimate
    }

    private static func latitudeOffset(x: Double, y: Double) -> Double {
        var offset = -100.0 + 2.0 * x + 3.0 * y + 0.2 * y * y + 0.1 * x * y + 0.2 * sqrt(abs(x))
        offset += (20.0 * sin(6.0 * x * .pi) + 20.0 * sin(2.0 * x * .pi)) * 2.0 / 3.0
        offset += (20.0 * sin(y * .pi) + 40.0 * sin(y / 3.0 * .pi)) * 2.0 / 3.0
        offset += (160.0 * sin(y / 12.0 * .pi) + 320.0 * sin(y * .pi / 30.0)) * 2.0 / 3.0
        return offset
    }

    private static func longitudeOffset(x: Double, y: Double) -> Double {
        var offset = 300.0 + x + 2.0 * y + 0.1 * x * x + 0.1 * x * y + 0.1 * sqrt(abs(x))
        offset += (20.0 * sin(6.0 * x * .pi) + 20.0 * sin(2.0 * x * .pi)) * 2.0 / 3.0
        offset += (20.0 * sin(x * .pi) + 40.0 * sin(x / 3.0 * .pi)) * 2.0 / 3.0
        offset += (150.0 * sin(x / 12.0 * .pi) + 300.0 * sin(x / 30.0 * .pi)) * 2.0 / 3.0
        return offset
    }
}
