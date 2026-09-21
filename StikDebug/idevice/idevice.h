// Jackson Coxson
// Bindings to idevice - https://github.com/jkcoxson/idevice

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
  #define WIN32_LEAN_AND_MEAN
  #endif
  #include <winsock2.h>
  #include <ws2tcpip.h>
  typedef int                idevice_socklen_t;
  typedef struct sockaddr    idevice_sockaddr;
#else
  #include <sys/types.h>
  #include <sys/socket.h>
  typedef socklen_t          idevice_socklen_t;
  typedef struct sockaddr    idevice_sockaddr;
#endif


#ifndef IDEVICE_H
#define IDEVICE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define LOCKDOWN_PORT 62078

/**
 * The nonce domain index cryptexes are personalized against
 */
#define IDEVICE_CRYPTEXD_NONCE_DOMAIN_CRYPTEX 2

/**
 * The `image-type-index` a DeveloperDiskImage install uses
 */
#define IDEVICE_CRYPTEXD_DDI_IMAGE_TYPE_INDEX 10

/**
 * The `persistence` a DeveloperDiskImage install uses
 */
#define IDEVICE_CRYPTEXD_DDI_PERSISTENCE 2

/**
 * The `nonce-persistence` a DeveloperDiskImage install uses
 */
#define IDEVICE_CRYPTEXD_DDI_NONCE_PERSISTENCE 1

typedef enum AfcFopenMode {
  AfcRdOnly = 1,
  AfcRw = 2,
  AfcWrOnly = 3,
  AfcWr = 4,
  AfcAppend = 5,
  AfcRdAppend = 6,
} AfcFopenMode;

/**
 * Link type for creating hard or symbolic links
 */
typedef enum AfcLinkType {
  Hard = 1,
  Symbolic = 2,
} AfcLinkType;

/**
 * The system's light/dark appearance
 */
typedef enum IdeviceUserInterfaceStyle {
  IdeviceUserInterfaceStyleLight = 0,
  IdeviceUserInterfaceStyleDark = 1,
} IdeviceUserInterfaceStyle;

/**
 * Which of the device's filesystem domains a session is scoped to
 */
typedef enum IdeviceFileServiceDomain {
  /**
   * An app's own data container. The identifier is the bundle ID.
   */
  IdeviceFileServiceDomainAppDataContainer = 1,
  /**
   * A shared app-group container. The identifier is the group ID.
   */
  IdeviceFileServiceDomainAppGroupDataContainer = 2,
  /**
   * The temporary directory.
   */
  IdeviceFileServiceDomainTemporary = 3,
  /**
   * The system crash-log store.
   */
  IdeviceFileServiceDomainSystemCrashLogs = 5,
} IdeviceFileServiceDomain;

/**
 * Network event type discriminant
 */
typedef enum IdeviceNetworkEventType {
  InterfaceDetection = 0,
  ConnectionDetection = 1,
  ConnectionUpdate = 2,
  Unknown = 255,
} IdeviceNetworkEventType;

typedef enum IdeviceLoggerError {
  Success = 0,
  FileError = -1,
  AlreadyInitialized = -2,
  InvalidPathString = -3,
} IdeviceLoggerError;

typedef enum IdeviceLogLevel {
  Disabled = 0,
  ErrorLevel = 1,
  Warn = 2,
  Info = 3,
  Debug = 4,
  Trace = 5,
} IdeviceLogLevel;

/**
 * The outcome of a `CreateStashbag` request.
 */
typedef enum IdeviceStashbagOutcome {
  /**
   * The device does not need a stashbag; nothing further to do.
   */
  NotRequired = 0,
  /**
   * A stashbag was created and must be committed with the AP ticket.
   */
  CommitRequired = 1,
} IdeviceStashbagOutcome;

typedef struct AdapterHandle AdapterHandle;

typedef struct AdapterStreamHandle AdapterStreamHandle;

typedef struct AfcClientHandle AfcClientHandle;

/**
 * Handle for an open file on the device
 */
typedef struct AfcFileHandle AfcFileHandle;

typedef struct AmfiClientHandle AmfiClientHandle;

/**
 * Opaque handle to an AppServiceClient
 */
typedef struct AppServiceHandle AppServiceHandle;

/**
 * Opaque handle to an ApplicationListingClient
 */
typedef struct ApplicationListingHandle ApplicationListingHandle;

typedef struct BtPacketLoggerClientHandle BtPacketLoggerClientHandle;

typedef struct CompanionProxyClientHandle CompanionProxyClientHandle;

/**
 * Opaque handle to a ConditionInducerClient
 */
typedef struct ConditionInducerHandle ConditionInducerHandle;

/**
 * Opaque handle to a ConfigurationServiceClient
 */
typedef struct ConfigurationServiceHandle ConfigurationServiceHandle;

typedef struct CoreDeviceProxyHandle CoreDeviceProxyHandle;

typedef struct CrashReportCopyMobileHandle CrashReportCopyMobileHandle;

/**
 * Opaque handle to the payloads a Cryptex1 DeveloperDiskImage install needs
 */
typedef struct Cryptex1AssetsHandle Cryptex1AssetsHandle;

/**
 * Opaque handle to a CryptexdClient
 *
 * The daemon serves one routine per connection, so every call below consumes
 * the handle: it is freed by the call and must not be used again, even when
 * the call fails.
 */
typedef struct CryptexdHandle CryptexdHandle;

/**
 * Opaque handle to a DebugProxyClient
 */
typedef struct DebugProxyHandle DebugProxyHandle;

/**
 * Opaque handle to a DeviceInfoClient
 */
typedef struct DeviceInfoHandle DeviceInfoHandle;

typedef struct DiagnosticsRelayClientHandle DiagnosticsRelayClientHandle;

/**
 * Opaque handle to an AppServiceClient
 */
typedef struct DiagnosticsServiceHandle DiagnosticsServiceHandle;

typedef struct EnergyMonitorHandle EnergyMonitorHandle;

/**
 * Opaque handle to a FileServiceClient
 */
typedef struct FileServiceHandle FileServiceHandle;

typedef struct GraphicsHandle GraphicsHandle;

typedef struct HeartbeatClientHandle HeartbeatClientHandle;

typedef struct HouseArrestClientHandle HouseArrestClientHandle;

/**
 * Opaque handle to an IconServiceClient
 */
typedef struct IconServiceHandle IconServiceHandle;

/**
 * Opaque C-compatible handle to an Idevice connection
 */
typedef struct IdeviceHandle IdeviceHandle;

/**
 * Opaque C-compatible handle to a PairingFile
 */
typedef struct IdevicePairingFile IdevicePairingFile;

typedef struct IdeviceProviderHandle IdeviceProviderHandle;

/**
 * An opaque, shareable cancellation flag for an in-flight restore.
 *
 * Create one with `idevice_restore_cancel_handle_new`, pass it to
 * `idevice_restore_run`, and call `idevice_restore_cancel` from another thread to
 * request a graceful cancel (the device is rebooted toward recovery). Free it with
 * `idevice_restore_cancel_handle_free` once the restore has returned.
 */
typedef struct IdeviceRestoreCancelHandle IdeviceRestoreCancelHandle;

typedef struct IdeviceSocketHandle IdeviceSocketHandle;

typedef struct ImageMounterHandle ImageMounterHandle;

typedef struct InstallationProxyClientHandle InstallationProxyClientHandle;

typedef struct InstallcoordinationProxyHandle InstallcoordinationProxyHandle;

/**
 * Opaque handle to an opened IPSW archive.
 */
typedef struct IpswHandle IpswHandle;

/**
 * Opaque handle to a ProcessControlClient
 */
typedef struct LocationSimulationHandle LocationSimulationHandle;

typedef struct LocationSimulationServiceHandle LocationSimulationServiceHandle;

typedef struct LockdowndClientHandle LockdowndClientHandle;

typedef struct MisagentClientHandle MisagentClientHandle;

/**
 * Opaque handle wrapping a provider pointer for MobileActivationd.
 * The client is recreated per call since each request requires a new connection.
 */
typedef struct MobileActivationdClientHandle MobileActivationdClientHandle;

typedef struct MobileBackup2ClientHandle MobileBackup2ClientHandle;

/**
 * Opaque handle to a NetworkMonitorClient
 */
typedef struct NetworkMonitorHandle NetworkMonitorHandle;

typedef struct NotificationProxyClientHandle NotificationProxyClientHandle;

typedef struct NotificationsHandle NotificationsHandle;

typedef struct OsTraceRelayClientHandle OsTraceRelayClientHandle;

typedef struct OsTraceRelayReceiverHandle OsTraceRelayReceiverHandle;

/**
 * Opaque cancellation token for [`pairable_host_accept`].
 *
 * Create one with `pairable_host_cancel_new`, hand it to `pairable_host_accept`,
 * and call `pairable_host_cancel_signal` from any other thread to abort the wait.
 * Free it with `pairable_host_cancel_free` once the accept has returned.
 */
typedef struct PairableHostCancel PairableHostCancel;

/**
 * Opaque handle holding a generated host identity between
 * `pairable_host_prepare` and `pairable_host_accept_fd`.
 */
typedef struct PairableHostHandle PairableHostHandle;

typedef struct PcapdClientHandle PcapdClientHandle;

typedef struct PreboardServiceClientHandle PreboardServiceClientHandle;

/**
 * Opaque handle to a ProcessControlClient
 */
typedef struct ProcessControlHandle ProcessControlHandle;

typedef struct ReadWriteOpaque ReadWriteOpaque;

/**
 * Opaque handle to a device in recovery/DFU mode.
 */
typedef struct RecoveryDeviceHandle RecoveryDeviceHandle;

/**
 * Opaque handle to the RemoteXPC-native notification proxy (iOS 17+)
 */
typedef struct RemoteNotificationProxyClientHandle RemoteNotificationProxyClientHandle;

/**
 * Opaque handle to a remote pairing client speaking `RPPairing` over lockdown
 */
typedef struct RemotePairingLockdownHandle RemotePairingLockdownHandle;

/**
 * Opaque handle to a RemoteServerClient
 */
typedef struct RemoteServerHandle RemoteServerHandle;

typedef struct RestoreServiceClientHandle RestoreServiceClientHandle;

/**
 * Opaque handle to a restore-mode `com.apple.mobile.restored` client.
 */
typedef struct RestoredClientHandle RestoredClientHandle;

/**
 * Opaque handle to an RPPairing file
 */
typedef struct RpPairingFileHandle RpPairingFileHandle;

/**
 * Opaque handle to an RsdHandshake
 */
typedef struct RsdHandshakeHandle RsdHandshakeHandle;

/**
 * An opaque FFI handle for a [`ScreenshotClient`].
 *
 * This type wraps a [`ScreenshotClient`] that communicates with
 * a connected device to capture screenshots through the DVT (Device Virtualization Toolkit) service.
 */
typedef struct ScreenshotClientHandle ScreenshotClientHandle;

typedef struct ScreenshotrClientHandle ScreenshotrClientHandle;

typedef struct SpringBoardServicesClientHandle SpringBoardServicesClientHandle;

typedef struct SysdiagnoseStreamHandle SysdiagnoseStreamHandle;

typedef struct SyslogRelayClientHandle SyslogRelayClientHandle;

/**
 * Opaque handle to a SysmontapClient
 */
typedef struct SysmontapHandle SysmontapHandle;

typedef struct TcpEatObject TcpEatObject;

typedef struct TcpFeedObject TcpFeedObject;

typedef struct UsbmuxdAddrHandle UsbmuxdAddrHandle;

typedef struct UsbmuxdConnectionHandle UsbmuxdConnectionHandle;

typedef struct UsbmuxdDeviceHandle UsbmuxdDeviceHandle;

typedef struct UsbmuxdListenerHandle UsbmuxdListenerHandle;

typedef struct Vec_u64 Vec_u64;

/**
 * Opaque handle wrapping a [`WdaBridge`].
 */
typedef struct WdaBridgeHandle WdaBridgeHandle;

/**
 * Opaque handle wrapping the WDA client state.
 *
 * The handle owns the provider so that subsequent calls can open fresh
 * per-request connections without the caller juggling a separate
 * `IdeviceProviderHandle`.
 */
typedef struct WdaClientHandle WdaClientHandle;

typedef struct IdeviceFfiError {
  int32_t code;
  int32_t sub_code;
  const char *message;
} IdeviceFfiError;

/**
 * Stub to avoid header problems
 */
typedef void *plist_t;

/**
 * File information structure for C bindings
 */
typedef struct AfcFileInfo {
  size_t size;
  size_t blocks;
  int64_t creation;
  int64_t modified;
  char *st_nlink;
  char *st_ifmt;
  char *st_link_target;
} AfcFileInfo;

/**
 * Device information structure for C bindings
 */
typedef struct AfcDeviceInfo {
  char *model;
  size_t total_bytes;
  size_t free_bytes;
  size_t block_size;
} AfcDeviceInfo;

/**
 * Represents a parsed BT packet from the logger
 */
typedef struct BtPacketHandle {
  /**
   * Header: advisory length
   */
  uint32_t length;
  /**
   * Header: timestamp seconds
   */
  uint32_t ts_secs;
  /**
   * Header: timestamp microseconds
   */
  uint32_t ts_usecs;
  /**
   * Packet kind byte (0x00=HciCmd, 0x01=HciEvt, 0x02=AclSent, 0x03=AclRecv, etc.)
   */
  uint8_t kind;
  /**
   * H4-ready payload data
   */
  uint8_t *h4_data;
  /**
   * Length of h4_data
   */
  uintptr_t h4_data_len;
} BtPacketHandle;

/**
 * C-compatible app list entry
 */
typedef struct AppListEntryC {
  int is_removable;
  char *name;
  int is_first_party;
  char *path;
  char *bundle_identifier;
  int is_developer_app;
  char *bundle_version;
  int is_internal;
  int is_hidden;
  int is_app_clip;
  char *version;
} AppListEntryC;

/**
 * C-compatible launch response
 */
typedef struct LaunchResponseC {
  uint32_t process_identifier_version;
  uint32_t pid;
  char *executable_url;
  uint32_t *audit_token;
  uintptr_t audit_token_len;
} LaunchResponseC;

/**
 * C-compatible process token
 */
typedef struct ProcessTokenC {
  uint32_t pid;
  char *executable_url;
} ProcessTokenC;

/**
 * C-compatible signal response
 */
typedef struct SignalResponseC {
  uint32_t pid;
  char *executable_url;
  uint64_t device_timestamp;
  uint32_t signal;
} SignalResponseC;

/**
 * The accessibility color filter's state
 */
typedef struct ColorFilterC {
  int enabled;
  /**
   * The filter preset's name, or NULL if the device didn't report one.
   * Free with `idevice_string_free`.
   */
  char *filter_type;
  /**
   * Filter strength, 0.0 to 1.0. Only meaningful when `has_intensity` is 1.
   */
  double intensity;
  int has_intensity;
} ColorFilterC;

/**
 * A rendered app icon
 */
typedef struct AppIconC {
  /**
   * PNG-encoded image data
   */
  uint8_t *png_data;
  uintptr_t png_data_len;
  /**
   * Icon dimensions in pixels, i.e. the points multiplied by the scale
   */
  double pixel_width;
  double pixel_height;
  /**
   * Icon dimensions in points, as actually rendered. May be smaller than
   * what was requested.
   */
  double width;
  double height;
  double scale;
  /**
   * 1 when the device had no real icon for the app and rendered a generic
   * placeholder instead
   */
  int is_placeholder;
} AppIconC;

/**
 * A cryptex installed on the device
 */
typedef struct InstalledCryptexC {
  /**
   * Free with `idevice_string_free`
   */
  char *identifier;
  /**
   * Free with `idevice_string_free`
   */
  char *version;
} InstalledCryptexC;

/**
 * Which nonce domain a get-nonce or roll-nonce request refers to
 */
typedef struct CryptexNonceDomain {
  /**
   * When 1, `value` is a nonce domain handle, e.g. a build identity's
   * `Cryptex1,NonceDomain`. When 0, it is a domain index, e.g.
   * `IDEVICE_CRYPTEXD_NONCE_DOMAIN_CRYPTEX`.
   */
  int is_handle;
  uint64_t value;
} CryptexNonceDomain;

/**
 * The payloads and parameters one install needs
 */
typedef struct CryptexInstallRequestC {
  /**
   * The cryptex disk image, i.e. the manifest's `Cryptex1,GenericDmg`
   */
  const uint8_t *image;
  uintptr_t image_len;
  /**
   * `Cryptex1,GenericTrustCache`
   */
  const uint8_t *trustcache;
  uintptr_t trustcache_len;
  /**
   * The Cryptex1 personalization ticket
   */
  const uint8_t *im4m;
  uintptr_t im4m_len;
  /**
   * `Cryptex1,CryptexInfoPlist`, which names and versions the cryptex
   */
  const uint8_t *info;
  uintptr_t info_len;
  /**
   * `Cryptex1,GenericVolume` root hash
   */
  const uint8_t *volumehash;
  uintptr_t volumehash_len;
  /**
   * The `Cryptex1,*` parameters from the build identity, as a plist
   * dictionary. Non-negative integers are sent as uint64, which the daemon
   * requires.
   */
  plist_t cryptex1_properties;
  int64_t image_type_index;
  uint64_t persistence;
  uint64_t nonce_persistence;
  uint64_t auth;
} CryptexInstallRequestC;

/**
 * Represents a debugserver command
 */
typedef struct DebugserverCommandHandle {
  char *name;
  char **argv;
  uintptr_t argv_count;
} DebugserverCommandHandle;

/**
 * A notification from the mobile notifications instruments channel
 */
typedef struct IdeviceNotificationInfo {
  char *notification_type;
  int64_t mach_absolute_time;
  char *exec_name;
  char *app_name;
  uint32_t pid;
  char *state_description;
} IdeviceNotificationInfo;

/**
 * A single condition profile
 */
typedef struct IdeviceConditionProfile {
  char *identifier;
  char *description;
} IdeviceConditionProfile;

/**
 * A condition inducer group containing profiles
 */
typedef struct IdeviceConditionGroup {
  char *identifier;
  struct IdeviceConditionProfile *profiles;
  uintptr_t profiles_count;
} IdeviceConditionGroup;

/**
 * A running process on the device
 */
typedef struct IdeviceRunningProcess {
  uint32_t pid;
  char *name;
  char *real_app_name;
  bool is_application;
  uint64_t start_page_count;
} IdeviceRunningProcess;

/**
 * A parsed per-PID energy sample
 */
typedef struct IdeviceEnergySample {
  uint32_t pid;
  int64_t timestamp;
  double total_energy;
  double cpu_energy;
  double gpu_energy;
  double networking_energy;
  double display_energy;
  double location_energy;
  double appstate_energy;
} IdeviceEnergySample;

/**
 * A graphics sample from tddhe GPU instruments channel
 */
typedef struct IdeviceGraphicsSample {
  uint64_t timestamp;
  double fps;
  uint64_t alloc_system_memory;
  uint64_t in_use_system_memory;
  uint64_t in_use_system_memory_driver;
  char *gpu_bundle_name;
  uint64_t recovery_count;
} IdeviceGraphicsSample;

/**
 * A socket address (IPv4 or IPv6), represented as a null-terminated string + port
 */
typedef struct IdeviceSocketAddress {
  /**
   * Address family (e.g. 2 = AF_INET, 30 = AF_INET6)
   */
  uint8_t family;
  uint16_t port;
  /**
   * Null-terminated address string. Must be freed with `idevice_string_free`.
   */
  char *addr;
} IdeviceSocketAddress;

/**
 * A network event emitted by the device
 */
typedef struct IdeviceNetworkEvent {
  enum IdeviceNetworkEventType event_type;
  uint32_t interface_index;
  /**
   * Null-terminated interface name. Must be freed with `idevice_string_free`.
   * Only valid when event_type == InterfaceDetection.
   */
  char *interface_name;
  struct IdeviceSocketAddress local_addr;
  struct IdeviceSocketAddress remote_addr;
  /**
   * PID of the process owning the connection. Valid for ConnectionDetection.
   */
  uint32_t pid;
  uint64_t recv_buffer_size;
  uint64_t recv_buffer_used;
  uint64_t serial_number;
  uint32_t kind;
  uint64_t rx_packets;
  uint64_t rx_bytes;
  uint64_t tx_packets;
  uint64_t tx_bytes;
  uint64_t rx_dups;
  uint64_t rx_ooo;
  uint64_t tx_retx;
  uint64_t min_rtt;
  uint64_t avg_rtt;
  uint64_t connection_serial;
  uint64_t time;
  uint64_t unknown_type;
} IdeviceNetworkEvent;

/**
 * Configuration for sysmontap sampling passed over FFI
 */
typedef struct IdeviceSysmontapConfig {
  /**
   * Sampling interval in milliseconds
   */
  uint32_t interval_ms;
  /**
   * Array of process attribute name strings (null-terminated C strings)
   */
  const char *const *process_attributes;
  uintptr_t process_attributes_count;
  /**
   * Array of system attribute name strings (null-terminated C strings)
   */
  const char *const *system_attributes;
  uintptr_t system_attributes_count;
} IdeviceSysmontapConfig;

/**
 * Progress snapshot passed to `on_progress`.
 *
 * A session is split into batches of files. `batch_*` describes the batch
 * currently streaming; `session_*` accumulates across the whole session.
 * Fields are only ever appended to, so a callback compiled against an older
 * header stays ABI-compatible.
 */
typedef struct Mobilebackup2BackupProgress {
  /**
   * Bytes transferred so far in the current batch.
   */
  uint64_t batch_bytes_done;
  /**
   * Bytes the device said this batch contains, or 0 if unknown. Approximate.
   */
  uint64_t batch_bytes_total;
  /**
   * Bytes transferred so far across every batch in this session. Monotonic.
   */
  uint64_t session_bytes_done;
  /**
   * Estimated total bytes for the session, or 0 while not estimable.
   * Derived from the device's percentage, so it drifts. Never exact.
   */
  uint64_t session_bytes_total;
  /**
   * Overall progress percentage (0.0-100.0), or negative if not yet known.
   * Interpolated within a batch and clamped to be monotonic. Not equal to
   * session_bytes_done / session_bytes_total.
   */
  double overall_progress;
} Mobilebackup2BackupProgress;

/**
 * C-compatible delegate for mobilebackup2 operations.
 *
 * All function pointers are required except `on_file_received` and
 * `on_progress` which may be NULL.
 *
 * Every path argument is a null-terminated UTF-8 string.
 * `context` is forwarded unchanged from the struct field.
 */
typedef struct Mobilebackup2BackupDelegateFFI {
  void *context;
  uint64_t (*get_free_disk_space)(const char *path, void *context);
  struct IdeviceFfiError *(*open_file_read)(const char *path,
                                            uint8_t **out_data,
                                            uintptr_t *out_len,
                                            void *context);
  struct IdeviceFfiError *(*create_file_write)(const char *path, void *context);
  struct IdeviceFfiError *(*write_chunk)(const char *path,
                                         const uint8_t *data,
                                         uintptr_t len,
                                         void *context);
  struct IdeviceFfiError *(*close_file)(const char *path, void *context);
  struct IdeviceFfiError *(*create_dir_all)(const char *path, void *context);
  struct IdeviceFfiError *(*remove)(const char *path, void *context);
  struct IdeviceFfiError *(*rename)(const char *from, const char *to, void *context);
  struct IdeviceFfiError *(*copy)(const char *src, const char *dst, void *context);
  bool (*exists)(const char *path, void *context);
  bool (*is_dir)(const char *path, void *context);
  /**
   * Optional cancellation callback. May be NULL.
   */
  bool (*is_cancelled)(void *context);
  /**
   * Optional progress callback. May be NULL.
   *
   * `progress` is owned by the caller and only valid for the duration of the
   * call; copy out any fields you need to keep.
   */
  void (*on_progress)(const struct Mobilebackup2BackupProgress *progress, void *context);
} Mobilebackup2BackupDelegateFFI;

typedef struct SyslogLabel {
  const char *subsystem;
  const char *category;
} SyslogLabel;

typedef struct OsTraceLog {
  uint32_t pid;
  int64_t timestamp;
  uint8_t level;
  const char *image_name;
  const char *filename;
  const char *message;
  const struct SyslogLabel *label;
  /**
   * Unique process ID (the activity stream's `procid` field). Equals `pid`
   * in practice on iOS.
   */
  uint64_t procid;
  /**
   * ID of the thread that emitted the entry
   */
  uint64_t thread_id;
  /**
   * Load address offset of the log call site within the sender image. Pair
   * with `image_uuid` to symbolicate.
   */
  uint32_t image_offset;
  /**
   * UUID of the sender image, i.e. the one named by `image_name`
   */
  uint8_t image_uuid[16];
  /**
   * UUID of the process' main executable, i.e. the one named by `filename`
   */
  uint8_t process_image_uuid[16];
  /**
   * Raw monotonic device timestamp in mach ticks
   */
  uint64_t mach_timestamp;
} OsTraceLog;

/**
 * The peer device identity learned during a successful pair-setup.
 *
 * Free with `rppairing_peer_device_free`.
 */
typedef struct RpPairingPeerDeviceC {
  /**
   * Peer identifier, the same identifier a later `verifyManualPairing` returns.
   */
  char *account_id;
  /**
   * The device's 16-byte `altIRK`, used to match its mDNS `authTag` records.
   */
  uint8_t alt_irk[16];
  /**
   * Hardware model identifier, e.g. "AppleTV14,1".
   */
  char *model;
  /**
   * User-visible device name, e.g. "Living Room".
   */
  char *name;
  /**
   * The device's UDID.
   */
  char *udid;
} RpPairingPeerDeviceC;

/**
 * Called when the device issues a setup PIN, so the caller can surface it to the
 * user. May be NULL.
 */
typedef void (*PairableHostPinCb)(const char *pin, void *context);

/**
 * Represents a captured device packet from pcapd
 */
typedef struct DevicePacketHandle {
  uint32_t header_length;
  uint8_t header_version;
  uint32_t packet_length;
  uint8_t interface_type;
  uint16_t unit;
  uint8_t io;
  uint32_t protocol_family;
  uint32_t frame_pre_length;
  uint32_t frame_post_length;
  char *interface_name;
  uint32_t pid;
  char *comm;
  uint32_t svc;
  uint32_t epid;
  char *ecomm;
  uint32_t seconds;
  uint32_t microseconds;
  uint8_t *data;
  uintptr_t data_len;
} DevicePacketHandle;

/**
 * C delegate supplying firmware component bytes by archive path.
 *
 * `read_component` (required) reads a whole component into a system-allocated
 * buffer (ownership transfers to the library, which frees it). The optional
 * streaming trio (`open_component`/`read_chunk`/`close_component`) lets large
 * source boot objects stream without buffering; when `open_component` is NULL the
 * library falls back to buffering via `read_component`.
 */
typedef struct IdeviceRestoreComponentSourceFFI {
  void *context;
  struct IdeviceFfiError *(*read_component)(const char *path,
                                            uint8_t **out_data,
                                            uintptr_t *out_len,
                                            void *context);
  struct IdeviceFfiError *(*open_component)(const char *path, void **out_reader, void *context);
  struct IdeviceFfiError *(*read_chunk)(void *reader,
                                        uint8_t *buf,
                                        uintptr_t buf_len,
                                        uintptr_t *out_read,
                                        void *context);
  void (*close_component)(void *reader, void *context);
} IdeviceRestoreComponentSourceFFI;

/**
 * C delegate exposing a seekable, sized filesystem (DMG) image for ASR.
 */
typedef struct IdeviceRestoreFilesystemImageFFI {
  void *context;
  /**
   * Returns the total image size in bytes.
   */
  struct IdeviceFfiError *(*size)(uint64_t *out_size, void *context);
  /**
   * Reads up to `len` bytes at `offset` into a system-allocated buffer whose
   * ownership transfers to the library.
   */
  struct IdeviceFfiError *(*read_at)(uint64_t offset,
                                     uintptr_t len,
                                     uint8_t **out_data,
                                     uintptr_t *out_len,
                                     void *context);
} IdeviceRestoreFilesystemImageFFI;

/**
 * C delegate opening fresh connections to restore-mode data ports.
 */
typedef struct IdeviceRestoreDataPortConnectorFFI {
  void *context;
  /**
   * Connects to `port`, yielding a new [`IdeviceHandle`] (ownership transfers
   * to the library).
   */
  struct IdeviceFfiError *(*connect)(uint16_t port,
                                     struct IdeviceHandle **out_idevice,
                                     void *context);
} IdeviceRestoreDataPortConnectorFFI;

/**
 * C delegate receiving restore progress callbacks. Any field may be NULL.
 */
typedef struct IdeviceRestoreProgressFFI {
  void *context;
  /**
   * The device's operation code and completion percentage (0–100).
   */
  void (*operation)(uint64_t operation, uint64_t progress, void *context);
  /**
   * A named host step (the `DataType` being serviced).
   */
  void (*step)(const char *name, void *context);
  void (*transfer)(const char *component,
                   uint64_t sent,
                   uint64_t total,
                   bool has_total,
                   void *context);
} IdeviceRestoreProgressFFI;

/**
 * C delegate implementing the raw USB surface of a recovery/DFU device.
 *
 * The library implements the iBoot/DFU protocol on top of these calls, so the
 * caller only supplies USB I/O (via nusb, libusb, etc) against the Apple device
 * already opened in a recovery/DFU mode.
 */
typedef struct IdeviceRestoreRecoveryTransportFFI {
  void *context;
  /**
   * Host to device control transfer; writes the byte count to `out_transferred`.
   */
  struct IdeviceFfiError *(*control_out)(uint8_t request_type,
                                         uint8_t request,
                                         uint16_t value,
                                         uint16_t index,
                                         const uint8_t *data,
                                         uintptr_t data_len,
                                         uint32_t timeout_ms,
                                         uintptr_t *out_transferred,
                                         void *context);
  /**
   * Device to host control transfer into a system-allocated buffer (ownership
   * transfers to the library).
   */
  struct IdeviceFfiError *(*control_in)(uint8_t request_type,
                                        uint8_t request,
                                        uint16_t value,
                                        uint16_t index,
                                        uint16_t length,
                                        uint32_t timeout_ms,
                                        uint8_t **out_data,
                                        uintptr_t *out_len,
                                        void *context);
  /**
   * Bulk OUT transfer; writes the byte count to `out_transferred`.
   */
  struct IdeviceFfiError *(*bulk_out)(uint8_t endpoint,
                                      const uint8_t *data,
                                      uintptr_t data_len,
                                      uint32_t timeout_ms,
                                      uintptr_t *out_transferred,
                                      void *context);
  /**
   * Writes the NUL-terminated USB serial-number string into `buf`
   * (capacity `buf_len`).
   */
  struct IdeviceFfiError *(*serial_number)(char *buf, uintptr_t buf_len, void *context);
  /**
   * Returns the device descriptor's `idProduct`.
   */
  uint16_t (*product_id)(void *context);
  /**
   * Selects a configuration.
   */
  struct IdeviceFfiError *(*set_configuration)(uint8_t configuration, void *context);
  /**
   * Claims an interface / alternate setting.
   */
  struct IdeviceFfiError *(*claim_interface)(uint8_t iface, uint8_t alt_setting, void *context);
  /**
   * Resets the device (it re-enumerates afterwards).
   */
  struct IdeviceFfiError *(*reset)(void *context);
} IdeviceRestoreRecoveryTransportFFI;

/**
 * C delegate opening FDR trust-channel connections to device ports.
 */
typedef struct IdeviceRestoreFdrConnectorFFI {
  void *context;
  /**
   * Connects to `port`, yielding a new [`IdeviceHandle`] (ownership
   * transfers to the library).
   */
  struct IdeviceFfiError *(*connect_device_port)(uint16_t port,
                                                 struct IdeviceHandle **out_idevice,
                                                 void *context);
} IdeviceRestoreFdrConnectorFFI;

/**
 * C-compatible representation of an RSD service
 */
typedef struct CRsdService {
  /**
   * Service name (null-terminated string)
   */
  char *name;
  /**
   * Required entitlement (null-terminated string)
   */
  char *entitlement;
  /**
   * Port number
   */
  uint16_t port;
  /**
   * Whether service uses remote XPC
   */
  bool uses_remote_xpc;
  /**
   * Number of features
   */
  size_t features_count;
  /**
   * Array of feature strings
   */
  char **features;
  /**
   * Service version (-1 if not present)
   */
  int64_t service_version;
} CRsdService;

/**
 * Array of RSD services returned by rsd_get_services
 */
typedef struct CRsdServiceArray {
  /**
   * Array of services
   */
  struct CRsdService *services;
  /**
   * Number of services in array
   */
  size_t count;
} CRsdServiceArray;

/**
 * Represents a screenshot data buffer
 */
typedef struct ScreenshotData {
  uint8_t *data;
  uintptr_t length;
} ScreenshotData;

/**
 * Localhost endpoints exposed by a running WDA bridge.
 *
 * Pointers in this struct are heap-allocated and must be released with
 * `wda_bridge_endpoints_free`.
 */
typedef struct WdaBridgeEndpointsC {
  char *udid;
  char *wda_url;
  char *mjpeg_url;
  uint16_t local_http;
  uint16_t local_mjpeg;
  uint16_t device_http;
  uint16_t device_mjpeg;
} WdaBridgeEndpointsC;

/**
 * Creates a new Idevice connection
 *
 * # Arguments
 * * [`socket`] - Socket for communication with the device
 * * [`label`] - Label for the connection
 * * [`idevice`] - On success, will be set to point to a newly allocated Idevice handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `label` must be a valid null-terminated C string
 * `idevice` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_new(struct IdeviceSocketHandle *socket,
                                    const char *label,
                                    struct IdeviceHandle **idevice);

/**
 * Creates an Idevice object from a socket file descriptor
 *
 * # Safety
 * The socket FD must be valid.
 * The pointers must be valid and non-null.
 */
struct IdeviceFfiError *idevice_from_fd(int32_t fd,
                                        const char *label,
                                        struct IdeviceHandle **idevice);

/**
 * Creates a new Idevice connection
 *
 * # Arguments
 * * [`addr`] - The socket address to connect to
 * * [`addr_len`] - Length of the socket
 * * [`label`] - Label for the connection
 * * [`idevice`] - On success, will be set to point to a newly allocated Idevice handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `addr` must be a valid sockaddr
 * `label` must be a valid null-terminated C string
 * `idevice` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_new_tcp_socket(const idevice_sockaddr *addr,
                                               idevice_socklen_t addr_len,
                                               const char *label,
                                               struct IdeviceHandle **idevice);

/**
 * Gets the device type
 *
 * # Arguments
 * * [`idevice`] - The Idevice handle
 * * [`device_type`] - On success, will be set to point to a newly allocated string containing the device type
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `idevice` must be a valid, non-null pointer to an Idevice handle
 * `device_type` must be a valid, non-null pointer to a location where the string pointer will be stored
 */
struct IdeviceFfiError *idevice_get_type(struct IdeviceHandle *idevice,
                                         char **device_type);

/**
 * Performs RSD checkin
 *
 * # Arguments
 * * [`idevice`] - The Idevice handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `idevice` must be a valid, non-null pointer to an Idevice handle
 */
struct IdeviceFfiError *idevice_rsd_checkin(struct IdeviceHandle *idevice);

/**
 * Starts a TLS session
 *
 * # Arguments
 * * [`idevice`] - The Idevice handle
 * * [`pairing_file`] - The pairing file to use for TLS
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `idevice` must be a valid, non-null pointer to an Idevice handle
 * `pairing_file` must be a valid, non-null pointer to a pairing file handle
 */
struct IdeviceFfiError *idevice_start_session(struct IdeviceHandle *idevice,
                                              const struct IdevicePairingFile *pairing_file,
                                              bool legacy);

