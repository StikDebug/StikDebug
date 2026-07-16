//
//  MapSelectionView.swift
//  StikDebug
//
//  Created by Stephen on 11/3/25.
//

import SwiftUI
import MapKit
import UIKit
import UniformTypeIdentifiers

private struct CoordinateSnapshot: Equatable {
    let latitude: Double
    let longitude: Double

    init(_ coordinate: CLLocationCoordinate2D) {
        latitude = coordinate.latitude
        longitude = coordinate.longitude
    }

    var coordinate: CLLocationCoordinate2D {
        CLLocationCoordinate2D(latitude: latitude, longitude: longitude)
    }
}

private struct RouteSearchSelection {
    let title: String
    let coordinate: CLLocationCoordinate2D
}

private enum RouteSearchField {
    case start
    case end
}

private struct RouteSimulationPlan {
    let displayCoordinates: [CLLocationCoordinate2D]
    let distance: CLLocationDistance
    let expectedTravelTime: TimeInterval
}

private enum RouteSimulationDefaults {
    static let pathSamplingDistance: CLLocationDistance = 10
    /// Cap on densified route points. Playback interpolates within segments per
    /// tick regardless, so coarser spacing on long routes costs no smoothness —
    /// it just keeps memory, polyline rendering, and sample building bounded.
    static let maxDisplayCoordinateCount = 25_000
    static let playbackTickInterval: TimeInterval = 0.5
    static let minimumSpeedMetersPerSecond: CLLocationSpeed = 1.0
    static let importedRouteFallbackSpeedMetersPerSecond: CLLocationSpeed = 13.4
}

/// 10 m spacing for short routes, widening once a route would exceed the
/// display-point cap (e.g. a 500 km route samples every ~20 m instead).
private func adaptiveSamplingDistance(for coordinates: [CLLocationCoordinate2D]) -> CLLocationDistance {
    let totalDistance = distanceAlong(coordinates)
    return max(
        RouteSimulationDefaults.pathSamplingDistance,
        totalDistance / Double(RouteSimulationDefaults.maxDisplayCoordinateCount)
    )
}

private struct RoutePlaybackSample {
    let coordinate: CLLocationCoordinate2D
    let delayFromPrevious: TimeInterval
}

enum SpeedProfile: String, CaseIterable, Identifiable {
    case walking
    case jogging
    case cycling
    case driving
    case bus
    case custom

    var id: String { rawValue }

    var title: String {
        switch self {
        case .walking: return "Walking"
        case .jogging: return "Jogging"
        case .cycling: return "Cycling"
        case .driving: return "Driving"
        case .bus: return "Bus"
        case .custom: return "Custom"
        }
    }

    var systemImage: String {
        switch self {
        case .walking: return "figure.walk"
        case .jogging: return "figure.run"
        case .cycling: return "bicycle"
        case .driving: return "car.fill"
        case .bus: return "bus.fill"
        case .custom: return "speedometer"
        }
    }

    /// Fixed pace in m/s, or nil when speed comes from road data / user input.
    var fixedSpeedMetersPerSecond: CLLocationSpeed? {
        switch self {
        case .walking: return 1.4
        case .jogging: return 2.7
        case .cycling: return 5.5
        case .driving, .bus, .custom: return nil
        }
    }

    /// Walking-ish profiles should route along footpaths, not roads.
    var prefersWalkingDirections: Bool {
        switch self {
        case .walking, .jogging: return true
        default: return false
        }
    }
}

enum MapDisplayMode: String, CaseIterable, Identifiable {
    case standard
    case satellite
    case hybrid

    var id: String { rawValue }

    var title: String {
        switch self {
        case .standard: return "Standard"
        case .satellite: return "Satellite"
        case .hybrid: return "Hybrid"
        }
    }

    var systemImage: String {
        switch self {
        case .standard: return "map"
        case .satellite: return "globe.americas.fill"
        case .hybrid: return "square.2.layers.3d"
        }
    }

    /// Pure satellite imagery can't render traffic or labeled points of interest.
    var supportsOverlays: Bool {
        self != .satellite
    }
}

enum MapPointsOfInterestMode: String, CaseIterable, Identifiable {
    case all
    case transit
    case hidden

    var id: String { rawValue }

    var title: String {
        switch self {
        case .all: return "All Places"
        case .transit: return "Transit Stops"
        case .hidden: return "Hidden"
        }
    }

    var systemImage: String {
        switch self {
        case .all: return "mappin.and.ellipse"
        case .transit: return "bus.fill"
        case .hidden: return "eye.slash"
        }
    }

    var categories: PointOfInterestCategories {
        switch self {
        case .all: return .all
        case .transit: return .including([.publicTransport])
        case .hidden: return .excludingAll
        }
    }
}

private struct SpeedProfileSettings {
    let profile: SpeedProfile
    let customSpeedMetersPerSecond: CLLocationSpeed

    static let busSpeedCapMetersPerSecond: CLLocationSpeed = 13.9   // ~50 km/h
    static let busStopApproachRadius: CLLocationDistance = 60      // begin slowing
    static let busStopApproachSpeed: CLLocationSpeed = 4.0         // crawl near stops
    static let busStopDwellSeconds: TimeInterval = 12              // doors open
    static let busStopSnapRadius: CLLocationDistance = 25          // dwell trigger

    /// Resolve the playback speed for one segment given the road limit (if any).
    func speed(forRoadLimit roadLimit: CLLocationSpeed?, fallback: CLLocationSpeed) -> CLLocationSpeed {
        if let fixed = profile.fixedSpeedMetersPerSecond {
            return fixed
        }
        switch profile {
        case .custom:
            return max(customSpeedMetersPerSecond, RouteSimulationDefaults.minimumSpeedMetersPerSecond)
        case .bus:
            return min(roadLimit ?? fallback, Self.busSpeedCapMetersPerSecond)
        default: // .driving — original behavior
            return roadLimit ?? fallback
        }
    }
}

private struct RouteSpeedContext {
    let ways: [OpenStreetMapWay]
    let busStops: [CLLocationCoordinate2D]
}

private struct OpenStreetMapWay {
    let geometry: [CLLocationCoordinate2D]
    let speedLimitMetersPerSecond: CLLocationSpeed
}

private enum OpenStreetMapSpeedLimitService {
    static let endpoint = URL(string: "https://overpass-api.de/api/interpreter")!
    static let copyrightURL = URL(string: "https://www.openstreetmap.org/copyright")!
    static let nearestWayThreshold: CLLocationDistance = 40

    // Route chunking. The whole route is split into ~5 km chunks fetched as
    // independent requests, each queried by its own small bounding box rather
    // than an `around(radius, points…)` list. `around` costs scale with
    // points-in-list × candidate-elements-in-area and reliably blew past the
    // server timeout on anything but the shortest stretches — measured
    // 16-23s (or outright timeout) for a dense-area chunk that a bbox query
    // resolved in 3-8s for the same area. A bbox test is an O(1) rectangle
    // check per candidate regardless of the route's shape, so cost no longer
    // scales with corridor point density — the actual cause of stops loading
    // inconsistently (denser areas or longer stretches pushed the old query
    // past its timeout more often, but never predictably).
    static let chunkTargetDistance: CLLocationDistance = 5_000 // ~5 km of route per request
    static let maxChunkCount = 80                              // caps requests on very long routes
    static let maxConcurrentChunkRequests = 3
    // Padding around each chunk's tight point bbox. Comfortably larger than
    // the client-side match thresholds below (40 m ways / 90 m stops) so nothing
    // near the edge of a chunk is missed by the coarse server-side box.
    static let chunkBBoxPadding: CLLocationDistance = 120
    static let chunkServerTimeout: TimeInterval = 20           // Overpass-side [timeout:]
    static let requestTimeout: TimeInterval = 30               // client-side, per chunk;
                                                                // must exceed the server timeout with
                                                                // margin — Overpass still takes several
                                                                // seconds to flush its own timeout reply
    static let chunkRetryAttempts = 2                          // total tries per chunk before giving up

    // Physical stops are often mapped twice (platform + stop_position a few
    // meters apart); merge anything closer than this so markers and dwells
    // aren't doubled. Opposite-direction stop pairs sit 20 m+ apart and survive.
    static let busStopDedupeRadius: CLLocationDistance = 15

    // Spatial index cell size (~440 m at the equator). Comfortably larger than
    // every search radius above, so a ±1 cell ring around a query point always
    // covers the relevant neighborhood.
    static let indexCellSizeDegrees = 0.004
}

/// Integer cell coordinate used by the spatial indexes below.
private struct SpatialCellKey: Hashable {
    let x: Int
    let y: Int

    init(x: Int, y: Int) {
        self.x = x
        self.y = y
    }

    init(latitude: Double, longitude: Double, cellSizeDegrees: Double) {
        x = Int(floor(longitude / cellSizeDegrees))
        y = Int(floor(latitude / cellSizeDegrees))
    }
}

private struct OverpassResponse: Decodable {
    let elements: [Element]
    // Present only on a server-side problem (e.g. "runtime error: Query timed
    // out…"), never on a clean result. Overpass replies HTTP 200 with an empty
    // `elements` array in this case rather than an error status, so without
    // checking this field a timed-out chunk looked identical to "no data near
    // this stretch of route" — the actual cause of stops loading inconsistently.
    let remark: String?

    struct Element: Decodable {
        let id: Int?
        let tags: [String: String]?
        let geometry: [Coordinate]?
        let lat: Double?
        let lon: Double?
    }

    struct Coordinate: Decodable {
        let lat: Double
        let lon: Double
    }
}

private extension MKPolyline {
    var coordinateArray: [CLLocationCoordinate2D] {
        var coordinates = [CLLocationCoordinate2D](
            repeating: CLLocationCoordinate2D(latitude: 0, longitude: 0),
            count: pointCount
        )
        getCoordinates(&coordinates, range: NSRange(location: 0, length: pointCount))
        return coordinates
    }
}

private func interpolateCoordinate(
    from start: CLLocationCoordinate2D,
    to end: CLLocationCoordinate2D,
    fraction: Double
) -> CLLocationCoordinate2D {
    CLLocationCoordinate2D(
        latitude: start.latitude + ((end.latitude - start.latitude) * fraction),
        longitude: start.longitude + ((end.longitude - start.longitude) * fraction)
    )
}

private func sampledRouteCoordinates(
    from coordinates: [CLLocationCoordinate2D],
    targetDistance: CLLocationDistance
) -> [CLLocationCoordinate2D] {
    guard coordinates.count > 1 else { return coordinates }

    var sampled = [coordinates[0]]
    for (start, end) in zip(coordinates, coordinates.dropFirst()) {
        let distance = CLLocation(latitude: start.latitude, longitude: start.longitude)
            .distance(from: CLLocation(latitude: end.latitude, longitude: end.longitude))
        let segmentCount = max(1, Int(ceil(distance / targetDistance)))
        for index in 1...segmentCount {
            let point = interpolateCoordinate(
                from: start,
                to: end,
                fraction: Double(index) / Double(segmentCount)
            )
            if sampled.last.map(CoordinateSnapshot.init) != CoordinateSnapshot(point) {
                sampled.append(point)
            }
        }
    }

    return sampled
}

private func midpointCoordinate(
    from start: CLLocationCoordinate2D,
    to end: CLLocationCoordinate2D
) -> CLLocationCoordinate2D {
    interpolateCoordinate(from: start, to: end, fraction: 0.5)
}

private func distanceAlong(_ coordinates: [CLLocationCoordinate2D]) -> CLLocationDistance {
    zip(coordinates, coordinates.dropFirst()).reduce(0) { total, pair in
        total + CLLocation(latitude: pair.0.latitude, longitude: pair.0.longitude)
            .distance(from: CLLocation(latitude: pair.1.latitude, longitude: pair.1.longitude))
    }
}

