#!/usr/bin/env python3
"""Bundle the verified generic DDI used in the iPhone19,2 mounting test."""
import argparse
import hashlib
import plistlib
import shutil
import tempfile
import urllib.request
from pathlib import Path

REVISION = "59a90f1e3f1e7190b9a429c768568b83c321582e"
BASE = f"https://raw.githubusercontent.com/doronz88/DeveloperDiskImage/{REVISION}/PersonalizedImages/Xcode_iOS_DDI_Cryptex"
PAYLOADS = {
    "Cryptex1,GenericDmg": "Image.dmg",
    "Cryptex1,GenericTrustCache": "Image.dmg.trustcache",
    "Cryptex1,CryptexInfoPlist": "Image.dmg.cryptex_info",
    "Cryptex1,GenericVolume": "Image.dmg.root_hash",
}


def prepare(output: Path):
    if output.exists():
        raise ValueError(f"Output already exists: {output}")
    output.parent.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix="cryptex-", dir=output.parent))
    try:
        def download(name):
            with urllib.request.urlopen(f"{BASE}/{name}", timeout=120) as response:
                data = response.read()
            (staging / name).write_bytes(data)
            return data

        manifest = plistlib.loads(download("BuildManifest.plist"))
        identity = next(i for i in manifest["BuildIdentities"]
                        if i.get("Info", {}).get("Variant", "").endswith("Developer Disk Image Cryptex"))
        for key, name in PAYLOADS.items():
            entry = identity["Manifest"][key]
            if entry["Info"]["Path"] != name:
                raise ValueError(f"Unexpected Cryptex path for {key}")
            data = download(name)
            if not data or hashlib.sha384(data).digest() != entry["Digest"]:
                raise ValueError(f"Cryptex payload failed verification: {name}")
        staging.rename(output)
        print(f"Verified Cryptex DDI {manifest['ProductBuildVersion']} from {REVISION}")
    finally:
        if staging.exists():
            shutil.rmtree(staging)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    prepare(parser.parse_args().output)