/**
 * Sets the timeout on async calls such as TCP connections
 *
 * # Safety
 * This function is safe to call from any thread at any time
 */
void idevice_set_global_timeout(uint64_t secs);

/**
 * Frees an Idevice handle
 *
 * # Arguments
 * * [`idevice`] - The Idevice handle to free
 *
 * # Safety
 * `idevice` must be a valid pointer to an Idevice handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void idevice_free(struct IdeviceHandle *idevice);

/**
 * Frees a stream handle
 *
 * # Safety
 * Pass a valid handle allocated by this library
 */
void idevice_stream_free(struct ReadWriteOpaque *stream_handle);

/**
 * Frees a string allocated by this library
 *
 * # Arguments
 * * [`string`] - The string to free
 *
 * # Safety
 * `string` must be a valid pointer to a string that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void idevice_string_free(char *string);

/**
 * Frees data allocated by this library
 *
 * # Arguments
 * * [`data`] - The data to free
 *
 * # Safety
 * `data` must be a valid pointer to data that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void idevice_data_free(uint8_t *data, uintptr_t len);

/**
 * Frees an array of plists allocated by this library
 *
 * # Safety
 * `data` must be a pointer to data allocated by this library,
 * NOT data allocated by libplist.
 */
void idevice_plist_array_free(plist_t *plists, uintptr_t len);

/**
 * Frees a slice of pointers allocated by this library that had an underlying
 * vec creation.
 *
 * The following functions use an underlying vec and are safe to use:
 * - idevice_usbmuxd_get_devices
 *
 * # Safety
 * Pass a valid pointer passed by the Vec creating functions
 */
void idevice_outer_slice_free(void *slice, uintptr_t len);

/**
 * Connects the adapter to a specific port
 *
 * # Arguments
 * * [`adapter_handle`] - The adapter handle
 * * [`port`] - The port to connect to
 * * [`stream_handle`] - A pointer to allocate the new stream to
 *
 * # Returns
 * Null on success, an IdeviceFfiError otherwise
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library.
 * Any stream allocated must be used in the same thread as the adapter. The handles are NOT thread
 * safe.
 */
struct IdeviceFfiError *adapter_connect(struct AdapterHandle *adapter_handle,
                                        uint16_t port,
                                        struct ReadWriteOpaque **stream_handle);

/**
 * Enables PCAP logging for the adapter
 *
 * # Arguments
 * * [`handle`] - The adapter handle
 * * [`path`] - The path to save the PCAP file (null-terminated string)
 *
 * # Returns
 * Null on success, an IdeviceFfiError otherwise
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `path` must be a valid null-terminated string
 */
struct IdeviceFfiError *adapter_pcap(struct AdapterHandle *handle, const char *path);

/**
 * Closes the adapter stream connection
 *
 * # Arguments
 * * [`handle`] - The adapter stream handle
 *
 * # Returns
 * Null on success, an IdeviceFfiError otherwise
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *adapter_stream_close(struct AdapterStreamHandle *handle);

/**
 * Stops the entire adapter TCP stack
 *
 * # Arguments
 * * [`handle`] - The adapter handle
 *
 * # Returns
 * Null on success, an IdeviceFfiError otherwise
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *adapter_close(struct AdapterHandle *handle);

/**
 * Sends data through the adapter stream
 *
 * # Arguments
 * * [`handle`] - The adapter stream handle
 * * [`data`] - The data to send
 * * [`length`] - The length of the data
 *
 * # Returns
 * Null on success, an IdeviceFfiError otherwise
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `data` must be a valid pointer to at least `length` bytes
 */
struct IdeviceFfiError *adapter_send(struct AdapterStreamHandle *handle,
                                     const uint8_t *data,
                                     uintptr_t length);

/**
 * Receives data from the adapter stream
 *
 * # Arguments
 * * [`handle`] - The adapter stream handle
 * * [`data`] - Pointer to a buffer where the received data will be stored
 * * [`length`] - Pointer to store the actual length of received data
 * * [`max_length`] - Maximum number of bytes that can be stored in `data`
 *
 * # Returns
 * Null on success, an IdeviceFfiError otherwise
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `data` must be a valid pointer to at least `max_length` bytes
 * `length` must be a valid pointer to a usize
 */
struct IdeviceFfiError *adapter_recv(struct AdapterStreamHandle *handle,
                                     uint8_t *data,
                                     uintptr_t *length,
                                     uintptr_t max_length);

/**
 * Connects to the AFC service using a TCP provider
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated AfcClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *afc_client_connect(struct IdeviceProviderHandle *provider,
                                           struct AfcClientHandle **client);

/**
 * Creates a new AfcClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated AfcClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *afc_client_connect_rsd(struct AdapterHandle *provider,
                                               struct RsdHandshakeHandle *handshake,
                                               struct AfcClientHandle **client);

/**
 * Connects to the AFC2 service using a TCP provider
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated AfcClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *afc2_client_connect(struct IdeviceProviderHandle *provider,
                                            struct AfcClientHandle **client);

/**
 * Creates a new AfcClient from an existing Idevice connection
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated AfcClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *afc_client_new(struct IdeviceHandle *socket,
                                       struct AfcClientHandle **client);

/**
 * Frees an AfcClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void afc_client_free(struct AfcClientHandle *handle);

/**
 * Lists the contents of a directory on the device
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`path`] - Path to the directory to list (UTF-8 null-terminated)
 * * [`entries`] - Will be set to point to an array of directory entries
 * * [`count`] - Will be set to the number of entries
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `path` must be a valid null-terminated C string
 */
struct IdeviceFfiError *afc_list_directory(struct AfcClientHandle *client,
                                           const char *path,
                                           char ***entries,
                                           size_t *count);

/**
 * Creates a new directory on the device
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`path`] - Path of the directory to create (UTF-8 null-terminated)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `path` must be a valid null-terminated C string
 */
struct IdeviceFfiError *afc_make_directory(struct AfcClientHandle *client, const char *path);

/**
 * Retrieves information about a file or directory
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`path`] - Path to the file or directory (UTF-8 null-terminated)
 * * [`info`] - Will be populated with file information
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` and `path` must be valid pointers
 * `info` must be a valid pointer to an AfcFileInfo struct
 */
struct IdeviceFfiError *afc_get_file_info(struct AfcClientHandle *client,
                                          const char *path,
                                          struct AfcFileInfo *info);

/**
 * Frees memory allocated by afc_get_file_info
 *
 * # Arguments
 * * [`info`] - Pointer to AfcFileInfo struct to free
 *
 * # Safety
 * `info` must be a valid pointer to an AfcFileInfo struct previously returned by afc_get_file_info
 */
void afc_file_info_free(struct AfcFileInfo *info);

/**
 * Retrieves information about the device's filesystem
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`info`] - Will be populated with device information
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` and `info` must be valid pointers
 */
struct IdeviceFfiError *afc_get_device_info(struct AfcClientHandle *client,
                                            struct AfcDeviceInfo *info);

/**
 * Frees memory allocated by afc_get_device_info
 *
 * # Arguments
 * * [`info`] - Pointer to AfcDeviceInfo struct to free
 *
 * # Safety
 * `info` must be a valid pointer to an AfcDeviceInfo struct previously returned by afc_get_device_info
 */
void afc_device_info_free(struct AfcDeviceInfo *info);

/**
 * Removes a file or directory
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`path`] - Path to the file or directory to remove (UTF-8 null-terminated)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `path` must be a valid null-terminated C string
 */
struct IdeviceFfiError *afc_remove_path(struct AfcClientHandle *client, const char *path);

/**
 * Recursively removes a directory and all its contents
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`path`] - Path to the directory to remove (UTF-8 null-terminated)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `path` must be a valid null-terminated C string
 */
struct IdeviceFfiError *afc_remove_path_and_contents(struct AfcClientHandle *client,
                                                     const char *path);

/**
 * Opens a file on the device
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`path`] - Path to the file to open (UTF-8 null-terminated)
 * * [`mode`] - File open mode
 * * [`handle`] - Will be set to a new file handle on success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `path` must be a valid null-terminated C string.
 * The file handle MAY NOT be used from another thread, and is
 * dependant upon the client it was created by.
 */
struct IdeviceFfiError *afc_file_open(struct AfcClientHandle *client,
                                      const char *path,
                                      enum AfcFopenMode mode,
                                      struct AfcFileHandle **handle);

/**
 * Closes a file handle
 *
 * # Arguments
 * * [`handle`] - File handle to close
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *afc_file_close(struct AfcFileHandle *handle);

/**
 * Reads data from an open file. This advances the cursor of the file.
 *
 * # Arguments
 * * [`handle`] - File handle to read from
 * * [`data`] - Will be set to point to the read data
 * * [`len`] - Number of bytes to read from the file
 * * [`bytes_read`] - The number of bytes read from the file
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 */
struct IdeviceFfiError *afc_file_read(struct AfcFileHandle *handle,
                                      uint8_t **data,
                                      uintptr_t len,
                                      size_t *bytes_read);

/**
 * Reads all data from an open file.
 *
 * # Arguments
 * * [`handle`] - File handle to read from
 * * [`data`] - Will be set to point to the read data
 * * [`length`] - The number of bytes read from the file
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 */
struct IdeviceFfiError *afc_file_read_entire(struct AfcFileHandle *handle,
                                             uint8_t **data,
                                             size_t *length);

/**
 * Moves the read/write cursor in an open file.
 *
 * # Arguments
 * * [`handle`] - File handle whose cursor should be moved
 * * [`offset`] - Distance to move the cursor, interpreted based on `whence`
 * * [`whence`] - Origin used for the seek operation:
 *     * `0` — Seek from the start of the file (`SeekFrom::Start`)
 *     * `1` — Seek from the current cursor position (`SeekFrom::Current`)
 *     * `2` — Seek from the end of the file (`SeekFrom::End`)
 * * [`new_pos`] - Output parameter; will be set to the new absolute cursor position
 *
 * # Returns
 * An [`IdeviceFfiError`] on error, or null on success.
 *
 * # Safety
 * All pointers must be valid and non-null.
 *
 * # Notes
 * * If `whence` is invalid, this function returns `FfiInvalidArg`.
 * * The AFC protocol may restrict seeking beyond certain bounds; such errors
 *   are reported through the returned [`IdeviceFfiError`].
 */
struct IdeviceFfiError *afc_file_seek(struct AfcFileHandle *handle,
                                      int64_t offset,
                                      int whence,
                                      int64_t *new_pos);

/**
 * Returns the current read/write cursor position of an open file.
 *
 * # Arguments
 * * [`handle`] - File handle whose cursor should be queried
 * * [`pos`] - Output parameter; will be set to the current absolute cursor position
 *
 * # Returns
 * An [`IdeviceFfiError`] on error, or null on success.
 *
 * # Safety
 * All pointers must be valid and non-null.
 *
 * # Notes
 * This function is equivalent to performing a seek operation with
 * `SeekFrom::Current(0)` internally.
 */
struct IdeviceFfiError *afc_file_tell(struct AfcFileHandle *handle, int64_t *pos);

/**
 * Writes data to an open file
 *
 * # Arguments
 * * [`handle`] - File handle to write to
 * * [`data`] - Data to write
 * * [`length`] - Length of data to write
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `data` must point to at least `length` bytes
 */
struct IdeviceFfiError *afc_file_write(struct AfcFileHandle *handle,
                                       const uint8_t *data,
                                       size_t length);

/**
 * Creates a hard or symbolic link
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`target`] - Target path of the link (UTF-8 null-terminated)
 * * [`source`] - Path where the link should be created (UTF-8 null-terminated)
 * * [`link_type`] - Type of link to create
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `target` and `source` must be valid null-terminated C strings
 */
struct IdeviceFfiError *afc_make_link(struct AfcClientHandle *client,
                                      const char *target,
                                      const char *source,
                                      enum AfcLinkType link_type);

/**
 * Renames a file or directory
 *
 * # Arguments
 * * [`client`] - A valid AfcClient handle
 * * [`source`] - Current path of the file/directory (UTF-8 null-terminated)
 * * [`target`] - New path for the file/directory (UTF-8 null-terminated)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `source` and `target` must be valid null-terminated C strings
 */
struct IdeviceFfiError *afc_rename_path(struct AfcClientHandle *client,
                                        const char *source,
                                        const char *target);

/**
 * Frees memory allocated by a file read function allocated by this library
 *
 * # Arguments
 * * [`info`] - Pointer to AfcDeviceInfo struct to free
 *
 * # Safety
 * `info` must be a valid pointer to an AfcDeviceInfo struct previously returned by afc_get_device_info
 */
void afc_file_read_data_free(uint8_t *data,
                             size_t length);

/**
 * Automatically creates and connects to AMFI service, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated AmfiClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *amfi_connect(struct IdeviceProviderHandle *provider,
                                     struct AmfiClientHandle **client);

/**
 * Creates a new AmfiClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated AmfiClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *amfi_connect_rsd(struct AdapterHandle *provider,
                                         struct RsdHandshakeHandle *handshake,
                                         struct AmfiClientHandle **client);

/**
 * Automatically creates and connects to AMFI service, returning a client handle
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated AmfiClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. It is consumed, and
 * should not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *amfi_new(struct IdeviceHandle *socket, struct AmfiClientHandle **client);

/**
 * Shows the option in the settings UI
 *
 * # Arguments
 * * `client` - A valid AmfiClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *amfi_reveal_developer_mode_option_in_ui(struct AmfiClientHandle *client);

/**
 * Enables developer mode on the device
 *
 * # Arguments
 * * `client` - A valid AmfiClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *amfi_enable_developer_mode(struct AmfiClientHandle *client);

/**
 * Accepts developer mode on the device
 *
 * # Arguments
 * * `client` - A valid AmfiClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *amfi_accept_developer_mode(struct AmfiClientHandle *client);

/**
 * Frees a handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void amfi_client_free(struct AmfiClientHandle *handle);

/**
 * Automatically creates and connects to BTPacketLogger, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated BtPacketLoggerClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *bt_packet_logger_connect(struct IdeviceProviderHandle *provider,
                                                 struct BtPacketLoggerClientHandle **client);

/**
 * Creates a new BtPacketLoggerClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated BtPacketLoggerClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *bt_packet_logger_connect_rsd(struct AdapterHandle *provider,
                                                     struct RsdHandshakeHandle *handshake,
                                                     struct BtPacketLoggerClientHandle **client);

/**
 * Creates a new BtPacketLoggerClient from an existing socket
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated BtPacketLoggerClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *bt_packet_logger_new(struct IdeviceHandle *socket,
                                             struct BtPacketLoggerClientHandle **client);

/**
 * Reads the next BT packet from the logger
 *
 * # Arguments
 * * `client` - A valid BtPacketLoggerClient handle
 * * `packet` - On success, will be set to point to a newly allocated BtPacketHandle.
 *   May be set to NULL if EOF was reached.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * The returned packet must be freed with `bt_packet_free`
 */
struct IdeviceFfiError *bt_packet_logger_next_packet(struct BtPacketLoggerClientHandle *client,
                                                     struct BtPacketHandle **packet);

/**
 * Frees a BtPacketHandle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void bt_packet_free(struct BtPacketHandle *handle);

/**
 * Frees a BtPacketLoggerClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void bt_packet_logger_client_free(struct BtPacketLoggerClientHandle *handle);

/**
 * Automatically creates and connects to Companion Proxy, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated CompanionProxy handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *companion_proxy_connect(struct IdeviceProviderHandle *provider,
                                                struct CompanionProxyClientHandle **client);

/**
 * Creates a new CompanionProxy client via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated CompanionProxy handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *companion_proxy_connect_rsd(struct AdapterHandle *provider,
                                                    struct RsdHandshakeHandle *handshake,
                                                    struct CompanionProxyClientHandle **client);

/**
 * Creates a new CompanionProxy client from an existing socket
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated CompanionProxy handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *companion_proxy_new(struct IdeviceHandle *socket,
                                            struct CompanionProxyClientHandle **client);

/**
 * Gets the device registry from Companion Proxy, returning paired watch UDIDs
 *
 * # Arguments
 * * `client` - A valid CompanionProxy handle
 * * `udids` - On success, will be set to point to a newly allocated array of C strings
 * * `udids_len` - On success, will be set to the length of the array
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * The returned strings must be freed with `idevice_string_free` and the outer array
 * with `idevice_outer_slice_free`
 */
struct IdeviceFfiError *companion_proxy_get_device_registry(struct CompanionProxyClientHandle *client,
                                                            char ***udids,
                                                            uintptr_t *udids_len);

/**
 * Starts forwarding a service port through the companion proxy
 *
 * # Arguments
 * * `client` - A valid CompanionProxy handle
 * * `port` - The remote port number on the watch
 * * `local_port` - On success, will be set to the local forwarded port number
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *companion_proxy_start_forwarding_service_port(struct CompanionProxyClientHandle *client,
                                                                      uint16_t port,
                                                                      uint16_t *local_port);

/**
 * Stops forwarding a service port through the companion proxy
 *
 * # Arguments
 * * `client` - A valid CompanionProxy handle
 * * `port` - The remote port number to stop forwarding
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *companion_proxy_stop_forwarding_service_port(struct CompanionProxyClientHandle *client,
                                                                     uint16_t port);

/**
 * Frees a CompanionProxy client handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void companion_proxy_client_free(struct CompanionProxyClientHandle *handle);

/**
 * Creates a new AppServiceClient using RSD connection
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` and `handshake` must be valid pointers to handles allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *app_service_connect_rsd(struct AdapterHandle *provider,
                                                struct RsdHandshakeHandle *handshake,
                                                struct AppServiceHandle **handle);

/**
 * Creates a new AppServiceClient from a socket
 *
 * # Arguments
 * * [`socket`] - The socket to use for communication
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *app_service_new(struct ReadWriteOpaque *socket,
                                        struct AppServiceHandle **handle);

/**
 * Frees an AppServiceClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void app_service_free(struct AppServiceHandle *handle);

/**
 * Lists applications on the device
 *
 * # Arguments
 * * [`handle`] - The AppServiceClient handle
 * * [`app_clips`] - Include app clips
 * * [`removable_apps`] - Include removable apps
 * * [`hidden_apps`] - Include hidden apps
 * * [`internal_apps`] - Include internal apps
 * * [`default_apps`] - Include default apps
 * * [`apps`] - Pointer to store the array of apps (caller must free)
 * * [`count`] - Pointer to store the number of apps
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle`, `apps`, and `count` must be valid pointers
 */
struct IdeviceFfiError *app_service_list_apps(struct AppServiceHandle *handle,
                                              int app_clips,
                                              int removable_apps,
                                              int hidden_apps,
                                              int internal_apps,
                                              int default_apps,
                                              struct AppListEntryC **apps,
                                              uintptr_t *count);

/**
 * Frees an array of AppListEntryC structures
 *
 * # Safety
 * `apps` must be a valid pointer to an array allocated by app_service_list_apps
 * `count` must match the count returned by app_service_list_apps
 */
void app_service_free_app_list(struct AppListEntryC *apps, uintptr_t count);

/**
 * Launches an application
 *
 * # Arguments
 * * [`handle`] - The AppServiceClient handle
 * * [`bundle_id`] - Bundle identifier of the app to launch
 * * [`argv`] - NULL-terminated array of arguments
 * * [`argc`] - Number of arguments
 * * [`kill_existing`] - Whether to kill existing instances
 * * [`start_suspended`] - Whether to start suspended
 * * [`stdio_uuid`] - The UUID received from openstdiosocket, null for none
 * * [`response`] - Pointer to store the launch response (caller must free)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *app_service_launch_app(struct AppServiceHandle *handle,
                                               const char *bundle_id,
                                               const char *const *argv,
                                               uintptr_t argc,
                                               int kill_existing,
                                               int start_suspended,
                                               const uint8_t *stdio_uuid,
                                               struct LaunchResponseC **response);

/**
 * Frees a LaunchResponseC structure
 *
 * # Safety
 * `response` must be a valid pointer allocated by app_service_launch_app
 */
void app_service_free_launch_response(struct LaunchResponseC *response);

/**
 * Lists running processes
 *
 * # Arguments
 * * [`handle`] - The AppServiceClient handle
 * * [`processes`] - Pointer to store the array of processes (caller must free)
 * * [`count`] - Pointer to store the number of processes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *app_service_list_processes(struct AppServiceHandle *handle,
                                                   struct ProcessTokenC **processes,
                                                   uintptr_t *count);

/**
 * Frees an array of ProcessTokenC structures
 *
 * # Safety
 * `processes` must be a valid pointer allocated by app_service_list_processes
 * `count` must match the count returned by app_service_list_processes
 */
void app_service_free_process_list(struct ProcessTokenC *processes, uintptr_t count);

/**
 * Uninstalls an application
 *
 * # Arguments
 * * [`handle`] - The AppServiceClient handle
 * * [`bundle_id`] - Bundle identifier of the app to uninstall
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *app_service_uninstall_app(struct AppServiceHandle *handle,
                                                  const char *bundle_id);

/**
 * Sends a signal to a process
 *
 * # Arguments
 * * [`handle`] - The AppServiceClient handle
 * * [`pid`] - Process ID
 * * [`signal`] - Signal number
 * * [`response`] - Pointer to store the signal response (caller must free)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *app_service_send_signal(struct AppServiceHandle *handle,
                                                uint32_t pid,
                                                uint32_t signal,
                                                struct SignalResponseC **response);

/**
 * Frees a SignalResponseC structure
 *
 * # Safety
 * `response` must be a valid pointer allocated by app_service_send_signal
 */
void app_service_free_signal_response(struct SignalResponseC *response);

/**
 * Creates a new ConfigurationServiceClient using RSD connection
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` and `handshake` must be valid pointers to handles allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *configuration_service_connect_rsd(struct AdapterHandle *provider,
                                                          struct RsdHandshakeHandle *handshake,
                                                          struct ConfigurationServiceHandle **handle);

/**
 * Creates a new ConfigurationServiceClient from a socket
 *
 * # Arguments
 * * [`socket`] - The socket to use for communication. Consumed regardless of the result.
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *configuration_service_new(struct ReadWriteOpaque *socket,
                                                  struct ConfigurationServiceHandle **handle);

/**
 * Reads the device's light/dark appearance
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`style`] - Pointer to store the appearance
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *configuration_service_get_user_interface_style(struct ConfigurationServiceHandle *handle,
                                                                       enum IdeviceUserInterfaceStyle *style);

/**
 * Switches the device between light and dark appearance
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`style`] - The appearance to set
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *configuration_service_set_user_interface_style(struct ConfigurationServiceHandle *handle,
                                                                       enum IdeviceUserInterfaceStyle style);

/**
 * Sets the system liquid-glass opacity
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`opacity`] - The opacity, 0.0 to 1.0
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *configuration_service_set_liquid_glass_opacity(struct ConfigurationServiceHandle *handle,
                                                                       float opacity);

/**
 * Reads the accessibility color filter's state
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`filter`] - Pointer to store the state. Free its `filter_type` with
 *   `idevice_string_free`.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *configuration_service_get_color_filter(struct ConfigurationServiceHandle *handle,
                                                               struct ColorFilterC *filter);

/**
 * Enables or disables the accessibility color filter
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`enabled`] - Whether the filter is on
 * * [`filter_type`] - The preset to use, e.g. `Protanopia`. Required when enabling,
 *   ignored otherwise, and may be NULL when disabling.
 * * [`intensity`] - Filter strength, 0.0 to 1.0. Ignored unless `has_intensity` is set.
 * * [`has_intensity`] - Whether to send `intensity`
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All non-NULL pointer parameters must be valid
 */
struct IdeviceFfiError *configuration_service_set_color_filter(struct ConfigurationServiceHandle *handle,
                                                               int enabled,
                                                               const char *filter_type,
                                                               float intensity,
                                                               int has_intensity);

/**
 * Reads the dynamic-type size's name, e.g. `medium` or `large`
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`size`] - Pointer to store the name. Free with `idevice_string_free`.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *configuration_service_get_device_text_size(struct ConfigurationServiceHandle *handle,
                                                                   char **size);

/**
 * Sets the dynamic-type size by name, e.g. `medium` or `large`
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`size`] - The size's name
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *configuration_service_set_device_text_size(struct ConfigurationServiceHandle *handle,
                                                                   const char *size);

/**
 * Reads whether Reduce Motion is on
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`enabled`] - Pointer to store the state
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *configuration_service_get_reduce_motion(struct ConfigurationServiceHandle *handle,
                                                                int *enabled);

/**
 * Toggles Reduce Motion
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`enabled`] - Whether to turn it on
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *configuration_service_set_reduce_motion(struct ConfigurationServiceHandle *handle,
                                                                int enabled);

/**
 * Reads whether Reduce Transparency is on
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`enabled`] - Pointer to store the state
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *configuration_service_get_reduce_transparency(struct ConfigurationServiceHandle *handle,
                                                                      int *enabled);

/**
 * Toggles Reduce Transparency
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`enabled`] - Whether to turn it on
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *configuration_service_set_reduce_transparency(struct ConfigurationServiceHandle *handle,
                                                                      int enabled);

/**
 * Reads whether the layout-debug borders overlay is on
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`enabled`] - Pointer to store the state
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *configuration_service_get_show_borders(struct ConfigurationServiceHandle *handle,
                                                               int *enabled);

/**
 * Toggles the layout-debug borders overlay
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`enabled`] - Whether to turn it on
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *configuration_service_set_show_borders(struct ConfigurationServiceHandle *handle,
                                                               int enabled);

/**
 * Toggles Increase Contrast
 *
 * The device offers no getter for this one.
 *
 * # Arguments
 * * [`handle`] - The ConfigurationServiceClient handle
 * * [`enabled`] - Whether to turn it on
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *configuration_service_set_increase_contrast(struct ConfigurationServiceHandle *handle,
                                                                    int enabled);

/**
 * Frees a ConfigurationServiceClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void configuration_service_free(struct ConfigurationServiceHandle *handle);

/**
 * Creates a new DiagnosticsServiceClient using RSD connection
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` and `handshake` must be valid pointers to handles allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *diagnostics_service_connect_rsd(struct AdapterHandle *provider,
                                                        struct RsdHandshakeHandle *handshake,
                                                        struct DiagnosticsServiceHandle **handle);

/**
 * Creates a new DiagnostisServiceClient from a socket
 *
 * # Arguments
 * * [`socket`] - The socket to use for communication
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *diagnostics_service_new(struct ReadWriteOpaque *socket,
                                                struct DiagnosticsServiceHandle **handle);

/**
 * Captures a sysdiagnose from the device.
 * Note that this will take a LONG time to return while the device collects enough information to
 * return to the service. This function returns a stream that can be called on to get the next
 * chunk of data. A typical sysdiagnose is roughly 1-2 GB.
 *
 * # Arguments
 * * [`handle`] - The handle to the client
 * * [`dry_run`] - Whether or not to do a dry run with a simple .txt file from the device
 * * [`preferred_filename`] - The name the device wants to save the sysdaignose as
 * * [`expected_length`] - The size in bytes of the sysdiagnose
 * * [`stream_handle`] - The handle that will be set to capture bytes for
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * Pointers must be all valid. Handle must be allocated by this library. Preferred filename must
 * be freed `idevice_string_free`.
 */
struct IdeviceFfiError *diagnostics_service_capture_sysdiagnose(struct DiagnosticsServiceHandle *handle,
                                                                bool dry_run,
                                                                char **preferred_filename,
                                                                uintptr_t *expected_length,
                                                                struct SysdiagnoseStreamHandle **stream_handle);

/**
 * Gets the next packet from the stream.
 * Data will be set to 0 when there is no more data to get from the stream.
 *
 * # Arguments
 * * [`handle`] - The handle to the stream
 * * [`data`] - A pointer to the bytes
 * * [`len`] - The length of the bytes written
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * Pass valid pointers. The handle must be allocated by this library.
 */
struct IdeviceFfiError *sysdiagnose_stream_next(struct SysdiagnoseStreamHandle *handle,
                                                uint8_t **data,
                                                uintptr_t *len);

/**
 * Frees a DiagnostisServiceClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void diagnostics_service_free(struct DiagnosticsServiceHandle *handle);

/**
 * Frees a SysdiagnoseStreamHandle handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void sysdiagnose_stream_free(struct SysdiagnoseStreamHandle *handle);

/**
 * Creates a new FileServiceClient using RSD connection
 *
 * This connects the service's control channel, i.e.
 * `com.apple.coredevice.fileservice.control`. Downloads additionally need the
 * data channel, `com.apple.coredevice.fileservice.data`.
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` and `handshake` must be valid pointers to handles allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *file_service_connect_rsd(struct AdapterHandle *provider,
                                                 struct RsdHandshakeHandle *handshake,
                                                 struct FileServiceHandle **handle);

/**
 * Creates a new FileServiceClient from a socket
 *
 * # Arguments
 * * [`socket`] - The socket to use for communication. Consumed regardless of the result.
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *file_service_new(struct ReadWriteOpaque *socket,
                                         struct FileServiceHandle **handle);

/**
 * Opens a session on a domain, which every later command is scoped to
 *
 * # Arguments
 * * [`handle`] - The FileServiceClient handle
 * * [`domain`] - The domain to scope the session to
 * * [`identifier`] - The container's identifier, i.e. a bundle ID or an app-group ID.
 *   The domains that don't take one ignore it, and it may be NULL for them.
 * * [`session_id`] - Pointer to store the new session's ID, or NULL to ignore it.
 *   Free with `idevice_string_free`.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All non-NULL pointer parameters must be valid
 */
struct IdeviceFfiError *file_service_create_session(struct FileServiceHandle *handle,
                                                    enum IdeviceFileServiceDomain domain,
                                                    const char *identifier,
                                                    char **session_id);

/**
 * The session ID from the last `file_service_create_session`
 *
 * # Arguments
 * * [`handle`] - The FileServiceClient handle
 * * [`session_id`] - Pointer to store the ID, set to NULL when there is no
 *   session. Free with `idevice_string_free`.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *file_service_session_id(struct FileServiceHandle *handle,
                                                char **session_id);

/**
 * Lists a directory, relative to the session's domain root
 *
 * # Arguments
 * * [`handle`] - The FileServiceClient handle
 * * [`path`] - The directory to list
 * * [`entries`] - Pointer to store the entry names, freed with
 *   `file_service_free_directory_list`
 * * [`len`] - Pointer to store the number of entries
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *file_service_retrieve_directory_list(struct FileServiceHandle *handle,
                                                             const char *path,
                                                             char ***entries,
                                                             uintptr_t *len);

/**
 * Frees the list from `file_service_retrieve_directory_list`
 *
 * # Safety
 * `entries` must be a pointer returned by `file_service_retrieve_directory_list`
 * with its reported length, or NULL
 */
void file_service_free_directory_list(char **entries, uintptr_t len);

/**
 * Downloads a file, relative to the session's domain root
 *
 * The transfer itself runs on the service's data channel, which the caller
 * opens by connecting the adapter to the port the RSD handshake reports for
 * `com.apple.coredevice.fileservice.data`.
 *
 * # Arguments
 * * [`handle`] - The FileServiceClient handle
 * * [`path`] - The file to download
 * * [`adapter`] - The adapter the control channel was connected over
 * * [`data_port`] - The port of `com.apple.coredevice.fileservice.data`
 * * [`data`] - Pointer to store the contents, freed with `idevice_data_free`
 * * [`len`] - Pointer to store the number of bytes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *file_service_retrieve_file(struct FileServiceHandle *handle,
                                                   const char *path,
                                                   struct AdapterHandle *adapter,
                                                   uint16_t data_port,
                                                   uint8_t **data,
                                                   uintptr_t *len);

/**
 * Downloads a file over a data channel the caller already opened
 *
 * Like `file_service_retrieve_file`, but takes the data channel itself instead
 * of opening one. Note that the device only accepts the connection once the
 * control channel has announced the transfer, so a stream opened well in
 * advance may have been dropped.
 *
 * # Arguments
 * * [`handle`] - The FileServiceClient handle
 * * [`path`] - The file to download
 * * [`data_stream`] - The data channel. Consumed regardless of the result.
 * * [`data`] - Pointer to store the contents, freed with `idevice_data_free`
 * * [`len`] - Pointer to store the number of bytes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *file_service_retrieve_file_with_stream(struct FileServiceHandle *handle,
                                                               const char *path,
                                                               struct ReadWriteOpaque *data_stream,
                                                               uint8_t **data,
                                                               uintptr_t *len);

/**
 * Creates an empty file, relative to the session's domain root
 *
 * # Arguments
 * * [`handle`] - The FileServiceClient handle
 * * [`path`] - The file to create
 * * [`file_permissions`] - The file's mode, e.g. 0644
 * * [`uid`] - The owning user's ID, e.g. 501
 * * [`gid`] - The owning group's ID, e.g. 501
 * * [`creation_time`] - The creation time to set
 * * [`last_modification_time`] - The modification time to set
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *file_service_propose_empty_file(struct FileServiceHandle *handle,
                                                        const char *path,
                                                        uint32_t file_permissions,
                                                        uint32_t uid,
                                                        uint32_t gid,
                                                        int64_t creation_time,
                                                        int64_t last_modification_time);

/**
 * Looks a domain up by the name the device uses, e.g. `appDataContainer`
 *
 * # Arguments
 * * [`name`] - The domain's name
 * * [`domain`] - Pointer to store the domain
 *
 * # Returns
 * 1 when the name is known, 0 otherwise
 *
 * # Safety
 * All pointer parameters must be valid
 */
int file_service_domain_from_name(const char *name, enum IdeviceFileServiceDomain *domain);

/**
 * Frees a FileServiceClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void file_service_free(struct FileServiceHandle *handle);

/**
 * Creates a new IconServiceClient using RSD connection
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` and `handshake` must be valid pointers to handles allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *icon_service_connect_rsd(struct AdapterHandle *provider,
                                                 struct RsdHandshakeHandle *handshake,
                                                 struct IconServiceHandle **handle);

/**
 * Creates a new IconServiceClient from a socket
 *
 * # Arguments
 * * [`socket`] - The socket to use for communication. Consumed regardless of the result.
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *icon_service_new(struct ReadWriteOpaque *socket,
                                         struct IconServiceHandle **handle);

/**
 * Fetches an app's icon, rendered as a PNG
 *
 * # Arguments
 * * [`handle`] - The IconServiceClient handle
 * * [`bundle_identifier`] - Bundle identifier of the app, or NULL to use `app_path`
 * * [`app_path`] - Path of the app on the device, or NULL to use `bundle_identifier`
 * * [`width`] - Requested icon width in points
 * * [`height`] - Requested icon height in points
 * * [`scale`] - Requested icon scale
 * * [`allow_placeholder`] - Whether the device may render a generic placeholder
 * * [`icon`] - Pointer to store the icon, freed with `icon_service_free_icon`
 *
 * Exactly one of `bundle_identifier` and `app_path` must be passed.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All non-NULL pointer parameters must be valid
 */
struct IdeviceFfiError *icon_service_fetch_icon(struct IconServiceHandle *handle,
                                                const char *bundle_identifier,
                                                const char *app_path,
                                                float width,
                                                float height,
                                                float scale,
                                                int allow_placeholder,
                                                struct AppIconC **icon);

/**
 * Frees an AppIconC
 *
 * # Safety
 * `icon` must be a pointer returned by `icon_service_fetch_icon`, or NULL
 */
void icon_service_free_icon(struct AppIconC *icon);