private func distanceFromPoint(
    _ point: MKMapPoint,
    toSegmentFrom start: MKMapPoint,
    to end: MKMapPoint
) -> CLLocationDistance {
    let dx = end.x - start.x
    let dy = end.y - start.y

    guard dx != 0 || dy != 0 else {
        return point.distance(to: start)
    }

    let projection = max(0, min(1, ((point.x - start.x) * dx + (point.y - start.y) * dy) / ((dx * dx) + (dy * dy))))
    let projectedPoint = MKMapPoint(
        x: start.x + (dx * projection),
        y: start.y + (dy * projection)
    )
    return point.distance(to: projectedPoint)
}

private func parseSpeedLimitMetersPerSecond(from rawValue: String) -> CLLocationSpeed? {
    let normalized = rawValue
        .lowercased()
        .split(separator: ";")
        .first?
        .trimmingCharacters(in: .whitespacesAndNewlines) ?? ""

    guard !normalized.isEmpty else { return nil }
    guard normalized != "none",
          normalized != "signals",
          normalized != "implicit",
          normalized != "walk" else {
        return nil
    }

    let scanner = Scanner(string: normalized)
    guard let numericValue = scanner.scanDouble() else { return nil }

    if normalized.contains("mph") {
        return numericValue * 0.44704
    }
    if normalized.contains("knot") {
        return numericValue * 0.514444
    }

    return numericValue / 3.6
}

private func speedLimitMetersPerSecond(from tags: [String: String]) -> CLLocationSpeed? {
    if let maxspeed = tags["maxspeed"],
       let parsed = parseSpeedLimitMetersPerSecond(from: maxspeed) {
        return parsed
    }

    let directionalValues = [
        tags["maxspeed:forward"],
        tags["maxspeed:backward"]
    ]
        .compactMap { $0 }
        .compactMap(parseSpeedLimitMetersPerSecond(from:))

    guard !directionalValues.isEmpty else { return nil }
    return directionalValues.min()
}

/// Split the route into chunks of about `chunkTargetDistance` real-world
/// meters each (walked along the polyline, not raw point count), capped at
/// `maxChunkCount` chunks so an extremely long route still bounds the number
/// of requests made. Consecutive chunks share one boundary point so nothing
/// near a seam falls outside every chunk's bounding box.
private func routeChunks(from coordinates: [CLLocationCoordinate2D]) -> [[CLLocationCoordinate2D]] {
    guard coordinates.count > 1 else { return [] }

    let totalDistance = distanceAlong(coordinates)
    let chunkDistance = max(
        OpenStreetMapSpeedLimitService.chunkTargetDistance,
        totalDistance / Double(OpenStreetMapSpeedLimitService.maxChunkCount)
    )

    var chunks: [[CLLocationCoordinate2D]] = []
    var current: [CLLocationCoordinate2D] = [coordinates[0]]
    var accumulated: CLLocationDistance = 0

    for (previous, point) in zip(coordinates, coordinates.dropFirst()) {
        accumulated += CLLocation(latitude: previous.latitude, longitude: previous.longitude)
            .distance(from: CLLocation(latitude: point.latitude, longitude: point.longitude))
        current.append(point)
        if accumulated >= chunkDistance {
            chunks.append(current)
            current = [point] // shared boundary point, so no coverage gap at the seam
            accumulated = 0
        }
    }
    if current.count > 1 {
        chunks.append(current)
    }
    return chunks
}

private struct ChunkBoundingBox {
    let south: Double
    let west: Double
    let north: Double
    let east: Double
}

/// Tight bounding box of `coordinates`, padded by `paddingMeters`. Since a
/// straight segment between two consecutive route points lies within their
/// combined bbox, padding the point-only bbox by at least the largest
/// client-side match radius guarantees nothing near the true path is missed.
private func boundingBox(for coordinates: [CLLocationCoordinate2D], paddingMeters: CLLocationDistance) -> ChunkBoundingBox? {
    guard let first = coordinates.first else { return nil }

    var minLatitude = first.latitude
    var maxLatitude = first.latitude
    var minLongitude = first.longitude
    var maxLongitude = first.longitude

    for coordinate in coordinates.dropFirst() {
        minLatitude = min(minLatitude, coordinate.latitude)
        maxLatitude = max(maxLatitude, coordinate.latitude)
        minLongitude = min(minLongitude, coordinate.longitude)
        maxLongitude = max(maxLongitude, coordinate.longitude)
    }

    let midLatitudeRadians = (minLatitude + maxLatitude) / 2 * .pi / 180
    let latitudePadding = paddingMeters / 111_320
    let longitudePadding = paddingMeters / (111_320 * max(cos(midLatitudeRadians), 0.01))

    return ChunkBoundingBox(
        south: minLatitude - latitudePadding,
        west: minLongitude - longitudePadding,
        north: maxLatitude + latitudePadding,
        east: maxLongitude + longitudePadding
    )
}

private func overpassChunkQuery(forBoundingBox bbox: ChunkBoundingBox, includeBusStops: Bool) -> String {
    let bboxString = String(format: "%.6f,%.6f,%.6f,%.6f", bbox.south, bbox.west, bbox.north, bbox.east)

    // Bus stops are mapped inconsistently in OSM: the legacy `highway=bus_stop`
    // tag, and the newer public_transport schema (platform / stop_position with
    // `bus=yes`). One regex clause pulls that superset in a single pass —
    // `tagsDescribeBusStop` keeps only real bus stops client-side.
    var busStopClause = ""
    if includeBusStops {
        busStopClause = """

          node(\(bboxString))[~"^(highway|public_transport)$"~"^(bus_stop|platform|stop_position)$"];
        """
    }

    // A bounding-box filter is an O(1) rectangle test per candidate element,
    // independent of the chunk's shape or point density — unlike `around`
    // with a point list, whose cost scales with points × candidates and
    // reliably exceeded the server timeout on anything but the shortest
    // stretches. Extra candidates a loose box admits are rejected client-side
    // by `nearestWayThreshold` / `hasBusStop(within:)`, so precision is
    // unaffected.
    return """
    [out:json][timeout:\(Int(OpenStreetMapSpeedLimitService.chunkServerTimeout))];
    way(\(bboxString))[highway]->.roads;
    (
      way.roads[maxspeed];
      way.roads["maxspeed:forward"];
      way.roads["maxspeed:backward"];\(busStopClause)
    );
    out tags geom;
    """
}

private func tagsDescribeBusStop(_ tags: [String: String]) -> Bool {
    if tags["highway"] == "bus_stop" {
        return true
    }
    let publicTransport = tags["public_transport"]
    if publicTransport == "platform" || publicTransport == "stop_position" {
        return tags["bus"] == "yes"
    }
    return false
}

private struct OverpassChunkResult {
    let ways: [(id: Int, value: OpenStreetMapWay)]
    let busStops: [(id: Int, value: CLLocationCoordinate2D)]
}

private func fetchSpeedContextChunk(
    bbox: ChunkBoundingBox,
    includeBusStops: Bool
) async throws -> OverpassChunkResult {
    let query = overpassChunkQuery(forBoundingBox: bbox, includeBusStops: includeBusStops)

    // POST the query: this stays small since the request body is now just a
    // bbox rather than a whole corridor of points. The explicit timeout keeps
    // a slow Overpass server from stalling this chunk — a failed chunk only
    // costs its own stretch of road data.
    var request = URLRequest(url: OpenStreetMapSpeedLimitService.endpoint)
    request.httpMethod = "POST"
    request.setValue("application/x-www-form-urlencoded", forHTTPHeaderField: "Content-Type")
    request.timeoutInterval = OpenStreetMapSpeedLimitService.requestTimeout

    var formAllowed = CharacterSet.alphanumerics
    formAllowed.insert(charactersIn: "-._~")
    let encodedQuery = query.addingPercentEncoding(withAllowedCharacters: formAllowed) ?? ""
    request.httpBody = "data=\(encodedQuery)".data(using: .utf8)

    let (data, response) = try await URLSession.shared.data(for: request)

    if let httpResponse = response as? HTTPURLResponse,
       !(200...299).contains(httpResponse.statusCode) {
        throw NSError(
            domain: "OpenStreetMapSpeedLimits",
            code: httpResponse.statusCode,
            userInfo: [NSLocalizedDescriptionKey: "Overpass returned HTTP \(httpResponse.statusCode)."]
        )
    }

    let decoded = try JSONDecoder().decode(OverpassResponse.self, from: data)

    // Overpass replies HTTP 200 with an empty `elements` array and a `remark`
    // when it hits its own timeout or otherwise can't complete the query —
    // treat that the same as a network failure (retryable) rather than
    // silently accepting "no data found here".
    if let remark = decoded.remark {
        throw NSError(
            domain: "OpenStreetMapSpeedLimits",
            code: -2,
            userInfo: [NSLocalizedDescriptionKey: "Overpass reported an error for this chunk: \(remark)"]
        )
    }

    let ways: [(id: Int, value: OpenStreetMapWay)] = decoded.elements.compactMap { element in
        guard let id = element.id,
              let tags = element.tags,
              let speedLimit = speedLimitMetersPerSecond(from: tags),
              let geometry = element.geometry?.map({ CLLocationCoordinate2D(latitude: $0.lat, longitude: $0.lon) }),
              geometry.count > 1 else {
            return nil
        }

        return (id, OpenStreetMapWay(
            geometry: geometry,
            speedLimitMetersPerSecond: speedLimit
        ))
    }

    let busStops: [(id: Int, value: CLLocationCoordinate2D)] = decoded.elements.compactMap { element in
        guard let id = element.id,
              let lat = element.lat, let lon = element.lon,
              let tags = element.tags,
              tagsDescribeBusStop(tags) else {
            return nil
        }
        return (id, CLLocationCoordinate2D(latitude: lat, longitude: lon))
    }

    return OverpassChunkResult(ways: ways, busStops: busStops)
}

/// Retries a chunk fetch on failure (network error, HTTP error, or a
/// server-side timeout surfaced via `remark`) with a short backoff — the
/// public Overpass instance is a shared, load-dependent resource, so a single
/// slow response shouldn't cost that stretch of the route its data. Returns
/// `nil` only once every attempt has failed.
private func fetchSpeedContextChunkWithRetry(
    bbox: ChunkBoundingBox,
    includeBusStops: Bool
) async -> OverpassChunkResult? {
    let maxAttempts = OpenStreetMapSpeedLimitService.chunkRetryAttempts
    for attempt in 1...maxAttempts {
        do {
            return try await fetchSpeedContextChunk(bbox: bbox, includeBusStops: includeBusStops)
        } catch {
            guard attempt < maxAttempts else { return nil }
            try? await Task.sleep(for: .seconds(Double(attempt) * 1.5))
        }
    }
    return nil
}

/// Merge stops closer together than `radius` — a stop's platform and
/// stop_position nodes are usually a few meters apart and would otherwise
/// double every marker and dwell. Grid-bucketed so it stays linear.
private func dedupedBusStops(
    _ stops: [CLLocationCoordinate2D],
    radius: CLLocationDistance
) -> [CLLocationCoordinate2D] {
    guard stops.count > 1 else { return stops }

    let cellSize = OpenStreetMapSpeedLimitService.indexCellSizeDegrees
    var kept: [CLLocationCoordinate2D] = []
    var grid: [SpatialCellKey: [CLLocationCoordinate2D]] = [:]

    for stop in stops {
        let location = CLLocation(latitude: stop.latitude, longitude: stop.longitude)
        let center = SpatialCellKey(
            latitude: stop.latitude,
            longitude: stop.longitude,
            cellSizeDegrees: cellSize
        )

        var isDuplicate = false
        search: for dx in -1...1 {
            for dy in -1...1 {
                guard let bucket = grid[SpatialCellKey(x: center.x + dx, y: center.y + dy)] else {
                    continue
                }
                for existing in bucket {
                    let existingLocation = CLLocation(latitude: existing.latitude, longitude: existing.longitude)
                    if location.distance(from: existingLocation) < radius {
                        isDuplicate = true
                        break search
                    }
                }
            }
        }

        if !isDuplicate {
            kept.append(stop)
            grid[center, default: []].append(stop)
        }
    }

    return kept
}

