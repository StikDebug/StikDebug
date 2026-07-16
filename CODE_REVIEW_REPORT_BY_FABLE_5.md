# Fork Code-Review Report — StikDebug-PLUS

**Date:** 2026-07-14
**Branch reviewed:** `claude/fork-code-review-33fkom`
**Scope:** All fork-specific changes since divergence from upstream StikDebug (commit `c5b23e2`).

## Summary

The fork adds four feature areas on top of upstream StikDebug: a location-simulator
overhaul (speed profiles, bus stops, map styles, long-route performance), a
cellular/Wi-Fi network path monitor with tunnel auto-reconnect, background
keep-alive improvements for debug sessions, and an experimental "hold app alive
in background" mode. 15 files changed, ~1,400 lines added.

I reviewed the full diff against upstream, cross-checked every new FFI call
against `idevice.h`, traced the state machines in the changed managers, and
confirmed build status via GitHub Actions history. **Two genuine defects were
found and fixed; both are pushed to the review branch.** No crashes, compile
errors, or logic bugs remain in the reviewed code.

---

## Defects fixed

### 1. Always-failing CI workflow — `.github/workflows/swift.yml` (removed)

**Severity:** Medium (broken CI signal on every push/PR)

The workflow ran `swift build` / `swift test`, which require a `Package.swift`.
This repository is an Xcode app project with no Swift package, so the job failed
with `error: Could not find Package.swift` on **every run since it was added**,
marking each push and PR with a red ❌ check.

The existing **Build Debug IPA** workflow already compiles the project with
`xcodebuild` on the same `push`/`pull_request` triggers and passes green, so
build validation is fully covered without it.

- **Fix:** Deleted `swift.yml` (commit `7b1df35`).
- **Evidence:** GitHub Actions run `29341384965` (job `87113733956`) —
  `Could not find Package.swift in this directory or any of its parent directories`.

### 2. Long rural road segments dropped from the speed index — `MapSelectionView.swift` (fixed)

**Severity:** Low (accuracy regression on long routes; no crash)

`RouteSpeedIndex` bucketed each OSM way segment into ~440 m grid cells, but
skipped any segment whose bounding box spanned more than **8 cells (~3.5 km)**:

```swift
guard maxX - minX <= 8, maxY - minY <= 8 else { continue }
```

Sparsely-noded rural highways can legitimately run tens of km between OSM
geometry nodes. Those segments were silently dropped from the index, so those
stretches lost their real speed limit and fell back to average-route pacing —
a behavior regression versus upstream, whose (slower) implementation scanned
every segment without this cap.

- **Fix:** Raised the guard to **128 cells (~57 km at the equator)** (commit
  `e11a606`). This still catches genuinely broken data such as antimeridian
  jumps (which span ~90,000 cells) while keeping realistic long segments
  indexed. Per-segment registration stays bounded, and the `nearestWayThreshold`
  (40 m) distance filter means indexing extra cells never produces false matches.

---

## Areas verified as correct (no change needed)

### Build / compilation
- **Build Debug IPA is green** on the fork tip (`346bff6`), confirming the app
  compiles and that the `xcode-version: 26.6.0` pin resolves on the runner.
- `packet.count` is correctly converted to `UInt` for the `uintptr_t`
  `debug_proxy_send_raw` parameter; `fetchRouteSpeedContext` returns a
  `RouteSpeedContext` on all paths (both were prior build-fix commits and hold up).

### Experimental "hold app alive" / JIT keep-alive (`JITEnableContext`, `BackgroundAliveManager`, `DebugKeepAliveLease`)
- Every FFI call matches `idevice.h`: `process_control_launch_app` argument
  order and `bool` flags, `process_control_disable_memory_limit`, and
  `debug_proxy_send_raw(handle, ptr, uintptr_t)`.
- The hand-built RSP continue packet `$c#63` has the correct payload and
  checksum (`0x63 % 256` → `63`), and is valid in no-ack mode.
- The hold sequence mirrors the proven `debugApp` path: drain acks →
  `QStartNoAckMode` → disable ack mode → `vAttach` → continue → poll for
  cancellation → interrupt (`0x03`) → detach (`D`).