/**
 * Frees an IconServiceClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void icon_service_free(struct IconServiceHandle *handle);

/**
 * Automatically creates and connects to Core Device Proxy, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated CoreDeviceProxy handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *core_device_proxy_connect(struct IdeviceProviderHandle *provider,
                                                  struct CoreDeviceProxyHandle **client);

/**
 * Automatically creates and connects to Core Device Proxy, returning a client handle
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated CoreDeviceProxy handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. It is consumed and
 * may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *core_device_proxy_new(struct IdeviceHandle *socket,
                                              struct CoreDeviceProxyHandle **client);

/**
 * Sends data through the CoreDeviceProxy tunnel
 *
 * # Arguments
 * * [`handle`] - The CoreDeviceProxy handle
 * * [`data`] - The data to send
 * * [`length`] - The length of the data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `data` must be a valid pointer to at least `length` bytes
 */
struct IdeviceFfiError *core_device_proxy_send(struct CoreDeviceProxyHandle *handle,
                                               const uint8_t *data,
                                               uintptr_t length);

/**
 * Receives data from the CoreDeviceProxy tunnel
 *
 * # Arguments
 * * [`handle`] - The CoreDeviceProxy handle
 * * [`data`] - Pointer to a buffer where the received data will be stored
 * * [`length`] - Pointer to store the actual length of received data
 * * [`max_length`] - Maximum number of bytes that can be stored in `data`
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `data` must be a valid pointer to at least `max_length` bytes
 * `length` must be a valid pointer to a usize
 */
struct IdeviceFfiError *core_device_proxy_recv(struct CoreDeviceProxyHandle *handle,
                                               uint8_t *data,
                                               uintptr_t *length,
                                               uintptr_t max_length);

/**
 * Gets the client parameters from the handshake
 *
 * # Arguments
 * * [`handle`] - The CoreDeviceProxy handle
 * * [`mtu`] - Pointer to store the MTU value
 * * [`address`] - Pointer to store the IP address string
 * * [`netmask`] - Pointer to store the netmask string
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `mtu` must be a valid pointer to a u16
 * `address` and `netmask` must be valid pointers to buffers of at least 16 bytes
 */
struct IdeviceFfiError *core_device_proxy_get_client_parameters(struct CoreDeviceProxyHandle *handle,
                                                                uint16_t *mtu,
                                                                char **address,
                                                                char **netmask);

/**
 * Gets the server address from the handshake
 *
 * # Arguments
 * * [`handle`] - The CoreDeviceProxy handle
 * * [`address`] - Pointer to store the server address string
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `address` must be a valid pointer to a buffer of at least 16 bytes
 */
struct IdeviceFfiError *core_device_proxy_get_server_address(struct CoreDeviceProxyHandle *handle,
                                                             char **address);

/**
 * Gets the server RSD port from the handshake
 *
 * # Arguments
 * * [`handle`] - The CoreDeviceProxy handle
 * * [`port`] - Pointer to store the port number
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `port` must be a valid pointer to a u16
 */
struct IdeviceFfiError *core_device_proxy_get_server_rsd_port(struct CoreDeviceProxyHandle *handle,
                                                              uint16_t *port);

/**
 * Creates a software TCP tunnel adapter
 *
 * # Arguments
 * * [`handle`] - The CoreDeviceProxy handle
 * * [`adapter`] - Pointer to store the newly created adapter handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library, and never used again
 * `adapter` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *core_device_proxy_create_tcp_adapter(struct CoreDeviceProxyHandle *handle,
                                                             struct AdapterHandle **adapter);

/**
 * Frees a handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void core_device_proxy_free(struct CoreDeviceProxyHandle *handle);

/**
 * Frees a handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void adapter_free(struct AdapterHandle *handle);

/**
 * Automatically creates and connects to the crash report copy mobile service,
 * returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *crash_report_client_connect(struct IdeviceProviderHandle *provider,
                                                    struct CrashReportCopyMobileHandle **client);

/**
 * Creates a new CrashReportCopyMobileClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *crash_report_client_connect_rsd(struct AdapterHandle *provider,
                                                        struct RsdHandshakeHandle *handshake,
                                                        struct CrashReportCopyMobileHandle **client);

/**
 * Creates a new CrashReportCopyMobile client from an existing Idevice connection
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *crash_report_client_new(struct IdeviceHandle *socket,
                                                struct CrashReportCopyMobileHandle **client);

/**
 * Lists crash report files in the specified directory
 *
 * # Arguments
 * * [`client`] - A valid CrashReportCopyMobile handle
 * * [`dir_path`] - Optional directory path (NULL for root "/")
 * * [`entries`] - Will be set to point to an array of C strings
 * * [`count`] - Will be set to the number of entries
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `dir_path` may be NULL (defaults to root)
 * Caller must free the returned array with `afc_free_directory_entries`
 */
struct IdeviceFfiError *crash_report_client_ls(struct CrashReportCopyMobileHandle *client,
                                               const char *dir_path,
                                               char ***entries,
                                               size_t *count);

/**
 * Downloads a crash report file from the device
 *
 * # Arguments
 * * [`client`] - A valid CrashReportCopyMobile handle
 * * [`log_name`] - Name of the log file to download (C string)
 * * [`data`] - Will be set to point to the file contents
 * * [`length`] - Will be set to the size of the data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `log_name` must be a valid C string
 * Caller must free the returned data with `idevice_data_free`
 */
struct IdeviceFfiError *crash_report_client_pull(struct CrashReportCopyMobileHandle *client,
                                                 const char *log_name,
                                                 uint8_t **data,
                                                 size_t *length);

/**
 * Removes a crash report file from the device
 *
 * # Arguments
 * * [`client`] - A valid CrashReportCopyMobile handle
 * * [`log_name`] - Name of the log file to remove (C string)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `log_name` must be a valid C string
 */
struct IdeviceFfiError *crash_report_client_remove(struct CrashReportCopyMobileHandle *client,
                                                   const char *log_name);

/**
 * Converts this client to an AFC client for advanced file operations
 *
 * # Arguments
 * * [`client`] - A valid CrashReportCopyMobile handle (will be consumed)
 * * [`afc_client`] - On success, will be set to an AFC client handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer (will be freed after this call)
 * `afc_client` must be a valid, non-null pointer where the new AFC client will be stored
 */
struct IdeviceFfiError *crash_report_client_to_afc(struct CrashReportCopyMobileHandle *client,
                                                   struct AfcClientHandle **afc_client);

/**
 * Triggers a flush of crash logs from system storage
 *
 * This connects to the crashreportmover service to move crash logs
 * into the AFC-accessible directory. Should be called before listing logs.
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *crash_report_flush(struct IdeviceProviderHandle *provider);

/**
 * Frees a CrashReportCopyMobile client handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void crash_report_client_free(struct CrashReportCopyMobileHandle *handle);

/**
 * Creates a new CryptexdClient using RSD connection
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` and `handshake` must be valid pointers to handles allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *cryptexd_connect_rsd(struct AdapterHandle *provider,
                                             struct RsdHandshakeHandle *handshake,
                                             struct CryptexdHandle **handle);

/**
 * Creates a new CryptexdClient from a socket
 *
 * # Arguments
 * * [`socket`] - The socket to use for communication. Consumed regardless of the result.
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *cryptexd_new(struct ReadWriteOpaque *socket,
                                     struct CryptexdHandle **handle);

/**
 * Reads the device's AppleImage4 chip instance, which identifies it in a
 * Cryptex1 personalization request
 *
 * The keys are the daemon's `img4_chip_*` names, e.g. `img4_chip_chip`
 * (ChipID), `img4_chip_bord` (BoardID) and `img4_chip_ecid` (ECID).
 *
 * # Arguments
 * * [`handle`] - The CryptexdClient handle. Consumed by this call.
 * * [`identifiers`] - Pointer to store the identifiers
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *cryptexd_read_personalization_identifiers(struct CryptexdHandle *handle,
                                                                  plist_t *identifiers);

/**
 * Lists the cryptexes installed on the device
 *
 * # Arguments
 * * [`handle`] - The CryptexdClient handle. Consumed by this call.
 * * [`cryptexes`] - Pointer to store the list, freed with `cryptexd_free_installed`
 * * [`len`] - Pointer to store the number of entries
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *cryptexd_copy_installed(struct CryptexdHandle *handle,
                                                struct InstalledCryptexC **cryptexes,
                                                uintptr_t *len);

/**
 * Frees the list from `cryptexd_copy_installed`
 *
 * # Safety
 * `cryptexes` must be a pointer returned by `cryptexd_copy_installed` with its
 * reported length, or NULL
 */
void cryptexd_free_installed(struct InstalledCryptexC *cryptexes, uintptr_t len);

/**
 * Frees an InstalledCryptexC allocated by this library
 *
 * # Safety
 * `cryptex` must be a pointer allocated by this library, or NULL
 */
void cryptexd_free_installed_cryptex(struct InstalledCryptexC *cryptex);

/**
 * Reads a nonce domain's nonce structure
 *
 * Use `cryptexd_cryptex_nonce` for the nonce a TSS request wants.
 *
 * # Arguments
 * * [`handle`] - The CryptexdClient handle. Consumed by this call.
 * * [`domain`] - The nonce domain to read
 * * [`nonce`] - Pointer to store the nonce, freed with `idevice_data_free`
 * * [`nonce_len`] - Pointer to store the number of bytes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *cryptexd_get_nonce(struct CryptexdHandle *handle,
                                           struct CryptexNonceDomain domain,
                                           uint8_t **nonce,
                                           uintptr_t *nonce_len);

/**
 * Reads the nonce a Cryptex1 TSS request is personalized against
 *
 * # Arguments
 * * [`handle`] - The CryptexdClient handle. Consumed by this call.
 * * [`nonce_domain_handle`] - The build identity's `Cryptex1,NonceDomain`
 * * [`nonce`] - Pointer to store the nonce, freed with `idevice_data_free`
 * * [`nonce_len`] - Pointer to store the number of bytes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *cryptexd_cryptex_nonce(struct CryptexdHandle *handle,
                                               uint64_t nonce_domain_handle,
                                               uint8_t **nonce,
                                               uintptr_t *nonce_len);

/**
 * Rolls (regenerates) a nonce domain's nonce, invalidating anything
 * personalized against the previous one
 *
 * # Arguments
 * * [`handle`] - The CryptexdClient handle. Consumed by this call.
 * * [`domain`] - The nonce domain to roll
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *cryptexd_roll_nonce(struct CryptexdHandle *handle,
                                            struct CryptexNonceDomain domain);

/**
 * Uninstalls a cryptex by the identifier `cryptexd_copy_installed` reports
 *
 * # Arguments
 * * [`handle`] - The CryptexdClient handle. Consumed by this call.
 * * [`identifier`] - The cryptex's identifier
 * * [`version`] - The version to scope the uninstall to, or NULL for all of them
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All non-NULL pointer parameters must be valid
 */
struct IdeviceFfiError *cryptexd_uninstall(struct CryptexdHandle *handle,
                                           const char *identifier,
                                           const char *version);

/**
 * Installs a cryptex
 *
 * # Arguments
 * * [`handle`] - The CryptexdClient handle. Consumed by this call.
 * * [`request`] - The payloads and parameters to install
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid, and the request's buffers must be
 * readable for their stated lengths
 */
struct IdeviceFfiError *cryptexd_install(struct CryptexdHandle *handle,
                                         const struct CryptexInstallRequestC *request);

/**
 * Extracts the nonce from cryptexd's nonce structure
 *
 * # Arguments
 * * [`blob`] - The structure `cryptexd_get_nonce` returned
 * * [`blob_len`] - Its length
 * * [`nonce`] - Pointer to store the nonce, freed with `idevice_data_free`
 * * [`nonce_len`] - Pointer to store the number of bytes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid, and `blob` must be readable for
 * `blob_len` bytes
 */
struct IdeviceFfiError *cryptexd_unwrap_nonce(const uint8_t *blob,
                                              uintptr_t blob_len,
                                              uint8_t **nonce,
                                              uintptr_t *nonce_len);

/**
 * Loads the DeveloperDiskImage payloads from an unpacked DDI `Restore` directory
 *
 * # Arguments
 * * [`restore_dir`] - The directory to read
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *cryptex1_assets_load(const char *restore_dir,
                                             struct Cryptex1AssetsHandle **handle);

/**
 * Builds the DeveloperDiskImage payloads from buffers the caller already has
 *
 * # Arguments
 * * [`image`] / [`image_len`] - `Cryptex1,GenericDmg`
 * * [`trustcache`] / [`trustcache_len`] - `Cryptex1,GenericTrustCache`
 * * [`info`] / [`info_len`] - `Cryptex1,CryptexInfoPlist`
 * * [`volumehash`] / [`volumehash_len`] - `Cryptex1,GenericVolume`
 * * [`build_identity`] - The build identity the payloads came from
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid, and each buffer must be readable for
 * its stated length
 */
struct IdeviceFfiError *cryptex1_assets_from_parts(const uint8_t *image,
                                                   uintptr_t image_len,
                                                   const uint8_t *trustcache,
                                                   uintptr_t trustcache_len,
                                                   const uint8_t *info,
                                                   uintptr_t info_len,
                                                   const uint8_t *volumehash,
                                                   uintptr_t volumehash_len,
                                                   plist_t build_identity,
                                                   struct Cryptex1AssetsHandle **handle);

/**
 * The handle of the nonce domain the assets are personalized against, i.e. the
 * build identity's `Cryptex1,NonceDomain`
 *
 * # Arguments
 * * [`handle`] - The assets handle
 * * [`nonce_domain`] - Pointer to store the handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *cryptex1_assets_nonce_domain(struct Cryptex1AssetsHandle *handle,
                                                     uint64_t *nonce_domain);

/**
 * Frees a Cryptex1Assets handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void cryptex1_assets_free(struct Cryptex1AssetsHandle *handle);

/**
 * Personalizes and installs the DeveloperDiskImage cryptex end to end
 *
 * The cryptex equivalent of the image mounter's auto-mount: reads the device's
 * personalization identifiers and cryptex nonce, has Apple sign a Cryptex1
 * ticket for them, and installs the assets. Each step opens its own connection
 * off the adapter, since the daemon serves one routine per connection.
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`assets`] - The payloads to install
 * * [`installed`] - Pointer to store the installed cryptex, freed with
 *   `cryptexd_free_installed_cryptex`. May be NULL to ignore it.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All non-NULL pointer parameters must be valid
 */
struct IdeviceFfiError *cryptexd_install_ddi(struct AdapterHandle *provider,
                                             struct RsdHandshakeHandle *handshake,
                                             struct Cryptex1AssetsHandle *assets,
                                             struct InstalledCryptexC **installed);

/**
 * The installed DeveloperDiskImage cryptex, if there is one
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`installed`] - Pointer to store the cryptex, set to NULL when no DDI is
 *   installed. Freed with `cryptexd_free_installed_cryptex`.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *cryptexd_installed_ddi(struct AdapterHandle *provider,
                                               struct RsdHandshakeHandle *handshake,
                                               struct InstalledCryptexC **installed);

/**
 * Frees a CryptexdClient handle
 *
 * Only needed for a handle no routine was invoked on: every routine consumes
 * the handle it is passed.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void cryptexd_free(struct CryptexdHandle *handle);

/**
 * Creates a new DebugserverCommand
 *
 * # Safety
 * Caller must free with debugserver_command_free
 */
struct DebugserverCommandHandle *debugserver_command_new(const char *name,
                                                         const char *const *argv,
                                                         uintptr_t argv_count);

/**
 * Frees a DebugserverCommand
 *
 * # Safety
 * `command` must be a valid pointer or NULL
 */
void debugserver_command_free(struct DebugserverCommandHandle *command);

/**
 * Creates a new DebugProxyClient
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *debug_proxy_connect_rsd(struct AdapterHandle *provider,
                                                struct RsdHandshakeHandle *handshake,
                                                struct DebugProxyHandle **handle);

/**
 * Creates a new DebugProxyClient
 *
 * # Arguments
 * * [`socket`] - The socket to use for communication. Any object that supports ReadWrite.
 * * [`handle`] - Pointer to store the newly created DebugProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *debug_proxy_new(struct ReadWriteOpaque *socket,
                                        struct DebugProxyHandle **handle);

/**
 * Frees a DebugProxyClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void debug_proxy_free(struct DebugProxyHandle *handle);

/**
 * Sends a command to the debug proxy
 *
 * # Arguments
 * * [`handle`] - The DebugProxyClient handle
 * * [`command`] - The command to send
 * * [`response`] - Pointer to store the response (caller must free)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` and `command` must be valid pointers
 * `response` must be a valid pointer to a location where the string will be stored
 */
struct IdeviceFfiError *debug_proxy_send_command(struct DebugProxyHandle *handle,
                                                 struct DebugserverCommandHandle *command,
                                                 char **response);

/**
 * Reads a response from the debug proxy
 *
 * # Arguments
 * * [`handle`] - The DebugProxyClient handle
 * * [`response`] - Pointer to store the response (caller must free)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer
 * `response` must be a valid pointer to a location where the string will be stored
 */
struct IdeviceFfiError *debug_proxy_read_response(struct DebugProxyHandle *handle, char **response);

/**
 * Sends raw data to the debug proxy
 *
 * # Arguments
 * * [`handle`] - The DebugProxyClient handle
 * * [`data`] - The data to send
 * * [`len`] - Length of the data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer
 * `data` must be a valid pointer to `len` bytes
 */
struct IdeviceFfiError *debug_proxy_send_raw(struct DebugProxyHandle *handle,
                                             const uint8_t *data,
                                             uintptr_t len);

/**
 * Reads data from the debug proxy
 *
 * # Arguments
 * * [`handle`] - The DebugProxyClient handle
 * * [`len`] - Maximum number of bytes to read
 * * [`response`] - Pointer to store the response (caller must free)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer
 * `response` must be a valid pointer to a location where the string will be stored
 */
struct IdeviceFfiError *debug_proxy_read(struct DebugProxyHandle *handle,
                                         uintptr_t len,
                                         char **response);

/**
 * Sets the argv for the debug proxy
 *
 * # Arguments
 * * [`handle`] - The DebugProxyClient handle
 * * [`argv`] - NULL-terminated array of arguments
 * * [`argv_count`] - Number of arguments
 * * [`response`] - Pointer to store the response (caller must free)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer
 * `argv` must be a valid pointer to `argv_count` C strings or NULL
 * `response` must be a valid pointer to a location where the string will be stored
 */
struct IdeviceFfiError *debug_proxy_set_argv(struct DebugProxyHandle *handle,
                                             const char *const *argv,
                                             uintptr_t argv_count,
                                             char **response);

/**
 * Sends an ACK to the debug proxy
 *
 * # Arguments
 * * [`handle`] - The DebugProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer
 */
struct IdeviceFfiError *debug_proxy_send_ack(struct DebugProxyHandle *handle);

/**
 * Sends a NACK to the debug proxy
 *
 * # Arguments
 * * [`handle`] - The DebugProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer
 */
struct IdeviceFfiError *debug_proxy_send_nack(struct DebugProxyHandle *handle);

/**
 * Sets the ACK mode for the debug proxy
 *
 * # Arguments
 * * [`handle`] - The DebugProxyClient handle
 * * [`enabled`] - Whether ACK mode should be enabled
 *
 * # Safety
 * `handle` must be a valid pointer
 */
void debug_proxy_set_ack_mode(struct DebugProxyHandle *handle, int enabled);

/**
 * Automatically creates and connects to Diagnostics Relay, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated DiagnosticsRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *diagnostics_relay_client_connect(struct IdeviceProviderHandle *provider,
                                                         struct DiagnosticsRelayClientHandle **client);

/**
 * Creates a new DiagnosticsRelayClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated DiagnosticsRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *diagnostics_relay_client_connect_rsd(struct AdapterHandle *provider,
                                                             struct RsdHandshakeHandle *handshake,
                                                             struct DiagnosticsRelayClientHandle **client);

/**
 * Automatically creates and connects to Diagnostics Relay, returning a client handle
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated DiagnosticsRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *diagnostics_relay_client_new(struct IdeviceHandle *socket,
                                                     struct DiagnosticsRelayClientHandle **client);

/**
 * Queries the device IO registry
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 * * `current_plane` - A string to search by or null
 * * `entry_name` - A string to search by or null
 * * `entry_class` - A string to search by or null
 * * `res` - Will be set to a pointer of a plist dictionary node on search success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success. Note that res can be null on success
 * if the search resulted in no values.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_ioregistry(struct DiagnosticsRelayClientHandle *client,
                                                            const char *current_plane,
                                                            const char *entry_name,
                                                            const char *entry_class,
                                                            plist_t *res);

/**
 * Requests MobileGestalt information from the device
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 * * `keys` - Optional list of specific keys to request. If None, requests all available keys
 * * `res` - Will be set to a pointer of a plist dictionary node on search success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success. Note that res can be null on success
 * if the search resulted in no values.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_mobilegestalt(struct DiagnosticsRelayClientHandle *client,
                                                               const char *const *keys,
                                                               uintptr_t keys_len,
                                                               plist_t *res);

/**
 * Requests gas gauge information from the device
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 * * `res` - Will be set to a pointer of a plist dictionary node on search success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success. Note that res can be null on success
 * if the search resulted in no values.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_gasguage(struct DiagnosticsRelayClientHandle *client,
                                                          plist_t *res);

/**
 * Requests nand information from the device
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 * * `res` - Will be set to a pointer of a plist dictionary node on search success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success. Note that res can be null on success
 * if the search resulted in no values.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_nand(struct DiagnosticsRelayClientHandle *client,
                                                      plist_t *res);

/**
 * Requests all available information from the device
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 * * `res` - Will be set to a pointer of a plist dictionary node on search success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success. Note that res can be null on success
 * if the search resulted in no values.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_all(struct DiagnosticsRelayClientHandle *client,
                                                     plist_t *res);

/**
 * Restarts the device
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_restart(struct DiagnosticsRelayClientHandle *client);

/**
 * Shuts down the device
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_shutdown(struct DiagnosticsRelayClientHandle *client);

/**
 * Puts the device to sleep
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_sleep(struct DiagnosticsRelayClientHandle *client);

/**
 * Requests WiFi diagnostics from the device
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 * * `res` - Will be set to a pointer of a plist dictionary node on search success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success. Note that res can be null on success
 * if the search resulted in no values.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_wifi(struct DiagnosticsRelayClientHandle *client,
                                                      plist_t *res);

/**
 * Puts the device to sleep
 *
 * # Arguments
 * * `client` - A valid DiagnosticsRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *diagnostics_relay_client_goodbye(struct DiagnosticsRelayClientHandle *client);

/**
 * Frees a handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void diagnostics_relay_client_free(struct DiagnosticsRelayClientHandle *handle);

/**
 * Creates a new ProcessControlClient from a RemoteServerClient
 *
 * # Arguments
 * * [`server`] - The RemoteServerClient to use
 * * [`handle`] - Pointer to store the newly created ProcessControlClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *location_simulation_new(struct RemoteServerHandle *server,
                                                struct LocationSimulationHandle **handle);

/**
 * Frees a ProcessControlClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void location_simulation_free(struct LocationSimulationHandle *handle);

/**
 * Clears the location set
 *
 * # Arguments
 * * [`handle`] - The LocationSimulation handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid or NULL where appropriate
 */
struct IdeviceFfiError *location_simulation_clear(struct LocationSimulationHandle *handle);

/**
 * Sets the location
 *
 * # Arguments
 * * [`handle`] - The LocationSimulation handle
 * * [`latitude`] - The latitude to set
 * * [`longitude`] - The longitude to set
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid or NULL where appropriate
 */
struct IdeviceFfiError *location_simulation_set(struct LocationSimulationHandle *handle,
                                                double latitude,
                                                double longitude);

/**
 * Frees an IdeviceNotificationInfo and its heap-allocated string fields
 *
 * # Safety
 * `info` must be a valid pointer allocated by this library or NULL
 */
void notifications_info_free(struct IdeviceNotificationInfo *info);

/**
 * Creates a new NotificationsClient from a RemoteServerClient
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *notifications_new(struct RemoteServerHandle *server,
                                          struct NotificationsHandle **handle);

/**
 * Frees a NotificationsClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void notifications_free(struct NotificationsHandle *handle);

/**
 * Enables application state and memory notifications on the device.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *notifications_start(struct NotificationsHandle *handle);

/**
 * Disables application state and memory notifications on the device.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *notifications_stop(struct NotificationsHandle *handle);

/**
 * Reads the next notification pushed by the device. Blocks until a notification arrives.
 *
 * # Arguments
 * * [`handle`] - The NotificationsClient handle
 * * [`info_out`] - On success, set to a heap-allocated IdeviceNotificationInfo
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null. Free the info with `notifications_info_free`.
 */
struct IdeviceFfiError *notifications_get_next(struct NotificationsHandle *handle,
                                               struct IdeviceNotificationInfo **info_out);

/**
 * Creates a new ProcessControlClient from a RemoteServerClient
 *
 * # Arguments
 * * [`server`] - The RemoteServerClient to use
 * * [`handle`] - Pointer to store the newly created ProcessControlClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *process_control_new(struct RemoteServerHandle *server,
                                            struct ProcessControlHandle **handle);

/**
 * Frees a ProcessControlClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void process_control_free(struct ProcessControlHandle *handle);

/**
 * Launches an application on the device
 *
 * # Arguments
 * * [`handle`] - The ProcessControlClient handle
 * * [`bundle_id`] - The bundle identifier of the app to launch
 * * [`env_vars`] - NULL-terminated array of environment variables (format "KEY=VALUE")
 * * [`arguments`] - NULL-terminated array of arguments
 * * [`start_suspended`] - Whether to start the app suspended
 * * [`kill_existing`] - Whether to kill existing instances of the app
 * * [`pid`] - Pointer to store the process ID of the launched app
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid or NULL where appropriate
 */
struct IdeviceFfiError *process_control_launch_app(struct ProcessControlHandle *handle,
                                                   const char *bundle_id,
                                                   const char *const *env_vars,
                                                   uintptr_t env_vars_count,
                                                   const char *const *arguments,
                                                   uintptr_t arguments_count,
                                                   bool start_suspended,
                                                   bool kill_existing,
                                                   uint64_t *pid);

/**
 * Kills a running process
 *
 * # Arguments
 * * [`handle`] - The ProcessControlClient handle
 * * [`pid`] - The process ID to kill
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *process_control_kill_app(struct ProcessControlHandle *handle, uint64_t pid);

/**
 * Disables memory limits for a process
 *
 * # Arguments
 * * [`handle`] - The ProcessControlClient handle
 * * [`pid`] - The process ID to modify
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *process_control_disable_memory_limit(struct ProcessControlHandle *handle,
                                                             uint64_t pid);

/**
 * Creates a new RemoteServerClient from a ReadWrite connection
 *
 * # Arguments
 * * [`socket`] - The connection to use for communication, an object that implements ReadWrite
 * * [`handle`] - Pointer to store the newly created RemoteServerClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. It is consumed and may
 * not be used again.
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *remote_server_new(struct ReadWriteOpaque *socket,
                                          struct RemoteServerHandle **handle);

/**
 * Creates a new RemoteServerClient from a handshake and adapter
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *remote_server_connect_rsd(struct AdapterHandle *provider,
                                                  struct RsdHandshakeHandle *handshake,
                                                  struct RemoteServerHandle **handle);

/**
 * Frees a RemoteServerClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void remote_server_free(struct RemoteServerHandle *handle);

/**
 * Creates a new [`ScreenshotClient`] associated with a given [`RemoteServerHandle`].
 *
 * # Arguments
 * * `server` - A pointer to a valid [`RemoteServerHandle`], previously created by this library.
 * * `handle` - A pointer to a location where the newly created [`ScreenshotClientHandle`] will be stored.
 *
 * # Returns
 * * `null_mut()` on success.
 * * A pointer to an [`IdeviceFfiError`] on failure.
 *
 * # Safety
 * - `server` must be a non-null pointer to a valid remote server handle allocated by this library.
 * - `handle` must be a non-null pointer to a writable memory location where the handle will be stored.
 * - The returned handle must later be freed using [`screenshot_client_free`].
 */
struct IdeviceFfiError *screenshot_client_new(struct RemoteServerHandle *server,
                                              struct ScreenshotClientHandle **handle);

/**
 * Frees a [`ScreenshotClientHandle`].
 *
 * This releases all memory associated with the handle.
 * After calling this function, the handle pointer must not be used again.
 *
 * # Arguments
 * * `handle` - Pointer to a [`ScreenshotClientHandle`] previously returned by [`screenshot_client_new`].
 *
 * # Safety
 * - `handle` must either be `NULL` or a valid pointer created by this library.
 * - Double-freeing or using the handle after freeing causes undefined behavior.
 */
void screenshot_client_free(struct ScreenshotClientHandle *handle);

/**
 * Captures a screenshot from the connected device.
 *
 * On success, this function writes a pointer to the PNG-encoded screenshot data and its length
 * into the provided output arguments. The caller is responsible for freeing this data using
 * `idevice_data_free`.
 *
 * # Arguments
 * * `handle` - A pointer to a valid [`ScreenshotClientHandle`].
 * * `data` - Output pointer where the screenshot buffer pointer will be written.
 * * `len` - Output pointer where the buffer length (in bytes) will be written.
 *
 * # Returns
 * * `null_mut()` on success.
 * * A pointer to an [`IdeviceFfiError`] on failure.
 *
 * # Safety
 * - `handle` must be a valid pointer to a [`ScreenshotClientHandle`].
 * - `data` and `len` must be valid writable pointers.
 * - The data returned through `*data` must be freed by the caller with `idevice_data_free`.
 */
struct IdeviceFfiError *screenshot_client_take_screenshot(struct ScreenshotClientHandle *handle,
                                                          uint8_t **data,
                                                          uintptr_t *len);

/**
 * Creates a new ApplicationListingClient from a RemoteServerClient
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *application_listing_new(struct RemoteServerHandle *server,
                                                struct ApplicationListingHandle **handle);

/**
 * Frees an ApplicationListingClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void application_listing_free(struct ApplicationListingHandle *handle);

/**
 * Returns the list of installed applications as an array of plist dictionaries
 *
 * # Arguments
 * * [`handle`] - The ApplicationListingClient handle
 * * [`apps_out`] - On success, set to a heap-allocated array of plist_t values (each is a dict)
 * * [`count_out`] - On success, set to the number of apps returned
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null.
 * Free the returned array with `idevice_plist_array_free`.
 */
struct IdeviceFfiError *application_listing_get_apps(struct ApplicationListingHandle *handle,
                                                     plist_t **apps_out,
                                                     uintptr_t *count_out);

/**
 * Creates a new ConditionInducerClient from a RemoteServerClient
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *condition_inducer_new(struct RemoteServerHandle *server,
                                              struct ConditionInducerHandle **handle);

/**
 * Frees a ConditionInducerClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void condition_inducer_free(struct ConditionInducerHandle *handle);

/**
 * Frees a single IdeviceConditionGroup and all its heap-allocated fields
 *
 * # Safety
 * `group` must be a valid pointer allocated by this library or NULL
 */
void condition_inducer_group_free(struct IdeviceConditionGroup *group);

/**
 * Frees an array of IdeviceConditionGroup pointers
 *
 * # Safety
 * `groups` must be a valid pointer to an array of length `count` allocated by this library,
 * or NULL
 */
void condition_inducer_groups_free(struct IdeviceConditionGroup **groups, uintptr_t count);

/**
 * Returns the available condition inducer groups
 *
 * # Arguments
 * * [`handle`] - The ConditionInducerClient handle
 * * [`groups_out`] - On success, set to a heap-allocated array of group pointers
 * * [`count_out`] - On success, set to the number of groups returned
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null. Free with `condition_inducer_groups_free`.
 */
struct IdeviceFfiError *condition_inducer_available_conditions(struct ConditionInducerHandle *handle,
                                                               struct IdeviceConditionGroup ***groups_out,
                                                               uintptr_t *count_out);

/**
 * Enables a specific condition profile
 *
 * # Arguments
 * * [`handle`] - The ConditionInducerClient handle
 * * [`condition_identifier`] - The condition group identifier (null-terminated C string)
 * * [`profile_identifier`] - The profile identifier within the group (null-terminated C string)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 */
struct IdeviceFfiError *condition_inducer_enable(struct ConditionInducerHandle *handle,
                                                 const char *condition_identifier,
                                                 const char *profile_identifier);

/**
 * Disables the currently active condition
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *condition_inducer_disable(struct ConditionInducerHandle *handle);

/**
 * Creates a new DeviceInfoClient from a RemoteServerClient
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *device_info_new(struct RemoteServerHandle *server,
                                        struct DeviceInfoHandle **handle);

/**
 * Frees a DeviceInfoClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void device_info_free(struct DeviceInfoHandle *handle);

/**
 * Frees a single IdeviceRunningProcess struct and its heap-allocated strings
 *
 * # Safety
 * `process` must be a valid pointer allocated by this library or NULL
 */
void device_info_running_process_free(struct IdeviceRunningProcess *process);

/**
 * Frees an array of IdeviceRunningProcess pointers
 *
 * # Safety
 * `processes` must be a valid pointer to an array of length `count` allocated by this library,
 * or NULL
 */
void device_info_running_processes_free(struct IdeviceRunningProcess **processes, uintptr_t count);

/**
 * Returns the list of running processes on the device
 *
 * # Arguments
 * * [`handle`] - The DeviceInfoClient handle
 * * [`processes`] - On success, set to a heap-allocated array of process pointers
 * * [`count`] - On success, set to the number of processes returned
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 */
struct IdeviceFfiError *device_info_running_processes(struct DeviceInfoHandle *handle,
                                                      struct IdeviceRunningProcess ***processes,
                                                      uintptr_t *count);

/**
 * Returns the executable name for the given PID
 *
 * # Safety
 * All pointers must be valid and non-null. Free the returned string with `idevice_string_free`.
 */
struct IdeviceFfiError *device_info_execname_for_pid(struct DeviceInfoHandle *handle,
                                                     uint32_t pid,
                                                     char **name_out);

/**
 * Returns whether the given PID is currently running
 *
 * # Safety
 * All pointers must be valid and non-null
 */
struct IdeviceFfiError *device_info_is_running_pid(struct DeviceInfoHandle *handle,
                                                   uint32_t pid,
                                                   bool *result);

/**
 * Returns hardware information as a plist dictionary
 *
 * # Safety
 * All pointers must be valid and non-null. Free the returned plist with `plist_free`.
 */
struct IdeviceFfiError *device_info_hardware_information(struct DeviceInfoHandle *handle,
                                                         plist_t *plist_out);

/**
 * Returns network information as a plist dictionary
 *
 * # Safety
 * All pointers must be valid and non-null. Free the returned plist with `plist_free`.
 */
struct IdeviceFfiError *device_info_network_information(struct DeviceInfoHandle *handle,
                                                        plist_t *plist_out);

/**
 * Returns the mach kernel name
 *
 * # Safety
 * All pointers must be valid and non-null. Free the returned string with `idevice_string_free`.
 */
struct IdeviceFfiError *device_info_mach_kernel_name(struct DeviceInfoHandle *handle,
                                                     char **name_out);

/**
 * Frees a null-terminated string array allocated by this library
 *
 * # Safety
 * `strings` must be a valid pointer to an array of `count` C strings allocated by this library,
 * or NULL
 */
void device_info_string_array_free(char **strings, uintptr_t count);

/**
 * Returns the list of sysmon process attribute names
 *
 * # Safety
 * All pointers must be valid and non-null. Free with `device_info_string_array_free`.
 */
struct IdeviceFfiError *device_info_sysmon_process_attributes(struct DeviceInfoHandle *handle,
                                                              char ***attrs_out,
                                                              uintptr_t *count_out);