/// Fetches road speed limits (and optionally bus stops) along the route.
///
/// The route is split into distance-based chunks, each queried by its own
/// bounding box, at most `maxConcurrentChunkRequests` in flight, deduplicated
/// by OSM element id (chunks overlap at their seams). A chunk that still
/// fails after retrying is tolerated — whatever data arrived from the other
/// chunks is still used. When bus stops are requested, `onPartialBusStops`
/// fires with the cumulative deduplicated stops as each chunk lands, so
/// markers appear progressively instead of after one all-or-nothing query.
private func fetchRouteSpeedContext(
    for coordinates: [CLLocationCoordinate2D],
    includeBusStops: Bool,
    onPartialBusStops: (@Sendable ([CLLocationCoordinate2D]) -> Void)? = nil
) async -> RouteSpeedContext {
    let bboxes = routeChunks(from: coordinates).compactMap {
        boundingBox(for: $0, paddingMeters: OpenStreetMapSpeedLimitService.chunkBBoxPadding)
    }
    guard !bboxes.isEmpty else { return RouteSpeedContext(ways: [], busStops: []) }

    var ways: [OpenStreetMapWay] = []
    var busStops: [CLLocationCoordinate2D] = []
    var seenWayIDs: Set<Int> = []
    var seenStopIDs: Set<Int> = []
    var failedChunkCount = 0

    await withTaskGroup(of: OverpassChunkResult?.self) { group in
        var pending = bboxes.makeIterator()
        var inFlight = 0
        while inFlight < OpenStreetMapSpeedLimitService.maxConcurrentChunkRequests,
              let bbox = pending.next() {
            group.addTask { await fetchSpeedContextChunkWithRetry(bbox: bbox, includeBusStops: includeBusStops) }
            inFlight += 1
        }

        for await result in group {
            if Task.isCancelled {
                group.cancelAll()
                break
            }
            if let bbox = pending.next() {
                group.addTask { await fetchSpeedContextChunkWithRetry(bbox: bbox, includeBusStops: includeBusStops) }
            }
            guard let result else {
                failedChunkCount += 1
                continue
            }

            for way in result.ways where seenWayIDs.insert(way.id).inserted {
                ways.append(way.value)
            }

            var addedStops = false
            for stop in result.busStops where seenStopIDs.insert(stop.id).inserted {
                busStops.append(stop.value)
                addedStops = true
            }

            if includeBusStops, addedStops {
                onPartialBusStops?(dedupedBusStops(
                    busStops,
                    radius: OpenStreetMapSpeedLimitService.busStopDedupeRadius
                ))
            }
        }
    }

    if failedChunkCount > 0 {
        LogManager.shared.addWarningLog(
            "Route data: \(failedChunkCount) of \(bboxes.count) map-data chunks failed after retrying; continuing with partial coverage"
        )
    }

    return RouteSpeedContext(
        ways: ways,
        busStops: dedupedBusStops(
            busStops,
            radius: OpenStreetMapSpeedLimitService.busStopDedupeRadius
        )
    )
}

private struct IndexedWaySegment {
    let start: MKMapPoint
    let end: MKMapPoint
    let speedLimitMetersPerSecond: CLLocationSpeed
}

/// Grid-bucketed index over the Overpass response. The old implementation
/// rescanned every way segment for every route segment (O(route × ways)),
/// which made long routes take minutes of CPU; bucketing by ~440 m cells and
/// probing only the 3×3 neighborhood turns each lookup into a handful of
/// segment checks. All search radii (≤ 90 m) are far smaller than a cell at
/// non-polar latitudes, so ring probing never misses a legitimate match.
private struct RouteSpeedIndex {
    private var waySegments: [SpatialCellKey: [IndexedWaySegment]] = [:]
    private var busStopCells: [SpatialCellKey: [CLLocationCoordinate2D]] = [:]
    private let cellSize = OpenStreetMapSpeedLimitService.indexCellSizeDegrees

    init(context: RouteSpeedContext) {
        for way in context.ways {
            for (wayStart, wayEnd) in zip(way.geometry, way.geometry.dropFirst()) {
                let segment = IndexedWaySegment(
                    start: MKMapPoint(wayStart),
                    end: MKMapPoint(wayEnd),
                    speedLimitMetersPerSecond: way.speedLimitMetersPerSecond
                )
                // A segment can cross cell boundaries; register it in every
                // cell its bounding box touches so ring probes always see it.
                let minX = Int(floor(min(wayStart.longitude, wayEnd.longitude) / cellSize))
                let maxX = Int(floor(max(wayStart.longitude, wayEnd.longitude) / cellSize))
                let minY = Int(floor(min(wayStart.latitude, wayEnd.latitude) / cellSize))
                let maxY = Int(floor(max(wayStart.latitude, wayEnd.latitude) / cellSize))
                // Most road segments span a cell or two, but sparsely-noded
                // rural highways can legitimately run tens of km between OSM
                // geometry points, so allow a generous span (~57 km at the
                // equator). Anything larger is broken data — e.g. an
                // antimeridian jump spans ~90,000 cells — and is skipped rather
                // than flooding the index. The earlier ≤8-cell cap (~3.5 km)
                // silently dropped those long segments, so those stretches lost
                // their real speed limit and fell back to average pacing.
                let maxCellSpan = 128
                guard maxX - minX <= maxCellSpan, maxY - minY <= maxCellSpan else { continue }
                for x in minX...maxX {
                    for y in minY...maxY {
                        waySegments[SpatialCellKey(x: x, y: y), default: []].append(segment)
                    }
                }
            }
        }

        for stop in context.busStops {
            let key = SpatialCellKey(
                latitude: stop.latitude,
                longitude: stop.longitude,
                cellSizeDegrees: cellSize
            )
            busStopCells[key, default: []].append(stop)
        }
    }

    func nearestSpeedLimit(
        forSegmentFrom start: CLLocationCoordinate2D,
        to end: CLLocationCoordinate2D
    ) -> CLLocationSpeed? {
        guard !waySegments.isEmpty else { return nil }

        let midpoint = midpointCoordinate(from: start, to: end)
        let midMapPoint = MKMapPoint(midpoint)
        let center = SpatialCellKey(
            latitude: midpoint.latitude,
            longitude: midpoint.longitude,
            cellSizeDegrees: cellSize
        )

        var bestMatch: (speed: CLLocationSpeed, distance: CLLocationDistance)?
        for dx in -1...1 {
            for dy in -1...1 {
                guard let bucket = waySegments[SpatialCellKey(x: center.x + dx, y: center.y + dy)] else {
                    continue
                }
                for segment in bucket {
                    let candidateDistance = distanceFromPoint(
                        midMapPoint,
                        toSegmentFrom: segment.start,
                        to: segment.end
                    )
                    if bestMatch == nil || candidateDistance < bestMatch!.distance {
                        bestMatch = (segment.speedLimitMetersPerSecond, candidateDistance)
                    }
                }
            }
        }

        guard let bestMatch,
              bestMatch.distance <= OpenStreetMapSpeedLimitService.nearestWayThreshold else {
            return nil
        }

        return bestMatch.speed
    }

    func hasBusStop(within radius: CLLocationDistance, of coordinate: CLLocationCoordinate2D) -> Bool {
        guard !busStopCells.isEmpty else { return false }

        let location = CLLocation(latitude: coordinate.latitude, longitude: coordinate.longitude)
        let center = SpatialCellKey(
            latitude: coordinate.latitude,
            longitude: coordinate.longitude,
            cellSizeDegrees: cellSize
        )

        for dx in -1...1 {
            for dy in -1...1 {
                guard let bucket = busStopCells[SpatialCellKey(x: center.x + dx, y: center.y + dy)] else {
                    continue
                }
                for stop in bucket {
                    let stopLocation = CLLocation(latitude: stop.latitude, longitude: stop.longitude)
                    if location.distance(from: stopLocation) <= radius {
                        return true
                    }
                }
            }
        }
        return false
    }
}

private func buildPlaybackSamples(
    from displayCoordinates: [CLLocationCoordinate2D],
    speedContext: RouteSpeedContext,
    fallbackSpeedMetersPerSecond: CLLocationSpeed,
    speedSettings: SpeedProfileSettings
) -> [RoutePlaybackSample] {
    guard let firstCoordinate = displayCoordinates.first else { return [] }

    let speedIndex = RouteSpeedIndex(context: speedContext)
    var samples = [RoutePlaybackSample(coordinate: firstCoordinate, delayFromPrevious: 0)]

    for (start, end) in zip(displayCoordinates, displayCoordinates.dropFirst()) {
        let segmentDistance = CLLocation(latitude: start.latitude, longitude: start.longitude)
            .distance(from: CLLocation(latitude: end.latitude, longitude: end.longitude))
        guard segmentDistance > 0 else { continue }

        let roadLimit = speedIndex.nearestSpeedLimit(forSegmentFrom: start, to: end)
        var speed = speedSettings.speed(forRoadLimit: roadLimit, fallback: fallbackSpeedMetersPerSecond)

        // Bus mode: ease off when approaching a stop.
        if speedSettings.profile == .bus,
           !speedContext.busStops.isEmpty {
            let midpoint = midpointCoordinate(from: start, to: end)
            if speedIndex.hasBusStop(
                within: SpeedProfileSettings.busStopApproachRadius,
                of: midpoint
            ) {
                speed = min(speed, SpeedProfileSettings.busStopApproachSpeed)
            }
        }

        let clampedSpeed = max(speed, RouteSimulationDefaults.minimumSpeedMetersPerSecond)
        let segmentTravelTime = segmentDistance / clampedSpeed
        let segmentStepCount = max(1, Int(ceil(segmentTravelTime / RouteSimulationDefaults.playbackTickInterval)))
        let stepDelay = segmentTravelTime / Double(segmentStepCount)

        for index in 1...segmentStepCount {
            let coordinate = interpolateCoordinate(
                from: start,
                to: end,
                fraction: Double(index) / Double(segmentStepCount)
            )
            if samples.last.map({ CoordinateSnapshot($0.coordinate) }) != CoordinateSnapshot(coordinate) {
                samples.append(RoutePlaybackSample(coordinate: coordinate, delayFromPrevious: stepDelay))
            }
        }
    }

    // Bus mode: dwell once at each stop along the route (doors open, people shuffle).
    if speedSettings.profile == .bus, !speedContext.busStops.isEmpty, samples.count > 1 {
        // Bucket the samples by grid cell so each stop only compares against
        // nearby samples instead of the entire (potentially huge) sample list.
        let cellSize = OpenStreetMapSpeedLimitService.indexCellSizeDegrees
        var sampleCells: [SpatialCellKey: [Int]] = [:]
        for (index, sample) in samples.enumerated() {
            let key = SpatialCellKey(
                latitude: sample.coordinate.latitude,
                longitude: sample.coordinate.longitude,
                cellSizeDegrees: cellSize
            )
            sampleCells[key, default: []].append(index)
        }

        var dwellIndices: Set<Int> = []
        for stop in speedContext.busStops {
            let stopLocation = CLLocation(latitude: stop.latitude, longitude: stop.longitude)
            let center = SpatialCellKey(
                latitude: stop.latitude,
                longitude: stop.longitude,
                cellSizeDegrees: cellSize
            )
            var bestIndex: Int?
            var bestDistance = SpeedProfileSettings.busStopSnapRadius
            for dx in -1...1 {
                for dy in -1...1 {
                    guard let bucket = sampleCells[SpatialCellKey(x: center.x + dx, y: center.y + dy)] else {
                        continue
                    }
                    for index in bucket {
                        let sample = samples[index]
                        let distance = stopLocation.distance(
                            from: CLLocation(
                                latitude: sample.coordinate.latitude,
                                longitude: sample.coordinate.longitude
                            )
                        )
                        if distance <= bestDistance {
                            bestDistance = distance
                            bestIndex = index
                        }
                    }
                }
            }
            if let bestIndex, bestIndex > 0 {
                dwellIndices.insert(bestIndex)
            }
        }

        if !dwellIndices.isEmpty {
            samples = samples.enumerated().map { index, sample in
                guard dwellIndices.contains(index) else { return sample }
                return RoutePlaybackSample(
                    coordinate: sample.coordinate,
                    delayFromPrevious: sample.delayFromPrevious + SpeedProfileSettings.busStopDwellSeconds
                )
            }
        }
    }

    return samples
}