- `HoldToken` is lock-guarded; `BackgroundAliveManager` balances start/stop and
  its `stillCurrent` check prevents a late-finishing session from clobbering a
  newer one. `DebugKeepAliveLease()` auto-activates in `init`, so the bare
  construction in both `BackgroundAliveManager` and `HomeView` is correct.
- Background-task renewal re-checks `isActive` before re-arming, so an expiring
  UIKit assertion no longer tears down long sessions.

### Background audio / location (`BackgroundAudioManager`, `BackgroundLocationManager`)
- The new `force:` (session) vs. unforced (user-toggle) activity counters are
  balanced on every start/stop path and never go negative (`max(… - 1, 0)`).
- `refreshRunningState()` correctly honors forced holds regardless of the user's
  keep-alive toggles, and `refreshFromSettings()` re-evaluates on toggle change.
- The non-zero silent-audio fill is inaudible (~-80 dBFS, DC-free) and guarded
  by `!format.isInterleaved` with the correct channel/frame iteration; the
  interleaved branch is an unreachable fallback on the standard mixer format.

### Network path monitor & tunnel reconnect (`NetworkPathMonitor`, `TunnelManager`)
- Path state is lock-guarded; change notifications are deduplicated by signature.
- Tunnel retry (3 attempts, 1s→2s backoff) correctly short-circuits on permanent
  errors (invalid/expired/missing pairing `-9/-17`, bad target IP `-18`, parse
  failures) instead of pointlessly retrying user-actionable failures.
- The reconnect observer is registered on the main queue, matching `start()`'s
  main-thread expectation; the debounced work item cancels prior pending work.

### Location simulator — speed profiles, bus stops, map styles (`MapSelectionView`)
- The Overpass query moved from a bounding-box GET to a corridor `around` POST;
  the body is percent-encoded with a form-safe character set and the client
  timeout (30 s) exceeds the server timeout (25 s).
- `OverpassResponse.Element` decodes both way `geometry` and node `lat`/`lon`,
  which `out tags geom;` emits for nodes — bus-stop parsing is sound.
- Bus-stop detection covers both the legacy `highway=bus_stop` and the modern
  `public_transport=platform/stop_position` + `bus=yes` schemas.
- Route state (`routeBusStops`, `isImportedRoute`, `lastFallbackSpeed`,
  `routeRequestID`) is reset consistently across import / clear / reset / refresh,
  and every async prefetch guards on `routeRequestID` before applying results.
- Profile switching re-plans directions only when walking-vs-driving changes and
  the route is searched (not imported); otherwise it rebuilds pacing in place.
- All SwiftUI/MapKit symbols are valid for the iOS 17.4 deployment target
  (`MapStyle`, `PointOfInterestCategories`, `Marker`, `.mapStyle`).

---

## Minor observations (intentional trade-offs — not changed)

- **Bus ETA** in the route summary uses MapKit's automobile travel time and does
  not add the per-stop dwell time, so a bus route's ETA is slightly optimistic.
- **Bus stop dwell** (12 s) is applied to the sample nearest the stop (≤25 m),
  so playback pauses just before arriving rather than exactly at the stop.
- **`build_ipa.yml`** has two stray blank lines from an earlier edit; harmless
  (YAML ignores them).
- **Bus-stop cell lookup** assumes non-polar latitudes (documented); above ~80°N
  a 90 m search radius could exceed a longitude cell. Not reachable in practice.
- **`keepAppAlive`** blocks a global-queue thread for the hold duration and opens
  multiple concurrent debug tunnels; acceptable for an explicitly experimental
  feature, but worth revisiting if it graduates out of experimental.

---

## Commits on the review branch

| Commit | Change |
|--------|--------|
| `7b1df35` | Remove Swift Package CI workflow that can never pass |
| `e11a606` | Index long rural road segments instead of dropping them |

Both are pushed to `origin/claude/fork-code-review-33fkom`. The Build Debug IPA
workflow will validate compilation on the branch tip.