/**
 * Returns the list of sysmon system attribute names
 *
 * # Safety
 * All pointers must be valid and non-null. Free with `device_info_string_array_free`.
 */
struct IdeviceFfiError *device_info_sysmon_system_attributes(struct DeviceInfoHandle *handle,
                                                             char ***attrs_out,
                                                             uintptr_t *count_out);

/**
 * Returns directory listing for the given path
 *
 * # Safety
 * All pointers must be valid and non-null. Free with `device_info_string_array_free`.
 */
struct IdeviceFfiError *device_info_directory_listing(struct DeviceInfoHandle *handle,
                                                      const char *path,
                                                      char ***entries_out,
                                                      uintptr_t *count_out);

/**
 * Creates a new EnergyMonitorClient from a RemoteServerClient
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *energy_monitor_new(struct RemoteServerHandle *server,
                                           struct EnergyMonitorHandle **handle);

/**
 * Frees an EnergyMonitorClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void energy_monitor_free(struct EnergyMonitorHandle *handle);

/**
 * Starts energy sampling for the given PIDs.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library.
 * If `pids` is non-null it must point to at least `pids_count` readable `u32` values.
 */
struct IdeviceFfiError *energy_monitor_start_sampling(struct EnergyMonitorHandle *handle,
                                                      const uint32_t *pids,
                                                      uintptr_t pids_count);

/**
 * Stops energy sampling for the given PIDs.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library.
 * If `pids` is non-null it must point to at least `pids_count` readable `u32` values.
 */
struct IdeviceFfiError *energy_monitor_stop_sampling(struct EnergyMonitorHandle *handle,
                                                     const uint32_t *pids,
                                                     uintptr_t pids_count);

/**
 * Requests a one-shot energy sample and parses the response.
 *
 * # Arguments
 * * [`handle`] - The EnergyMonitorClient handle
 * * [`pids`] - Pointer to an array of u32 PIDs to sample
 * * [`pids_count`] - Number of elements in `pids`
 * * [`samples_out`] - On success, set to a heap-allocated array of IdeviceEnergySample
 * * [`samples_count_out`] - On success, set to the number of samples
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All output pointers must be valid and non-null. Free the array with
 * `energy_monitor_samples_free`.
 */
struct IdeviceFfiError *energy_monitor_sample_attributes(struct EnergyMonitorHandle *handle,
                                                         const uint32_t *pids,
                                                         uintptr_t pids_count,
                                                         struct IdeviceEnergySample **samples_out,
                                                         uintptr_t *samples_count_out);

/**
 * Frees an array of IdeviceEnergySample allocated by `energy_monitor_sample_attributes`.
 *
 * # Safety
 * `samples` must be a pointer returned by this library with the matching `count`, or NULL
 */
void energy_monitor_samples_free(struct IdeviceEnergySample *samples, uintptr_t count);

/**
 * Frees an IdeviceGraphicsSample and its heap-allocated string field
 *
 * # Safety
 * `sample` must be a valid pointer allocated by this library or NULL
 */
void graphics_sample_free(struct IdeviceGraphicsSample *sample);

/**
 * Creates a new GraphicsClient from a RemoteServerClient
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *graphics_new(struct RemoteServerHandle *server,
                                     struct GraphicsHandle **handle);

/**
 * Frees a GraphicsClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void graphics_free(struct GraphicsHandle *handle);

/**
 * Starts graphics sampling at the given interval. Consumes the device's initial reply internally.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *graphics_start_sampling(struct GraphicsHandle *handle, double interval);

/**
 * Stops graphics sampling.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *graphics_stop_sampling(struct GraphicsHandle *handle);

/**
 * Reads the next graphics data frame pushed by the device. Blocks until a frame arrives.
 *
 * # Arguments
 * * [`handle`] - The GraphicsClient handle
 * * [`sample_out`] - On success, set to a heap-allocated IdeviceGraphicsSample
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null. Free the sample with `graphics_sample_free`.
 */
struct IdeviceFfiError *graphics_next_sample(struct GraphicsHandle *handle,
                                             struct IdeviceGraphicsSample **sample_out);

/**
 * Frees an IdeviceNetworkEvent and its heap-allocated string fields
 *
 * # Safety
 * `event` must be a valid pointer allocated by this library or NULL
 */
void network_monitor_event_free(struct IdeviceNetworkEvent *event);

/**
 * Creates a new NetworkMonitorClient from a RemoteServerClient
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *network_monitor_new(struct RemoteServerHandle *server,
                                            struct NetworkMonitorHandle **handle);

/**
 * Frees a NetworkMonitorClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void network_monitor_free(struct NetworkMonitorHandle *handle);

/**
 * Starts network monitoring. No reply is expected.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *network_monitor_start(struct NetworkMonitorHandle *handle);

/**
 * Stops network monitoring.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *network_monitor_stop(struct NetworkMonitorHandle *handle);

/**
 * Reads the next network event pushed by the device. Blocks until an event arrives.
 *
 * # Arguments
 * * [`handle`] - The NetworkMonitorClient handle
 * * [`event_out`] - On success, set to a heap-allocated IdeviceNetworkEvent
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null. Free the event with `network_monitor_event_free`.
 */
struct IdeviceFfiError *network_monitor_next_event(struct NetworkMonitorHandle *handle,
                                                   struct IdeviceNetworkEvent **event_out);

/**
 * Creates a new SysmontapClient from a RemoteServerClient
 *
 * # Safety
 * `server` must be a valid pointer to a handle allocated by this library
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *sysmontap_new(struct RemoteServerHandle *server,
                                      struct SysmontapHandle **handle);

/**
 * Frees a SysmontapClient handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void sysmontap_free(struct SysmontapHandle *handle);

/**
 * Sends configuration to the device
 *
 * # Arguments
 * * [`handle`] - The SysmontapClient handle
 * * [`config`] - Pointer to an IdeviceSysmontapConfig struct
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null. String arrays must contain valid C strings.
 */
struct IdeviceFfiError *sysmontap_set_config(struct SysmontapHandle *handle,
                                             const struct IdeviceSysmontapConfig *config);

/**
 * Starts sampling. Consumes the device's initial ack message internally.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *sysmontap_start(struct SysmontapHandle *handle);

/**
 * Stops sampling.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *sysmontap_stop(struct SysmontapHandle *handle);

/**
 * Reads the next sysmontap sample. Blocks until data arrives.
 *
 * Each output plist is a dictionary (or NULL if that field was not present in the sample):
 * - `processes_out`: dict of PID → per-process attribute array
 * - `system_out`:    plist array of system attribute values
 * - `cpu_usage_out`: dict of CPU usage keys
 *
 * The caller is responsible for freeing non-NULL plists with `plist_free`.
 *
 * # Safety
 * `handle` must be valid and non-null. Output pointers may be null to ignore that field.
 */
struct IdeviceFfiError *sysmontap_next_sample(struct SysmontapHandle *handle,
                                              plist_t *processes_out,
                                              plist_t *system_out,
                                              plist_t *cpu_usage_out);

/**
 * Frees the IdeviceFfiError
 *
 * # Safety
 * `err` must be a struct allocated by this library
 */
void idevice_error_free(struct IdeviceFfiError *err);

/**
 * Automatically creates and connects to Installation Proxy, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated InstallationProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *heartbeat_connect(struct IdeviceProviderHandle *provider,
                                          struct HeartbeatClientHandle **client);

/**
 * Creates a new HeartbeatClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated HeartbeatClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *heartbeat_connect_rsd(struct AdapterHandle *provider,
                                              struct RsdHandshakeHandle *handshake,
                                              struct HeartbeatClientHandle **client);

/**
 * Automatically creates and connects to Installation Proxy, returning a client handle
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated InstallationProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *heartbeat_new(struct IdeviceHandle *socket,
                                      struct HeartbeatClientHandle **client);

/**
 * Sends a polo to the device
 *
 * # Arguments
 * * `client` - A valid HeartbeatClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *heartbeat_send_polo(struct HeartbeatClientHandle *client);

/**
 * Sends a polo to the device
 *
 * # Arguments
 * * `client` - A valid HeartbeatClient handle
 * * `interval` - The time to wait for a marco
 * * `new_interval` - A pointer to set the requested marco
 *
 * # Returns
 * An IdeviceFfiError on error, null on success.
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *heartbeat_get_marco(struct HeartbeatClientHandle *client,
                                            uint64_t interval,
                                            uint64_t *new_interval);

/**
 * Frees a handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void heartbeat_client_free(struct HeartbeatClientHandle *handle);

/**
 * Connects to the House Arrest service using a TCP provider
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated HouseArrestClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *house_arrest_client_connect(struct IdeviceProviderHandle *provider,
                                                    struct HouseArrestClientHandle **client);

/**
 * Creates a new HouseArrestClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated HouseArrestClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *house_arrest_client_connect_rsd(struct AdapterHandle *provider,
                                                        struct RsdHandshakeHandle *handshake,
                                                        struct HouseArrestClientHandle **client);

/**
 * Creates a new HouseArrestClient from an existing Idevice connection
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated HouseArrestClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *house_arrest_client_new(struct IdeviceHandle *socket,
                                                struct HouseArrestClientHandle **client);

/**
 * Vends a container for an app
 *
 * # Arguments
 * * [`client`] - The House Arrest client
 * * [`bundle_id`] - The bundle ID to vend for
 * * [`afc_client`] - The new AFC client for the underlying connection
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a allocated by this library
 * `bundle_id` must be a NULL-terminated string
 * `afc_client` must be a valid, non-null pointer where the new AFC client will be stored
 */
struct IdeviceFfiError *house_arrest_vend_container(struct HouseArrestClientHandle *client,
                                                    const char *bundle_id,
                                                    struct AfcClientHandle **afc_client);

/**
 * Vends documents for an app
 *
 * # Arguments
 * * [`client`] - The House Arrest client
 * * [`bundle_id`] - The bundle ID to vend for
 * * [`afc_client`] - The new AFC client for the underlying connection
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a allocated by this library
 * `bundle_id` must be a NULL-terminated string
 * `afc_client` must be a valid, non-null pointer where the new AFC client will be stored
 */
struct IdeviceFfiError *house_arrest_vend_documents(struct HouseArrestClientHandle *client,
                                                    const char *bundle_id,
                                                    struct AfcClientHandle **afc_client);

/**
 * Frees an HouseArrestClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void house_arrest_client_free(struct HouseArrestClientHandle *handle);

/**
 * Automatically creates and connects to Installation Proxy, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated InstallationProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *installation_proxy_connect(struct IdeviceProviderHandle *provider,
                                                   struct InstallationProxyClientHandle **client);

/**
 * Creates a new InstallationProxyClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated InstallationProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *installation_proxy_connect_rsd(struct AdapterHandle *provider,
                                                       struct RsdHandshakeHandle *handshake,
                                                       struct InstallationProxyClientHandle **client);

/**
 * Automatically creates and connects to Installation Proxy, returning a client handle
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated InstallationProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *installation_proxy_new(struct IdeviceHandle *socket,
                                               struct InstallationProxyClientHandle **client);

/**
 * Gets installed apps on the device
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`application_type`] - The application type to filter by (optional, NULL for "Any")
 * * [`bundle_identifiers`] - The identifiers to filter by (optional, NULL for all apps)
 * * [`out_result`] - On success, will be set to point to a newly allocated array of PlistRef
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `out_result` must be a valid, non-null pointer to a location where the result will be stored
 */
struct IdeviceFfiError *installation_proxy_get_apps(struct InstallationProxyClientHandle *client,
                                                    const char *application_type,
                                                    const char *const *bundle_identifiers,
                                                    size_t bundle_identifiers_len,
                                                    void **out_result,
                                                    size_t *out_result_len);

/**
 * Frees a handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void installation_proxy_client_free(struct InstallationProxyClientHandle *handle);

/**
 * Installs an application package on the device
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`package_path`] - Path to the .ipa package in the AFC jail
 * * [`options`] - Optional installation options as a plist dictionary (can be NULL)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `package_path` must be a valid C string
 * `options` must be a valid plist dictionary or NULL
 */
struct IdeviceFfiError *installation_proxy_install(struct InstallationProxyClientHandle *client,
                                                   const char *package_path,
                                                   plist_t options);

/**
 * Installs an application package on the device
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`package_path`] - Path to the .ipa package in the AFC jail
 * * [`options`] - Optional installation options as a plist dictionary (can be NULL)
 * * [`callback`] - Progress callback function
 * * [`context`] - User context to pass to callback
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `package_path` must be a valid C string
 * `options` must be a valid plist dictionary or NULL
 */
struct IdeviceFfiError *installation_proxy_install_with_callback(struct InstallationProxyClientHandle *client,
                                                                 const char *package_path,
                                                                 plist_t options,
                                                                 void (*callback)(uint64_t progress,
                                                                                  void *context),
                                                                 void *context);

/**
 * Upgrades an existing application on the device
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`package_path`] - Path to the .ipa package in the AFC jail
 * * [`options`] - Optional upgrade options as a plist dictionary (can be NULL)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `package_path` must be a valid C string
 * `options` must be a valid plist dictionary or NULL
 */
struct IdeviceFfiError *installation_proxy_upgrade(struct InstallationProxyClientHandle *client,
                                                   const char *package_path,
                                                   plist_t options);

/**
 * Upgrades an existing application on the device
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`package_path`] - Path to the .ipa package in the AFC jail
 * * [`options`] - Optional upgrade options as a plist dictionary (can be NULL)
 * * [`callback`] - Progress callback function
 * * [`context`] - User context to pass to callback
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `package_path` must be a valid C string
 * `options` must be a valid plist dictionary or NULL
 */
struct IdeviceFfiError *installation_proxy_upgrade_with_callback(struct InstallationProxyClientHandle *client,
                                                                 const char *package_path,
                                                                 plist_t options,
                                                                 void (*callback)(uint64_t progress,
                                                                                  void *context),
                                                                 void *context);

/**
 * Uninstalls an application from the device
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`bundle_id`] - Bundle identifier of the application to uninstall
 * * [`options`] - Optional uninstall options as a plist dictionary (can be NULL)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `bundle_id` must be a valid C string
 * `options` must be a valid plist dictionary or NULL
 */
struct IdeviceFfiError *installation_proxy_uninstall(struct InstallationProxyClientHandle *client,
                                                     const char *bundle_id,
                                                     plist_t options);

/**
 * Uninstalls an application from the device
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`bundle_id`] - Bundle identifier of the application to uninstall
 * * [`options`] - Optional uninstall options as a plist dictionary (can be NULL)
 * * [`callback`] - Progress callback function
 * * [`context`] - User context to pass to callback
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `bundle_id` must be a valid C string
 * `options` must be a valid plist dictionary or NULL
 */
struct IdeviceFfiError *installation_proxy_uninstall_with_callback(struct InstallationProxyClientHandle *client,
                                                                   const char *bundle_id,
                                                                   plist_t options,
                                                                   void (*callback)(uint64_t progress,
                                                                                    void *context),
                                                                   void *context);

/**
 * Checks if the device capabilities match the required capabilities
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`capabilities`] - Array of plist values representing required capabilities
 * * [`capabilities_len`] - Length of the capabilities array
 * * [`options`] - Optional check options as a plist dictionary (can be NULL)
 * * [`out_result`] - Will be set to true if all capabilities are supported, false otherwise
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `capabilities` must be a valid array of plist values or NULL
 * `options` must be a valid plist dictionary or NULL
 * `out_result` must be a valid pointer to a bool
 */
struct IdeviceFfiError *installation_proxy_check_capabilities_match(struct InstallationProxyClientHandle *client,
                                                                    const plist_t *capabilities,
                                                                    size_t capabilities_len,
                                                                    plist_t options,
                                                                    bool *out_result);

/**
 * Browses installed applications on the device
 *
 * # Arguments
 * * [`client`] - A valid InstallationProxyClient handle
 * * [`options`] - Optional browse options as a plist dictionary (can be NULL)
 * * [`out_result`] - On success, will be set to point to a newly allocated array of PlistRef
 * * [`out_result_len`] - Will be set to the length of the result array
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `options` must be a valid plist dictionary or NULL
 * `out_result` must be a valid, non-null pointer to a location where the result will be stored
 * `out_result_len` must be a valid, non-null pointer to a location where the length will be stored
 */
struct IdeviceFfiError *installation_proxy_browse(struct InstallationProxyClientHandle *client,
                                                  plist_t options,
                                                  plist_t **out_result,
                                                  size_t *out_result_len);

/**
 * Creates a new InstallcoordinationProxy client from a ReadWrite stream
 *
 * # Arguments
 * * [`socket`] - A ReadWriteOpaque handle (consumed)
 * * [`client`] - On success, will be set to point to a newly allocated handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *installcoordination_proxy_new(struct ReadWriteOpaque *socket,
                                                      struct InstallcoordinationProxyHandle **client);

/**
 * Creates a new InstallcoordinationProxy client via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated InstallcoordinationProxy handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *installcoordination_proxy_connect_rsd(struct AdapterHandle *provider,
                                                              struct RsdHandshakeHandle *handshake,
                                                              struct InstallcoordinationProxyHandle **client);

/**
 * Uninstalls an app by bundle ID
 *
 * # Arguments
 * * `client` - A valid InstallcoordinationProxy handle
 * * `bundle_id` - The bundle identifier of the app to uninstall
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `bundle_id` must be a valid null-terminated C string
 */
struct IdeviceFfiError *installcoordination_proxy_uninstall_app(struct InstallcoordinationProxyHandle *client,
                                                                const char *bundle_id);

/**
 * Queries the install path of an app by bundle ID
 *
 * # Arguments
 * * `client` - A valid InstallcoordinationProxy handle
 * * `bundle_id` - The bundle identifier of the app to query
 * * `path` - On success, will be set to a newly allocated C string with the install path
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `bundle_id` must be a valid null-terminated C string
 * The returned string must be freed with `idevice_string_free`
 */
struct IdeviceFfiError *installcoordination_proxy_query_app_path(struct InstallcoordinationProxyHandle *client,
                                                                 const char *bundle_id,
                                                                 char **path);

/**
 * Frees an InstallcoordinationProxy client handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void installcoordination_proxy_client_free(struct InstallcoordinationProxyHandle *handle);

/**
 * Connects to the Location Simulation service using a provider
 * This is the location_simulation api for iOS 16 and below
 * You must have a developer disk image mounted to use this API
 *
 * # Safety
 * `provider` must be valid; `client` must be a non-null pointer to store the handle.
 */
struct IdeviceFfiError *lockdown_location_simulation_connect(struct IdeviceProviderHandle *provider,
                                                             struct LocationSimulationServiceHandle **handle);

/**
 * Creates a new Location Simulation service client directly from an existing `IdeviceHandle` (socket).
 *
 * # Safety
 * - `socket` must be a valid, unowned pointer to an `IdeviceHandle` that has been properly
 *   initialized and represents an open connection to the Location Simulation service.
 *   Ownership of the `IdeviceHandle` is transferred to this function.
 * - `client` must be a non-null pointer to a location where the newly created
 *   `*mut LocationSimulationServiceHandle` will be stored.
 *
 */
struct IdeviceFfiError *lockdown_location_simulation_new(struct IdeviceHandle *socket,
                                                         struct LocationSimulationServiceHandle **client);

/**
 * Sets the device's simulated location.
 * This is the location_simulation api for iOS 16 and below.
 *
 * # Safety
 * `handle` must be a valid pointer to a `LocationSimulationServiceHandle` returned by `lockdown_location_simulation_connect`.
 * `latitude` and `longitude` must be valid, null-terminated C strings.
 */
struct IdeviceFfiError *lockdown_location_simulation_set(struct LocationSimulationServiceHandle *handle,
                                                         const char *latitude,
                                                         const char *longitude);

/**
 * Clears the device's simulated location, returning it to the actual location.
 * This is the location_simulation api for iOS 16 and below.
 *
 * # Safety
 * `handle` must be a valid pointer to a `LocationSimulationServiceHandle` returned by `lockdown_location_simulation_connect`.
 */
struct IdeviceFfiError *lockdown_location_simulation_clear(struct LocationSimulationServiceHandle *handle);

/**
 * Frees a LocationSimulationService handle
 *
 * # Safety
 * `handle` must be a pointer returned by `lockdown_location_simulation_connect`.
 */
void lockdown_location_simulation_free(struct LocationSimulationServiceHandle *handle);

/**
 * Connects to lockdownd service using provider
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated LockdowndClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *lockdownd_connect(struct IdeviceProviderHandle *provider,
                                          struct LockdowndClientHandle **client);

/**
 * Creates a new LockdownClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated LockdownClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *lockdownd_connect_rsd(struct AdapterHandle *provider,
                                              struct RsdHandshakeHandle *handshake,
                                              struct LockdowndClientHandle **client);

/**
 * Creates a new LockdowndClient from an existing Idevice connection
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle.
 * * [`client`] - On success, will be set to point to a newly allocated LockdowndClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and maybe not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *lockdownd_new(struct IdeviceHandle *socket,
                                      struct LockdowndClientHandle **client);

/**
 * Starts a session with lockdownd
 *
 * # Arguments
 * * `client` - A valid LockdowndClient handle
 * * `pairing_file` - An IdevicePairingFile alocated by this library
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `pairing_file` must be a valid plist_t containing a pairing file
 */
struct IdeviceFfiError *lockdownd_start_session(struct LockdowndClientHandle *client,
                                                struct IdevicePairingFile *pairing_file);

/**
 * Starts a service through lockdownd
 *
 * # Arguments
 * * `client` - A valid LockdowndClient handle
 * * `identifier` - The service identifier to start (null-terminated string)
 * * `port` - Pointer to store the returned port number
 * * `ssl` - Pointer to store whether SSL should be enabled
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `identifier` must be a valid null-terminated string
 * `port` and `ssl` must be valid pointers
 */
struct IdeviceFfiError *lockdownd_start_service(struct LockdowndClientHandle *client,
                                                const char *identifier,
                                                uint16_t *port,
                                                bool *ssl);

/**
 * Pairs with the device using lockdownd
 *
 * # Arguments
 * * `client` - A valid LockdowndClient handle
 * * `host_id` - The host ID (null-terminated string)
 * * `system_buid` - The system BUID (null-terminated string)
 * * `pairing_file` - On success, will be set to point to a newly allocated IdevicePairingFile handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `host_id` must be a valid null-terminated string
 * `system_buid` must be a valid null-terminated string
 * `pairing_file` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *lockdownd_pair(struct LockdowndClientHandle *client,
                                       const char *host_id,
                                       const char *system_buid,
                                       const char *host_name,
                                       struct IdevicePairingFile **pairing_file);

/**
 * Gets a value from lockdownd
 *
 * # Arguments
 * * `client` - A valid LockdowndClient handle
 * * `key` - The value to get (null-terminated string)
 * * `domain` - The value to get (null-terminated string)
 * * `out_plist` - Pointer to store the returned plist value
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `value` must be a valid null-terminated string
 * `out_plist` must be a valid pointer to store the plist
 */
struct IdeviceFfiError *lockdownd_get_value(struct LockdowndClientHandle *client,
                                            const char *key,
                                            const char *domain,
                                            plist_t *out_plist);

/**
 * Tells the device to enter recovery mode
 *
 * # Arguments
 * * `client` - A valid LockdowndClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *lockdownd_enter_recovery(struct LockdowndClientHandle *client);

/**
 * Sets a value in lockdownd
 *
 * # Arguments
 * * `client` - A valid LockdowndClient handle
 * * `key` - The key to set (null-terminated string)
 * * `value` - The value to set as a plist
 * * `domain` - The domain to set in (null-terminated string, optional)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `key` must be a valid null-terminated string
 * `value` must be a valid plist
 * `domain` must be a valid null-terminated string or NULL
 */
struct IdeviceFfiError *lockdownd_set_value(struct LockdowndClientHandle *client,
                                            const char *key,
                                            plist_t value,
                                            const char *domain);

/**
 * Frees a LockdowndClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void lockdownd_client_free(struct LockdowndClientHandle *handle);

/**
 * Initializes the global logger
 *
 * # Safety
 * Pass a valid file path string
 */
enum IdeviceLoggerError idevice_init_logger(enum IdeviceLogLevel console_level,
                                            enum IdeviceLogLevel file_level,
                                            char *file_path);

/**
 * Automatically creates and connects to Misagent, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated MisagentClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *misagent_connect(struct IdeviceProviderHandle *provider,
                                         struct MisagentClientHandle **client);

/**
 * Creates a new MisagentClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated MisagentClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *misagent_connect_rsd(struct AdapterHandle *provider,
                                             struct RsdHandshakeHandle *handshake,
                                             struct MisagentClientHandle **client);

/**
 * Installs a provisioning profile on the device
 *
 * # Arguments
 * * [`client`] - A valid MisagentClient handle
 * * [`profile_data`] - The provisioning profile data to install
 * * [`profile_len`] - Length of the profile data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `profile_data` must be a valid pointer to profile data of length `profile_len`
 */
struct IdeviceFfiError *misagent_install(struct MisagentClientHandle *client,
                                         const uint8_t *profile_data,
                                         size_t profile_len);

/**
 * Removes a provisioning profile from the device
 *
 * # Arguments
 * * [`client`] - A valid MisagentClient handle
 * * [`profile_id`] - The UUID of the profile to remove (C string)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `profile_id` must be a valid C string
 */
struct IdeviceFfiError *misagent_remove(struct MisagentClientHandle *client,
                                        const char *profile_id);

/**
 * Retrieves all provisioning profiles from the device
 *
 * # Arguments
 * * [`client`] - A valid MisagentClient handle
 * * [`out_profiles`] - On success, will be set to point to an array of profile data
 * * [`out_profiles_len`] - On success, will be set to the number of profiles
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `out_profiles` must be a valid pointer to store the resulting array
 * `out_profiles_len` must be a valid pointer to store the array length
 */
struct IdeviceFfiError *misagent_copy_all(struct MisagentClientHandle *client,
                                          uint8_t ***out_profiles,
                                          size_t **out_profiles_len,
                                          size_t *out_count);

/**
 * Frees profiles array returned by misagent_copy_all
 *
 * # Arguments
 * * [`profiles`] - Array of profile data pointers
 * * [`lens`] - Array of profile lengths
 * * [`count`] - Number of profiles in the array
 *
 * # Safety
 * Must only be called with values returned from misagent_copy_all
 */
void misagent_free_profiles(uint8_t **profiles, size_t *lens, size_t count);

/**
 * Frees a misagent client handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void misagent_client_free(struct MisagentClientHandle *handle);

/**
 * Connects to the Image Mounter service using a provider
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated ImageMounter handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *image_mounter_connect(struct IdeviceProviderHandle *provider,
                                              struct ImageMounterHandle **client);

/**
 * Creates a new ImageMounter via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated ImageMounter handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *image_mounter_connect_rsd(struct AdapterHandle *provider,
                                                  struct RsdHandshakeHandle *handshake,
                                                  struct ImageMounterHandle **client);

/**
 * Creates a new ImageMounter client from an existing Idevice connection
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated ImageMounter handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *image_mounter_new(struct IdeviceHandle *socket,
                                          struct ImageMounterHandle **client);

/**
 * Frees an ImageMounter handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void image_mounter_free(struct ImageMounterHandle *handle);

/**
 * Gets a list of mounted devices
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`devices`] - Will be set to point to a slice of device plists on success
 * * [`devices_len`] - Will be set to the number of devices copied
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `devices` must be a valid, non-null pointer to a location where the plist will be stored
 */
struct IdeviceFfiError *image_mounter_copy_devices(struct ImageMounterHandle *client,
                                                   plist_t **devices,
                                                   size_t *devices_len);

/**
 * Looks up an image and returns its signature
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`image_type`] - The type of image to look up
 * * [`signature`] - Will be set to point to the signature data on success
 * * [`signature_len`] - Will be set to the length of the signature data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `image_type` must be a valid null-terminated C string
 * `signature` and `signature_len` must be valid pointers
 */
struct IdeviceFfiError *image_mounter_lookup_image(struct ImageMounterHandle *client,
                                                   const char *image_type,
                                                   uint8_t **signature,
                                                   size_t *signature_len);

/**
 * Uploads an image to the device
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`image_type`] - The type of image being uploaded
 * * [`image`] - Pointer to the image data
 * * [`image_len`] - Length of the image data
 * * [`signature`] - Pointer to the signature data
 * * [`signature_len`] - Length of the signature data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `image_type` must be a valid null-terminated C string
 */
struct IdeviceFfiError *image_mounter_upload_image(struct ImageMounterHandle *client,
                                                   const char *image_type,
                                                   const uint8_t *image,
                                                   size_t image_len,
                                                   const uint8_t *signature,
                                                   size_t signature_len);

/**
 * Mounts an image on the device
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`image_type`] - The type of image being mounted
 * * [`signature`] - Pointer to the signature data
 * * [`signature_len`] - Length of the signature data
 * * [`trust_cache`] - Pointer to trust cache data (optional)
 * * [`trust_cache_len`] - Length of trust cache data (0 if none)
 * * [`info_plist`] - Pointer to info plist (optional)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid (except optional ones which can be null)
 * `image_type` must be a valid null-terminated C string
 */
struct IdeviceFfiError *image_mounter_mount_image(struct ImageMounterHandle *client,
                                                  const char *image_type,
                                                  const uint8_t *signature,
                                                  size_t signature_len,
                                                  const uint8_t *trust_cache,
                                                  size_t trust_cache_len,
                                                  const void *info_plist);

/**
 * Unmounts an image from the device
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`mount_path`] - The path where the image is mounted
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `mount_path` must be a valid null-terminated C string
 */
struct IdeviceFfiError *image_mounter_unmount_image(struct ImageMounterHandle *client,
                                                    const char *mount_path);

/**
 * Queries the developer mode status
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`status`] - Will be set to the developer mode status (1 = enabled, 0 = disabled)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `status` must be a valid pointer
 */
struct IdeviceFfiError *image_mounter_query_developer_mode_status(struct ImageMounterHandle *client,
                                                                  int *status);

/**
 * Mounts a developer image
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`image`] - Pointer to the image data
 * * [`image_len`] - Length of the image data
 * * [`signature`] - Pointer to the signature data
 * * [`signature_len`] - Length of the signature data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 */
struct IdeviceFfiError *image_mounter_mount_developer(struct ImageMounterHandle *client,
                                                      const uint8_t *image,
                                                      size_t image_len,
                                                      const uint8_t *signature,
                                                      size_t signature_len);

/**
 * Queries the personalization manifest from the device
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`image_type`] - The type of image to query
 * * [`signature`] - Pointer to the signature data
 * * [`signature_len`] - Length of the signature data
 * * [`manifest`] - Will be set to point to the manifest data on success
 * * [`manifest_len`] - Will be set to the length of the manifest data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid and non-null
 * `image_type` must be a valid null-terminated C string
 */
struct IdeviceFfiError *image_mounter_query_personalization_manifest(struct ImageMounterHandle *client,
                                                                     const char *image_type,
                                                                     const uint8_t *signature,
                                                                     size_t signature_len,
                                                                     uint8_t **manifest,
                                                                     size_t *manifest_len);

/**
 * Queries the nonce from the device
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`personalized_image_type`] - The type of image to query (optional)
 * * [`nonce`] - Will be set to point to the nonce data on success
 * * [`nonce_len`] - Will be set to the length of the nonce data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client`, `nonce`, and `nonce_len` must be valid pointers
 * `personalized_image_type` can be NULL
 */
struct IdeviceFfiError *image_mounter_query_nonce(struct ImageMounterHandle *client,
                                                  const char *personalized_image_type,
                                                  uint8_t **nonce,
                                                  size_t *nonce_len);

/**
 * Queries personalization identifiers from the device
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`image_type`] - The type of image to query (optional)
 * * [`identifiers`] - Will be set to point to the identifiers plist on success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` and `identifiers` must be valid pointers
 * `image_type` can be NULL
 */
struct IdeviceFfiError *image_mounter_query_personalization_identifiers(struct ImageMounterHandle *client,
                                                                        const char *image_type,
                                                                        plist_t *identifiers);

/**
 * Rolls the personalization nonce
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *image_mounter_roll_personalization_nonce(struct ImageMounterHandle *client);

/**
 * Rolls the cryptex nonce
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *image_mounter_roll_cryptex_nonce(struct ImageMounterHandle *client);

/**
 * Mounts a personalized developer image
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`provider`] - A valid provider handle
 * * [`image`] - Pointer to the image data
 * * [`image_len`] - Length of the image data
 * * [`trust_cache`] - Pointer to the trust cache data
 * * [`trust_cache_len`] - Length of the trust cache data
 * * [`build_manifest`] - Pointer to the build manifest data
 * * [`build_manifest_len`] - Length of the build manifest data
 * * [`info_plist`] - Pointer to info plist (optional)
 * * [`unique_chip_id`] - The device's unique chip ID
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid (except optional ones which can be null)
 */
struct IdeviceFfiError *image_mounter_mount_personalized(struct ImageMounterHandle *client,
                                                         struct IdeviceProviderHandle *provider,
                                                         const uint8_t *image,
                                                         size_t image_len,
                                                         const uint8_t *trust_cache,
                                                         size_t trust_cache_len,
                                                         const uint8_t *build_manifest,
                                                         size_t build_manifest_len,
                                                         const void *info_plist,
                                                         uint64_t unique_chip_id);

/**
 * Mounts a personalized developer image via RSD
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`provider`] - An adapter handle
 * * [`handshake`] - An RSD handshake handle
 * * [`image`] - Pointer to the image data
 * * [`image_len`] - Length of the image data
 * * [`trust_cache`] - Pointer to the trust cache data
 * * [`trust_cache_len`] - Length of the trust cache data
 * * [`build_manifest`] - Pointer to the build manifest data
 * * [`build_manifest_len`] - Length of the build manifest data
 * * [`info_plist`] - Pointer to info plist (optional)
 * * [`unique_chip_id`] - The device's unique chip ID
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid (except optional ones which can be null)
 */
struct IdeviceFfiError *image_mounter_mount_personalized_rsd(struct ImageMounterHandle *client,
                                                             struct AdapterHandle *provider,
                                                             struct RsdHandshakeHandle *handshake,
                                                             const uint8_t *image,
                                                             size_t image_len,
                                                             const uint8_t *trust_cache,
                                                             size_t trust_cache_len,
                                                             const uint8_t *build_manifest,
                                                             size_t build_manifest_len,
                                                             const void *info_plist,
                                                             uint64_t unique_chip_id);

/**
 * Mounts a personalized developer image via RSD with progress callback
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`provider`] - An adapter handle
 * * [`handshake`] - An RSD handshake handle
 * * [`image`] - Pointer to the image data
 * * [`image_len`] - Length of the image data
 * * [`trust_cache`] - Pointer to the trust cache data
 * * [`trust_cache_len`] - Length of the trust cache data
 * * [`build_manifest`] - Pointer to the build manifest data
 * * [`build_manifest_len`] - Length of the build manifest data
 * * [`info_plist`] - Pointer to info plist (optional)
 * * [`unique_chip_id`] - The device's unique chip ID
 * * [`callback`] - Progress callback function
 * * [`context`] - User context to pass to callback
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid (except optional ones which can be null)
 */
struct IdeviceFfiError *image_mounter_mount_personalized_with_callback_rsd(struct ImageMounterHandle *client,
                                                                           struct AdapterHandle *provider,
                                                                           struct RsdHandshakeHandle *handshake,
                                                                           const uint8_t *image,
                                                                           size_t image_len,
                                                                           const uint8_t *trust_cache,
                                                                           size_t trust_cache_len,
                                                                           const uint8_t *build_manifest,
                                                                           size_t build_manifest_len,
                                                                           const void *info_plist,
                                                                           uint64_t unique_chip_id,
                                                                           void (*callback)(size_t progress,
                                                                                            size_t total,
                                                                                            void *context),
                                                                           void *context);