private struct RoutePlaybackPrefetchResult {
    let samples: [RoutePlaybackSample]
    let busStops: [CLLocationCoordinate2D]
}

private func prefetchRoutePlaybackSamples(
    displayCoordinates: [CLLocationCoordinate2D],
    fallbackSpeedMetersPerSecond: CLLocationSpeed,
    speedSettings: SpeedProfileSettings,
    onPartialBusStops: (@Sendable ([CLLocationCoordinate2D]) -> Void)? = nil
) async -> RoutePlaybackPrefetchResult {
    let needsRoadData = speedSettings.profile.fixedSpeedMetersPerSecond == nil
        && speedSettings.profile != .custom
    let context: RouteSpeedContext
    if needsRoadData {
        context = await fetchRouteSpeedContext(
            for: displayCoordinates,
            includeBusStops: speedSettings.profile == .bus,
            onPartialBusStops: onPartialBusStops
        )
    } else {
        // Fixed/custom pace: no need to bother Overpass at all.
        context = RouteSpeedContext(ways: [], busStops: [])
    }
    let samples = buildPlaybackSamples(
        from: displayCoordinates,
        speedContext: context,
        fallbackSpeedMetersPerSecond: fallbackSpeedMetersPerSecond,
        speedSettings: speedSettings
    )
    return RoutePlaybackPrefetchResult(samples: samples, busStops: context.busStops)
}

private enum CoordinateImportError: LocalizedError {
    case emptyFile
    case noCoordinates

    var errorDescription: String? {
        switch self {
        case .emptyFile:
            return "The selected file is empty."
        case .noCoordinates:
            return "No valid coordinates were found. Use GPX, GeoJSON, JSON, CSV, or plain text with latitude and longitude values."
        }
    }
}

private enum CoordinateImportParser {
    static let supportedContentTypes: [UTType] = [
        .plainText,
        .commaSeparatedText,
        .json,
        .xml,
        UTType(filenameExtension: "gpx", conformingTo: .xml) ?? .xml,
        UTType(filenameExtension: "kml", conformingTo: .xml) ?? .xml,
        UTType(filenameExtension: "geojson", conformingTo: .json) ?? .json
    ]

    private enum CoordinateOrder {
        case latitudeLongitude
        case longitudeLatitude
    }

    static func parse(url: URL) throws -> [CLLocationCoordinate2D] {
        let accessing = url.startAccessingSecurityScopedResource()
        defer {
            if accessing {
                url.stopAccessingSecurityScopedResource()
            }
        }

        let data = try Data(contentsOf: url)
        guard !data.isEmpty else { throw CoordinateImportError.emptyFile }

        let fileExtension = url.pathExtension.lowercased()
        if fileExtension == "json" || fileExtension == "geojson" {
            if let coordinates = try? parseJSONCoordinates(from: data),
               !coordinates.isEmpty {
                return coordinates
            }
        }

        if fileExtension == "gpx" || fileExtension == "kml" || fileExtension == "xml" {
            let coordinates = parseXMLCoordinates(from: data)
            if !coordinates.isEmpty {
                return coordinates
            }
        }

        if let text = decodedText(from: data) {
            let coordinates = parseInline(text)
            if !coordinates.isEmpty {
                return coordinates
            }
        }

        if let coordinates = try? parseJSONCoordinates(from: data),
           !coordinates.isEmpty {
            return coordinates
        }

        let coordinates = parseXMLCoordinates(from: data)
        if !coordinates.isEmpty {
            return coordinates
        }

        throw CoordinateImportError.noCoordinates
    }

    static func parseInline(_ text: String) -> [CLLocationCoordinate2D] {
        sanitized(parseTextCoordinates(from: text))
    }

    private static func decodedText(from data: Data) -> String? {
        String(data: data, encoding: .utf8)
            ?? String(data: data, encoding: .utf16)
            ?? String(data: data, encoding: .ascii)
    }

    private static func sanitized(_ coordinates: [CLLocationCoordinate2D]) -> [CLLocationCoordinate2D] {
        var result: [CLLocationCoordinate2D] = []
        for coordinate in coordinates where CLLocationCoordinate2DIsValid(coordinate) {
            if result.last.map(CoordinateSnapshot.init) == CoordinateSnapshot(coordinate) {
                continue
            }
            result.append(coordinate)
        }
        return result
    }

    private static func coordinate(
        first: Double,
        second: Double,
        order: CoordinateOrder
    ) -> CLLocationCoordinate2D? {
        let preferred: CLLocationCoordinate2D
        let fallback: CLLocationCoordinate2D

        switch order {
        case .latitudeLongitude:
            preferred = CLLocationCoordinate2D(latitude: first, longitude: second)
            fallback = CLLocationCoordinate2D(latitude: second, longitude: first)
        case .longitudeLatitude:
            preferred = CLLocationCoordinate2D(latitude: second, longitude: first)
            fallback = CLLocationCoordinate2D(latitude: first, longitude: second)
        }

        if CLLocationCoordinate2DIsValid(preferred) {
            return preferred
        }
        if CLLocationCoordinate2DIsValid(fallback) {
            return fallback
        }
        return nil
    }

    private static func parseJSONCoordinates(from data: Data) throws -> [CLLocationCoordinate2D] {
        let object = try JSONSerialization.jsonObject(with: data)
        return sanitized(coordinates(fromJSONObject: object, order: .latitudeLongitude))
    }

    private static func coordinates(
        fromJSONObject object: Any,
        order: CoordinateOrder
    ) -> [CLLocationCoordinate2D] {
        if let dictionary = object as? [String: Any] {
            if let latitude = numberValue(forAnyKey: ["latitude", "lat"], in: dictionary),
               let longitude = numberValue(forAnyKey: ["longitude", "lon", "lng"], in: dictionary),
               let coordinate = coordinate(first: latitude, second: longitude, order: .latitudeLongitude) {
                return [coordinate]
            }

            if let geometry = dictionary["geometry"] {
                return coordinates(fromJSONObject: geometry, order: order)
            }

            if let type = dictionary["type"] as? String {
                let loweredType = type.lowercased()
                if loweredType == "featurecollection",
                   let features = dictionary["features"] as? [Any] {
                    return features.flatMap { coordinates(fromJSONObject: $0, order: .longitudeLatitude) }
                }
                if loweredType == "geometrycollection",
                   let geometries = dictionary["geometries"] as? [Any] {
                    return geometries.flatMap { coordinates(fromJSONObject: $0, order: .longitudeLatitude) }
                }
                if let coordinateObject = dictionary["coordinates"] {
                    return coordinates(fromJSONObject: coordinateObject, order: .longitudeLatitude)
                }
            }

            return dictionary.values.flatMap { coordinates(fromJSONObject: $0, order: order) }
        }

        if let array = object as? [Any] {
            if array.count >= 2,
               let first = numericValue(array[0]),
               let second = numericValue(array[1]),
               let coordinate = coordinate(first: first, second: second, order: order) {
                return [coordinate]
            }

            return array.flatMap { coordinates(fromJSONObject: $0, order: order) }
        }

        return []
    }

    private static func numericValue(_ value: Any) -> Double? {
        if let number = value as? NSNumber {
            return number.doubleValue
        }
        if let string = value as? String {
            return Double(string.trimmingCharacters(in: .whitespacesAndNewlines))
        }
        return nil
    }

    private static func numberValue(forAnyKey keys: [String], in dictionary: [String: Any]) -> Double? {
        let keyedValues = Dictionary(uniqueKeysWithValues: dictionary.map { ($0.key.lowercased(), $0.value) })
        for key in keys {
            if let value = keyedValues[key],
               let number = numericValue(value) {
                return number
            }
        }
        return nil
    }

    private static func parseXMLCoordinates(from data: Data) -> [CLLocationCoordinate2D] {
        let collector = XMLCoordinateCollector()
        let parser = XMLParser(data: data)
        parser.delegate = collector
        guard parser.parse() else { return [] }
        return sanitized(collector.coordinates)
    }

    private final class XMLCoordinateCollector: NSObject, XMLParserDelegate {
        var coordinates: [CLLocationCoordinate2D] = []
        private var isCollectingKMLCoordinates = false
        private var kmlCoordinateBuffer = ""

        func parser(
            _ parser: XMLParser,
            didStartElement elementName: String,
            namespaceURI: String?,
            qualifiedName qName: String?,
            attributes attributeDict: [String: String] = [:]
        ) {
            let name = elementName.lowercased()
            if ["wpt", "trkpt", "rtept"].contains(name),
               let latitude = Double(attributeDict["lat"] ?? ""),
               let longitude = Double(attributeDict["lon"] ?? ""),
               let coordinate = CoordinateImportParser.coordinate(
                    first: latitude,
                    second: longitude,
                    order: .latitudeLongitude
               ) {
                coordinates.append(coordinate)
            } else if name == "coordinates" {
                isCollectingKMLCoordinates = true
                kmlCoordinateBuffer = ""
            }
        }

        func parser(_ parser: XMLParser, foundCharacters string: String) {
            if isCollectingKMLCoordinates {
                kmlCoordinateBuffer += string
            }
        }

        func parser(
            _ parser: XMLParser,
            didEndElement elementName: String,
            namespaceURI: String?,
            qualifiedName qName: String?
        ) {
            guard elementName.lowercased() == "coordinates" else { return }
            coordinates.append(contentsOf: CoordinateImportParser.parseKMLCoordinateText(kmlCoordinateBuffer))
            isCollectingKMLCoordinates = false
            kmlCoordinateBuffer = ""
        }
    }

    private static func parseKMLCoordinateText(_ text: String) -> [CLLocationCoordinate2D] {
        text
            .split(whereSeparator: { $0.isWhitespace })
            .compactMap { token -> CLLocationCoordinate2D? in
                let values = token
                    .split(separator: ",")
                    .compactMap { Double($0.trimmingCharacters(in: .whitespacesAndNewlines)) }
                guard values.count >= 2 else { return nil }
                return coordinate(first: values[0], second: values[1], order: .longitudeLatitude)
            }
    }

