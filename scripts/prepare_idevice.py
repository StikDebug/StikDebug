#!/usr/bin/env python3
"""Install the official idevice iOS library from a checksum-pinned release."""
import argparse
import hashlib
import io
import urllib.request
import zipfile
from pathlib import Path

VERSION = "0.1.68"
SHA256 = "c9eccdd1942de756d746a2569d93302774ab0eff1cff314ec8118f768b3d8dc7"
URL = f"https://github.com/jkcoxson/idevice/releases/download/v{VERSION}/idevice-xcframework-v{VERSION}.zip"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, help="Use an already downloaded release archive")
    args = parser.parse_args()
    if args.archive:
        data = args.archive.read_bytes()
    else:
        with urllib.request.urlopen(URL, timeout=120) as response:
            data = response.read()
    if hashlib.sha256(data).hexdigest() != SHA256:
        raise SystemExit("idevice archive checksum mismatch")
    destination = Path(__file__).resolve().parents[1] / "StikDebug/idevice"
    with zipfile.ZipFile(io.BytesIO(data)) as archive:
        root = "swift/IDevice.xcframework/ios-arm64/"
        for member, name in [("libidevice_ffi.a", "libidevice_ffi.a"), ("Headers/idevice.h", "idevice.h")]:
            target = destination / name
            temporary = target.with_suffix(target.suffix + ".tmp")
            temporary.write_bytes(archive.read(root + member))
            temporary.replace(target)
    print(f"Prepared checksum-verified idevice {VERSION} for iOS arm64")


if __name__ == "__main__":
    main()