/**
 * Mounts a personalized developer image with progress callback
 *
 * # Arguments
 * * [`client`] - A valid ImageMounter handle
 * * [`provider`] - A valid provider handle
 * * [`image`] - Pointer to the image data
 * * [`image_len`] - Length of the image data
 * * [`trust_cache`] - Pointer to the trust cache data
 * * [`trust_cache_len`] - Length of the trust cache data
 * * [`build_manifest`] - Pointer to the build manifest data
 * * [`build_manifest_len`] - Length of the build manifest data
 * * [`info_plist`] - Pointer to info plist (optional)
 * * [`unique_chip_id`] - The device's unique chip ID
 * * [`callback`] - Progress callback function
 * * [`context`] - User context to pass to callback
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointers must be valid (except optional ones which can be null)
 */
struct IdeviceFfiError *image_mounter_mount_personalized_with_callback(struct ImageMounterHandle *client,
                                                                       struct IdeviceProviderHandle *provider,
                                                                       const uint8_t *image,
                                                                       size_t image_len,
                                                                       const uint8_t *trust_cache,
                                                                       size_t trust_cache_len,
                                                                       const uint8_t *build_manifest,
                                                                       size_t build_manifest_len,
                                                                       const void *info_plist,
                                                                       uint64_t unique_chip_id,
                                                                       void (*callback)(size_t progress,
                                                                                        size_t total,
                                                                                        void *context),
                                                                       void *context);

/**
 * Creates a new MobileActivationd client handle from a provider
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider (not consumed, must remain valid for the lifetime of the handle)
 * * [`client`] - On success, will be set to point to a newly allocated handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library.
 * The provider must remain valid for the lifetime of the returned handle.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *mobileactivationd_connect(struct IdeviceProviderHandle *provider,
                                                  struct MobileActivationdClientHandle **client);

/**
 * Gets the activation state of the device
 *
 * # Arguments
 * * `client` - A valid MobileActivationd handle
 * * `state` - On success, will be set to a newly allocated C string with the activation state
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * The returned string must be freed with `idevice_string_free`
 */
struct IdeviceFfiError *mobileactivationd_get_state(struct MobileActivationdClientHandle *client,
                                                    char **state);

/**
 * Checks if the device is activated
 *
 * # Arguments
 * * `client` - A valid MobileActivationd handle
 * * `activated` - On success, will be set to true if the device is activated
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *mobileactivationd_is_activated(struct MobileActivationdClientHandle *client,
                                                       bool *activated);

/**
 * Deactivates the device
 *
 * # Arguments
 * * `client` - A valid MobileActivationd handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *mobileactivationd_deactivate(struct MobileActivationdClientHandle *client);

/**
 * Frees a MobileActivationd client handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void mobileactivationd_client_free(struct MobileActivationdClientHandle *handle);

/**
 * Connects to the mobilebackup2 service via a provider
 *
 * # Safety
 * All pointer arguments must be valid and non-null
 */
struct IdeviceFfiError *mobilebackup2_connect(struct IdeviceProviderHandle *provider,
                                              struct MobileBackup2ClientHandle **client);

/**
 * Creates a new MobileBackup2Client via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated MobileBackup2Client handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *mobilebackup2_connect_rsd(struct AdapterHandle *provider,
                                                  struct RsdHandshakeHandle *handshake,
                                                  struct MobileBackup2ClientHandle **client);

/**
 * Creates a mobilebackup2 client from an existing connection (consumes the socket)
 *
 * # Safety
 * `socket` is consumed and must not be used after this call
 */
struct IdeviceFfiError *mobilebackup2_new(struct IdeviceHandle *socket,
                                          struct MobileBackup2ClientHandle **client);

/**
 * Frees a mobilebackup2 client handle
 *
 * # Safety
 * `handle` must be valid or NULL
 */
void mobilebackup2_client_free(struct MobileBackup2ClientHandle *handle);

/**
 * Creates a backup of the device
 *
 * # Arguments
 * * `client` - A valid MobileBackup2Client handle
 * * `backup_root` - Path to the backup root directory (null-terminated UTF-8)
 * * `source_identifier` - Source UDID (null-terminated UTF-8, or NULL for current device)
 * * `options` - Optional plist dictionary of backup options (NULL for defaults)
 * * `delegate` - Pointer to a populated Mobilebackup2BackupDelegateFFI struct
 * * `out_response` - On success, receives the device response plist (caller must free). May be NULL.
 *
 * # Safety
 * All non-null pointers must be valid. `delegate` must remain valid for the entire call.
 */
struct IdeviceFfiError *mobilebackup2_backup(struct MobileBackup2ClientHandle *client,
                                             const char *backup_root,
                                             const char *source_identifier,
                                             plist_t options,
                                             const struct Mobilebackup2BackupDelegateFFI *delegate,
                                             plist_t *out_response);

/**
 * Restores a backup to the device
 *
 * # Safety
 * All non-null pointers must be valid. `delegate` must remain valid for the entire call.
 */
struct IdeviceFfiError *mobilebackup2_restore(struct MobileBackup2ClientHandle *client,
                                              const char *backup_root,
                                              const char *source_identifier,
                                              plist_t options,
                                              const struct Mobilebackup2BackupDelegateFFI *delegate,
                                              plist_t *out_response);

/**
 * Changes the backup password on the device
 *
 * # Safety
 * All non-null pointers must be valid.
 */
struct IdeviceFfiError *mobilebackup2_change_password(struct MobileBackup2ClientHandle *client,
                                                      const char *backup_root,
                                                      const char *old_password,
                                                      const char *new_password,
                                                      const struct Mobilebackup2BackupDelegateFFI *delegate);

/**
 * Disconnects from the mobilebackup2 service
 *
 * # Safety
 * `client` must be a valid handle
 */
struct IdeviceFfiError *mobilebackup2_disconnect(struct MobileBackup2ClientHandle *client);

/**
 * Automatically creates and connects to Notification Proxy, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated NotificationProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *notification_proxy_connect(struct IdeviceProviderHandle *provider,
                                                   struct NotificationProxyClientHandle **client);

/**
 * Creates a new NotificationProxyClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated NotificationProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *notification_proxy_connect_rsd(struct AdapterHandle *provider,
                                                       struct RsdHandshakeHandle *handshake,
                                                       struct NotificationProxyClientHandle **client);

/**
 * Creates a new NotificationProxyClient from an existing Idevice connection
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated NotificationProxyClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *notification_proxy_new(struct IdeviceHandle *socket,
                                               struct NotificationProxyClientHandle **client);

/**
 * Posts a notification to the device
 *
 * # Arguments
 * * `client` - A valid NotificationProxyClient handle
 * * `name` - C string containing the notification name
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `name` must be a valid null-terminated C string
 */
struct IdeviceFfiError *notification_proxy_post(struct NotificationProxyClientHandle *client,
                                                const char *name);

/**
 * Observes a specific notification
 *
 * # Arguments
 * * `client` - A valid NotificationProxyClient handle
 * * `name` - C string containing the notification name to observe
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `name` must be a valid null-terminated C string
 */
struct IdeviceFfiError *notification_proxy_observe(struct NotificationProxyClientHandle *client,
                                                   const char *name);

/**
 * Observes multiple notifications at once
 *
 * # Arguments
 * * `client` - A valid NotificationProxyClient handle
 * * `names` - A null-terminated array of C strings containing notification names
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `names` must be a valid pointer to a null-terminated array of null-terminated C strings
 */
struct IdeviceFfiError *notification_proxy_observe_multiple(struct NotificationProxyClientHandle *client,
                                                            const char *const *names);

/**
 * Receives the next notification from the device
 *
 * # Arguments
 * * `client` - A valid NotificationProxyClient handle
 * * `name_out` - On success, will be set to a newly allocated C string containing the notification name
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `name_out` must be a valid pointer. The returned string must be freed with `notification_proxy_free_string`
 */
struct IdeviceFfiError *notification_proxy_receive(struct NotificationProxyClientHandle *client,
                                                   char **name_out);

/**
 * Receives the next notification with a timeout
 *
 * # Arguments
 * * `client` - A valid NotificationProxyClient handle
 * * `interval` - Timeout in seconds to wait for a notification
 * * `name_out` - On success, will be set to a newly allocated C string containing the notification name
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `name_out` must be a valid pointer. The returned string must be freed with `notification_proxy_free_string`
 */
struct IdeviceFfiError *notification_proxy_receive_with_timeout(struct NotificationProxyClientHandle *client,
                                                                uint64_t interval,
                                                                char **name_out);

/**
 * Frees a string returned by notification_proxy_receive
 *
 * # Safety
 * `s` must be a valid pointer returned from `notification_proxy_receive`
 */
void notification_proxy_free_string(char *s);

/**
 * Frees a handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void notification_proxy_client_free(struct NotificationProxyClientHandle *handle);

/**
 * Connects to the remote notification proxy over RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` and `handshake` must be valid pointers to handles allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *remote_notification_proxy_connect_rsd(struct AdapterHandle *provider,
                                                              struct RsdHandshakeHandle *handshake,
                                                              struct RemoteNotificationProxyClientHandle **client);

/**
 * Creates a remote notification proxy client from a socket
 *
 * # Arguments
 * * [`socket`] - The socket to use for communication. Consumed regardless of the result.
 * * [`client`] - On success, will be set to point to a newly allocated handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *remote_notification_proxy_new(struct ReadWriteOpaque *socket,
                                                      struct RemoteNotificationProxyClientHandle **client);

/**
 * Posts a notification on the device
 *
 * # Arguments
 * * [`client`] - A valid handle
 * * [`name`] - The notification to post
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *remote_notification_proxy_post(struct RemoteNotificationProxyClientHandle *client,
                                                       const char *name);

/**
 * Registers interest in a notification, after which the device relays it back
 * whenever it fires
 *
 * # Arguments
 * * [`client`] - A valid handle
 * * [`name`] - The notification to observe
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *remote_notification_proxy_observe(struct RemoteNotificationProxyClientHandle *client,
                                                          const char *name);

/**
 * Registers interest in several notifications at once
 *
 * # Arguments
 * * [`client`] - A valid handle
 * * [`names`] - The notifications to observe
 * * [`len`] - How many notifications were passed
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid, and `names` must hold `len` strings
 */
struct IdeviceFfiError *remote_notification_proxy_observe_multiple(struct RemoteNotificationProxyClientHandle *client,
                                                                   const char *const *names,
                                                                   uintptr_t len);

/**
 * Waits for the next relayed notification and returns its name
 *
 * # Arguments
 * * [`client`] - A valid handle
 * * [`name_out`] - On success, set to the notification's name. Free with
 *   `notification_proxy_free_string`.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *remote_notification_proxy_receive(struct RemoteNotificationProxyClientHandle *client,
                                                          char **name_out);

/**
 * Frees a remote notification proxy handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library, or NULL
 */
void remote_notification_proxy_free(struct RemoteNotificationProxyClientHandle *handle);

/**
 * Connects to the relay with the given provider
 *
 * # Arguments
 * * [`provider`] - A provider created by this library
 * * [`client`] - A pointer where the handle will be allocated
 *
 * # Returns
 * 0 for success, an *mut IdeviceFfiError otherwise
 *
 * # Safety
 * None of the arguments can be null. Provider must be allocated by this library.
 */
struct IdeviceFfiError *os_trace_relay_connect(struct IdeviceProviderHandle *provider,
                                               struct OsTraceRelayClientHandle **client);

/**
 * Creates a new OsTraceRelayClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated OsTraceRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *os_trace_relay_connect_rsd(struct AdapterHandle *provider,
                                                   struct RsdHandshakeHandle *handshake,
                                                   struct OsTraceRelayClientHandle **client);

/**
 * Frees the relay client
 *
 * # Arguments
 * * [`handle`] - The relay client handle
 *
 * # Safety
 * The handle must be allocated by this library
 */
void os_trace_relay_free(struct OsTraceRelayClientHandle *handle);

/**
 * Creates a handle and starts receiving logs
 *
 * # Arguments
 * * [`client`] - The relay client handle
 * * [`receiver`] - A pointer to allocate the new handle to
 * * [`pid`] - An optional pointer to a PID to get logs for. May be null.
 *
 * # Returns
 * 0 for success, an *mut IdeviceFfiError otherwise
 *
 * # Safety
 * The handle must be allocated by this library. It is consumed, and must never be used again.
 */
struct IdeviceFfiError *os_trace_relay_start_trace(struct OsTraceRelayClientHandle *client,
                                                   struct OsTraceRelayReceiverHandle **receiver,
                                                   const uint32_t *pid);

/**
 * Frees the receiver handle
 *
 * # Arguments
 * * [`handle`] - The relay receiver client handle
 *
 * # Safety
 * The handle must be allocated by this library. It is consumed, and must never be used again.
 */
void os_trace_relay_receiver_free(struct OsTraceRelayReceiverHandle *handle);

/**
 * Gets the PID list from the device
 *
 * # Arguments
 * * [`client`] - The relay receiver client handle
 * * [`list`] - A pointer to allocate a list of PIDs to
 *
 * # Returns
 * 0 for success, an *mut IdeviceFfiError otherwise
 *
 * # Safety
 * The handle must be allocated by this library.
 */
struct IdeviceFfiError *os_trace_relay_get_pid_list(struct OsTraceRelayClientHandle *client,
                                                    struct Vec_u64 **list);

/**
 * Gets the next log from the relay
 *
 * # Arguments
 * * [`client`] - The relay receiver client handle
 * * [`log`] - A pointer to allocate the new log
 *
 * # Returns
 * 0 for success, an *mut IdeviceFfiError otherwise
 *
 * # Safety
 * The handle must be allocated by this library.
 */
struct IdeviceFfiError *os_trace_relay_next(struct OsTraceRelayReceiverHandle *client,
                                            struct OsTraceLog **log);

/**
 * Frees a log received from the relay
 *
 * # Arguments
 * * [`log`] - The log to free
 *
 * # Returns
 * 0 for success, an *mut IdeviceFfiError otherwise
 *
 * # Safety
 * The log must be allocated by this library. It is consumed and must not be used again.
 */
void os_trace_relay_free_log(struct OsTraceLog *log);

/**
 * Creates a cancellation token for `pairable_host_accept`.
 *
 * Returns NULL only if allocation fails. Free with `pairable_host_cancel_free`.
 */
struct PairableHostCancel *pairable_host_cancel_new(void);

/**
 * Signals a cancellation token, unblocking the `pairable_host_accept` it was passed
 * to. That call returns the `CanceledByUser` error.
 *
 * Safe to call from any thread, before or during the accept, and safe to call more
 * than once. Cancelling a token that was never passed to an accept, or one whose
 * accept already returned, does nothing.
 *
 * # Safety
 * `cancel` must be a pointer returned by `pairable_host_cancel_new` that has not yet
 * been freed.
 */
void pairable_host_cancel_signal(const struct PairableHostCancel *cancel);

/**
 * Frees a cancellation token.
 *
 * The in-flight accept holds its own reference to the shared state, so freeing the
 * token while an accept is still running is safe — it just means nothing can cancel
 * that accept any more.
 *
 * # Safety
 * `cancel` must be a pointer returned by `pairable_host_cancel_new` or NULL, and must
 * not be used afterwards.
 */
void pairable_host_cancel_free(struct PairableHostCancel *cancel);

/**
 * Advertises this computer as a pairable host and accepts a single device-initiated
 * pairing.
 *
 * This blocks the calling thread until a device discovers the advertised
 * `_remotepairing-pairable-host._tcp` service, connects, and the pairing either
 * completes or fails — or until `cancel` is signalled from another thread. While the
 * pairing is in progress `pin_callback` is invoked once with the 6-digit setup code
 * that the user must type into the device.
 *
 * On success a freshly generated [`RpPairingFileHandle`] is written to
 * `out_pairing_file`; it carries this host's long-term keys plus the paired
 * device's `altIRK`. Persist it (and `out_host_alt_irk`, see below) so the device
 * keeps recognizing this host on future connections.
 *
 * # Arguments
 * * `name` - human-readable name shown on the device (e.g. "Jackson's MacBook Pro").
 * * `model` - hardware model identifier shown on the device. `NULL` defaults to
 *   `"Mac17,7"`. iOS treats the host as a computer, so keep this a Mac identifier.
 * * `port` - TCP port to listen on. `0` picks a free port.
 * * `pin_callback` - invoked with the setup PIN to display. May be `NULL`.
 * * `pin_context` - opaque pointer passed back to `pin_callback`.
 * * `cancel` - optional cancellation token from `pairable_host_cancel_new`. Signal it
 *   from another thread to abort the wait (e.g. the user dismissed the pairing UI).
 *   `NULL` means the call can only be ended by a device connecting. Without one there
 *   is no way to stop advertising short of exiting the process.
 * * `out_host_alt_irk` - optional. If non-NULL, must point to a 16-byte buffer that
 *   receives the host's generated `altIRK` (needed to re-advertise this host so an
 *   already-paired device recognizes it). May be `NULL`.
 * * `out_peer_device` - optional. If non-NULL, receives the paired device's identity
 *   (name, model, UDID, `altIRK`), which the caller must free with
 *   `rppairing_peer_device_free`. May be `NULL`.
 * * `out_pairing_file` - receives the resulting pairing file on success.
 *
 * # Safety
 * `name` must be a valid null-terminated C string. `model` must be NULL or a valid
 * null-terminated C string. `cancel` must be NULL or a live token from
 * `pairable_host_cancel_new`. `out_host_alt_irk` must be NULL or point to at least 16
 * writable bytes. `out_peer_device` must be NULL or a valid writable pointer.
 * `out_pairing_file` must be valid and non-null.
 */
struct IdeviceFfiError *pairable_host_accept(const char *name,
                                             const char *model,
                                             uint16_t port,
                                             void (*pin_callback)(const char *pin, void *context),
                                             void *pin_context,
                                             const struct PairableHostCancel *cancel,
                                             uint8_t *out_host_alt_irk,
                                             struct RpPairingPeerDeviceC **out_peer_device,
                                             struct RpPairingFileHandle **out_pairing_file);

/**
 * Same as `pairable_host_accept`, with explicit pairable-host policy options.
 *
 * # Safety
 * Same pointer validity requirements as `pairable_host_accept`.
 */
struct IdeviceFfiError *pairable_host_accept_with_options(const char *name,
                                                          const char *model,
                                                          uint16_t port,
                                                          bool allows_pinless_pairing,
                                                          void (*pin_callback)(const char *pin,
                                                                               void *context),
                                                          void *pin_context,
                                                          const struct PairableHostCancel *cancel,
                                                          uint8_t *out_host_alt_irk,
                                                          struct RpPairingPeerDeviceC **out_peer_device,
                                                          struct RpPairingFileHandle **out_pairing_file);

/**
 * Reads a pairing file from the specified path
 *
 * # Arguments
 * * [`path`] - Path to the pairing file
 * * [`pairing_file`] - On success, will be set to point to a newly allocated pairing file instance
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `path` must be a valid null-terminated C string
 * `pairing_file` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_pairing_file_read(const char *path,
                                                  struct IdevicePairingFile **pairing_file);

/**
 * Parses a pairing file from a byte buffer
 *
 * # Arguments
 * * [`data`] - Pointer to the buffer containing pairing file data
 * * [`size`] - Size of the buffer in bytes
 * * [`pairing_file`] - On success, will be set to point to a newly allocated pairing file instance
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `data` must be a valid pointer to a buffer of at least `size` bytes
 * `pairing_file` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_pairing_file_from_bytes(const uint8_t *data,
                                                        uintptr_t size,
                                                        struct IdevicePairingFile **pairing_file);

/**
 * Serializes a pairing file to XML format
 *
 * # Arguments
 * * [`pairing_file`] - The pairing file to serialize
 * * [`data`] - On success, will be set to point to a newly allocated buffer containing the serialized data
 * * [`size`] - On success, will be set to the size of the allocated buffer
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `pairing_file` must be a valid, non-null pointer to a pairing file instance
 * `data` must be a valid, non-null pointer to a location where the buffer pointer will be stored
 * `size` must be a valid, non-null pointer to a location where the buffer size will be stored
 */
struct IdeviceFfiError *idevice_pairing_file_serialize(const struct IdevicePairingFile *pairing_file,
                                                       uint8_t **data,
                                                       uintptr_t *size);

/**
 * Frees a pairing file instance
 *
 * # Arguments
 * * [`pairing_file`] - The pairing file to free
 *
 * # Safety
 * `pairing_file` must be a valid pointer to a pairing file instance that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void idevice_pairing_file_free(struct IdevicePairingFile *pairing_file);

/**
 * Generates a fresh host identity and returns the data a caller needs to publish
 * its own `_remotepairing-pairable-host._tcp` Bonjour service.
 *
 * # Arguments
 * * `name` - human-readable name shown on the device.
 * * `model` - hardware model shown on the device. `NULL` defaults to `"Mac17,7"`.
 * * `allows_pinless_pairing` - if true, advertise pinless pairing and use the
 *   all-zero setup code expected by that flow; if false, generate a random PIN.
 * * `out_handle` - receives the host handle; pass it to `pairable_host_accept_fd`
 *   and free it with `pairable_host_free`.
 * * `out_service_id` - receives the Bonjour service instance name. Free with
 *   `idevice_string_free`.
 * * `out_txt_data`/`out_txt_len` - receive an XML plist dictionary of the TXT
 *   records to publish. Free with `idevice_data_free`.
 * * `out_host_alt_irk` - optional. If non-NULL, must point to a 16-byte buffer
 *   that receives the generated host `altIRK`; persist it with the pairing file.
 *
 * A fresh identity is generated on every call.
 *
 * # Safety
 * `name` must be a valid null-terminated C string. `model` must be NULL or a
 * valid null-terminated C string. All required out-pointers must be valid and
 * non-null. `out_host_alt_irk` must be NULL or point to at least 16 writable bytes.
 */
struct IdeviceFfiError *pairable_host_prepare(const char *name,
                                              const char *model,
                                              bool allows_pinless_pairing,
                                              struct PairableHostHandle **out_handle,
                                              char **out_service_id,
                                              uint8_t **out_txt_data,
                                              uintptr_t *out_txt_len,
                                              uint8_t *out_host_alt_irk);

/**
 * Backwards-compatible alias for AltStore's original function name.
 * Prefer `pairable_host_prepare` for new callers.
 *
 * # Safety
 * Same requirements as `pairable_host_prepare`, except `model` is required and
 * pinless pairing is disabled.
 */
struct IdeviceFfiError *pairable_host_new(const char *name,
                                          const char *model,
                                          struct PairableHostHandle **out_handle,
                                          char **out_service_id,
                                          uint8_t **out_txt_data,
                                          uintptr_t *out_txt_len);

/**
 * Runs pair-setup against a device that has already connected to `fd`.
 *
 * Blocks the calling thread until pairing succeeds or fails. The fd is duplicated
 * before use, so the caller keeps ownership of the original socket.
 *
 * # Safety
 * `handle` must be a valid handle from `pairable_host_prepare` or
 * `pairable_host_new`. `fd` must be a valid connected TCP socket.
 * `out_pairing_file` must be valid and non-null. `out_peer_device` must be NULL
 * or a valid writable pointer. `pin_cb`/`ctx` must stay valid until this call returns.
 */
struct IdeviceFfiError *pairable_host_accept_fd(struct PairableHostHandle *handle,
                                                int32_t fd,
                                                PairableHostPinCb pin_cb,
                                                void *ctx,
                                                struct RpPairingPeerDeviceC **out_peer_device,
                                                struct RpPairingFileHandle **out_pairing_file);

/**
 * Backwards-compatible alias for AltStore's original function name.
 * Prefer `pairable_host_accept_fd` for new callers.
 *
 * # Safety
 * Same requirements as `pairable_host_accept_fd`.
 */
struct IdeviceFfiError *pairable_host_handshake(struct PairableHostHandle *handle,
                                                int32_t fd,
                                                PairableHostPinCb pin_cb,
                                                void *ctx,
                                                struct RpPairingFileHandle **out_pairing_file);

/**
 * Frees a `PairableHostHandle`.
 *
 * # Safety
 * `handle` must be a handle from `pairable_host_prepare`/`pairable_host_new`, or NULL.
 */
void pairable_host_free(struct PairableHostHandle *handle);

/**
 * Automatically creates and connects to pcapd, returning a client handle.
 * Note that this service only works over USB or through RSD.
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated PcapdClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *pcapd_connect(struct IdeviceProviderHandle *provider,
                                      struct PcapdClientHandle **client);

/**
 * Creates a new PcapdClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated PcapdClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *pcapd_connect_rsd(struct AdapterHandle *provider,
                                          struct RsdHandshakeHandle *handshake,
                                          struct PcapdClientHandle **client);

/**
 * Creates a new PcapdClient from an existing socket
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated PcapdClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *pcapd_new(struct IdeviceHandle *socket, struct PcapdClientHandle **client);

/**
 * Reads the next packet from the pcapd service
 *
 * # Arguments
 * * `client` - A valid PcapdClient handle
 * * `packet` - On success, will be set to point to a newly allocated DevicePacketHandle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * The returned packet must be freed with `pcapd_device_packet_free`
 */
struct IdeviceFfiError *pcapd_next_packet(struct PcapdClientHandle *client,
                                          struct DevicePacketHandle **packet);

/**
 * Frees a DevicePacketHandle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void pcapd_device_packet_free(struct DevicePacketHandle *handle);

/**
 * Frees a PcapdClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void pcapd_client_free(struct PcapdClientHandle *handle);

/**
 * Automatically creates and connects to Preboard Service, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated PreboardServiceClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *preboard_service_connect(struct IdeviceProviderHandle *provider,
                                                 struct PreboardServiceClientHandle **client);

/**
 * Creates a new PreboardServiceClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated PreboardServiceClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *preboard_service_connect_rsd(struct AdapterHandle *provider,
                                                     struct RsdHandshakeHandle *handshake,
                                                     struct PreboardServiceClientHandle **client);

/**
 * Creates a new PreboardServiceClient from an existing socket
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated PreboardServiceClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *preboard_service_new(struct IdeviceHandle *socket,
                                             struct PreboardServiceClientHandle **client);

/**
 * Creates a stashbag on the device from a local preboard manifest (will prompt
 * for the passcode on the device), writing the outcome to `out_outcome`
 *
 * # Arguments
 * * `client` - A valid PreboardServiceClient handle
 * * `manifest` - Pointer to the manifest data
 * * `manifest_len` - Length of the manifest data
 * * `out_outcome` - On success, set to whether a commit is required
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `manifest` must be a valid pointer to `manifest_len` bytes of data
 * `out_outcome` must be a valid, non-null pointer
 */
struct IdeviceFfiError *preboard_service_create_stashbag(struct PreboardServiceClientHandle *client,
                                                         const uint8_t *manifest,
                                                         uintptr_t manifest_len,
                                                         enum IdeviceStashbagOutcome *out_outcome);

/**
 * Commits a stashbag on the device
 *
 * # Arguments
 * * `client` - A valid PreboardServiceClient handle
 * * `manifest` - Pointer to the manifest data
 * * `manifest_len` - Length of the manifest data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `manifest` must be a valid pointer to `manifest_len` bytes of data
 */
struct IdeviceFfiError *preboard_service_commit_stashbag(struct PreboardServiceClientHandle *client,
                                                         const uint8_t *manifest,
                                                         uintptr_t manifest_len);

/**
 * Frees a PreboardServiceClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void preboard_service_client_free(struct PreboardServiceClientHandle *handle);

/**
 * Creates a TCP provider for idevice
 *
 * # Arguments
 * * [`ip`] - The sockaddr IP to connect to
 * * [`pairing_file`] - The pairing file handle to use
 * * [`label`] - The label to use with the connection
 * * [`provider`] - A pointer to a newly allocated provider
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `ip` must be a valid sockaddr
 * `pairing_file` is consumed must never be used again
 * `label` must be a valid Cstr
 * `provider` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_tcp_provider_new(const idevice_sockaddr *ip,
                                                 struct IdevicePairingFile *pairing_file,
                                                 const char *label,
                                                 struct IdeviceProviderHandle **provider);

/**
 * Frees an IdeviceProvider handle
 *
 * # Arguments
 * * [`provider`] - The provider handle to free
 *
 * # Safety
 * `provider` must be a valid pointer to a IdeviceProvider handle that was allocated this library
 *  or NULL (in which case this function does nothing)
 */
void idevice_provider_free(struct IdeviceProviderHandle *provider);

/**
 * Creates a usbmuxd provider for idevice
 *
 * # Arguments
 * * [`addr`] - The UsbmuxdAddr handle to connect to
 * * [`tag`] - The tag returned in usbmuxd responses
 * * [`udid`] - The UDID of the device to connect to
 * * [`device_id`] - The muxer ID of the device to connect to
 * * [`label`] - The label to use with the connection
 * * [`provider`] - A pointer to a newly allocated provider
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `addr` must be a valid pointer to UsbmuxdAddrHandle created by this library, and never used again
 * `udid` must be a valid CStr
 * `label` must be a valid Cstr
 * `provider` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *usbmuxd_provider_new(struct UsbmuxdAddrHandle *addr,
                                             uint32_t tag,
                                             const char *udid,
                                             uint32_t device_id,
                                             const char *label,
                                             struct IdeviceProviderHandle **provider);

/**
 * Gets the pairing file for the device
 *
 * # Arguments
 * * [`provider`] - A pointer to the provider
 * * [`pairing_file`] - A pointer to the newly allocated pairing file
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid, non-null pointer to the provider
 */
struct IdeviceFfiError *idevice_provider_get_pairing_file(struct IdeviceProviderHandle *provider,
                                                          struct IdevicePairingFile **pairing_file);

/**
 * Connects to `remotepairingdeviced` over lockdown
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`sending_host`] - The name this computer identifies itself by, the same
 *   value the wireless flow uses
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *remote_pairing_lockdown_connect(struct IdeviceProviderHandle *provider,
                                                        const char *sending_host,
                                                        struct RemotePairingLockdownHandle **handle);

/**
 * Wraps an existing lockdown connection to `remotepairingdeviced`
 *
 * # Arguments
 * * [`socket`] - A connection to `com.apple.dt.remotepairingdeviced.lockdown`.
 *   Consumed regardless of the result.
 * * [`sending_host`] - The name this computer identifies itself by
 * * [`handle`] - Pointer to store the newly created handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *remote_pairing_lockdown_new(struct IdeviceHandle *socket,
                                                    const char *sending_host,
                                                    struct RemotePairingLockdownHandle **handle);

/**
 * Runs the control channel's handshake and returns what the device reports
 * about itself
 *
 * # Arguments
 * * [`handle`] - The client handle
 * * [`handshake`] - Pointer to store the device's response
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *remote_pairing_lockdown_attempt_pair_verify(struct RemotePairingLockdownHandle *handle,
                                                                    plist_t *handshake);

/**
 * Checks whether the device still recognizes a pairing record
 *
 * The handshake must have run first, i.e.
 * `remote_pairing_lockdown_attempt_pair_verify`.
 *
 * # Arguments
 * * [`handle`] - The client handle
 * * [`pairing_file`] - The RPPairing file to validate
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *remote_pairing_lockdown_validate_pairing(struct RemotePairingLockdownHandle *handle,
                                                                 struct RpPairingFileHandle *pairing_file);

/**
 * Pairs with the device, saving the record into `pairing_file`
 *
 * # Arguments
 * * [`handle`] - The client handle
 * * [`pairing_file`] - The RPPairing file to pair with, e.g. a fresh one from
 *   `rp_pairing_file_generate`. Updated in place on success, so write it out
 *   afterwards to keep the pairing.
 * * [`pin`] - The PIN to answer a Trust prompt with, or NULL for `000000`.
 *   Pairing over USB is promptless, so the device should never ask.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All non-NULL pointer parameters must be valid
 */
struct IdeviceFfiError *remote_pairing_lockdown_pair(struct RemotePairingLockdownHandle *handle,
                                                     struct RpPairingFileHandle *pairing_file,
                                                     const char *pin);

/**
 * Pairs only if the device doesn't already recognize the pairing record
 *
 * Runs the handshake, validates `pairing_file`, and pairs when that fails.
 *
 * # Arguments
 * * [`handle`] - The client handle
 * * [`pairing_file`] - The RPPairing file to validate or pair with. Updated in
 *   place when pairing happens, so write it out afterwards.
 * * [`pin`] - The PIN to answer a Trust prompt with, or NULL for `000000`
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All non-NULL pointer parameters must be valid
 */
struct IdeviceFfiError *remote_pairing_lockdown_connect_pairing(struct RemotePairingLockdownHandle *handle,
                                                                struct RpPairingFileHandle *pairing_file,
                                                                const char *pin);

/**
 * The encryption key established during pairing, used as the TLS-PSK for
 * tunnel connections
 *
 * # Arguments
 * * [`handle`] - The client handle
 * * [`key`] - Pointer to store the key, freed with `idevice_data_free`
 * * [`key_len`] - Pointer to store the number of bytes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * All pointer parameters must be valid
 */
struct IdeviceFfiError *remote_pairing_lockdown_encryption_key(struct RemotePairingLockdownHandle *handle,
                                                               uint8_t **key,
                                                               uintptr_t *key_len);

/**
 * Frees a remote pairing lockdown handle
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL
 */
void remote_pairing_lockdown_free(struct RemotePairingLockdownHandle *handle);

/**
 * Connects to `restored` over an existing [`IdeviceHandle`] (consumes it).
 *
 * # Safety
 * `idevice` is consumed and must not be used afterwards. `out_client` must be a
 * valid, non-null location for the resulting handle.
 */
struct IdeviceFfiError *idevice_restored_connect(struct IdeviceHandle *idevice,
                                                 struct RestoredClientHandle **out_client);

/**
 * Finds a restore-mode device by ECID over usbmux and connects to `restored`.
 *
 * After a normal to restore transition the device re-enumerates with a new usbmux
 * id, so this polls the device list and matches on `HardwareInfo.UniqueChipID`,
 * retrying until `timeout_ms` elapses.
 *
 * # Safety
 * `addr` must be a valid `UsbmuxdAddrHandle` (it is borrowed, not consumed);
 * `out_client` must be valid; `label` a valid C string or NULL.
 */
struct IdeviceFfiError *idevice_restored_connect_by_ecid(struct UsbmuxdAddrHandle *addr,
                                                         uint64_t ecid,
                                                         const char *label,
                                                         uint64_t timeout_ms,
                                                         struct RestoredClientHandle **out_client);

/**
 * Reads the device's ECID (from `HardwareInfo`).
 *
 * # Safety
 * `client` and `out_ecid` must be valid, non-null pointers.
 */
struct IdeviceFfiError *idevice_restored_get_ecid(struct RestoredClientHandle *client,
                                                  uint64_t *out_ecid);

/**
 * Reads the usbmux `device_id` the client was found on.
 *
 * Only meaningful when the client was created with
 * `idevice_restored_connect_by_ecid`; writes `true` to `out_has_device_id` in
 * that case (and the id to `out_device_id`), or `false` otherwise (e.g. clients
 * built from an existing `Idevice`). Pass the id to
 * `idevice_restore_connect_usb_port` so data-port / FDR connections target this
 * same device.
 *
 * # Safety
 * `client`, `out_device_id`, `out_has_device_id` must be valid, non-null pointers.
 */
struct IdeviceFfiError *idevice_restored_get_device_id(struct RestoredClientHandle *client,
                                                       uint32_t *out_device_id,
                                                       bool *out_has_device_id);