    private static func parseTextCoordinates(from text: String) -> [CLLocationCoordinate2D] {
        var coordinates: [CLLocationCoordinate2D] = []
        var headerIndices: (latitude: Int, longitude: Int)?

        for line in text.components(separatedBy: .newlines) {
            let trimmed = line.trimmingCharacters(in: .whitespacesAndNewlines)
            guard !trimmed.isEmpty else { continue }

            let fields = splitFields(trimmed)
            if headerIndices == nil,
               let detectedHeader = detectHeader(in: fields) {
                headerIndices = detectedHeader
                continue
            }

            if let headerIndices,
               fields.indices.contains(headerIndices.latitude),
               fields.indices.contains(headerIndices.longitude),
               let latitude = numbers(in: fields[headerIndices.latitude]).first,
               let longitude = numbers(in: fields[headerIndices.longitude]).first,
               let coordinate = coordinate(first: latitude, second: longitude, order: .latitudeLongitude) {
                coordinates.append(coordinate)
                continue
            }

            let values = numbers(in: trimmed)
            if values.count >= 2,
               let coordinate = coordinate(first: values[0], second: values[1], order: .latitudeLongitude) {
                coordinates.append(coordinate)
            }
        }

        return coordinates
    }

    private static func splitFields(_ line: String) -> [String] {
        line
            .split { character in
                character == "," ||
                character == ";" ||
                character == "\t"
            }
            .map { String($0).trimmingCharacters(in: .whitespacesAndNewlines) }
    }

    private static func detectHeader(in fields: [String]) -> (latitude: Int, longitude: Int)? {
        let lowered = fields.map { $0.lowercased() }
        guard let latitude = lowered.firstIndex(where: { $0 == "lat" || $0 == "latitude" }),
              let longitude = lowered.firstIndex(where: { $0 == "lon" || $0 == "lng" || $0 == "long" || $0 == "longitude" }) else {
            return nil
        }
        return (latitude, longitude)
    }

    private static func numbers(in text: String) -> [Double] {
        let pattern = #"[-+]?(?:\d+(?:\.\d*)?|\.\d+)"#
        guard let regex = try? NSRegularExpression(pattern: pattern) else { return [] }
        let range = NSRange(text.startIndex..<text.endIndex, in: text)
        return regex.matches(in: text, range: range).compactMap { match in
            guard let matchRange = Range(match.range, in: text) else { return nil }
            return Double(text[matchRange])
        }
    }
}

// MARK: - Bookmark Model

struct LocationBookmark: Identifiable, Codable {
    var id: UUID = UUID()
    var name: String
    var latitude: Double
    var longitude: Double

    var coordinate: CLLocationCoordinate2D {
        CLLocationCoordinate2D(latitude: latitude, longitude: longitude)
    }
}

// MARK: - Search Completer

@MainActor
final class LocationSearchCompleter: NSObject, ObservableObject, MKLocalSearchCompleterDelegate {
    @Published var results: [MKLocalSearchCompletion] = []
    private let completer = MKLocalSearchCompleter()

    override init() {
        super.init()
        completer.delegate = self
        completer.resultTypes = [.address, .pointOfInterest]
    }

    func update(query: String) {
        guard !query.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty else {
            results = []
            completer.queryFragment = ""
            return
        }
        completer.queryFragment = query
    }

    nonisolated func completerDidUpdateResults(_ completer: MKLocalSearchCompleter) {
        let results = completer.results
        Task { @MainActor in self.results = results }
    }

    nonisolated func completer(_ completer: MKLocalSearchCompleter, didFailWithError error: Error) {
        Task { @MainActor in self.results = [] }
    }
}

struct LocationSimulationView: View {
    @State private var coordinate: CLLocationCoordinate2D?
    @State private var position: MapCameraPosition = .userLocation(fallback: .automatic)

    @State private var backgroundTaskID: UIBackgroundTaskIdentifier = .invalid
    @State private var resendTimer: Timer?
    @State private var routeLoadTask: Task<Void, Never>?
    @State private var routeSpeedPrefetchTask: Task<Void, Never>?
    @State private var routePlaybackTask: Task<Void, Never>?
    @State private var isBusy = false
    @State private var isLoadingRoute = false
    @State private var isPrefetchingRouteSpeeds = false
    @State private var isImportingCoordinates = false
    @State private var showAlert = false
    @State private var alertTitle = ""
    @State private var alertMessage = ""

    @State private var searchText = ""
    @StateObject private var searchCompleter = LocationSearchCompleter()
    @State private var showCoordinateImporter = false
    @State private var showRouteSearch = false
    @State private var routeStartSelection: RouteSearchSelection?
    @State private var routeEndSelection: RouteSearchSelection?
    @State private var routePlan: RouteSimulationPlan?
    @State private var routePolyline: MKPolyline?
    @State private var routePlaybackSamples: [RoutePlaybackSample] = []
    @State private var routeBusStops: [CLLocationCoordinate2D] = []
    @State private var visibleLatitudeDelta: CLLocationDegrees = 0.05
    @State private var routePlaybackCoordinate: CLLocationCoordinate2D?
    @State private var simulatedCoordinate: CLLocationCoordinate2D?
    @State private var routeRequestID = UUID()

    private static let routeDurationFormatter: DateComponentsFormatter = {
        let formatter = DateComponentsFormatter()
        formatter.allowedUnits = [.hour, .minute]
        formatter.unitsStyle = .abbreviated
        formatter.zeroFormattingBehavior = .dropAll
        return formatter
    }()

    // Bookmarks
    @State private var bookmarks: [LocationBookmark] = []
    @State private var showBookmarks = false
    @State private var showSaveBookmark = false
    @State private var newBookmarkName = ""

    // Map appearance
    @AppStorage("mapDisplayMode") private var mapDisplayModeRawValue: String = MapDisplayMode.standard.rawValue
    @AppStorage("mapShowsTraffic") private var mapShowsTraffic: Bool = false
    @AppStorage("mapPointsOfInterestMode") private var mapPointsOfInterestRawValue: String = MapPointsOfInterestMode.all.rawValue

    // Speed profile
    @AppStorage("routeSpeedProfile") private var speedProfileRawValue: String = SpeedProfile.driving.rawValue
    @AppStorage("routeSpeedCustomKmh") private var customSpeedKmh: Double = 30
    @State private var showCustomSpeedPrompt = false
    @State private var customSpeedInput = ""
    @State private var lastFallbackSpeed: CLLocationSpeed = RouteSimulationDefaults.importedRouteFallbackSpeedMetersPerSecond
    @State private var isImportedRoute = false

    private var mapDisplayMode: MapDisplayMode {
        MapDisplayMode(rawValue: mapDisplayModeRawValue) ?? .standard
    }

    private var mapPointsOfInterestMode: MapPointsOfInterestMode {
        MapPointsOfInterestMode(rawValue: mapPointsOfInterestRawValue) ?? .all
    }

    private var mapStyle: MapStyle {
        switch mapDisplayMode {
        case .standard:
            return .standard(
                elevation: .realistic,
                pointsOfInterest: mapPointsOfInterestMode.categories,
                showsTraffic: mapShowsTraffic
            )
        case .satellite:
            return .imagery(elevation: .realistic)
        case .hybrid:
            return .hybrid(
                elevation: .realistic,
                pointsOfInterest: mapPointsOfInterestMode.categories,
                showsTraffic: mapShowsTraffic
            )
        }
    }

    /// Bus stop markers to actually draw on the map. Two adjustments on top of
    /// the raw fetched `routeBusStops`:
    ///
    /// - Honors "Points of Interest: Hidden" — the stops are drawn as our own
    ///   Marker overlay, not Apple's native POI layer, so Apple's
    ///   `pointsOfInterest` style option (set from the same picker) has no
    ///   effect on them; that had to be handled here explicitly.
    /// - Thins markers as the map zooms out, since a bus route easily has
    ///   dozens of stops that overlap into an unreadable stack of pins at a
    ///   city-wide zoom. Spacing scales with the visible latitude span so
    ///   roughly the same number of markers are visible on screen at any
    ///   zoom level; the full-fidelity `routeBusStops` list (used for dwell
    ///   pacing during playback) is untouched.
    private var displayedBusStops: [CLLocationCoordinate2D] {
        guard speedProfile == .bus, mapPointsOfInterestMode != .hidden else { return [] }
        let visibleSpanMeters = visibleLatitudeDelta * 111_320
        let minimumSpacing = max(
            SpeedProfileSettings.busStopSnapRadius,
            visibleSpanMeters / 25
        )
        return dedupedBusStops(routeBusStops, radius: minimumSpacing)
    }

    private var speedProfile: SpeedProfile {
        SpeedProfile(rawValue: speedProfileRawValue) ?? .driving
    }

    private var speedSettings: SpeedProfileSettings {
        SpeedProfileSettings(
            profile: speedProfile,
            customSpeedMetersPerSecond: max(customSpeedKmh, 1) / 3.6
        )
    }

    private var speedProfileDetailText: String {
        switch speedProfile {
        case .custom:
            return String(format: "%.0f km/h", max(customSpeedKmh, 1))
        case .bus:
            return "Road speed, pauses at stops"
        case .driving:
            return "Road speed limits"
        default:
            if let fixed = speedProfile.fixedSpeedMetersPerSecond {
                return String(format: "%.0f km/h", fixed * 3.6)
            }
            return ""
        }
    }

    private var pairingFilePath: String {
        PairingFileStore.prepareURL().path
    }

    private var pairingExists: Bool {
        FileManager.default.fileExists(atPath: pairingFilePath)
    }

    private var deviceIP: String {
        DeviceConnectionContext.targetIPAddress
    }

    private var routeStartCoordinate: CLLocationCoordinate2D? {
        routeStartSelection?.coordinate
    }

    private var routeEndCoordinate: CLLocationCoordinate2D? {
        routeEndSelection?.coordinate
    }

    private var hasActiveSimulation: Bool {
        simulatedCoordinate != nil || routePlaybackTask != nil
    }

    private var isRouteRunning: Bool {
        routePlaybackTask != nil
    }

    private var hasRouteContext: Bool {
        routeStartSelection != nil ||
        routeEndSelection != nil ||
        routePlan != nil ||
        isLoadingRoute ||
        isPrefetchingRouteSpeeds ||
        routePlaybackCoordinate != nil
    }

    private var routeSummaryText: String? {
        guard let routePlan else { return nil }
        let distanceText = Measurement(
            value: routePlan.distance / 1000,
            unit: UnitLength.kilometers
        ).formatted(.measurement(width: .abbreviated, usage: .road))

        let travelTime: TimeInterval
        if let fixed = speedProfile.fixedSpeedMetersPerSecond {
            travelTime = routePlan.distance / fixed
        } else if speedProfile == .custom {
            travelTime = routePlan.distance / speedSettings.customSpeedMetersPerSecond
        } else {
            travelTime = routePlan.expectedTravelTime
        }

        let durationText = Self.routeDurationFormatter.string(from: travelTime)
        if let durationText, !durationText.isEmpty {
            return "\(distanceText) • ETA \(durationText)"
        }
        return distanceText
    }

    private var routeStatusText: String {
        if isLoadingRoute {
            return "Calculating route…"
        }
        if isPrefetchingRouteSpeeds {
            return "Prefetching road speeds…"
        }
        if routePlan != nil {
            return "Route ready."
        }
        if routeStartSelection != nil || routeEndSelection != nil {
            return "Pick both route endpoints to build the drive."
        }
        return "Plan a route from the toolbar."
    }

    private var routeAttributionLink: some View {
        Link(
            "Speed limit data © OpenStreetMap contributors (ODbL)",
            destination: OpenStreetMapSpeedLimitService.copyrightURL
        )
        .font(.caption2)
        .foregroundStyle(.secondary)
    }

    private var searchResultsListBase: some View {
        List(searchCompleter.results.prefix(5), id: \.self) { result in
            Button {
                selectSearchResult(result)
            } label: {
                VStack(alignment: .leading, spacing: 2) {
                    Text(result.title)
                        .font(.subheadline)
                    if !result.subtitle.isEmpty {
                        Text(result.subtitle)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                }
            }
        }
        .listStyle(.plain)
        .frame(maxHeight: 350)
        .scrollDisabled(true)
        .padding(.horizontal, 16)
        .padding(.top, 8)
    }

    @ViewBuilder
    private var searchResultsList: some View {
        if #available(iOS 26, *) {
            searchResultsListBase
                .glassEffect(in: .rect(cornerRadius: 12))
        } else {
            searchResultsListBase
        }
    }

