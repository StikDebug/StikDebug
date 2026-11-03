# Pairing Your Device

StikDebug requires a specialized **pairing file** to communicate with your device remotely. This file is specific to your device and is required to use StikDebug. Without it, StikDebug will not work.

> [!WARNING]
> Because of iOS/iPadOS limitations, resetting or changing your device's passcode will invalidate its pairing file. You will need to generate and place a new one.
>
> Keep your pairing file private. It contains connection credentials for your device. Do not share it publicly or attach it to bug reports.

## Method 1: On-device

On iOS/iPadOS 27 or later, StikDebug can generate and save its own pairing file without a computer.

Open **Settings → Pair a Device → Pair iPhone or iPad**, then follow the in-app instructions. Once pairing finishes, StikDebug saves the file automatically.

**Pairing another device:** You can also pair another iPhone or iPad running iOS/iPadOS 27 or later, or an Apple TV running tvOS 14 or later. After pairing, export its file or place it directly in a supported app on that device. For an Apple TV, choose **Pair Apple TV** instead. Apple TV pairing is not yet supported inside LiveContainer.

## Method 2: Computer

This method uses [iloader](https://iloader.app) on a computer to generate and place the pairing file. You will need a computer and a USB cable.

### Prerequisites

#### Mac

1. Download and install [iloader for Mac](https://github.com/nab138/iloader/releases/latest/download/iloader-darwin-universal.dmg).

#### Windows

1. Download and install [iTunes](https://apple.co/ms).
2. Download the iloader installer: [MSI (recommended)](https://github.com/nab138/iloader/releases/latest/download/iloader-windows-x64.msi) or [EXE](https://github.com/nab138/iloader/releases/latest/download/iloader-windows-x64.exe).
3. Run the installer.

#### Linux

1. Install `usbmuxd` using your package manager if it is not already installed.
2. Download and install iloader for your distribution: [Debian/Ubuntu](https://github.com/nab138/iloader/releases/latest/download/iloader-linux-amd64.deb), [Fedora/openSUSE](https://github.com/nab138/iloader/releases/latest/download/iloader-linux-x86_64.rpm), or [AppImage for other distributions](https://github.com/nab138/iloader/releases/latest/download/iloader-linux-amd64.AppImage).

### Generating and placing the pairing file

1. For best results, connect your device to your computer using a USB cable. If your device appears wirelessly in iloader, that should work too.
2. Click **Manage Pairing File**.
3. Click **Place** beside **StikDebug** and any other apps you want to pair. A green **"Pairing file placed successfully!"** message should appear.