/**
 * Frees a [`RestoredClientHandle`].
 *
 * # Safety
 * `client` must be a handle allocated by this library, or NULL.
 */
void idevice_restored_free(struct RestoredClientHandle *client);

/**
 * Connects to `port` on the usbmux device identified by `device_id`, returning a
 * new [`IdeviceHandle`]. A convenience for restore data-port and FDR connectors.
 *
 * `device_id` must be the id `idevice_restored_get_device_id` reported for the
 * restore-mode client, so the connection targets the device being restored.
 *
 * The entire find-device-and-connect sequence runs in one async task; splitting
 * it across separate blocking calls corrupts the shared tokio I/O state, so this
 * is the supported way to build those connectors from C.
 *
 * This is a single attempt (the restore state machine retries data-port
 * connects itself); it errors rather than blocking when the device or port is
 * not yet available.
 *
 * # Safety
 * `addr` must be a valid `UsbmuxdAddrHandle` (it is borrowed, not consumed);
 * `out_idevice` must be valid; `label` a valid C string or NULL.
 */
struct IdeviceFfiError *idevice_restore_connect_usb_port(struct UsbmuxdAddrHandle *addr,
                                                         uint32_t device_id,
                                                         uint16_t port,
                                                         const char *label,
                                                         struct IdeviceHandle **out_idevice);

/**
 * Allocates a cancellation handle.
 *
 * # Safety
 * `out_handle` must be a valid, non-null location for the handle pointer.
 */
struct IdeviceFfiError *idevice_restore_cancel_handle_new(struct IdeviceRestoreCancelHandle **out_handle);

/**
 * Requests cancellation of the restore this handle was passed to.
 *
 * Safe to call from any thread while the restore runs; it is a no-op if `handle`
 * is NULL.
 *
 * # Safety
 * `handle` must be a valid handle from `idevice_restore_cancel_handle_new` (or NULL).
 */
void idevice_restore_cancel(struct IdeviceRestoreCancelHandle *handle);

/**
 * Frees a cancellation handle.
 *
 * # Safety
 * `handle` must be a valid handle from `idevice_restore_cancel_handle_new` (or NULL)
 * and must not be used after this call.
 */
void idevice_restore_cancel_handle_free(struct IdeviceRestoreCancelHandle *handle);

/**
 * Builds the default iOS `RestoreOptions` dictionary sent with `StartRestore`.
 *
 * The caller may tweak the returned plist before passing it to
 * `idevice_restore_run`, and must free it with `plist_free`.
 *
 * # Safety
 * `out_options` must be a valid, non-null location for the plist.
 */
struct IdeviceFfiError *idevice_restore_options_new(plist_t *out_options);

/**
 * Drives the restore-mode state machine to completion.
 *
 * Sends `StartRestore` with `options`, then services the device's data requests
 * (personalizing components with `tss_ticket`, streaming the filesystem over
 * ASR, proxying its key requests) until the device reports success.
 *
 * # Arguments
 * * `client` - connected [`RestoredClientHandle`].
 * * `build_identity` - the selected build-identity dictionary (plist).
 * * `board_id`, `chip_id`, `ecid` - device identifiers.
 * * `tss_ticket`/`tss_ticket_len` - the `ApImg4Ticket` (IM4M) from TSS.
 * * `components` - component-source delegate (required).
 * * `filesystem` - filesystem-image delegate, or NULL for a restore that sends
 *   no filesystem.
 * * `data_ports` - data-port connector delegate (required).
 * * `progress` - progress delegate, or NULL.
 * * `cancel` - cancellation handle from `idevice_restore_cancel_handle_new`, or
 *   NULL. When another thread calls `idevice_restore_cancel` on it, the restore
 *   stops and the device is rebooted toward recovery (returning a `Cancelled`
 *   error).
 * * `options` - the `RestoreOptions` plist (see `idevice_restore_options_new`).
 *
 * # Safety
 * All non-NULL pointers must be valid for the duration of the call, and each
 * delegate struct must remain valid until this returns.
 */
struct IdeviceFfiError *idevice_restore_run(struct RestoredClientHandle *client,
                                            plist_t build_identity,
                                            uint64_t board_id,
                                            uint64_t chip_id,
                                            uint64_t ecid,
                                            const uint8_t *tss_ticket,
                                            uintptr_t tss_ticket_len,
                                            struct IdeviceRestoreComponentSourceFFI *components,
                                            struct IdeviceRestoreFilesystemImageFFI *filesystem,
                                            struct IdeviceRestoreDataPortConnectorFFI *data_ports,
                                            struct IdeviceRestoreProgressFFI *progress,
                                            struct IdeviceRestoreCancelHandle *cancel,
                                            plist_t options);

/**
 * Opens an IPSW archive from a filesystem path.
 *
 * # Safety
 * `path` must be a valid C string; `out_ipsw` a valid, non-null location.
 */
struct IdeviceFfiError *idevice_ipsw_open(const char *path, struct IpswHandle **out_ipsw);

/**
 * Reads and parses the archive's `BuildManifest.plist`.
 *
 * # Safety
 * `ipsw` must be a valid handle; `out_manifest` a valid, non-null location. The
 * returned plist must be freed with `plist_free`.
 */
struct IdeviceFfiError *idevice_ipsw_build_manifest(struct IpswHandle *ipsw, plist_t *out_manifest);

/**
 * Reads a component named in `build_identity` into a caller-freed buffer.
 *
 * # Safety
 * `ipsw`, `name`, `out_data`, `out_len` must be valid. Free the buffer with
 * `idevice_data_free`.
 */
struct IdeviceFfiError *idevice_ipsw_read_component(struct IpswHandle *ipsw,
                                                    plist_t build_identity,
                                                    const char *name,
                                                    uint8_t **out_data,
                                                    uintptr_t *out_len);

/**
 * Reads an arbitrary archive entry by exact path into a caller-freed buffer.
 *
 * # Safety
 * `ipsw`, `path`, `out_data`, `out_len` must be valid. Free with
 * `idevice_data_free`.
 */
struct IdeviceFfiError *idevice_ipsw_read_file(struct IpswHandle *ipsw,
                                               const char *path,
                                               uint8_t **out_data,
                                               uintptr_t *out_len);

/**
 * Extracts an archive entry to a file on disk (streamed, for large images).
 *
 * # Safety
 * `ipsw`, `entry_path`, `dest_path` must be valid C strings.
 */
struct IdeviceFfiError *idevice_ipsw_extract_to_file(struct IpswHandle *ipsw,
                                                     const char *entry_path,
                                                     const char *dest_path);

/**
 * Frees an [`IpswHandle`].
 *
 * # Safety
 * `ipsw` must be a handle allocated by this library, or NULL.
 */
void idevice_ipsw_free(struct IpswHandle *ipsw);

/**
 * Selects the `BuildIdentity` matching `board_id`/`chip_id` (and, when non-NULL,
 * `restore_behavior`, e.g. "Erase"/"Update") from a `BuildManifest` plist.
 *
 * # Safety
 * `build_manifest`, `out_identity` must be valid. The result plist must be freed
 * with `plist_free`.
 */
struct IdeviceFfiError *idevice_restore_select_build_identity(plist_t build_manifest,
                                                              uint64_t board_id,
                                                              uint64_t chip_id,
                                                              const char *restore_behavior,
                                                              plist_t *out_identity);

/**
 * Resolves the archive path of a component from a build identity's `Manifest`.
 *
 * # Safety
 * `build_identity`, `name`, `out_path` must be valid. Free the string with
 * `idevice_string_free`.
 */
struct IdeviceFfiError *idevice_restore_component_path(plist_t build_identity,
                                                       const char *name,
                                                       char **out_path);

/**
 * Fetches the AP `ApImg4Ticket` (IM4M) from Apple's TSS server for a build
 * identity and device, returning the ticket bytes.
 *
 * `ap_nonce`/`sep_nonce` may be NULL (length 0); a NULL `sep_nonce` is signed
 * with a zeroed nonce.
 *
 * # Safety
 * `build_identity`, `out_ticket`, `out_ticket_len` must be valid. Free the ticket
 * with `idevice_data_free`.
 */
struct IdeviceFfiError *idevice_restore_fetch_ap_ticket(plist_t build_identity,
                                                        uint64_t board_id,
                                                        uint64_t chip_id,
                                                        uint64_t ecid,
                                                        const uint8_t *ap_nonce,
                                                        uintptr_t ap_nonce_len,
                                                        const uint8_t *sep_nonce,
                                                        uintptr_t sep_nonce_len,
                                                        uint8_t **out_ticket,
                                                        uintptr_t *out_ticket_len);

/**
 * Stitches an `IM4P` component with the `ApImg4Ticket` into a personalized
 * `IMG4` the device will accept.
 *
 * `fourcc` is either NULL (keep the payload's own type) or a pointer to exactly
 * four bytes to re-tag the payload with (required for some `Restore*` components;
 * see the library's `restore_fourcc_override`).
 *
 * # Safety
 * `im4p`, `ticket`, `out_data`, `out_len` must be valid. If non-NULL, `fourcc`
 * must point to 4 readable bytes. Free the buffer with `idevice_data_free`.
 */
struct IdeviceFfiError *idevice_img4_stitch_component(const uint8_t *im4p,
                                                      uintptr_t im4p_len,
                                                      const uint8_t *ticket,
                                                      uintptr_t ticket_len,
                                                      const uint8_t *fourcc,
                                                      uint8_t **out_data,
                                                      uintptr_t *out_len);

/**
 * Returns the four-character code a `Restore*` component must be re-tagged with,
 * if any, writing four bytes to `out_fourcc`.
 *
 * Returns `true` and fills `out_fourcc` when the component needs re-tagging;
 * returns `false` and leaves `out_fourcc` untouched otherwise.
 *
 * # Safety
 * `component_name` must be a valid C string; `out_fourcc` must point to 4
 * writable bytes.
 */
bool idevice_img4_restore_fourcc_override(const char *component_name, uint8_t *out_fourcc);

/**
 * Returns the components iBoot loads during the restore boot, in manifest order,
 * as a newline-separated, NUL-terminated string (empty when none).
 *
 * # Safety
 * `build_identity`, `out_names` must be valid. Free the string with
 * `idevice_string_free`.
 */
struct IdeviceFfiError *idevice_restore_boot_component_names(plist_t build_identity,
                                                             char **out_names);

/**
 * Builds the local (unsigned) `IM4M` preboard manifest for a stashbag request
 * from a build identity, into a caller-freed buffer.
 *
 * # Safety
 * `build_identity`, `out_data`, `out_len` must be valid. Free the buffer with
 * `idevice_data_free`.
 */
struct IdeviceFfiError *idevice_restore_build_preboard_manifest(plist_t build_identity,
                                                                uint64_t board_id,
                                                                uint64_t chip_id,
                                                                uint8_t **out_data,
                                                                uintptr_t *out_len);

/**
 * Opens a recovery/DFU device over a caller-supplied transport delegate.
 *
 * The `transport` struct is copied by value; the caller may free its own
 * storage after this returns (the `context` pointer must stay valid).
 *
 * # Safety
 * `transport` and `out_device` must be valid, non-null pointers.
 */
struct IdeviceFfiError *idevice_recovery_device_new(const struct IdeviceRestoreRecoveryTransportFFI *transport,
                                                    struct RecoveryDeviceHandle **out_device);

/**
 * Sends an iBoot command (NUL-terminated), with an explicit `bRequest`.
 *
 * # Safety
 * `device`, `command` must be valid.
 */
struct IdeviceFfiError *idevice_recovery_send_command(struct RecoveryDeviceHandle *device,
                                                      const char *command,
                                                      uint8_t b_request);

/**
 * Uploads a firmware image (bulk in recovery mode, chunked control transfers
 * in DFU mode).
 *
 * # Safety
 * `device`, `data` must be valid.
 */
struct IdeviceFfiError *idevice_recovery_send_buffer(struct RecoveryDeviceHandle *device,
                                                     const uint8_t *data,
                                                     uintptr_t len);

/**
 * Reads an environment variable via `getenv` into a caller-freed buffer.
 *
 * # Safety
 * `device`, `name`, `out_data`, `out_len` must be valid. Free with
 * `idevice_data_free`.
 */
struct IdeviceFfiError *idevice_recovery_getenv(struct RecoveryDeviceHandle *device,
                                                const char *name,
                                                uint8_t **out_data,
                                                uintptr_t *out_len);

/**
 * Sets an environment variable via `setenv`.
 *
 * # Safety
 * `device`, `name`, `value` must be valid.
 */
struct IdeviceFfiError *idevice_recovery_setenv(struct RecoveryDeviceHandle *device,
                                                const char *name,
                                                const char *value);

/**
 * Enables or disables auto-boot and persists it (`saveenv`).
 *
 * # Safety
 * `device` must be valid.
 */
struct IdeviceFfiError *idevice_recovery_set_autoboot(struct RecoveryDeviceHandle *device,
                                                      bool enable);

/**
 * Issues the zero-length `finish_transfer` control request.
 *
 * # Safety
 * `device` must be valid.
 */
struct IdeviceFfiError *idevice_recovery_finish_transfer(struct RecoveryDeviceHandle *device);

/**
 * Reboots the device.
 *
 * # Safety
 * `device` must be valid.
 */
struct IdeviceFfiError *idevice_recovery_reboot(struct RecoveryDeviceHandle *device);

/**
 * Returns the device's USB `idProduct` (identifying its mode), and whether it
 * is a recovery (iBoot) mode as opposed to DFU/WTF.
 *
 * # Safety
 * `device`, `out_product_id`, `out_is_recovery` must be valid.
 */
struct IdeviceFfiError *idevice_recovery_get_mode(struct RecoveryDeviceHandle *device,
                                                  uint16_t *out_product_id,
                                                  bool *out_is_recovery);

/**
 * Fills device identifiers parsed from the recovery serial string.
 *
 * Each `has_*` output is set to whether the corresponding value was present;
 * missing values leave their `out_*` untouched.
 *
 * # Safety
 * All non-null out-pointers must be valid.
 */
struct IdeviceFfiError *idevice_recovery_get_info(struct RecoveryDeviceHandle *device,
                                                  uint64_t *out_cpid,
                                                  bool *out_has_cpid,
                                                  uint64_t *out_bdid,
                                                  bool *out_has_bdid,
                                                  uint64_t *out_ecid,
                                                  bool *out_has_ecid);

/**
 * Returns the AP nonce (`NONC`) from the recovery serial, if present, into a
 * caller-freed buffer. Returns `true` when a nonce was present.
 *
 * # Safety
 * `device`, `out_data`, `out_len` must be valid. Free with `idevice_data_free`.
 */
bool idevice_recovery_get_ap_nonce(struct RecoveryDeviceHandle *device,
                                   uint8_t **out_data,
                                   uintptr_t *out_len);

/**
 * Frees a [`RecoveryDeviceHandle`].
 *
 * # Safety
 * `device` must be a handle allocated by this library, or NULL.
 */
void idevice_recovery_device_free(struct RecoveryDeviceHandle *device);

/**
 * Starts the FDR trust channel: control handshake, then a background listener
 * running for the rest of the restore.
 *
 * The `connector` struct is copied by value (its `context` must stay valid for
 * the duration of the restore).
 *
 * # Safety
 * `connector` must be a valid, non-null pointer.
 */
struct IdeviceFfiError *idevice_restore_fdr_start(const struct IdeviceRestoreFdrConnectorFFI *connector);

/**
 * Creates a new RestoreServiceClient from a ReadWrite stream
 *
 * # Arguments
 * * [`socket`] - A ReadWriteOpaque handle (consumed)
 * * [`client`] - On success, will be set to point to a newly allocated handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *restore_service_new(struct ReadWriteOpaque *socket,
                                            struct RestoreServiceClientHandle **client);

/**
 * Creates a new RestoreServiceClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated RestoreServiceClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *restore_service_connect_rsd(struct AdapterHandle *provider,
                                                    struct RsdHandshakeHandle *handshake,
                                                    struct RestoreServiceClientHandle **client);

/**
 * Enters recovery mode on the device
 *
 * # Arguments
 * * `client` - A valid RestoreServiceClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *restore_service_enter_recovery(struct RestoreServiceClientHandle *client);

/**
 * Reboots the device
 *
 * # Arguments
 * * `client` - A valid RestoreServiceClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *restore_service_reboot(struct RestoreServiceClientHandle *client);

/**
 * Gets preflight info from the device
 *
 * # Arguments
 * * `client` - A valid RestoreServiceClient handle
 * * `res` - Will be set to a pointer of a plist dictionary node on success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *restore_service_get_preflightinfo(struct RestoreServiceClientHandle *client,
                                                          plist_t *res);

/**
 * Gets nonces from the device
 *
 * # Arguments
 * * `client` - A valid RestoreServiceClient handle
 * * `res` - Will be set to a pointer of a plist dictionary node on success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *restore_service_get_nonces(struct RestoreServiceClientHandle *client,
                                                   plist_t *res);

/**
 * Gets app parameters from the device
 *
 * # Arguments
 * * `client` - A valid RestoreServiceClient handle
 * * `res` - Will be set to a pointer of a plist dictionary node on success
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 */
struct IdeviceFfiError *restore_service_get_app_parameters(struct RestoreServiceClientHandle *client,
                                                           plist_t *res);

/**
 * Restores the device language
 *
 * # Arguments
 * * `client` - A valid RestoreServiceClient handle
 * * `language` - The language to restore to
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `language` must be a valid null-terminated C string
 */
struct IdeviceFfiError *restore_service_restore_lang(struct RestoreServiceClientHandle *client,
                                                     const char *language);

/**
 * Frees a RestoreServiceClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void restore_service_client_free(struct RestoreServiceClientHandle *handle);

/**
 * Generates a new RPPairing file with fresh Ed25519 keys.
 *
 * # Safety
 * `hostname` must be a valid null-terminated C string.
 * `out` must be valid and non-null.
 */
struct IdeviceFfiError *rp_pairing_file_generate(const char *hostname,
                                                 struct RpPairingFileHandle **out);

/**
 * Reads an RPPairing file from a path.
 *
 * # Safety
 * `path` must be a valid null-terminated C string.
 * `out` must be valid and non-null.
 */
struct IdeviceFfiError *rp_pairing_file_read(const char *path, struct RpPairingFileHandle **out);

/**
 * Parses an RPPairing file from plist bytes (XML or binary).
 *
 * # Safety
 * `data` must point to `len` valid bytes.
 * `out` must be valid and non-null.
 */
struct IdeviceFfiError *rp_pairing_file_from_bytes(const uint8_t *data,
                                                   uintptr_t len,
                                                   struct RpPairingFileHandle **out);

/**
 * Serializes an RPPairing file to XML plist bytes.
 *
 * The caller must free the returned bytes with `idevice_data_free(data, len)`.
 *
 * # Safety
 * `handle`, `out_data`, and `out_len` must be valid and non-null.
 */
struct IdeviceFfiError *rp_pairing_file_to_bytes(struct RpPairingFileHandle *handle,
                                                 uint8_t **out_data,
                                                 uintptr_t *out_len);

/**
 * Writes an RPPairing file to a path.
 *
 * # Safety
 * `handle` and `path` must be valid.
 */
struct IdeviceFfiError *rp_pairing_file_write(struct RpPairingFileHandle *handle, const char *path);

/**
 * Frees an RPPairing file handle.
 *
 * # Safety
 * `handle` must be valid or NULL.
 */
void rp_pairing_file_free(struct RpPairingFileHandle *handle);

/**
 * Frees a peer device struct and its heap-allocated string fields.
 *
 * # Safety
 * `peer_device` must be a pointer returned by `rppairing_pair_network` or
 * `pairable_host_accept`, or NULL.
 */
void rppairing_peer_device_free(struct RpPairingPeerDeviceC *peer_device);

/**
 * Creates a new RSD handshake from a ReadWrite connection
 *
 * # Arguments
 * * [`socket`] - The connection to use for communication
 * * [`handle`] - Pointer to store the newly created RsdHandshake handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a ReadWrite handle allocated by this library. It is
 * consumed and may not be used again.
 * `handle` must be a valid pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *rsd_handshake_new(struct ReadWriteOpaque *socket,
                                          struct RsdHandshakeHandle **handle);

/**
 * Gets the protocol version from the RSD handshake
 *
 * # Arguments
 * * [`handle`] - A valid RsdHandshake handle
 * * [`version`] - Pointer to store the protocol version
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `version` must be a valid pointer to store the version
 */
struct IdeviceFfiError *rsd_get_protocol_version(struct RsdHandshakeHandle *handle,
                                                 size_t *version);

/**
 * Gets the UUID from the RSD handshake
 *
 * # Arguments
 * * [`handle`] - A valid RsdHandshake handle
 * * [`uuid`] - Pointer to store the UUID string (caller must free with rsd_free_string)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `uuid` must be a valid pointer to store the string pointer
 */
struct IdeviceFfiError *rsd_get_uuid(struct RsdHandshakeHandle *handle, char **uuid);

/**
 * Gets all available services from the RSD handshake
 *
 * # Arguments
 * * [`handle`] - A valid RsdHandshake handle
 * * [`services`] - Pointer to store the services array
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `services` must be a valid pointer to store the services array
 * Caller must free the returned array with rsd_free_services
 */
struct IdeviceFfiError *rsd_get_services(struct RsdHandshakeHandle *handle,
                                         struct CRsdServiceArray **services);

/**
 * Checks if a specific service is available
 *
 * # Arguments
 * * [`handle`] - A valid RsdHandshake handle
 * * [`service_name`] - Name of the service to check for
 * * [`available`] - Pointer to store the availability result
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `service_name` must be a valid C string
 * `available` must be a valid pointer to store the boolean result
 */
struct IdeviceFfiError *rsd_service_available(struct RsdHandshakeHandle *handle,
                                              const char *service_name,
                                              bool *available);

/**
 * Gets information about a specific service
 *
 * # Arguments
 * * [`handle`] - A valid RsdHandshake handle
 * * [`service_name`] - Name of the service to get info for
 * * [`service_info`] - Pointer to store the service information
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library
 * `service_name` must be a valid C string
 * `service_info` must be a valid pointer to store the service info
 * Caller must free the returned service with rsd_free_service
 */
struct IdeviceFfiError *rsd_get_service_info(struct RsdHandshakeHandle *handle,
                                             const char *service_name,
                                             struct CRsdService **service_info);

/**
 * Clones an RSD handshake
 *
 * # Safety
 * Pass a valid pointer allocated by this library
 */
struct RsdHandshakeHandle *rsd_handshake_clone(struct RsdHandshakeHandle *handshake);

/**
 * Frees a string returned by RSD functions
 *
 * # Arguments
 * * [`string`] - The string to free
 *
 * # Safety
 * Must only be called with strings returned from RSD functions
 */
void rsd_free_string(char *string);

/**
 * Frees a single service returned by rsd_get_service_info
 *
 * # Arguments
 * * [`service`] - The service to free
 *
 * # Safety
 * Must only be called with services returned from rsd_get_service_info
 */
void rsd_free_service(struct CRsdService *service);

/**
 * Frees services array returned by rsd_get_services
 *
 * # Arguments
 * * [`services`] - The services array to free
 *
 * # Safety
 * Must only be called with arrays returned from rsd_get_services
 */
void rsd_free_services(struct CRsdServiceArray *services);

/**
 * Frees an RSD handshake handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void rsd_handshake_free(struct RsdHandshakeHandle *handle);

/**
 * Connects to screenshotr service using provider
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated ScreenshotrClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *screenshotr_connect(struct IdeviceProviderHandle *provider,
                                            struct ScreenshotrClientHandle **client);

/**
 * Creates a new ScreenshotService via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated ScreenshotrClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *screenshotr_connect_rsd(struct AdapterHandle *provider,
                                                struct RsdHandshakeHandle *handshake,
                                                struct ScreenshotrClientHandle **client);

/**
 * Takes a screenshot from the device
 *
 * # Arguments
 * * `client` - A valid ScreenshotrClient handle
 * * `screenshot` - Pointer to store the screenshot data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `screenshot` must be a valid pointer to store the screenshot data
 * The caller is responsible for freeing the screenshot data using screenshotr_screenshot_free
 */
struct IdeviceFfiError *screenshotr_take_screenshot(struct ScreenshotrClientHandle *client,
                                                    struct ScreenshotData *screenshot);

/**
 * Frees screenshot data
 *
 * # Arguments
 * * `screenshot` - The screenshot data to free
 *
 * # Safety
 * `screenshot` must be a valid ScreenshotData that was allocated by screenshotr_take_screenshot
 * or NULL (in which case this function does nothing)
 */
void screenshotr_screenshot_free(struct ScreenshotData screenshot);

/**
 * Frees a ScreenshotrClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void screenshotr_client_free(struct ScreenshotrClientHandle *handle);

/**
 * Connects to the Springboard service using a provider
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated SpringBoardServicesClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *springboard_services_connect(struct IdeviceProviderHandle *provider,
                                                     struct SpringBoardServicesClientHandle **client);

/**
 * Creates a new SpringBoardServicesClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated SpringBoardServicesClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *springboard_services_connect_rsd(struct AdapterHandle *provider,
                                                         struct RsdHandshakeHandle *handshake,
                                                         struct SpringBoardServicesClientHandle **client);

/**
 * Creates a new SpringBoardServices client from an existing Idevice connection
 *
 * # Arguments
 * * [`socket`] - An IdeviceSocket handle
 * * [`client`] - On success, will be set to point to a newly allocated SpringBoardServicesClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `socket` must be a valid pointer to a handle allocated by this library. The socket is consumed,
 * and may not be used again.
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *springboard_services_new(struct IdeviceHandle *socket,
                                                 struct SpringBoardServicesClientHandle **client);

/**
 * Gets the icon of the specified app by bundle identifier
 *
 * # Arguments
 * * `client` - A valid SpringBoardServicesClient handle
 * * `bundle_identifier` - The identifiers of the app to get icon
 * * `out_result` - On success, will be set to point to a newly allocated png data
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `out_result` must be a valid, non-null pointer to a location where the result will be stored
 */
struct IdeviceFfiError *springboard_services_get_icon(struct SpringBoardServicesClientHandle *client,
                                                      const char *bundle_identifier,
                                                      void **out_result,
                                                      size_t *out_result_len);

/**
 * Gets the home screen wallpaper preview as PNG image
 *
 * # Arguments
 * * `client` - A valid SpringBoardServicesClient handle
 * * `out_result` - On success, will be set to point to newly allocated png image
 * * `out_result_len` - On success, will contain the size of the data in bytes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `out_result` and `out_result_len` must be valid, non-null pointers
 */
struct IdeviceFfiError *springboard_services_get_home_screen_wallpaper_preview(struct SpringBoardServicesClientHandle *client,
                                                                               void **out_result,
                                                                               size_t *out_result_len);

/**
 * Gets the lock screen wallpaper preview as PNG image
 *
 * # Arguments
 * * `client` - A valid SpringBoardServicesClient handle
 * * `out_result` - On success, will be set to point to newly allocated png image
 * * `out_result_len` - On success, will contain the size of the data in bytes
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `out_result` and `out_result_len` must be valid, non-null pointers
 */
struct IdeviceFfiError *springboard_services_get_lock_screen_wallpaper_preview(struct SpringBoardServicesClientHandle *client,
                                                                               void **out_result,
                                                                               size_t *out_result_len);

/**
 * Gets the current interface orientation of the device
 *
 * # Arguments
 * * `client` - A valid SpringBoardServicesClient handle
 * * `out_orientation` - On success, will contain the orientation value (0-4)
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `out_orientation` must be a valid, non-null pointer
 */
struct IdeviceFfiError *springboard_services_get_interface_orientation(struct SpringBoardServicesClientHandle *client,
                                                                       uint8_t *out_orientation);

/**
 * Gets the home screen icon layout metrics
 *
 * # Arguments
 * * `client` - A valid SpringBoardServicesClient handle
 * * `res` - On success, will point to a plist dictionary node containing the metrics
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `res` must be a valid, non-null pointer
 */
struct IdeviceFfiError *springboard_services_get_homescreen_icon_metrics(struct SpringBoardServicesClientHandle *client,
                                                                         plist_t *res);

/**
 * Frees an SpringBoardServicesClient handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void springboard_services_free(struct SpringBoardServicesClientHandle *handle);

/**
 * Automatically creates and connects to syslog relay, returning a client handle
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider
 * * [`client`] - On success, will be set to point to a newly allocated SyslogRelayClient handle
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *syslog_relay_connect_tcp(struct IdeviceProviderHandle *provider,
                                                 struct SyslogRelayClientHandle **client);

/**
 * Creates a new SyslogRelayClient via RSD
 *
 * # Arguments
 * * [`provider`] - An adapter created by this library
 * * [`handshake`] - An RSD handshake from the same provider
 * * [`client`] - On success, will be set to point to a newly allocated SyslogRelayClient handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library
 * `handshake` must be a valid pointer to a handle allocated by this library
 * `client` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *syslog_relay_connect_rsd(struct AdapterHandle *provider,
                                                 struct RsdHandshakeHandle *handshake,
                                                 struct SyslogRelayClientHandle **client);

/**
 * Frees a handle
 *
 * # Arguments
 * * [`handle`] - The handle to free
 *
 * # Safety
 * `handle` must be a valid pointer to the handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void syslog_relay_client_free(struct SyslogRelayClientHandle *handle);

/**
 * Gets the next log message from the relay
 *
 * # Arguments
 * * [`client`] - The SyslogRelayClient handle
 * * [`log_message`] - On success a newly allocated cstring will be set to point to the log message
 *
 * # Safety
 * `client` must be a valid pointer to a handle allocated by this library
 * `log_message` must be a valid, non-null pointer to a location where the log message will be stored
 */
struct IdeviceFfiError *syslog_relay_next(struct SyslogRelayClientHandle *client,
                                          char **log_message);

/**
 * # Safety
 * Pass valid pointers.
 */
struct IdeviceFfiError *idevice_tcp_stack_into_sync_objects(const char *our_ip,
                                                            const char *their_ip,
                                                            struct TcpFeedObject **feeder,
                                                            struct TcpEatObject **tcp_receiver,
                                                            struct AdapterHandle **adapter_handle);

/**
 * Feed the TCP stack with data
 * # Safety
 * Pass valid pointers. Data is cloned out of slice.
 */
struct IdeviceFfiError *idevice_tcp_feed_object_write(struct TcpFeedObject *object,
                                                      const uint8_t *data,
                                                      uintptr_t len);

/**
 * Block on getting a block of data to write to the underlying stream.
 * Write this to the stream as is, and free the data with idevice_data_free
 *
 * # Safety
 * Pass valid pointers
 */
struct IdeviceFfiError *idevice_tcp_eat_object_read(struct TcpEatObject *object,
                                                    uint8_t **data,
                                                    uintptr_t *len);

/**
 * # Safety
 * Pass a valid pointer allocated by this library
 */
void idevice_free_tcp_feed_object(struct TcpFeedObject *object);

/**
 * # Safety
 * Pass a valid pointer allocated by this library
 */
void idevice_free_tcp_eat_object(struct TcpEatObject *object);

/**
 * Creates a tunnel over USB via CoreDeviceProxy.
 * No need to stop remoted.
 *
 * # Safety
 * All pointer arguments must be valid and non-null.
 */
struct IdeviceFfiError *tunnel_create_usb(struct IdeviceProviderHandle *lockdown_provider,
                                          struct AdapterHandle **out_adapter,
                                          struct RsdHandshakeHandle **out_handshake);

/**
 * Pairs via USB CoreDeviceProxy tunnel (no SIGSTOP needed).
 *
 * For iOS, `pin_callback` can be NULL (defaults to "000000").
 * For Apple TV / Vision Pro, provide a callback returning the on-screen PIN.
 *
 * # Safety
 * All pointer arguments must be valid and non-null (except `pin_callback`/`pin_context`).
 */
struct IdeviceFfiError *tunnel_pair_usb(struct IdeviceProviderHandle *lockdown_provider,
                                        const char *hostname,
                                        const char *(*pin_callback)(void *context),
                                        void *pin_context,
                                        struct RpPairingFileHandle **out_pairing_file);

/**
 * Creates a tunnel over the network via RemoteXPC.
 *
 * Use this when connecting to a device discovered via `_remoted._tcp` (RSD port).
 * The connection goes: RSD → find tunnel service → RemoteXPC → RPPairing → tunnel.
 *
 * # Safety
 * All pointer arguments must be valid and non-null (except `pin_callback`/`pin_context`).
 * `pairing_file` is borrowed, not consumed.
 */
struct IdeviceFfiError *tunnel_create_remotexpc(const idevice_sockaddr *addr,
                                                idevice_socklen_t addr_len,
                                                const char *hostname,
                                                struct RpPairingFileHandle *pairing_file,
                                                const char *(*pin_callback)(void *context),
                                                void *pin_context,
                                                struct AdapterHandle **out_adapter,
                                                struct RsdHandshakeHandle **out_handshake);

/**
 * Creates a tunnel over the network via raw RPPairing protocol.
 *
 * Use this when connecting to a device discovered via `_remotepairing._tcp`.
 * The connection goes: direct TCP → RPPairing (JSON) → tunnel.
 *
 * `pairing_file` is used for pair-verify. If verification fails (typically
 * because the device has never been paired with this host) a full pair-setup
 * runs on the same connection and `pairing_file` is updated in place, so the
 * caller should persist it afterwards regardless of whether it was freshly
 * generated.
 *
 *
 * # Safety
 * All pointer arguments must be valid and non-null (except `pin_callback`/`pin_context`).
 * `pairing_file` is borrowed, not consumed.
 */
struct IdeviceFfiError *tunnel_create_rppairing(const idevice_sockaddr *addr,
                                                idevice_socklen_t addr_len,
                                                const char *hostname,
                                                struct RpPairingFileHandle *pairing_file,
                                                const char *(*pin_callback)(void *context),
                                                void *pin_context,
                                                struct AdapterHandle **out_adapter,
                                                struct RsdHandshakeHandle **out_handshake);

/**
 * Pairs with a device over the network via raw RPPairing, without creating a tunnel.
 *
 * This is for tvOS.
 *
 * On iOS `tunnel_create_rppairing` handles both halves on its own; this function
 * is only needed there if you want to pair and connect as separate steps.
 *
 * # Arguments
 * * `addr` / `addr_len` - address of the pairing service to connect to.
 * * `hostname` - name this host presents to the device.
 * * `pairing_file` - borrowed, not consumed. Updated in place on success. Pass a
 *   freshly generated file (`rp_pairing_file_generate`) for a first-time pairing.
 * * `pin_callback` / `pin_context` - invoked to obtain the PIN shown on the
 *   device. May be `NULL`.
 * * `out_peer_device` - optional. If non-NULL, receives the paired device's
 *   identity, which the caller must free with `rppairing_peer_device_free`. Only
 *   written when a pair-setup actually ran; a successful pair-verify leaves it
 *   NULL.
 *
 * # Safety
 * All pointer arguments must be valid and non-null except `pin_callback`,
 * `pin_context`, and `out_peer_device`.
 */
struct IdeviceFfiError *rppairing_pair_network(const idevice_sockaddr *addr,
                                               idevice_socklen_t addr_len,
                                               const char *hostname,
                                               struct RpPairingFileHandle *pairing_file,
                                               const char *(*pin_callback)(void *context),
                                               void *pin_context,
                                               struct RpPairingPeerDeviceC **out_peer_device);