    var body: some View {
        ZStack(alignment: .bottom) {
            MapReader { proxy in
                Map(position: $position) {
                    if hasRouteContext {
                        if let routePolyline {
                            MapPolyline(routePolyline)
                                .stroke(.blue.opacity(0.8), lineWidth: 5)
                        }
                        ForEach(Array(displayedBusStops.enumerated()), id: \.offset) { _, stop in
                            Marker("Bus Stop", systemImage: "bus.fill", coordinate: stop)
                                .tint(.orange)
                        }
                        if let routeStartCoordinate {
                            Marker("Start", coordinate: routeStartCoordinate)
                                .tint(.green)
                        }
                        if let routeEndCoordinate {
                            Marker("End", coordinate: routeEndCoordinate)
                                .tint(.red)
                        }
                        if let routePlaybackCoordinate {
                            Marker("Current", coordinate: routePlaybackCoordinate)
                                .tint(.blue)
                        }
                    } else if let coordinate {
                        Marker("Pin", coordinate: coordinate)
                            .tint(.red)
                    }
                }
                .mapStyle(mapStyle)
                .onTapGesture { point in
                    if let loc = proxy.convert(point, from: .local) {
                        applySelection(loc)
                    }
                }
                .mapControls {
                    MapCompass()
                }
                .onMapCameraChange(frequency: .onEnd) { context in
                    visibleLatitudeDelta = context.region.span.latitudeDelta
                }
            }
                .ignoresSafeArea()
                .onChange(of: coordinate.map(CoordinateSnapshot.init)) { _, new in
                    if let new {
                        position = .region(
                            MKCoordinateRegion(
                                center: new.coordinate,
                                latitudinalMeters: 1000,
                                longitudinalMeters: 1000
                            )
                        )
                    }
                }

            VStack(spacing: 0) {
                if !searchCompleter.results.isEmpty {
                    searchResultsList
                }

                Spacer()

                VStack(spacing: 12) {
                    if isImportingCoordinates {
                        ProgressView("Importing coordinates…")
                            .font(.footnote)
                    }

                    if hasRouteContext {
                        routeControls
                    } else {
                        pinControls
                    }
                }
                .padding(.bottom, 24)
                .padding(.horizontal, 16)
                .padding(.horizontal, 16)
            }
        }
        .navigationBarTitleDisplayMode(.inline)
        .toolbar {
            ToolbarItemGroup(placement: .topBarLeading) {
                mapStyleMenu

                Button {
                    showBookmarks = true
                } label: {
                    Image(systemName: "bookmark.fill")
                }

                Button {
                    showRouteSearch = true
                } label: {
                    Image(systemName: "point.topleft.down.curvedto.point.bottomright.up")
                }
                .disabled(isBusy || isRouteRunning)

                Button {
                    showCoordinateImporter = true
                } label: {
                    Image(systemName: "square.and.arrow.down")
                }
                .disabled(isBusy || isRouteRunning || isImportingCoordinates)
                .accessibilityLabel("Import Coordinates")
            }
            ToolbarItem(placement: .topBarTrailing) {
                TextField("Search location...", text: $searchText)
                    .padding(.leading, 6)
                    .autocorrectionDisabled()
                    .submitLabel(.go)
                    .onChange(of: searchText) { _, newValue in
                        searchCompleter.update(query: newValue)
                    }
                    .onSubmit {
                        applyCoordinatesFromSearchText()
                    }
            }
        }
        .alert(alertTitle, isPresented: $showAlert) {
            Button("OK", role: .cancel) { }
        } message: {
            Text(alertMessage)
        }
        .alert("Save Bookmark", isPresented: $showSaveBookmark) {
            TextField("Name", text: $newBookmarkName)
            Button("Save") { addBookmark() }
            Button("Cancel", role: .cancel) { newBookmarkName = "" }
        } message: {
            Text("Enter a name for this location.")
        }
        .alert("Custom Speed", isPresented: $showCustomSpeedPrompt) {
            TextField("Speed (km/h)", text: $customSpeedInput)
                .keyboardType(.decimalPad)
            Button("Set") { applyCustomSpeedInput() }
            Button("Cancel", role: .cancel) { }
        } message: {
            Text("Enter a playback speed in km/h.")
        }
        .sheet(isPresented: $showBookmarks) {
            BookmarksView(bookmarks: $bookmarks) { bookmark in
                applySelection(bookmark.coordinate)
                showBookmarks = false
            } onDelete: { offsets in
                bookmarks.remove(atOffsets: offsets)
                saveBookmarks()
            }
        }
        .sheet(isPresented: $showRouteSearch) {
            RouteSearchSheet(
                initialStart: routeStartSelection,
                initialEnd: routeEndSelection
            ) { startSelection, endSelection in
                routeStartSelection = startSelection
                routeEndSelection = endSelection
                refreshRoute()
            }
        }
        .fileImporter(
            isPresented: $showCoordinateImporter,
            allowedContentTypes: CoordinateImportParser.supportedContentTypes,
            allowsMultipleSelection: false
        ) { result in
            importCoordinates(result)
        }
        .onAppear {
            loadBookmarks()
        }
        .onDisappear {
            routeLoadTask?.cancel()
            routeLoadTask = nil
            routeSpeedPrefetchTask?.cancel()
            routeSpeedPrefetchTask = nil
            cancelRoutePlayback(resetMarker: true)
            stopResendLoop()
            if backgroundTaskID != .invalid {
                BackgroundLocationManager.shared.requestStop()
            }
            endBackgroundTask()
        }
    }

    // MARK: - Bookmarks

    private func loadBookmarks() {
        guard let data = UserDefaults.standard.data(forKey: "locationBookmarks"),
              let decoded = try? JSONDecoder().decode([LocationBookmark].self, from: data) else { return }
        bookmarks = decoded
    }

    private func saveBookmarks() {
        if let data = try? JSONEncoder().encode(bookmarks) {
            UserDefaults.standard.set(data, forKey: "locationBookmarks")
        }
    }

    private func addBookmark() {
        guard let coord = coordinate else { return }
        let name = newBookmarkName.trimmingCharacters(in: .whitespacesAndNewlines)
        let bookmark = LocationBookmark(
            name: name.isEmpty ? String(format: "%.4f, %.4f", coord.latitude, coord.longitude) : name,
            latitude: coord.latitude,
            longitude: coord.longitude
        )
        bookmarks.append(bookmark)
        saveBookmarks()
        newBookmarkName = ""
    }

    private func setRoutePlan(_ plan: RouteSimulationPlan?) {
        routePlan = plan
        routePolyline = plan.flatMap { makeRoutePolyline(for: $0.displayCoordinates) }
    }

    private func makeRoutePolyline(for coordinates: [CLLocationCoordinate2D]) -> MKPolyline? {
        guard coordinates.count > 1 else { return nil }
        return coordinates.withUnsafeBufferPointer { buffer in
            guard let baseAddress = buffer.baseAddress else { return nil }
            return MKPolyline(coordinates: baseAddress, count: buffer.count)
        }
    }

    // MARK: - Location

    private func selectSearchResult(_ result: MKLocalSearchCompletion) {
        searchText = ""
        searchCompleter.results = []

        let request = MKLocalSearch.Request(completion: result)
        MKLocalSearch(request: request).start { response, _ in
            if let item = response?.mapItems.first {
                applySelection(item.placemark.coordinate)
            }
        }
    }

    private func applyCoordinatesFromSearchText() {
        let importedCoordinates = CoordinateImportParser.parseInline(searchText)
        guard !importedCoordinates.isEmpty else { return }

        searchText = ""
        searchCompleter.results = []
        applyImportedCoordinates(importedCoordinates, sourceName: "Imported")
    }

    private func importCoordinates(_ result: Result<[URL], Error>) {
        switch result {
        case .success(let urls):
            guard let url = urls.first else { return }
            let sourceName = url.deletingPathExtension().lastPathComponent
            isImportingCoordinates = true

            Task {
                do {
                    let coordinates = try await Task.detached(priority: .userInitiated) {
                        try CoordinateImportParser.parse(url: url)
                    }.value

                    await MainActor.run {
                        isImportingCoordinates = false
                        applyImportedCoordinates(
                            coordinates,
                            sourceName: sourceName.isEmpty ? "Imported" : sourceName
                        )
                    }
                } catch {
                    await MainActor.run {
                        isImportingCoordinates = false
                        showImportError(error)
                    }
                }
            }
        case .failure(let error):
            showImportError(error)
        }
    }

    private func applyImportedCoordinates(
        _ importedCoordinates: [CLLocationCoordinate2D],
        sourceName: String
    ) {
        guard !isRouteRunning else { return }

        let coordinates = importedCoordinates.filter(CLLocationCoordinate2DIsValid)
        guard let firstCoordinate = coordinates.first else {
            showImportError(CoordinateImportError.noCoordinates)
            return
        }

        if coordinates.count == 1 {
            applySelection(firstCoordinate)
            return
        }

        routeLoadTask?.cancel()
        routeLoadTask = nil
        routeSpeedPrefetchTask?.cancel()
        routeSpeedPrefetchTask = nil
        routeRequestID = UUID()
        setRoutePlan(nil)
        routePlaybackSamples = []
        routeBusStops = []
        routePlaybackCoordinate = nil
        isLoadingRoute = false
        isPrefetchingRouteSpeeds = false
        coordinate = nil

        let displayCoordinates = sampledRouteCoordinates(
            from: coordinates,
            targetDistance: adaptiveSamplingDistance(for: coordinates)
        )

        guard displayCoordinates.count > 1,
              let lastCoordinate = displayCoordinates.last else {
            applySelection(firstCoordinate)
            return
        }

        let distance = distanceAlong(displayCoordinates)
        let fallbackSpeed = RouteSimulationDefaults.importedRouteFallbackSpeedMetersPerSecond
        isImportedRoute = true
        routeStartSelection = RouteSearchSelection(title: "\(sourceName) Start", coordinate: firstCoordinate)
        routeEndSelection = RouteSearchSelection(title: "\(sourceName) End", coordinate: lastCoordinate)
        setRoutePlan(RouteSimulationPlan(
            displayCoordinates: displayCoordinates,
            distance: distance,
            expectedTravelTime: distance / fallbackSpeed
        ))

        if let routePolyline {
            position = .rect(routePolyline.boundingMapRect)
        }

        let requestID = UUID()
        routeRequestID = requestID
        isPrefetchingRouteSpeeds = true
        lastFallbackSpeed = fallbackSpeed
        let settings = speedSettings
        routeSpeedPrefetchTask = Task.detached(priority: .utility) {
            let prefetch = await prefetchRoutePlaybackSamples(
                displayCoordinates: displayCoordinates,
                fallbackSpeedMetersPerSecond: fallbackSpeed,
                speedSettings: settings,
                onPartialBusStops: { stops in
                    Task { @MainActor in
                        guard routeRequestID == requestID else { return }
                        routeBusStops = stops
                    }
                }
            )
            guard !Task.isCancelled else { return }
            await MainActor.run {
                guard routeRequestID == requestID else { return }
                routePlaybackSamples = prefetch.samples
                routeBusStops = prefetch.busStops
                isPrefetchingRouteSpeeds = false
            }
        }
    }

    private func showImportError(_ error: Error) {
        alertTitle = "Import Failed"
        alertMessage = error.localizedDescription
        showAlert = true
    }

    @ViewBuilder
    private var pinControls: some View {
        if let coord = coordinate {
            Text(String(format: "%.6f, %.6f", coord.latitude, coord.longitude))
                .font(.footnote.monospaced())
                .foregroundStyle(.secondary)

            HStack(spacing: 12) {
                Button("Stop", action: clear)
                    .buttonStyle(.bordered)
                    .tint(.red)
                    .disabled(!pairingExists || isBusy || !hasActiveSimulation)

                Button("Simulate Location", action: simulate)
                    .buttonStyle(.borderedProminent)
                    .disabled(!pairingExists || isBusy || isLoadingRoute)

                Button {
                    showSaveBookmark = true
                } label: {
                    Image(systemName: "bookmark")
                }
                .buttonStyle(.bordered)
                .tint(.blue)
                .disabled(isRouteRunning)
            }
        } else {
            Text("Tap map to drop pin")
                .font(.subheadline)
                .foregroundStyle(.secondary)
        }
    }

    private var routeControls: some View {
        VStack(spacing: 10) {
            Text(routeStatusText)
                .font(.footnote)
                .foregroundStyle(.secondary)

            if isLoadingRoute || isPrefetchingRouteSpeeds {
                ProgressView()
                    .controlSize(.small)
            } else if let routeSummaryText {
                Text(routeSummaryText)
                    .font(.footnote.monospaced())
                    .foregroundStyle(.secondary)
            }

            routeAttributionLink

            speedProfileMenu

            HStack(spacing: 12) {
                Button("Stop", action: clear)
                    .buttonStyle(.bordered)
                    .tint(.red)
                    .disabled(!pairingExists || isBusy || !hasActiveSimulation)

                Button("Play Route", action: simulateRoute)
                    .buttonStyle(.borderedProminent)
                    .disabled(
                        !pairingExists ||
                        isBusy ||
                        isLoadingRoute ||
                        isPrefetchingRouteSpeeds ||
                        routePlan == nil ||
                        routePlaybackSamples.isEmpty
                    )

                Button("Reset", action: resetRouteSelection)
                    .buttonStyle(.bordered)
                    .disabled(isBusy || isRouteRunning)
            }
        }
    }

    private var mapStyleMenu: some View {
        Menu {
            Picker("Map Type", selection: $mapDisplayModeRawValue) {
                ForEach(MapDisplayMode.allCases) { mode in
                    Label(mode.title, systemImage: mode.systemImage)
                        .tag(mode.rawValue)
                }
            }

            if mapDisplayMode.supportsOverlays {
                Toggle(isOn: $mapShowsTraffic) {
                    Label("Traffic", systemImage: "car.2.fill")
                }

                Picker("Points of Interest", selection: $mapPointsOfInterestRawValue) {
                    ForEach(MapPointsOfInterestMode.allCases) { mode in
                        Label(mode.title, systemImage: mode.systemImage)
                            .tag(mode.rawValue)
                    }
                }
                .pickerStyle(.menu)
            }
        } label: {
            Image(systemName: "map.fill")
        }
        .accessibilityLabel("Map style: \(mapDisplayMode.title)")
    }