/**
 * Connects to a usbmuxd instance over TCP
 *
 * # Arguments
 * * [`addr`] - The socket address to connect to
 * * [`addr_len`] - Length of the socket
 * * [`tag`] - A tag that will be returned by usbmuxd responses
 * * [`usbmuxd_connection`] - On success, will be set to point to a newly allocated UsbmuxdConnection handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `addr` must be a valid sockaddr
 * `usbmuxd_connection` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_usbmuxd_new_tcp_connection(const idevice_sockaddr *addr,
                                                           idevice_socklen_t addr_len,
                                                           uint32_t tag,
                                                           struct UsbmuxdConnectionHandle **out);

/**
 * Connects to a usbmuxd instance over unix socket
 *
 * # Arguments
 * * [`addr`] - The socket path to connect to
 * * [`tag`] - A tag that will be returned by usbmuxd responses
 * * [`usbmuxd_connection`] - On success, will be set to point to a newly allocated UsbmuxdConnection handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `addr` must be a valid CStr
 * `usbmuxd_connection` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_usbmuxd_new_unix_socket_connection(const char *addr,
                                                                   uint32_t tag,
                                                                   struct UsbmuxdConnectionHandle **usbmuxd_connection);

/**
 * Connects to a usbmuxd instance over the default connection for the platform
 *
 * # Arguments
 * * [`addr`] - The socket path to connect to
 * * [`tag`] - A tag that will be returned by usbmuxd responses
 * * [`usbmuxd_connection`] - On success, will be set to point to a newly allocated UsbmuxdConnection handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `addr` must be a valid CStr
 * `usbmuxd_connection` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_usbmuxd_new_default_connection(uint32_t tag,
                                                               struct UsbmuxdConnectionHandle **usbmuxd_connection);

/**
 * Gets a list of connected devices from usbmuxd.
 *
 * The returned list must be freed with `idevice_usbmuxd_device_list_free`.
 *
 * # Arguments
 * * `usbmuxd_conn` - A valid connection to usbmuxd.
 * * `devices` - A pointer to a C-style array of `UsbmuxdDeviceHandle` pointers. On success, this will be filled.
 * * `count` - A pointer to an integer. On success, this will be filled with the number of devices found.
 *
 * # Returns
 * An `IdeviceFfiError` on error, `null` on success.
 *
 * # Safety
 * * `usbmuxd_conn` must be a valid pointer.
 * * `devices` and `count` must be valid, non-null pointers.
 */
struct IdeviceFfiError *idevice_usbmuxd_get_devices(struct UsbmuxdConnectionHandle *usbmuxd_conn,
                                                    struct UsbmuxdDeviceHandle ***devices,
                                                    int *count);

/**
 * Connects to a service on a given device.
 *
 * This function consumes the `UsbmuxdConnectionHandle`. The handle will be invalid after this call
 * and must not be used again. The caller is NOT responsible for freeing it.
 * A new `IdeviceHandle` is returned on success, which must be freed by the caller.
 *
 * # Arguments
 * * `usbmuxd_connection` - The connection to use. It will be consumed.
 * * `device_id` - The ID of the device to connect to.
 * * `port` - The TCP port on the device to connect to.
 * * `idevice` - On success, points to the new device connection handle.
 *
 * # Returns
 * An `IdeviceFfiError` on error, `null` on success.
 *
 * # Safety
 * * `usbmuxd_connection` must be a valid pointer allocated by this library and never used again.
 *   The value is consumed.
 * * `idevice` must be a valid, non-null pointer.
 */
struct IdeviceFfiError *idevice_usbmuxd_connect_to_device(struct UsbmuxdConnectionHandle *usbmuxd_connection,
                                                          uint32_t device_id,
                                                          uint16_t port,
                                                          const char *label,
                                                          struct IdeviceHandle **idevice);

/**
 * Reads the pairing record for a given device UDID.
 *
 * The returned `PairingFileHandle` must be freed with `idevice_pair_record_free`.
 *
 * # Arguments
 * * `usbmuxd_conn` - A valid connection to usbmuxd.
 * * `udid` - The UDID of the device.
 * * `pair_record` - On success, points to the new pairing file handle.
 *
 * # Returns
 * An `IdeviceFfiError` on error, `null` on success.
 *
 * # Safety
 * * `usbmuxd_conn` must be a valid pointer.
 * * `udid` must be a valid, null-terminated C string.
 * * `pair_record` must be a valid, non-null pointer.
 */
struct IdeviceFfiError *idevice_usbmuxd_get_pair_record(struct UsbmuxdConnectionHandle *usbmuxd_conn,
                                                        const char *udid,
                                                        struct IdevicePairingFile **pair_record);

/**
 * Saves the pairing record for a given device UDID.
 *
 * # Arguments
 * * `usbmuxd_conn` - A valid connection to usbmuxd.
 * * `device_id` - The muxer ID for the device
 * * `udid` - The UDID of the device.
 * * `pair_record` - The bytes of the pairing record plist to save
 * * `pair_record_len` - the length of the pairing record bytes
 *
 * # Returns
 * An `IdeviceFfiError` on error, `null` on success.
 *
 * # Safety
 * * `usbmuxd_conn` must be a valid pointer.
 * * `udid` must be a valid, null-terminated C string.
 * * `pair_record` must be a valid, non-null pointer.
 */
struct IdeviceFfiError *idevice_usbmuxd_save_pair_record(struct UsbmuxdConnectionHandle *usbmuxd_conn,
                                                         const char *udid,
                                                         uint8_t *pair_record,
                                                         uintptr_t pair_record_len);

/**
 * Listens on the socket for connections and disconnections
 *
 * # Safety
 * Pass valid pointers. Free the stream with ``idevice_usbmuxd_listener_handle_free``.
 * The stream must outlive the usbmuxd connection, and the usbmuxd connection cannot
 * be used for other requests.
 */
struct IdeviceFfiError *idevice_usbmuxd_listen(struct UsbmuxdConnectionHandle *usbmuxd_conn,
                                               struct UsbmuxdListenerHandle **stream_handle);

/**
 * Frees a stream created by ``listen`` or does nothing on null
 *
 * # Safety
 * Pass a valid pointer.
 */
void idevice_usbmuxd_listener_handle_free(struct UsbmuxdListenerHandle *stream_handle);

/**
 * Gets the next event from the stream.
 * Connect will be set to true if the event is a connection event,
 * and the connection_device will be filled with the device information.
 * If connection is false, the mux ID of the device will be filled.
 *
 * # Arguments
 * * `stream_handle` - The handle to the stream returned by listen
 * * `connect` - The bool that will be set
 * * `connection_device` - The pointer that will be filled on a connect event
 * * `disconnection_id` - The mux ID that will be set on a disconnect event
 *
 * # Safety
 * Pass valid pointers
 */
struct IdeviceFfiError *idevice_usbmuxd_listener_next(struct UsbmuxdListenerHandle *stream_handle,
                                                      bool *connect,
                                                      struct UsbmuxdDeviceHandle **connection_device,
                                                      uint32_t *disconnection_id);

/**
 * Reads the BUID (Boot-Unique ID) from usbmuxd.
 *
 * The returned string must be freed with `idevice_string_free`.
 *
 * # Arguments
 * * `usbmuxd_conn` - A valid connection to usbmuxd.
 * * `buid` - On success, points to a newly allocated, null-terminated C string.
 *
 * # Returns
 * An `IdeviceFfiError` on error, `null` on success.
 *
 * # Safety
 * * `usbmuxd_conn` must be a valid pointer.
 * * `buid` must be a valid, non-null pointer.
 */
struct IdeviceFfiError *idevice_usbmuxd_get_buid(struct UsbmuxdConnectionHandle *usbmuxd_conn,
                                                 char **buid);

/**
 * Frees a UsbmuxdConnection handle
 *
 * # Arguments
 * * [`usbmuxd_connection`] - The UsbmuxdConnection handle to free
 *
 * # Safety
 * `usbmuxd_connection` must be a valid pointer to a UsbmuxdConnection handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void idevice_usbmuxd_connection_free(struct UsbmuxdConnectionHandle *usbmuxd_connection);

/**
 * Creates a usbmuxd TCP address struct
 *
 * # Arguments
 * * [`addr`] - The socket address to connect to
 * * [`addr_len`] - Length of the socket
 * * [`usbmuxd_addr`] - On success, will be set to point to a newly allocated UsbmuxdAddr handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `addr` must be a valid sockaddr
 * `usbmuxd_Addr` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_usbmuxd_tcp_addr_new(const idevice_sockaddr *addr,
                                                     idevice_socklen_t addr_len,
                                                     struct UsbmuxdAddrHandle **usbmuxd_addr);

/**
 * Creates a new UsbmuxdAddr struct with a unix socket
 *
 * # Arguments
 * * [`addr`] - The socket path to connect to
 * * [`usbmuxd_addr`] - On success, will be set to point to a newly allocated UsbmuxdAddr handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `addr` must be a valid CStr
 * `usbmuxd_addr` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_usbmuxd_unix_addr_new(const char *addr,
                                                      struct UsbmuxdAddrHandle **usbmuxd_addr);

/**
 * Creates a default UsbmuxdAddr struct for the platform
 *
 * # Arguments
 * * [`usbmuxd_addr`] - On success, will be set to point to a newly allocated UsbmuxdAddr handle
 *
 * # Returns
 * An IdeviceFfiError on error, null on success
 *
 * # Safety
 * `usbmuxd_addr` must be a valid, non-null pointer to a location where the handle will be stored
 */
struct IdeviceFfiError *idevice_usbmuxd_default_addr_new(struct UsbmuxdAddrHandle **usbmuxd_addr);

/**
 * Frees a UsbmuxdAddr handle
 *
 * # Arguments
 * * [`usbmuxd_addr`] - The UsbmuxdAddr handle to free
 *
 * # Safety
 * `usbmuxd_addr` must be a valid pointer to a UsbmuxdAddr handle that was allocated by this library,
 * or NULL (in which case this function does nothing)
 */
void idevice_usbmuxd_addr_free(struct UsbmuxdAddrHandle *usbmuxd_addr);

/**
 * Frees a list of devices returned by `idevice_usbmuxd_get_devices`.
 *
 * # Arguments
 * * `devices` - The array of device handles to free.
 * * `count` - The number of elements in the array.
 *
 * # Safety
 * `devices` must be a valid pointer to an array of `count` device handles
 * allocated by this library, or NULL.
 */
void idevice_usbmuxd_device_list_free(struct UsbmuxdDeviceHandle **devices, int count);

/**
 * Frees a usbmuxd device
 *
 * # Arguments
 * * `device` - The device handle to free.
 *
 * # Safety
 * `device` must be a valid pointer to the device handle
 * allocated by this library, or NULL.
 */
void idevice_usbmuxd_device_free(struct UsbmuxdDeviceHandle *device);

/**
 * Gets the UDID from a device handle.
 * The returned string must be freed by the caller using `idevice_string_free`.
 *
 * # Safety
 * `device` must be a valid pointer to a `UsbmuxdDeviceHandle`.
 */
char *idevice_usbmuxd_device_get_udid(const struct UsbmuxdDeviceHandle *device);

/**
 * Gets the device ID from a device handle.
 *
 * # Safety
 * `device` must be a valid pointer to a `UsbmuxdDeviceHandle`.
 */
uint32_t idevice_usbmuxd_device_get_device_id(const struct UsbmuxdDeviceHandle *device);

/**
 * Gets the connection type (UsbmuxdConnectionType) from a device handle.
 *
 * # Returns
 * The enum value of the connection type, or 0 for null device handles
 *
 * # Safety
 * `device` must be a valid pointer to a `UsbmuxdDeviceHandle`.
 */
uint8_t idevice_usbmuxd_device_get_connection_type(const struct UsbmuxdDeviceHandle *device);

/**
 * Creates a new WDA client bound to the given provider.
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider. The provider is consumed and may not
 *   be used again, regardless of whether this call succeeds or fails.
 * * [`handle`] - On success, set to a newly allocated WdaClientHandle.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success.
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library.
 * The provider is consumed, and may not be used again.
 * `handle` must be a valid, non-null pointer to a location where the handle will be stored.
 */
struct IdeviceFfiError *wda_client_new(struct IdeviceProviderHandle *provider,
                                       struct WdaClientHandle **handle);

/**
 * Frees a WDA client handle.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library or NULL.
 */
void wda_client_free(struct WdaClientHandle *handle);

/**
 * Sets the device-side WDA HTTP and MJPEG ports.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library.
 */
struct IdeviceFfiError *wda_client_set_ports(struct WdaClientHandle *handle,
                                             uint16_t http,
                                             uint16_t mjpeg);

/**
 * Sets the per-request timeout in milliseconds.
 *
 * # Safety
 * `handle` must be a valid pointer to a handle allocated by this library.
 */
struct IdeviceFfiError *wda_client_set_timeout_ms(struct WdaClientHandle *handle, uint64_t ms);

/**
 * Reads the configured device-side WDA ports.
 *
 * # Safety
 * All pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_get_ports(struct WdaClientHandle *handle,
                                             uint16_t *out_http,
                                             uint16_t *out_mjpeg);

/**
 * Returns the currently tracked session id, or NULL if none.
 *
 * # Arguments
 * * [`handle`] - The WDA client handle.
 * * [`out_str`] - On success, set to a heap-allocated UTF-8 string, or NULL
 *   if no session is tracked. Free with `idevice_string_free` if non-null.
 *
 * # Safety
 * All pointers must be valid; `out_str` must be non-null.
 */
struct IdeviceFfiError *wda_client_session_id(struct WdaClientHandle *handle, char **out_str);

/**
 * Fetches `/status` from the WDA HTTP endpoint and returns the JSON response.
 *
 * # Arguments
 * * [`handle`] - The WDA client handle.
 * * [`out_json`] - On success, set to a heap-allocated JSON string. Free with
 *   `idevice_string_free`.
 *
 * # Safety
 * All pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_status(struct WdaClientHandle *handle, char **out_json);

/**
 * Waits until WDA begins responding on its HTTP endpoint.
 *
 * # Safety
 * All pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_wait_until_ready(struct WdaClientHandle *handle,
                                                    uint64_t timeout_ms,
                                                    char **out_json);

/**
 * Starts a WDA session and stores the resulting session id on the handle.
 *
 * # Arguments
 * * [`handle`] - The WDA client handle.
 * * [`bundle_id`] - Optional bundle identifier; pass NULL for an anonymous session.
 * * [`out_session_id`] - On success, set to a heap-allocated UTF-8 string.
 *   Free with `idevice_string_free`.
 *
 * # Safety
 * `handle` and `out_session_id` must be valid and non-null. `bundle_id` may be NULL.
 */
struct IdeviceFfiError *wda_client_start_session(struct WdaClientHandle *handle,
                                                 const char *bundle_id,
                                                 char **out_session_id);

/**
 * Deletes a WDA session.
 *
 * # Safety
 * `handle` and `session_id` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_delete_session(struct WdaClientHandle *handle,
                                                  const char *session_id);

/**
 * Finds a single element and returns its WDA element id.
 *
 * # Safety
 * `handle`, `using`, `value`, and `out_element_id` must be valid and non-null.
 * `session_id` may be NULL to use the handle's tracked session.
 */
struct IdeviceFfiError *wda_client_find_element(struct WdaClientHandle *handle,
                                                const char *using_,
                                                const char *value,
                                                const char *session_id,
                                                char **out_element_id);

/**
 * Finds multiple elements and returns their WDA element ids.
 *
 * # Arguments
 * * [`out_array`] - On success, set to a heap-allocated array of NUL-terminated strings.
 * * [`out_count`] - On success, set to the number of strings.
 *
 * Free the array with `wda_client_string_array_free`.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_find_elements(struct WdaClientHandle *handle,
                                                 const char *using_,
                                                 const char *value,
                                                 const char *session_id,
                                                 char ***out_array,
                                                 uintptr_t *out_count);

/**
 * Frees an array of strings allocated by `wda_client_find_elements`.
 *
 * # Safety
 * `arr` must be a pointer returned by `wda_client_find_elements` with the
 * matching `count`, or NULL.
 */
void wda_client_string_array_free(char **arr, uintptr_t count);

/**
 * Returns a raw attribute value as a JSON-encoded string.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_element_attribute(struct WdaClientHandle *handle,
                                                     const char *element_id,
                                                     const char *name,
                                                     const char *session_id,
                                                     char **out_json);

/**
 * Returns the element text-like value as a string.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_element_text(struct WdaClientHandle *handle,
                                                const char *element_id,
                                                const char *session_id,
                                                char **out_str);

/**
 * Returns the element bounds rectangle as a JSON-encoded string.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_element_rect(struct WdaClientHandle *handle,
                                                const char *element_id,
                                                const char *session_id,
                                                char **out_json);

/**
 * Returns whether an element is displayed.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_element_displayed(struct WdaClientHandle *handle,
                                                     const char *element_id,
                                                     const char *session_id,
                                                     bool *out_bool);

/**
 * Returns whether an element is enabled.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_element_enabled(struct WdaClientHandle *handle,
                                                   const char *element_id,
                                                   const char *session_id,
                                                   bool *out_bool);

/**
 * Returns whether an element is selected.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_element_selected(struct WdaClientHandle *handle,
                                                    const char *element_id,
                                                    const char *session_id,
                                                    bool *out_bool);

/**
 * Clicks an element by its WDA element id.
 *
 * # Safety
 * `handle` and `element_id` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_click(struct WdaClientHandle *handle,
                                         const char *element_id,
                                         const char *session_id);

/**
 * Sends text input to the currently focused element.
 *
 * # Safety
 * `handle` and `text` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_send_keys(struct WdaClientHandle *handle,
                                             const char *text,
                                             const char *session_id);

/**
 * Presses a hardware button through WDA.
 *
 * # Safety
 * `handle` and `name` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_press_button(struct WdaClientHandle *handle,
                                                const char *name,
                                                const char *session_id);

/**
 * Unlocks the device via WDA.
 *
 * # Safety
 * `handle` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_unlock(struct WdaClientHandle *handle, const char *session_id);

/**
 * Swipes from one coordinate to another.
 *
 * # Safety
 * `handle` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_swipe(struct WdaClientHandle *handle,
                                         int64_t start_x,
                                         int64_t start_y,
                                         int64_t end_x,
                                         int64_t end_y,
                                         double duration,
                                         const char *session_id);

/**
 * Performs a tap gesture.
 *
 * `Option<f64>` arguments are encoded as `(has, value)` pairs. When `has_*`
 * is false the underlying value is ignored.
 *
 * # Safety
 * `handle` must be valid and non-null. Optional pointers may be NULL.
 */
struct IdeviceFfiError *wda_client_tap(struct WdaClientHandle *handle,
                                       bool has_x,
                                       double x,
                                       bool has_y,
                                       double y,
                                       const char *element_id,
                                       const char *session_id);

/**
 * Performs a double-tap gesture.
 *
 * # Safety
 * `handle` must be valid and non-null. Optional pointers may be NULL.
 */
struct IdeviceFfiError *wda_client_double_tap(struct WdaClientHandle *handle,
                                              bool has_x,
                                              double x,
                                              bool has_y,
                                              double y,
                                              const char *element_id,
                                              const char *session_id);

/**
 * Performs a long-press gesture.
 *
 * # Safety
 * `handle` must be valid and non-null. Optional pointers may be NULL.
 */
struct IdeviceFfiError *wda_client_touch_and_hold(struct WdaClientHandle *handle,
                                                  double duration,
                                                  bool has_x,
                                                  double x,
                                                  bool has_y,
                                                  double y,
                                                  const char *element_id,
                                                  const char *session_id);

/**
 * Scrolls the current view or an element using a WDA mobile command.
 *
 * `Option<bool>` arguments are encoded as `(has, value)`.
 *
 * # Safety
 * `handle` must be valid and non-null. Optional string arguments may be NULL.
 */
struct IdeviceFfiError *wda_client_scroll(struct WdaClientHandle *handle,
                                          const char *direction,
                                          const char *name,
                                          const char *predicate_string,
                                          bool has_to_visible,
                                          bool to_visible,
                                          const char *element_id,
                                          const char *session_id);

/**
 * Returns the current UI source tree as XML.
 *
 * # Safety
 * `handle` and `out_str` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_source(struct WdaClientHandle *handle,
                                          const char *session_id,
                                          char **out_str);

/**
 * Returns a PNG screenshot as raw bytes.
 *
 * # Arguments
 * * [`out_bytes`] - On success, set to a heap-allocated PNG buffer.
 * * [`out_len`] - On success, set to the buffer length in bytes.
 *
 * Free the buffer with `idevice_data_free`.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_screenshot(struct WdaClientHandle *handle,
                                              const char *session_id,
                                              uint8_t **out_bytes,
                                              uintptr_t *out_len);

/**
 * Returns the current window size payload from WDA.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_window_size(struct WdaClientHandle *handle,
                                               const char *session_id,
                                               char **out_json);

/**
 * Returns the current viewport rectangle.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_viewport_rect(struct WdaClientHandle *handle,
                                                 const char *session_id,
                                                 char **out_json);

/**
 * Returns the current orientation as a string.
 *
 * # Safety
 * All non-optional pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_orientation(struct WdaClientHandle *handle,
                                               const char *session_id,
                                               char **out_str);

/**
 * Launches or activates an application via WDA.
 *
 * # Arguments
 * * [`bundle_id`] - The bundle identifier of the app to launch.
 * * [`arguments`] - Optional array of argument strings; pass NULL for none.
 * * [`arguments_count`] - Number of strings in `arguments`; ignored if NULL.
 * * [`environment_json`] - Optional JSON object string of environment variables; pass NULL for none.
 *
 * # Safety
 * `handle`, `bundle_id`, and `out_json` must be valid and non-null. Optional
 * pointers may be NULL.
 */
struct IdeviceFfiError *wda_client_launch_app(struct WdaClientHandle *handle,
                                              const char *bundle_id,
                                              const char *const *arguments,
                                              uintptr_t arguments_count,
                                              const char *environment_json,
                                              const char *session_id,
                                              char **out_json);

/**
 * Activates an already running application.
 *
 * # Safety
 * `handle`, `bundle_id`, and `out_json` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_activate_app(struct WdaClientHandle *handle,
                                                const char *bundle_id,
                                                const char *session_id,
                                                char **out_json);

/**
 * Terminates an application and returns whether termination succeeded.
 *
 * # Safety
 * `handle`, `bundle_id`, and `out_bool` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_terminate_app(struct WdaClientHandle *handle,
                                                 const char *bundle_id,
                                                 const char *session_id,
                                                 bool *out_bool);

/**
 * Queries the XCTest application state for the given bundle id.
 *
 * # Safety
 * `handle`, `bundle_id`, and `out_state` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_query_app_state(struct WdaClientHandle *handle,
                                                   const char *bundle_id,
                                                   const char *session_id,
                                                   int64_t *out_state);

/**
 * Backgrounds the current app for the given number of seconds.
 *
 * `Option<f64>` is encoded as `(has_seconds, seconds)`.
 *
 * # Safety
 * `handle` and `out_json` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_background_app(struct WdaClientHandle *handle,
                                                  bool has_seconds,
                                                  double seconds,
                                                  const char *session_id,
                                                  char **out_json);

/**
 * Returns whether the device is currently locked.
 *
 * # Safety
 * `handle` and `out_bool` must be valid and non-null.
 */
struct IdeviceFfiError *wda_client_is_locked(struct WdaClientHandle *handle,
                                             const char *session_id,
                                             bool *out_bool);

/**
 * Starts a localhost bridge to the device's default WDA ports.
 *
 * # Arguments
 * * [`provider`] - An IdeviceProvider. Provider ownership is transferred —
 *   the caller must not free or reuse the IdeviceProviderHandle on success or failure.
 * * [`handle`] - On success, set to a newly allocated WdaBridgeHandle.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success.
 *
 * # Safety
 * `provider` must be a valid pointer to a handle allocated by this library.
 * The provider is consumed, and may not be used again.
 * `handle` must be a valid, non-null pointer to a location where the handle will be stored.
 */
struct IdeviceFfiError *wda_bridge_start(struct IdeviceProviderHandle *provider,
                                         struct WdaBridgeHandle **handle);

/**
 * Starts a localhost bridge to custom device-side WDA ports.
 *
 * # Safety
 * Same requirements as [`wda_bridge_start`].
 */
struct IdeviceFfiError *wda_bridge_start_with_ports(struct IdeviceProviderHandle *provider,
                                                    uint16_t device_http,
                                                    uint16_t device_mjpeg,
                                                    struct WdaBridgeHandle **handle);

/**
 * Reads the endpoints assigned to the running bridge.
 *
 * # Arguments
 * * [`handle`] - The bridge handle.
 * * [`out_endpoints`] - On success, set to a heap-allocated WdaBridgeEndpointsC.
 *   Free with `wda_bridge_endpoints_free`.
 *
 * # Returns
 * An IdeviceFfiError on error, null on success.
 *
 * # Safety
 * All pointers must be valid and non-null.
 */
struct IdeviceFfiError *wda_bridge_endpoints(struct WdaBridgeHandle *handle,
                                             struct WdaBridgeEndpointsC **out_endpoints);

/**
 * Frees a WdaBridgeEndpointsC struct and its heap-allocated string fields.
 *
 * # Safety
 * `endpoints` must be a pointer returned by `wda_bridge_endpoints` or NULL.
 */
void wda_bridge_endpoints_free(struct WdaBridgeEndpointsC *endpoints);

/**
 * Frees a WDA bridge handle. Dropping aborts the underlying forwarder tasks.
 *
 * # Safety
 * `handle` must be a pointer returned by this library or NULL.
 */
void wda_bridge_free(struct WdaBridgeHandle *handle);

#endif  /* IDEVICE_H */



// THIS FILE IS UNDER ITS ORIGINAL LICENSE FROM LIBIMOBILEDEVICE
// THIS IS NOT PART OF IDEVICE AND ITS LICENSE
// MORE INFORMATION CAN BE FOUND AT https://github.com/libimobiledevice/libplist

/**
 * @file plist/plist.h
 * @brief Main include of libplist
 * \internal
 *
 * Copyright (c) 2012-2023 Nikias Bassen, All Rights Reserved.
 * Copyright (c) 2008-2009 Jonathan Beck, All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef LIBPLIST_H
#define LIBPLIST_H

#ifdef __cplusplus
extern "C"
{
#endif

#if _MSC_VER && _MSC_VER < 1700
    typedef __int8 int8_t;
    typedef __int16 int16_t;
    typedef __int32 int32_t;
    typedef __int64 int64_t;

    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
    typedef unsigned __int64 uint64_t;

#else
#include <stdint.h>
#endif

/*{{{ deprecation macros */
#ifdef __llvm__
  #if defined(__has_extension)
    #if (__has_extension(attribute_deprecated_with_message))
      #ifndef PLIST_WARN_DEPRECATED
        #define PLIST_WARN_DEPRECATED(x) __attribute__((deprecated(x)))
      #endif
    #else
      #ifndef PLIST_WARN_DEPRECATED
        #define PLIST_WARN_DEPRECATED(x) __attribute__((deprecated))
      #endif
    #endif
  #else
    #ifndef PLIST_WARN_DEPRECATED
      #define PLIST_WARN_DEPRECATED(x) __attribute__((deprecated))
    #endif
  #endif
#elif (__GNUC__ > 4 || (__GNUC__ == 4 && (__GNUC_MINOR__ >= 5)))
  #ifndef PLIST_WARN_DEPRECATED
    #define PLIST_WARN_DEPRECATED(x) __attribute__((deprecated(x)))
  #endif
#elif defined(_MSC_VER)
  #ifndef PLIST_WARN_DEPRECATED
    #define PLIST_WARN_DEPRECATED(x) __declspec(deprecated(x))
  #endif
#else
  #define PLIST_WARN_DEPRECATED(x)
  #pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#endif
/*}}}*/

#ifndef PLIST_API
  #ifdef LIBPLIST_STATIC
    #define PLIST_API
  #elif defined(_WIN32)
    #define PLIST_API __declspec(dllimport)
  #else
    #define PLIST_API
  #endif
#endif