    private var speedProfileMenu: some View {
        Menu {
            ForEach(SpeedProfile.allCases) { profile in
                Button {
                    selectSpeedProfile(profile)
                } label: {
                    if profile == speedProfile {
                        Label(profile.title, systemImage: "checkmark")
                    } else {
                        Label(profile.title, systemImage: profile.systemImage)
                    }
                }
            }
        } label: {
            HStack(spacing: 6) {
                Image(systemName: speedProfile.systemImage)
                Text(speedProfile.title)
                    .font(.subheadline.weight(.medium))
                if !speedProfileDetailText.isEmpty {
                    Text(speedProfileDetailText)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                Image(systemName: "chevron.up.chevron.down")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            .padding(.horizontal, 12)
            .padding(.vertical, 7)
        }
        .buttonStyle(.bordered)
        .tint(.blue)
        .disabled(isBusy || isRouteRunning || isPrefetchingRouteSpeeds)
        .accessibilityLabel("Playback speed: \(speedProfile.title)")
    }

    private func selectSpeedProfile(_ profile: SpeedProfile) {
        if profile == .custom {
            customSpeedInput = String(format: "%.0f", max(customSpeedKmh, 1))
            showCustomSpeedPrompt = true
            return
        }
        guard profile != speedProfile else { return }
        let directionsChanged = profile.prefersWalkingDirections != speedProfile.prefersWalkingDirections
        speedProfileRawValue = profile.rawValue

        // Searched routes can be re-planned along footpaths; imported routes keep
        // their exact geometry and only get their pacing rebuilt.
        if directionsChanged,
           !isImportedRoute,
           routeStartSelection != nil,
           routeEndSelection != nil {
            refreshRoute()
        } else {
            rebuildPlaybackSamplesForCurrentRoute()
        }
    }

    private func applyCustomSpeedInput() {
        let normalized = customSpeedInput.replacingOccurrences(of: ",", with: ".")
        guard let value = Double(normalized), value > 0 else {
            alertTitle = "Invalid Speed"
            alertMessage = "Enter a speed above 0 km/h."
            showAlert = true
            return
        }
        customSpeedKmh = min(value, 1000)
        speedProfileRawValue = SpeedProfile.custom.rawValue
        rebuildPlaybackSamplesForCurrentRoute()
    }

    private func rebuildPlaybackSamplesForCurrentRoute() {
        guard let routePlan, !isRouteRunning else { return }

        routeSpeedPrefetchTask?.cancel()
        let requestID = UUID()
        routeRequestID = requestID
        isPrefetchingRouteSpeeds = true

        let displayCoordinates = routePlan.displayCoordinates
        let fallbackSpeed = lastFallbackSpeed
        let settings = speedSettings

        routeSpeedPrefetchTask = Task.detached(priority: .utility) {
            let prefetch = await prefetchRoutePlaybackSamples(
                displayCoordinates: displayCoordinates,
                fallbackSpeedMetersPerSecond: fallbackSpeed,
                speedSettings: settings,
                onPartialBusStops: { stops in
                    Task { @MainActor in
                        guard routeRequestID == requestID else { return }
                        routeBusStops = stops
                    }
                }
            )
            guard !Task.isCancelled else { return }
            await MainActor.run {
                guard routeRequestID == requestID else { return }
                routePlaybackSamples = prefetch.samples
                routeBusStops = prefetch.busStops
                isPrefetchingRouteSpeeds = false
            }
        }
    }

    private func simulate() {
        guard pairingExists, let coord = coordinate, !isBusy else { return }
        runLocationCommand(
            errorTitle: "Simulation Failed",
            errorMessage: { code in
                "Could not simulate location (error \(code)). Make sure the device is connected and the DDI is mounted."
            },
            operation: { locationUpdateCode(for: coord) }
        ) {
            routePlaybackCoordinate = nil
            beginBackgroundTask()
            startResendLoop(with: coord)
            BackgroundLocationManager.shared.requestStart()
        }
    }

    private func simulateRoute() {
        guard pairingExists,
              routePlan != nil,
              let firstCoordinate = routePlaybackSamples.first?.coordinate,
              !isBusy else {
            return
        }
        stopResendLoop()
        cancelRoutePlayback(resetMarker: false)
        runLocationCommand(
            errorTitle: "Route Simulation Failed",
            errorMessage: { code in
                "Could not start route simulation (error \(code)). Make sure the device is connected and the DDI is mounted."
            },
            operation: { locationUpdateCode(for: firstCoordinate) }
        ) {
            beginBackgroundTask()
            BackgroundLocationManager.shared.requestStart()
            simulatedCoordinate = nil
            routePlaybackCoordinate = firstCoordinate
            startRoutePlayback()
        }
    }

    private func runLocationCommand(
        errorTitle: String,
        errorMessage: @escaping (Int32) -> String,
        operation: @escaping () -> Int32,
        onSuccess: @escaping () -> Void
    ) {
        isBusy = true
        LocationSimulationCommandQueue.shared.async {
            let code = operation()
            DispatchQueue.main.async {
                isBusy = false
                if code == 0 {
                    onSuccess()
                } else {
                    alertTitle = errorTitle
                    alertMessage = errorMessage(code)
                    showAlert = true
                }
            }
        }
    }

    private func clear() {
        guard pairingExists, !isBusy else { return }
        routeLoadTask?.cancel()
        routeLoadTask = nil
        routeSpeedPrefetchTask?.cancel()
        routeSpeedPrefetchTask = nil
        cancelRoutePlayback(resetMarker: true)
        stopResendLoop()
        runLocationCommand(
            errorTitle: "Clear Failed",
            errorMessage: { code in "Could not clear simulated location (error \(code))." },
            operation: clear_simulated_location
        ) {
            endBackgroundTask()
            BackgroundLocationManager.shared.requestStop()
        }
    }

    private func beginBackgroundTask() {
        guard backgroundTaskID == .invalid else { return }
        backgroundTaskID = UIApplication.shared.beginBackgroundTask { endBackgroundTask() }
    }

    private func endBackgroundTask() {
        guard backgroundTaskID != .invalid else { return }
        UIApplication.shared.endBackgroundTask(backgroundTaskID)
        backgroundTaskID = .invalid
    }

    private func startResendLoop(with coordinate: CLLocationCoordinate2D) {
        simulatedCoordinate = coordinate
        resendTimer?.invalidate()
        resendTimer = Timer.scheduledTimer(withTimeInterval: 4, repeats: true) { _ in
            guard let simulatedCoordinate else { return }
            LocationSimulationCommandQueue.shared.async {
                _ = locationUpdateCode(for: simulatedCoordinate)
            }
        }
    }

    private func stopResendLoop() {
        resendTimer?.invalidate()
        resendTimer = nil
        simulatedCoordinate = nil
    }

    private func cancelRoutePlayback(resetMarker: Bool) {
        routePlaybackTask?.cancel()
        routePlaybackTask = nil
        if resetMarker {
            routePlaybackCoordinate = nil
        }
    }

    private func applySelection(_ coordinate: CLLocationCoordinate2D) {
        guard !isRouteRunning else { return }
        if hasRouteContext {
            resetRouteSelection()
        }
        self.coordinate = coordinate
    }

    private func resetRouteSelection() {
        routeLoadTask?.cancel()
        routeLoadTask = nil
        routeSpeedPrefetchTask?.cancel()
        routeSpeedPrefetchTask = nil
        routeRequestID = UUID()
        setRoutePlan(nil)
        routeStartSelection = nil
        routeEndSelection = nil
        routePlaybackSamples = []
        routeBusStops = []
        routePlaybackCoordinate = nil
        isLoadingRoute = false
        isPrefetchingRouteSpeeds = false
    }

    private func refreshRoute() {
        routeLoadTask?.cancel()
        routeSpeedPrefetchTask?.cancel()
        setRoutePlan(nil)
        routePlaybackSamples = []
        routeBusStops = []
        isImportedRoute = false

        guard let routeStart = routeStartSelection?.coordinate,
              let routeEnd = routeEndSelection?.coordinate else {
            isLoadingRoute = false
            isPrefetchingRouteSpeeds = false
            return
        }

        let requestID = UUID()
        routeRequestID = requestID
        isLoadingRoute = true
        isPrefetchingRouteSpeeds = false

        let request = MKDirections.Request()
        request.source = MKMapItem(placemark: MKPlacemark(coordinate: routeStart))
        request.destination = MKMapItem(placemark: MKPlacemark(coordinate: routeEnd))
        request.requestsAlternateRoutes = false
        request.transportType = speedProfile.prefersWalkingDirections ? .walking : .automobile

        routeLoadTask = Task {
            do {
                let response = try await MKDirections(request: request).calculate()
                guard !Task.isCancelled else { return }
                guard let route = response.routes.first else {
                    throw NSError(
                        domain: "RouteSimulation",
                        code: -1,
                        userInfo: [NSLocalizedDescriptionKey: "No drivable route was returned."]
                    )
                }

                let routeCoordinates = route.polyline.coordinateArray
                let displayCoordinates = sampledRouteCoordinates(
                    from: routeCoordinates,
                    targetDistance: adaptiveSamplingDistance(for: routeCoordinates)
                )
                let routePlan = RouteSimulationPlan(
                    displayCoordinates: displayCoordinates,
                    distance: route.distance,
                    expectedTravelTime: route.expectedTravelTime
                )

                await MainActor.run {
                    guard routeRequestID == requestID else { return }
                    self.setRoutePlan(routePlan)
                    isLoadingRoute = false
                    isPrefetchingRouteSpeeds = true
                    if let routePolyline {
                        position = .rect(routePolyline.boundingMapRect)
                    }
                }

                let fallbackSpeed = route.expectedTravelTime > 0
                    ? route.distance / route.expectedTravelTime
                    : 13.4

                await MainActor.run {
                    guard routeRequestID == requestID else { return }
                    lastFallbackSpeed = fallbackSpeed
                    let settings = speedSettings
                    routeSpeedPrefetchTask?.cancel()
                    routeSpeedPrefetchTask = Task.detached(priority: .utility) {
                        let prefetch = await prefetchRoutePlaybackSamples(
                            displayCoordinates: displayCoordinates,
                            fallbackSpeedMetersPerSecond: fallbackSpeed,
                            speedSettings: settings,
                            onPartialBusStops: { stops in
                                Task { @MainActor in
                                    guard routeRequestID == requestID else { return }
                                    routeBusStops = stops
                                }
                            }
                        )
                        guard !Task.isCancelled else { return }
                        await MainActor.run {
                            guard routeRequestID == requestID else { return }
                            routePlaybackSamples = prefetch.samples
                            routeBusStops = prefetch.busStops
                            isPrefetchingRouteSpeeds = false
                        }
                    }
                }
            } catch is CancellationError {
                await MainActor.run {
                    guard routeRequestID == requestID else { return }
                    isLoadingRoute = false
                    isPrefetchingRouteSpeeds = false
                }
            } catch {
                await MainActor.run {
                    guard routeRequestID == requestID else { return }
                    isLoadingRoute = false
                    isPrefetchingRouteSpeeds = false
                    alertTitle = "Route Failed"
                    alertMessage = error.localizedDescription
                    showAlert = true
                }
            }
        }
    }

    private func startRoutePlayback() {
        routePlaybackTask = Task {
            var lastSuccessfulCoordinate = routePlaybackSamples.first?.coordinate

            for sample in routePlaybackSamples.dropFirst() {
                try? await Task.sleep(for: .seconds(sample.delayFromPrevious))
                guard !Task.isCancelled else { return }

                let code = await sendLocationUpdate(for: sample.coordinate)
                guard code == 0 else {
                    await MainActor.run {
                        routePlaybackTask = nil
                        routePlaybackCoordinate = lastSuccessfulCoordinate
                        if let lastSuccessfulCoordinate {
                            startResendLoop(with: lastSuccessfulCoordinate)
                        }
                        alertTitle = "Route Simulation Failed"
                        alertMessage = "Could not continue route simulation (error \(code))."
                        showAlert = true
                    }
                    return
                }

                lastSuccessfulCoordinate = sample.coordinate
                await MainActor.run {
                    routePlaybackCoordinate = sample.coordinate
                }
            }

            await MainActor.run {
                routePlaybackTask = nil
                if let lastSuccessfulCoordinate {
                    routePlaybackCoordinate = lastSuccessfulCoordinate
                    startResendLoop(with: lastSuccessfulCoordinate)
                }
            }
        }
    }

    private func sendLocationUpdate(for coordinate: CLLocationCoordinate2D) async -> Int32 {
        await withCheckedContinuation { continuation in
            LocationSimulationCommandQueue.shared.async {
                continuation.resume(returning: locationUpdateCode(for: coordinate))
            }
        }
    }

    private func locationUpdateCode(for coordinate: CLLocationCoordinate2D) -> Int32 {
        simulate_location(deviceIP, coordinate.latitude, coordinate.longitude, pairingFilePath)
    }
}

private struct RouteSearchSheet: View {
    @Environment(\.dismiss) private var dismiss

    let initialStart: RouteSearchSelection?
    let initialEnd: RouteSearchSelection?
    let onApply: (RouteSearchSelection, RouteSearchSelection) -> Void

    @StateObject private var startCompleter = LocationSearchCompleter()
    @StateObject private var endCompleter = LocationSearchCompleter()
    @State private var startQuery: String
    @State private var endQuery: String
    @State private var startSelection: RouteSearchSelection?
    @State private var endSelection: RouteSearchSelection?
    @State private var isResolvingSelection = false
    @State private var errorMessage: String?
    @FocusState private var focusedField: RouteSearchField?

    init(
        initialStart: RouteSearchSelection?,
        initialEnd: RouteSearchSelection?,
        onApply: @escaping (RouteSearchSelection, RouteSearchSelection) -> Void
    ) {
        self.initialStart = initialStart
        self.initialEnd = initialEnd
        self.onApply = onApply
        _startQuery = State(initialValue: initialStart?.title ?? "")
        _endQuery = State(initialValue: initialEnd?.title ?? "")
        _startSelection = State(initialValue: initialStart)
        _endSelection = State(initialValue: initialEnd)
    }

    private var activeResults: [MKLocalSearchCompletion] {
        switch focusedField {
        case .start:
            return startCompleter.results
        case .end:
            return endCompleter.results
        case .none:
            return []
        }
    }

    private var canApply: Bool {
        startSelection != nil && endSelection != nil && !isResolvingSelection
    }

    var body: some View {
        NavigationStack {
            VStack(alignment: .leading, spacing: 16) {
                routeField(
                    title: "Start",
                    icon: "circle.fill",
                    tint: .green,
                    text: $startQuery,
                    selection: startSelection,
                    field: .start
                )

                routeField(
                    title: "End",
                    icon: "flag.checkered.circle.fill",
                    tint: .red,
                    text: $endQuery,
                    selection: endSelection,
                    field: .end
                )

                if let errorMessage {
                    Text(errorMessage)
                        .font(.footnote)
                        .foregroundStyle(.red)
                }

                if isResolvingSelection {
                    ProgressView("Resolving location…")
                        .font(.footnote)
                } else if !activeResults.isEmpty {
                    ScrollView {
                        LazyVStack(spacing: 0) {
                            ForEach(Array(activeResults.enumerated()), id: \.element) { index, result in
                                Button {
                                    resolve(result)
                                } label: {
                                    VStack(alignment: .leading, spacing: 2) {
                                        Text(result.title)
                                            .font(.subheadline)
                                            .foregroundStyle(.primary)
                                            .frame(maxWidth: .infinity, alignment: .leading)
                                        if !result.subtitle.isEmpty {
                                            Text(result.subtitle)
                                                .font(.caption)
                                                .foregroundStyle(.secondary)
                                                .frame(maxWidth: .infinity, alignment: .leading)
                                        }
                                    }
                                    .padding(.vertical, 10)
                                    .padding(.horizontal, 12)
                                }
                                .buttonStyle(.plain)

                                if index < activeResults.count - 1 {
                                    Divider()
                                }
                            }
                        }
                    }
                    .frame(maxHeight: 260)
                } else {
                    Text("Search for a start and destination to build the route.")
                        .font(.footnote)
                        .foregroundStyle(.secondary)
                }

                Spacer(minLength: 0)
            }
            .padding(16)
            .navigationTitle("Simulate Route")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") {
                        dismiss()
                    }
                }

                ToolbarItem(placement: .confirmationAction) {
                    Button("Use Route") {
                        guard let startSelection, let endSelection else { return }
                        onApply(startSelection, endSelection)
                        dismiss()
                    }
                    .disabled(!canApply)
                }
            }
        }
        .presentationDetents([.medium, .large])
        .onAppear {
            if startSelection == nil {
                focusedField = .start
            } else if endSelection == nil {
                focusedField = .end
            }
        }
    }

    private func routeField(
        title: String,
        icon: String,
        tint: Color,
        text: Binding<String>,
        selection: RouteSearchSelection?,
        field: RouteSearchField
    ) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(title)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)

            HStack(spacing: 10) {
                Image(systemName: icon)
                    .foregroundStyle(tint)

                TextField(title, text: text)
                    .textInputAutocapitalization(.words)
                    .autocorrectionDisabled()
                    .focused($focusedField, equals: field)
                    .submitLabel(field == .start ? .next : .done)
                    .onChange(of: text.wrappedValue) { _, newValue in
                        errorMessage = nil
                        update(query: newValue, for: field)
                    }
                    .onSubmit {
                        if field == .start {
                            focusedField = .end
                        } else {
                            focusedField = nil
                        }
                    }
            }
            .padding(.horizontal, 2)
            .padding(.vertical, 4)

            if let selection {
                Text(String(format: "%.5f, %.5f", selection.coordinate.latitude, selection.coordinate.longitude))
                    .font(.caption.monospaced())
                    .foregroundStyle(.secondary)
            }
        }
    }

    private func update(query: String, for field: RouteSearchField) {
        switch field {
        case .start:
            if query != startSelection?.title {
                startSelection = nil
            }
            startCompleter.update(query: query)
        case .end:
            if query != endSelection?.title {
                endSelection = nil
            }
            endCompleter.update(query: query)
        }
    }

    private func resolve(_ completion: MKLocalSearchCompletion) {
        let field = focusedField ?? .start
        let request = MKLocalSearch.Request(completion: completion)
        isResolvingSelection = true
        errorMessage = nil

        MKLocalSearch(request: request).start { response, error in
            DispatchQueue.main.async {
                isResolvingSelection = false

                guard let item = response?.mapItems.first else {
                    errorMessage = error?.localizedDescription ?? "Could not resolve that location."
                    return
                }

                let name = item.name?.trimmingCharacters(in: .whitespacesAndNewlines) ?? ""
                let title = name.isEmpty ? completion.title : name
                let selection = RouteSearchSelection(title: title, coordinate: item.placemark.coordinate)

                switch field {
                case .start:
                    startSelection = selection
                    startQuery = title
                    startCompleter.results = []
                    focusedField = .end
                case .end:
                    endSelection = selection
                    endQuery = title
                    endCompleter.results = []
                    focusedField = nil
                }
            }
        }
    }
}

// MARK: - Bookmarks Sheet

struct BookmarksView: View {
    @Binding var bookmarks: [LocationBookmark]
    let onSelect: (LocationBookmark) -> Void
    let onDelete: (IndexSet) -> Void

    var body: some View {
        NavigationStack {
            Group {
                if bookmarks.isEmpty {
                    ContentUnavailableView(
                        "No Bookmarks",
                        systemImage: "bookmark.slash",
                        description: Text("Drop a pin on the map and tap the bookmark icon to save a location.")
                    )
                } else {
                    List {
                        ForEach(bookmarks) { bookmark in
                            Button {
                                onSelect(bookmark)
                            } label: {
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(bookmark.name)
                                        .foregroundStyle(.primary)
                                    Text(String(format: "%.6f, %.6f", bookmark.latitude, bookmark.longitude))
                                        .font(.caption.monospaced())
                                        .foregroundStyle(.secondary)
                                }
                            }
                        }
                        .onDelete(perform: onDelete)
                    }
                }
            }
            .navigationTitle("Bookmarks")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                if !bookmarks.isEmpty {
                    EditButton()
                }
            }
        }
    }
}