#include <sys/types.h>
#include <stdarg.h>
#include <stdio.h>

    /**
     * libplist : A library to handle Apple Property Lists
     * \defgroup PublicAPI Public libplist API
     */
    /*@{*/


    /**
     * The basic plist abstract data type.
     */
    typedef void *plist_t;

    /**
     * The plist dictionary iterator.
     */
    typedef void* plist_dict_iter;

    /**
     * The plist array iterator.
     */
    typedef void* plist_array_iter;

    /**
     * The enumeration of plist node types.
     */
    typedef enum
    {
        PLIST_NONE =-1, /**< No type */
        PLIST_BOOLEAN,  /**< Boolean, scalar type */
        PLIST_INT,      /**< Integer, scalar type */
        PLIST_REAL,     /**< Real, scalar type */
        PLIST_STRING,   /**< ASCII string, scalar type */
        PLIST_ARRAY,    /**< Ordered array, structured type */
        PLIST_DICT,     /**< Unordered dictionary (key/value pair), structured type */
        PLIST_DATE,     /**< Date, scalar type */
        PLIST_DATA,     /**< Binary data, scalar type */
        PLIST_KEY,      /**< Key in dictionaries (ASCII String), scalar type */
        PLIST_UID,      /**< Special type used for 'keyed encoding' */
        PLIST_NULL,     /**< NULL type */
    } plist_type;

    /* for backwards compatibility */
    #define PLIST_UINT PLIST_INT

    /**
     * libplist error values
     */
    typedef enum
    {
        PLIST_ERR_SUCCESS      =  0,  /**< operation successful */
        PLIST_ERR_INVALID_ARG  = -1,  /**< one or more of the parameters are invalid */
        PLIST_ERR_FORMAT       = -2,  /**< the plist contains nodes not compatible with the output format */
        PLIST_ERR_PARSE        = -3,  /**< parsing of the input format failed */
        PLIST_ERR_NO_MEM       = -4,  /**< not enough memory to handle the operation */
        PLIST_ERR_IO           = -5,  /**< I/O error */
        PLIST_ERR_CIRCULAR_REF = -6,  /**< circular reference detected */
        PLIST_ERR_MAX_NESTING  = -7,  /**< maximum nesting depth exceeded */
        PLIST_ERR_UNKNOWN      = -255 /**< an unspecified error occurred */
    } plist_err_t;

    /**
     * libplist format types
     */
    typedef enum
    {
        PLIST_FORMAT_NONE    = 0,  /**< No format */
        PLIST_FORMAT_XML     = 1,  /**< XML format */
        PLIST_FORMAT_BINARY  = 2,  /**< bplist00 format */
        PLIST_FORMAT_JSON    = 3,  /**< JSON format */
        PLIST_FORMAT_OSTEP   = 4,  /**< OpenStep "old-style" plist format */
        /* 5-9 are reserved for possible future use */
        PLIST_FORMAT_PRINT   = 10, /**< human-readable output-only format */
        PLIST_FORMAT_LIMD    = 11, /**< "libimobiledevice" output-only format (ideviceinfo) */
        PLIST_FORMAT_PLUTIL  = 12, /**< plutil-style output-only format */
    } plist_format_t;

    /**
     * libplist write options
     */
    typedef enum
    {
        PLIST_OPT_NONE      = 0, /**< Default value to use when none of the options is needed. */
        PLIST_OPT_COMPACT   = 1 << 0, /**< Use a compact representation (non-prettified). Only valid for #PLIST_FORMAT_JSON and #PLIST_FORMAT_OSTEP. */
        PLIST_OPT_PARTIAL_DATA = 1 << 1, /**< Print 24 bytes maximum of #PLIST_DATA values. If the data is longer than 24 bytes,  the first 16 and last 8 bytes will be written. Only valid for #PLIST_FORMAT_PRINT. */
        PLIST_OPT_NO_NEWLINE = 1 << 2, /**< Do not print a final newline character. Only valid for #PLIST_FORMAT_PRINT, #PLIST_FORMAT_LIMD, and #PLIST_FORMAT_PLUTIL. */
        PLIST_OPT_INDENT = 1 << 3, /**< Indent each line of output. Currently only #PLIST_FORMAT_PRINT and #PLIST_FORMAT_LIMD are supported. Use #PLIST_OPT_INDENT_BY() macro to specify the level of indentation. */
    } plist_write_options_t;

    /** To be used with #PLIST_OPT_INDENT - encodes the level of indentation for OR'ing it into the #plist_write_options_t bitfield. */
    #define PLIST_OPT_INDENT_BY(x) ((x & 0xFF) << 24)


    /********************************************
     *                                          *
     *          Creation & Destruction          *
     *                                          *
     ********************************************/

    /**
     * Create a new root plist_t type #PLIST_DICT
     *
     * @return the created plist
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_dict(void);

    /**
     * Create a new root plist_t type #PLIST_ARRAY
     *
     * @return the created plist
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_array(void);

    /**
     * Create a new plist_t type #PLIST_STRING
     *
     * @param val the sting value, encoded in UTF8.
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_string(const char *val);

    /**
     * Create a new plist_t type #PLIST_BOOLEAN
     *
     * @param val the boolean value, 0 is false, other values are true.
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_bool(uint8_t val);

    /**
     * Create a new plist_t type #PLIST_INT with an unsigned integer value
     *
     * @param val the unsigned integer value
     * @return the created item
     * @sa #plist_type
     * @note The value is always stored as uint64_t internally.
     *    Use #plist_get_uint_val or #plist_get_int_val to get the unsigned or signed value.
     */
    PLIST_API plist_t plist_new_uint(uint64_t val);

    /**
     * Create a new plist_t type #PLIST_INT with a signed integer value
     *
     * @param val the signed integer value
     * @return the created item
     * @sa #plist_type
     * @note The value is always stored as uint64_t internally.
     *    Use #plist_get_uint_val or #plist_get_int_val to get the unsigned or signed value.
     */
    PLIST_API plist_t plist_new_int(int64_t val);

    /**
     * Create a new plist_t type #PLIST_REAL
     *
     * @param val the real value
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_real(double val);

    /**
     * Create a new plist_t type #PLIST_DATA
     *
     * @param val the binary buffer
     * @param length the length of the buffer
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_data(const char *val, uint64_t length);

    /**
     * Create a new plist_t type #PLIST_DATE
     *
     * @param sec The number of seconds since 01/01/1970 (UNIX timestamp)
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_unix_date(int64_t sec);

    /**
     * Create a new plist_t type #PLIST_UID
     *
     * @param val the unsigned integer value
     * @return the created item
     * @sa #plist_type
     */
    PLIST_API plist_t plist_new_uid(uint64_t val);

    /**
     * Create a new plist_t type #PLIST_NULL
     * @return the created item
     * @sa #plist_type
     * @note This type is not valid for all formats, e.g. the XML format
     *     does not support it.
     */
    PLIST_API plist_t plist_new_null(void);

    /**
     * Destruct a plist_t node and all its children recursively
     *
     * @param plist the plist to free
     */
    PLIST_API void plist_free(plist_t plist);

    /**
     * Return a copy of passed node and it's children
     *
     * @param node the plist to copy
     * @return copied plist
     */
    PLIST_API plist_t plist_copy(plist_t node);


    /********************************************
     *                                          *
     *            Array functions               *
     *                                          *
     ********************************************/

    /**
     * Get size of a #PLIST_ARRAY node.
     *
     * @param node the node of type #PLIST_ARRAY
     * @return size of the #PLIST_ARRAY node
     */
    PLIST_API uint32_t plist_array_get_size(plist_t node);

    /**
     * Get the nth item in a #PLIST_ARRAY node.
     *
     * @param node the node of type #PLIST_ARRAY
     * @param n the index of the item to get. Range is [0, array_size[
     * @return the nth item or NULL if node is not of type #PLIST_ARRAY
     */
    PLIST_API plist_t plist_array_get_item(plist_t node, uint32_t n);

    /**
     * Get the index of an item. item must be a member of a #PLIST_ARRAY node.
     *
     * @param node the node
     * @return the node index or UINT_MAX if node index can't be determined
     */
    PLIST_API uint32_t plist_array_get_item_index(plist_t node);

    /**
     * Set the nth item in a #PLIST_ARRAY node.
     * The previous item at index n will be freed using #plist_free
     *
     * @param node the node of type #PLIST_ARRAY
     * @param item the new item at index n. The array is responsible for freeing item when it is no longer needed.
     * @param n the index of the item to get. Range is [0, array_size[. Assert if n is not in range.
     */
    PLIST_API void plist_array_set_item(plist_t node, plist_t item, uint32_t n);

    /**
     * Append a new item at the end of a #PLIST_ARRAY node.
     *
     * @param node the node of type #PLIST_ARRAY
     * @param item the new item. The array is responsible for freeing item when it is no longer needed.
     */
    PLIST_API void plist_array_append_item(plist_t node, plist_t item);

    /**
     * Insert a new item at position n in a #PLIST_ARRAY node.
     *
     * @param node the node of type #PLIST_ARRAY
     * @param item the new item to insert. The array is responsible for freeing item when it is no longer needed.
     * @param n The position at which the node will be stored. Range is [0, array_size[. Assert if n is not in range.
     */
    PLIST_API void plist_array_insert_item(plist_t node, plist_t item, uint32_t n);

    /**
     * Remove an existing position in a #PLIST_ARRAY node.
     * Removed position will be freed using #plist_free.
     *
     * @param node the node of type #PLIST_ARRAY
     * @param n The position to remove. Range is [0, array_size[. Assert if n is not in range.
     */
    PLIST_API void plist_array_remove_item(plist_t node, uint32_t n);

    /**
     * Remove a node that is a child node of a #PLIST_ARRAY node.
     * node will be freed using #plist_free.
     *
     * @param node The node to be removed from its #PLIST_ARRAY parent.
     */
    PLIST_API void plist_array_item_remove(plist_t node);

    /**
     * Create an iterator of a #PLIST_ARRAY node.
     * The allocated iterator should be freed with the standard free function.
     *
     * @param node The node of type #PLIST_ARRAY
     * @param iter Location to store the iterator for the array.
     */
    PLIST_API void plist_array_new_iter(plist_t node, plist_array_iter *iter);

    /**
     * Increment iterator of a #PLIST_ARRAY node.
     *
     * @param node The node of type #PLIST_ARRAY.
     * @param iter Iterator of the array
     * @param item Location to store the item. The caller must *not* free the
     *          returned item. Will be set to NULL when no more items are left
     *          to iterate.
     */
    PLIST_API void plist_array_next_item(plist_t node, plist_array_iter iter, plist_t *item);

    /**
     * Free #PLIST_ARRAY iterator.
     *
     * @param iter Iterator to free.
     */
    PLIST_API void plist_array_free_iter(plist_array_iter iter);

    /********************************************
     *                                          *
     *         Dictionary functions             *
     *                                          *
     ********************************************/

    /**
     * Get size of a #PLIST_DICT node.
     *
     * @param node the node of type #PLIST_DICT
     * @return size of the #PLIST_DICT node
     */
    PLIST_API uint32_t plist_dict_get_size(plist_t node);

    /**
     * Create an iterator of a #PLIST_DICT node.
     * The allocated iterator should be freed with the standard free function.
     *
     * @param node The node of type #PLIST_DICT.
     * @param iter Location to store the iterator for the dictionary.
     */
    PLIST_API void plist_dict_new_iter(plist_t node, plist_dict_iter *iter);

    /**
     * Increment iterator of a #PLIST_DICT node.
     *
     * @param node The node of type #PLIST_DICT
     * @param iter Iterator of the dictionary
     * @param key Location to store the key, or NULL. The caller is responsible
     *		for freeing the the returned string.
     * @param val Location to store the value, or NULL. The caller must *not*
     *		free the returned value. Will be set to NULL when no more
     *		key/value pairs are left to iterate.
     */
    PLIST_API void plist_dict_next_item(plist_t node, plist_dict_iter iter, char **key, plist_t *val);

    /**
     * Free #PLIST_DICT iterator.
     *
     * @param iter Iterator to free.
     */
    PLIST_API void plist_dict_free_iter(plist_dict_iter iter);

    /**
     * Get key associated key to an item. Item must be member of a dictionary.
     *
     * @param node the item
     * @param key a location to store the key. The caller is responsible for freeing the returned string.
     */
    PLIST_API void plist_dict_get_item_key(plist_t node, char **key);

    /**
     * Get the nth item in a #PLIST_DICT node.
     *
     * @param node the node of type #PLIST_DICT
     * @param key the identifier of the item to get.
     * @return the item or NULL if node is not of type #PLIST_DICT. The caller should not free
     *		the returned node.
     */
    PLIST_API plist_t plist_dict_get_item(plist_t node, const char* key);

    /**
     * Get key node associated to an item. Item must be member of a dictionary.
     *
     * @param node the item
     * @return the key node of the given item, or NULL.
     */
    PLIST_API plist_t plist_dict_item_get_key(plist_t node);

    /**
     * Set item identified by key in a #PLIST_DICT node.
     * The previous item identified by key will be freed using #plist_free.
     * If there is no item for the given key a new item will be inserted.
     *
     * @param node the node of type #PLIST_DICT
     * @param item the new item associated to key
     * @param key the identifier of the item to set.
     */
    PLIST_API void plist_dict_set_item(plist_t node, const char* key, plist_t item);

    /**
     * Remove an existing position in a #PLIST_DICT node.
     * Removed position will be freed using #plist_free
     *
     * @param node the node of type #PLIST_DICT
     * @param key The identifier of the item to remove. Assert if identifier is not present.
     */
    PLIST_API void plist_dict_remove_item(plist_t node, const char* key);

    /**
     * Merge a dictionary into another. This will add all key/value pairs
     * from the source dictionary to the target dictionary, overwriting
     * any existing key/value pairs that are already present in target.
     *
     * @param target pointer to an existing node of type #PLIST_DICT
     * @param source node of type #PLIST_DICT that should be merged into target
     */
    PLIST_API void plist_dict_merge(plist_t *target, plist_t source);

    /**
     * Get a boolean value from a given #PLIST_DICT entry.
     *
     * The value node can be of type #PLIST_BOOLEAN, but also
     * #PLIST_STRING (either 'true' or 'false'),
     * #PLIST_INT with a numerical value of 0 or >= 1,
     * or #PLIST_DATA with a single byte with a value of 0 or >= 1.
     *
     * @note This function returns 0 if the dictionary does not contain an
     * entry for the given key, if the value node is of any other than
     * the above mentioned type, or has any mismatching value.
     *
     * @param dict A node of type #PLIST_DICT
     * @param key The key to look for in dict
     * @return 0 or 1 depending on the value of the node.
     */
    PLIST_API uint8_t plist_dict_get_bool(plist_t dict, const char *key);

    /**
     * Get a signed integer value from a given #PLIST_DICT entry.
     * The value node can be of type #PLIST_INT, but also
     * #PLIST_STRING with a numerical value as string (decimal or hexadecimal),
     * or #PLIST_DATA with a size of 1, 2, 4, or 8 bytes in little endian byte order.
     *
     * @note This function returns 0 if the dictionary does not contain an
     * entry for the given key, if the value node is of any other than
     * the above mentioned type, or has any mismatching value.
     *
     * @param dict A node of type #PLIST_DICT
     * @param key The key to look for in dict
     * @return Signed integer value depending on the value of the node.
     */
    PLIST_API int64_t plist_dict_get_int(plist_t dict, const char *key);

    /**
     * Get an unsigned integer value from a given #PLIST_DICT entry.
     * The value node can be of type #PLIST_INT, but also
     * #PLIST_STRING with a numerical value as string (decimal or hexadecimal),
     * or #PLIST_DATA with a size of 1, 2, 4, or 8 bytes in little endian byte order.
     *
     * @note This function returns 0 if the dictionary does not contain an
     * entry for the given key, if the value node is of any other than
     * the above mentioned type, or has any mismatching value.
     *
     * @param dict A node of type #PLIST_DICT
     * @param key The key to look for in dict
     * @return Signed integer value depending on the value of the node.
     */
    PLIST_API uint64_t plist_dict_get_uint(plist_t dict, const char *key);

    /**
     * Copy a node from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key.
     */
    PLIST_API plist_err_t plist_dict_copy_item(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy a boolean value from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note The boolean value from *source_dict* is retrieved with #plist_dict_get_bool,
     *     but is **always** created as #PLIST_BOOLEAN in *target_dict*.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key.
     */
    PLIST_API plist_err_t plist_dict_copy_bool(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy a signed integer value from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note The signed integer value from *source_dict* is retrieved with #plist_dict_get_int,
     *     but is **always** created as #PLIST_INT.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node value to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key.
     */
    PLIST_API plist_err_t plist_dict_copy_int(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy an unsigned integer value from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note The unsigned integer value from *source_dict* is retrieved with #plist_dict_get_uint,
     *     but is **always** created as #PLIST_INT.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node value to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key.
     */
    PLIST_API plist_err_t plist_dict_copy_uint(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy a #PLIST_DATA node from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note This function is like #plist_dict_copy_item, except that it fails
     *     if the source node is not of type #PLIST_DATA.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node value to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key, or if it is not of type #PLIST_DATA.
     */
    PLIST_API plist_err_t plist_dict_copy_data(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /**
     * Copy a #PLIST_STRING node from *source_dict* to *target_dict*.
     * The node is looked up in *source_dict* with given *key*, unless *alt_source_key*
     * is non-NULL, in which case it is looked up with *alt_source_key*.
     * The entry in *target_dict* is **always** created with *key*.
     *
     * @note This function is like #plist_dict_copy_item, except that it fails
     *     if the source node is not of type #PLIST_STRING.
     *
     * @param target_dict The target dictionary to copy to.
     * @param source_dict The source dictionary to copy from.
     * @param key The key for the node value to copy.
     * @param alt_source_key The alternative source key for lookup in *source_dict* or NULL.
     *
     * @result PLIST_ERR_SUCCESS on success or PLIST_ERR_INVALID_ARG if the source dictionary does not contain
     *     any entry with given key or alt_source_key, or if it is not of type #PLIST_STRING.
     */
    PLIST_API plist_err_t plist_dict_copy_string(plist_t target_dict, plist_t source_dict, const char *key, const char *alt_source_key);

    /********************************************
     *                                          *
     *                Getters                   *
     *                                          *
     ********************************************/

    /**
     * Get the parent of a node
     *
     * @param node the parent (NULL if node is root)
     */
    PLIST_API plist_t plist_get_parent(plist_t node);

    /**
     * Get the #plist_type of a node.
     *
     * @param node the node
     * @return the type of the node
     */
    PLIST_API plist_type plist_get_node_type(plist_t node);

    /**
     * Get the value of a #PLIST_KEY node.
     * This function does nothing if node is not of type #PLIST_KEY
     *
     * @param node the node
     * @param val a pointer to a C-string. This function allocates the memory,
     *            caller is responsible for freeing it.
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API void plist_get_key_val(plist_t node, char **val);

    /**
     * Get the value of a #PLIST_STRING node.
     * This function does nothing if node is not of type #PLIST_STRING
     *
     * @param node the node
     * @param val a pointer to a C-string. This function allocates the memory,
     *            caller is responsible for freeing it. Data is UTF-8 encoded.
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API void plist_get_string_val(plist_t node, char **val);

    /**
     * Get a pointer to the buffer of a #PLIST_STRING node.
     *
     * @note DO NOT MODIFY the buffer. Mind that the buffer is only available
     *   until the plist node gets freed. Make a copy if needed.
     *
     * @param node The node
     * @param length If non-NULL, will be set to the length of the string
     *
     * @return Pointer to the NULL-terminated buffer.
     */
    PLIST_API const char* plist_get_string_ptr(plist_t node, uint64_t* length);

    /**
     * Get the value of a #PLIST_BOOLEAN node.
     * This function does nothing if node is not of type #PLIST_BOOLEAN
     *
     * @param node the node
     * @param val a pointer to a uint8_t variable.
     */
    PLIST_API void plist_get_bool_val(plist_t node, uint8_t * val);

    /**
     * Get the unsigned integer value of a #PLIST_INT node.
     * This function does nothing if node is not of type #PLIST_INT
     *
     * @param node the node
     * @param val a pointer to a uint64_t variable.
     */
    PLIST_API void plist_get_uint_val(plist_t node, uint64_t * val);

    /**
     * Get the signed integer value of a #PLIST_INT node.
     * This function does nothing if node is not of type #PLIST_INT
     *
     * @param node the node
     * @param val a pointer to a int64_t variable.
     */
    PLIST_API void plist_get_int_val(plist_t node, int64_t * val);

    /**
     * Get the value of a #PLIST_REAL node.
     * This function does nothing if node is not of type #PLIST_REAL
     *
     * @param node the node
     * @param val a pointer to a double variable.
     */
    PLIST_API void plist_get_real_val(plist_t node, double *val);

    /**
     * Get the value of a #PLIST_DATA node.
     * This function does nothing if node is not of type #PLIST_DATA
     *
     * @param node the node
     * @param val a pointer to an unallocated char buffer. This function allocates the memory,
     *            caller is responsible for freeing it.
     * @param length the length of the buffer
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API void plist_get_data_val(plist_t node, char **val, uint64_t * length);

    /**
     * Get a pointer to the data buffer of a #PLIST_DATA node.
     *
     * @note DO NOT MODIFY the buffer. Mind that the buffer is only available
     *   until the plist node gets freed. Make a copy if needed.
     *
     * @param node The node
     * @param length Pointer to a uint64_t that will be set to the length of the buffer
     *
     * @return Pointer to the buffer
     */
    PLIST_API const char* plist_get_data_ptr(plist_t node, uint64_t* length);

    /**
     * Get the value of a #PLIST_DATE node.
     * This function does nothing if node is not of type #PLIST_DATE
     *
     * @param node the node
     * @param sec a pointer to an int64_t variable. Represents the number of seconds since 01/01/1970 (UNIX timestamp).
     */
    PLIST_API void plist_get_unix_date_val(plist_t node, int64_t *sec);

    /**
     * Get the value of a #PLIST_UID node.
     * This function does nothing if node is not of type #PLIST_UID
     *
     * @param node the node
     * @param val a pointer to a uint64_t variable.
     */
    PLIST_API void plist_get_uid_val(plist_t node, uint64_t * val);


    /********************************************
     *                                          *
     *                Setters                   *
     *                                          *
     ********************************************/

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_KEY
     *
     * @param node the node
     * @param val the key value
     */
    PLIST_API void plist_set_key_val(plist_t node, const char *val);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_STRING
     *
     * @param node the node
     * @param val the string value. The string is copied when set and will be
     *		freed by the node.
     */
    PLIST_API void plist_set_string_val(plist_t node, const char *val);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_BOOLEAN
     *
     * @param node the node
     * @param val the boolean value
     */
    PLIST_API void plist_set_bool_val(plist_t node, uint8_t val);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_INT
     *
     * @param node the node
     * @param val the unsigned integer value
     */
    PLIST_API void plist_set_uint_val(plist_t node, uint64_t val);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_INT
     *
     * @param node the node
     * @param val the signed integer value
     */
    PLIST_API void plist_set_int_val(plist_t node, int64_t val);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_REAL
     *
     * @param node the node
     * @param val the real value
     */
    PLIST_API void plist_set_real_val(plist_t node, double val);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_DATA
     *
     * @param node the node
     * @param val the binary buffer. The buffer is copied when set and will
     *		be freed by the node.
     * @param length the length of the buffer
     */
    PLIST_API void plist_set_data_val(plist_t node, const char *val, uint64_t length);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_DATE
     *
     * @param node the node
     * @param sec the number of seconds since 01/01/1970 (UNIX timestamp)
     */
    PLIST_API void plist_set_unix_date_val(plist_t node, int64_t sec);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_UID
     *
     * @param node the node
     * @param val the unsigned integer value
     */
    PLIST_API void plist_set_uid_val(plist_t node, uint64_t val);


    /********************************************
     *                                          *
     *            Import & Export               *
     *                                          *
     ********************************************/

    /**
     * Export the #plist_t structure to XML format.
     *
     * @param plist the root node to export
     * @param plist_xml a pointer to a C-string. This function allocates the memory,
     *            caller is responsible for freeing it. Data is UTF-8 encoded.
     * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_to_xml(plist_t plist, char **plist_xml, uint32_t * length);

    /**
     * Export the #plist_t structure to binary format.
     *
     * @param plist the root node to export
     * @param plist_bin a pointer to a char* buffer. This function allocates the memory,
     *            caller is responsible for freeing it.
     * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_to_bin(plist_t plist, char **plist_bin, uint32_t * length);

    /**
     * Export the #plist_t structure to JSON format.
     *
     * @param plist the root node to export
     * @param plist_json a pointer to a char* buffer. This function allocates the memory,
     *     caller is responsible for freeing it.
     * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
     * @param prettify pretty print the output if != 0
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_to_json(plist_t plist, char **plist_json, uint32_t* length, int prettify);

    /**
     * Export the #plist_t structure to OpenStep format.
     *
     * @param plist the root node to export
     * @param plist_openstep a pointer to a char* buffer. This function allocates the memory,
     *     caller is responsible for freeing it.
     * @param length a pointer to an uint32_t variable. Represents the length of the allocated buffer.
     * @param prettify pretty print the output if != 0
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_to_openstep(plist_t plist, char **plist_openstep, uint32_t* length, int prettify);


    /**
     * Import the #plist_t structure from XML format.
     *
     * @param plist_xml a pointer to the xml buffer.
     * @param length length of the buffer to read.
     * @param plist a pointer to the imported plist.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_xml(const char *plist_xml, uint32_t length, plist_t * plist);

    /**
     * Import the #plist_t structure from binary format.
     *
     * @param plist_bin a pointer to the xml buffer.
     * @param length length of the buffer to read.
     * @param plist a pointer to the imported plist.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_bin(const char *plist_bin, uint32_t length, plist_t * plist);

    /**
     * Import the #plist_t structure from JSON format.
     *
     * @param json a pointer to the JSON buffer.
     * @param length length of the buffer to read.
     * @param plist a pointer to the imported plist.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_json(const char *json, uint32_t length, plist_t * plist);

    /**
     * Import the #plist_t structure from OpenStep plist format.
     *
     * @param openstep a pointer to the OpenStep plist buffer.
     * @param length length of the buffer to read.
     * @param plist a pointer to the imported plist.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_openstep(const char *openstep, uint32_t length, plist_t * plist);

    /**
     * Import the #plist_t structure from memory data.
     *
     * This function will look at the first bytes of plist_data
     * to determine if plist_data contains a binary, JSON, OpenStep, or XML plist
     * and tries to parse the data in the appropriate format.
     * @note This is just a convenience function and the format detection is
     *     very basic. It checks with plist_is_binary() if the data supposedly
     *     contains binary plist data, if not it checks if the first bytes have
     *     either '{' or '[' and assumes JSON format, and XML tags will result
     *     in parsing as XML, otherwise it will try to parse as OpenStep.
     *
     * @param plist_data A pointer to the memory buffer containing plist data.
     * @param length Length of the buffer to read.
     * @param plist A pointer to the imported plist.
     * @param format If non-NULL, the #plist_format_t value pointed to will be set to the parsed format.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_from_memory(const char *plist_data, uint32_t length, plist_t *plist, plist_format_t *format);

    /**
     * Import the #plist_t structure directly from file.
     *
     * This function will look at the first bytes of the file data
     * to determine if it contains a binary, JSON, OpenStep, or XML plist
     * and tries to parse the data in the appropriate format.
     * Uses plist_from_memory() internally.
     *
     * @param filename The name of the file to parse.
     * @param plist A pointer to the imported plist.
     * @param format If non-NULL, the #plist_format_t value pointed to will be set to the parsed format.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure
     */
    PLIST_API plist_err_t plist_read_from_file(const char *filename, plist_t *plist, plist_format_t *format);

    /**
     * Write the #plist_t structure to a NULL-terminated string using the given format and options.
     *
     * @param plist The input plist structure
     * @param output Pointer to a char* buffer. This function allocates the memory,
     *     caller is responsible for freeing it.
     * @param length A pointer to a uint32_t value that will receive the lenght of the allocated buffer.
     * @param format A #plist_format_t value that specifies the output format to use.
     * @param options One or more bitwise ORed values of #plist_write_options_t.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure.
     * @note Use plist_mem_free() to free the allocated memory.
     * @note #PLIST_FORMAT_BINARY is not supported by this function.
     */
    PLIST_API plist_err_t plist_write_to_string(plist_t plist, char **output, uint32_t* length, plist_format_t format, plist_write_options_t options);

    /**
     * Write the #plist_t structure to a FILE* stream using the given format and options.
     *
     * @param plist The input plist structure
     * @param stream A writeable FILE* stream that the data will be written to.
     * @param format A #plist_format_t value that specifies the output format to use.
     * @param options One or more bitwise ORed values of #plist_write_options_t.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure.
     * @note While this function allows all formats to be written to the given stream,
     *     only the formats #PLIST_FORMAT_PRINT, #PLIST_FORMAT_LIMD, and #PLIST_FORMAT_PLUTIL
     *     (basically all output-only formats) are directly and efficiently written to the stream;
     *     the other formats are written to a memory buffer first.
     */
    PLIST_API plist_err_t plist_write_to_stream(plist_t plist, FILE* stream, plist_format_t format, plist_write_options_t options);

    /**
     * Write the #plist_t structure to a file at given path using the given format and options.
     *
     * @param plist The input plist structure
     * @param filename The file name of the file to write to. Existing files will be overwritten.
     * @param format A #plist_format_t value that specifies the output format to use.
     * @param options One or more bitwise ORed values of #plist_write_options_t.
     * @return PLIST_ERR_SUCCESS on success or a #plist_err_t on failure.
     * @note Use plist_mem_free() to free the allocated memory.
     */
    PLIST_API plist_err_t plist_write_to_file(plist_t plist, const char *filename, plist_format_t format, plist_write_options_t options);

    /**
     * Print the given plist in human-readable format to standard output.
     * This is equivalent to
     * <code>plist_write_to_stream(plist, stdout, PLIST_FORMAT_PRINT, PLIST_OPT_PARTIAL_DATA);</code>
     * @param plist The #plist_t structure to print
     * @note For #PLIST_DATA nodes, only a maximum of 24 bytes (first 16 and last 8) are written.
     */
    PLIST_API void plist_print(plist_t plist);

    /**
     * Test if in-memory plist data is in binary format.
     * This function will look at the first bytes of plist_data to determine
     * if it supposedly contains a binary plist.
     * @note The function is not validating the whole memory buffer to check
     * if the content is truly a plist, it is only using some heuristic on
     * the first few bytes of plist_data.
     *
     * @param plist_data a pointer to the memory buffer containing plist data.
     * @param length length of the buffer to read.
     * @return 1 if the buffer is a binary plist, 0 otherwise.
     */
    PLIST_API int plist_is_binary(const char *plist_data, uint32_t length);

    /********************************************
     *                                          *
     *                 Utils                    *
     *                                          *
     ********************************************/

    /**
     * Get a node from its path. Each path element depends on the associated father node type.
     * For Dictionaries, var args are casted to const char*, for arrays, var args are caster to uint32_t
     * Search is breath first order.
     *
     * @param plist the node to access result from.
     * @param length length of the path to access
     * @return the value to access.
     */
    PLIST_API plist_t plist_access_path(plist_t plist, uint32_t length, ...);

    /**
     * Variadic version of #plist_access_path.
     *
     * @param plist the node to access result from.
     * @param length length of the path to access
     * @param v list of array's index and dic'st key
     * @return the value to access.
     */
    PLIST_API plist_t plist_access_pathv(plist_t plist, uint32_t length, va_list v);

    /**
     * Compare two node values
     *
     * @param node_l left node to compare
     * @param node_r rigth node to compare
     * @return TRUE is type and value match, FALSE otherwise.
     */
    PLIST_API char plist_compare_node_value(plist_t node_l, plist_t node_r);

    /** Helper macro used by PLIST_IS_* macros that will evaluate the type of a plist node. */
    #define _PLIST_IS_TYPE(__plist, __plist_type) (__plist && (plist_get_node_type(__plist) == PLIST_##__plist_type))

    /* Helper macros for the different plist types */
    /** Evaluates to true if the given plist node is of type PLIST_BOOLEAN */
    #define PLIST_IS_BOOLEAN(__plist) _PLIST_IS_TYPE(__plist, BOOLEAN)
    /** Evaluates to true if the given plist node is of type PLIST_INT */
    #define PLIST_IS_INT(__plist)     _PLIST_IS_TYPE(__plist, INT)
    /** Evaluates to true if the given plist node is of type PLIST_REAL */
    #define PLIST_IS_REAL(__plist)    _PLIST_IS_TYPE(__plist, REAL)
    /** Evaluates to true if the given plist node is of type PLIST_STRING */
    #define PLIST_IS_STRING(__plist)  _PLIST_IS_TYPE(__plist, STRING)
    /** Evaluates to true if the given plist node is of type PLIST_ARRAY */
    #define PLIST_IS_ARRAY(__plist)   _PLIST_IS_TYPE(__plist, ARRAY)
    /** Evaluates to true if the given plist node is of type PLIST_DICT */
    #define PLIST_IS_DICT(__plist)    _PLIST_IS_TYPE(__plist, DICT)
    /** Evaluates to true if the given plist node is of type PLIST_DATE */
    #define PLIST_IS_DATE(__plist)    _PLIST_IS_TYPE(__plist, DATE)
    /** Evaluates to true if the given plist node is of type PLIST_DATA */
    #define PLIST_IS_DATA(__plist)    _PLIST_IS_TYPE(__plist, DATA)
    /** Evaluates to true if the given plist node is of type PLIST_KEY */
    #define PLIST_IS_KEY(__plist)     _PLIST_IS_TYPE(__plist, KEY)
    /** Evaluates to true if the given plist node is of type PLIST_UID */
    #define PLIST_IS_UID(__plist)     _PLIST_IS_TYPE(__plist, UID)
    /* for backwards compatibility */
    #define PLIST_IS_UINT             PLIST_IS_INT

    /**
     * Helper function to check the value of a PLIST_BOOL node.
     *
     * @param boolnode node of type PLIST_BOOL
     * @return 1 if the boolean node has a value of TRUE or 0 if FALSE.
     */
    PLIST_API int plist_bool_val_is_true(plist_t boolnode);

    /**
     * Helper function to test if a given #PLIST_INT node's value is negative
     *
     * @param intnode node of type PLIST_INT
     * @return 1 if the node's value is negative, or 0 if positive.
     */
    PLIST_API int plist_int_val_is_negative(plist_t intnode);

    /**
     * Helper function to compare the value of a PLIST_INT node against
     * a given signed integer value.
     *
     * @param uintnode node of type PLIST_INT
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *         1 if the node's value is greater than cmpval,
     *         or -1 if the node's value is less than cmpval.
     */
    PLIST_API int plist_int_val_compare(plist_t uintnode, int64_t cmpval);

    /**
     * Helper function to compare the value of a PLIST_INT node against
     * a given unsigned integer value.
     *
     * @param uintnode node of type PLIST_INT
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *         1 if the node's value is greater than cmpval,
     *         or -1 if the node's value is less than cmpval.
     */
    PLIST_API int plist_uint_val_compare(plist_t uintnode, uint64_t cmpval);

    /**
     * Helper function to compare the value of a PLIST_UID node against
     * a given value.
     *
     * @param uidnode node of type PLIST_UID
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *         1 if the node's value is greater than cmpval,
     *         or -1 if the node's value is less than cmpval.
     */
    PLIST_API int plist_uid_val_compare(plist_t uidnode, uint64_t cmpval);

    /**
     * Helper function to compare the value of a PLIST_REAL node against
     * a given value.
     *
     * @note WARNING: Comparing floating point values can give inaccurate
     *     results because of the nature of floating point values on computer
     *     systems. While this function is designed to be as accurate as
     *     possible, please don't rely on it too much.
     *
     * @param realnode node of type PLIST_REAL
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are (almost) equal,
     *         1 if the node's value is greater than cmpval,
     *         or -1 if the node's value is less than cmpval.
     */
    PLIST_API int plist_real_val_compare(plist_t realnode, double cmpval);

    /**
     * Helper function to compare the value of a PLIST_DATE node against
     * a given number of seconds since epoch (UNIX timestamp).
     *
     * @param datenode node of type PLIST_DATE
     * @param cmpval Number of seconds to compare against (UNIX timestamp)
     * @return 0 if the node's date is equal to the supplied values,
     *         1 if the node's date is greater than the supplied values,
     *         or -1 if the node's date is less than the supplied values.
     */
    PLIST_API int plist_unix_date_val_compare(plist_t datenode, int64_t cmpval);

    /**
     * Helper function to compare the value of a PLIST_STRING node against
     * a given value.
     * This function basically behaves like strcmp.
     *
     * @param strnode node of type PLIST_STRING
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_string_val_compare(plist_t strnode, const char* cmpval);

    /**
     * Helper function to compare the value of a PLIST_STRING node against
     * a given value, while not comparing more than n characters.
     * This function basically behaves like strncmp.
     *
     * @param strnode node of type PLIST_STRING
     * @param cmpval value to compare against
     * @param n maximum number of characters to compare
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_string_val_compare_with_size(plist_t strnode, const char* cmpval, size_t n);

    /**
     * Helper function to match a given substring in the value of a
     * PLIST_STRING node.
     *
     * @param strnode node of type PLIST_STRING
     * @param substr value to match
     * @return 1 if the node's value contains the given substring,
     *     or 0 if not.
     */
    PLIST_API int plist_string_val_contains(plist_t strnode, const char* substr);

    /**
     * Helper function to compare the value of a PLIST_KEY node against
     * a given value.
     * This function basically behaves like strcmp.
     *
     * @param keynode node of type PLIST_KEY
     * @param cmpval value to compare against
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_key_val_compare(plist_t keynode, const char* cmpval);

    /**
     * Helper function to compare the value of a PLIST_KEY node against
     * a given value, while not comparing more than n characters.
     * This function basically behaves like strncmp.
     *
     * @param keynode node of type PLIST_KEY
     * @param cmpval value to compare against
     * @param n maximum number of characters to compare
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_key_val_compare_with_size(plist_t keynode, const char* cmpval, size_t n);

    /**
     * Helper function to match a given substring in the value of a
     * PLIST_KEY node.
     *
     * @param keynode node of type PLIST_KEY
     * @param substr value to match
     * @return 1 if the node's value contains the given substring,
     *     or 0 if not.
     */
    PLIST_API int plist_key_val_contains(plist_t keynode, const char* substr);

    /**
     * Helper function to compare the data of a PLIST_DATA node against
     * a given blob and size.
     * This function basically behaves like memcmp after making sure the
     * size of the node's data value is equal to the size of cmpval (n),
     * making this a "full match" comparison.
     *
     * @param datanode node of type PLIST_DATA
     * @param cmpval data blob to compare against
     * @param n size of data blob passed in cmpval
     * @return 0 if the node's data blob and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_data_val_compare(plist_t datanode, const uint8_t* cmpval, size_t n);

    /**
     * Helper function to compare the data of a PLIST_DATA node against
     * a given blob and size, while no more than n bytes are compared.
     * This function basically behaves like memcmp after making sure the
     * size of the node's data value is at least n, making this a
     * "starts with" comparison.
     *
     * @param datanode node of type PLIST_DATA
     * @param cmpval data blob to compare against
     * @param n size of data blob passed in cmpval
     * @return 0 if the node's value and cmpval are equal,
     *     > 0 if the node's value is lexicographically greater than cmpval,
     *     or < 0 if the node's value is lexicographically less than cmpval.
     */
    PLIST_API int plist_data_val_compare_with_size(plist_t datanode, const uint8_t* cmpval, size_t n);

    /**
     * Helper function to match a given data blob within the value of a
     * PLIST_DATA node.
     *
     * @param datanode node of type PLIST_KEY
     * @param cmpval data blob to match
     * @param n size of data blob passed in cmpval
     * @return 1 if the node's value contains the given data blob
     *     or 0 if not.
     */
    PLIST_API int plist_data_val_contains(plist_t datanode, const uint8_t* cmpval, size_t n);

    /**
     * Sort all PLIST_DICT key/value pairs in a property list lexicographically
     * by key. Recurses into the child nodes if necessary.
     *
     * @param plist The property list to perform the sorting operation on.
     */
    PLIST_API void plist_sort(plist_t plist);

    /**
     * Free memory allocated by relevant libplist API calls:
     * - plist_to_xml()
     * - plist_to_bin()
     * - plist_get_key_val()
     * - plist_get_string_val()
     * - plist_get_data_val()
     *
     * @param ptr pointer to the memory to free
     *
     * @note Do not use this function to free plist_t nodes, use plist_free()
     *     instead.
     */
    PLIST_API void plist_mem_free(void* ptr);

    /**
     * Set debug level for the format parsers.
     * @note This function does nothing if libplist was not configured with --enable-debug .
     *
     * @param debug Debug level. Currently, only 0 (off) and 1 (enabled) are supported.
     */
    PLIST_API void plist_set_debug(int debug);

    /**
     * Returns a static string of the libplist version.
     *
     * @return The libplist version as static ascii string
     */
    PLIST_API const char* libplist_version();


    /********************************************
     *                                          *
     *              Deprecated API              *
     *                                          *
     ********************************************/

    /**
     * Create a new plist_t type #PLIST_DATE
     *
     * @deprecated Deprecated. Use plist_new_unix_date instead.
     *
     * @param sec the number of seconds since 01/01/2001
     * @param usec the number of microseconds
     * @return the created item
     * @sa #plist_type
     */
    PLIST_WARN_DEPRECATED("use plist_new_unix_date instead")
    PLIST_API plist_t plist_new_date(int32_t sec, int32_t usec);

    /**
     * Get the value of a #PLIST_DATE node.
     * This function does nothing if node is not of type #PLIST_DATE
     *
     * @deprecated Deprecated. Use plist_get_unix_date_val instead.
     *
     * @param node the node
     * @param sec a pointer to an int32_t variable. Represents the number of seconds since 01/01/2001.
     * @param usec a pointer to an int32_t variable. Represents the number of microseconds
     */
    PLIST_WARN_DEPRECATED("use plist_get_unix_date_val instead")
    PLIST_API void plist_get_date_val(plist_t node, int32_t * sec, int32_t * usec);

    /**
     * Set the value of a node.
     * Forces type of node to #PLIST_DATE
     *
     * @deprecated Deprecated. Use plist_set_unix_date_val instead.
     *
     * @param node the node
     * @param sec the number of seconds since 01/01/2001
     * @param usec the number of microseconds
     */
    PLIST_WARN_DEPRECATED("use plist_set_unix_date_val instead")
    PLIST_API void plist_set_date_val(plist_t node, int32_t sec, int32_t usec);

    /**
     * Helper function to compare the value of a PLIST_DATE node against
     * a given set of seconds and fraction of a second since epoch.
     *
     * @deprecated Deprecated. Use plist_unix_date_val_compare instead.
     *
     * @param datenode node of type PLIST_DATE
     * @param cmpsec number of seconds since epoch to compare against
     * @param cmpusec fraction of a second in microseconds to compare against
     * @return 0 if the node's date is equal to the supplied values,
     *         1 if the node's date is greater than the supplied values,
     *         or -1 if the node's date is less than the supplied values.
     */
    PLIST_WARN_DEPRECATED("use plist_unix_date_val_compare instead")
    PLIST_API int plist_date_val_compare(plist_t datenode, int32_t cmpsec, int32_t cmpusec);

    /*@}*/

#ifdef __cplusplus
}
#endif
#endif
