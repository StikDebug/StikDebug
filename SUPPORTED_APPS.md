# Officially Supported Apps

These apps are officially supported by the StikDebug team. To submit your app, get in touch in the [idevice Discord server](https://discord.gg/qtgv6QtYbV), [open a GitHub issue](https://github.com/StikDebug/StikDebug/issues/new), or [submit a pull request](https://github.com/StikDebug/StikDebug/pulls). We can review your project for inclusion and offer support to your users.

We review submissions on a case-by-case basis and generally decline projects without an established user base, low-quality projects that are substantially AI-generated, and forks of apps we already support.

Apps not listed here may still work with StikDebug. Inclusion in this list means official support from the StikDebug team.

## StikDebug Compatibility

The following apps are supported by StikDebug and have built-in script support on iOS/iPadOS 26 and later:

If you run an app that requires StikDebug inside LiveContainer, enable **Run with JIT** in that app’s settings and assign the script indicated below, then select **StikDebug** as the **JIT Enabler** in LiveContainer’s global settings.

To download a script, select its link below, then select **Download raw file** on GitHub.

| App | Script |
| --- | --- |
| [Amethyst](https://github.com/AngelAuraMC/Amethyst-iOS) | [universal.js](StikDebug/Scripts/universal.js) |
| [Applesauce](https://github.com/0xjohnnydev/Applesauce) | [universal.js](StikDebug/Scripts/universal.js) |
| [ARMSX2 iOS](https://github.com/ARMSX2/ARMSX2) | [legacy.js](StikDebug/Scripts/legacy.js) |
| [DolphiniOS](https://github.com/OatmealDome/dolphin-ios) | [legacy.js](StikDebug/Scripts/legacy.js) |
| [DukeX](https://github.com/MaftyManicEMU/DukeX) | [universal.js](StikDebug/Scripts/universal.js) |
| [Flycast](https://github.com/chachillie/Flycast-iOS) | [legacy.js](StikDebug/Scripts/legacy.js) |
| [Geode](https://github.com/geode-sdk/ios-launcher) | [Geode.js](StikDebug/Scripts/Geode.js) |
| [Henge](https://github.com/Leviidev/Henge) | [universal.js](StikDebug/Scripts/universal.js) |
| [Husk](https://github.com/Leviidev/Husk) | [universal.js](StikDebug/Scripts/universal.js) |
| [Manic EMU](https://github.com/Manic-EMU/ManicEMU) | [universal.js](StikDebug/Scripts/universal.js) |
| [MeloCafé](https://github.com/stossy11/MeloCafe) | [universal.js](StikDebug/Scripts/universal.js) |
| [MeloNX](https://starforgejo.dev/projects/MeloNX) | [universal.js](StikDebug/Scripts/universal.js) |
| [RPCS3](https://github.com/XITRIX/RPCS3-iOS-Releases) | [universal.js](StikDebug/Scripts/universal.js) |
| [UTM](https://github.com/utmapp/UTM) | [legacy.js](StikDebug/Scripts/legacy.js) |
| [Utsuro](https://github.com/Leviidev/Utsuro) | [universal.js](StikDebug/Scripts/universal.js) |
| [XeniOS](https://github.com/xenios-jp/XeniOS) | [universal.js](StikDebug/Scripts/universal.js) |

## Built-in JIT

These apps can enable JIT from within the app. [StikJIT](https://github.com/StikDebug/StikJIT) lets developers integrate this functionality into their projects.

To use built-in JIT in these apps inside LiveContainer, use LiveContainer nightly and enable **Helper Process Support** in the app’s per-app settings. Compatibility may vary by app and version.

In-app pairing requires iOS/iPadOS 27 or later. Apps without it require an imported pairing file.

| App | Supports In-App Pairing |
| --- | --- |
| [DolphiniOS (CelloSerenity’s fork)](https://github.com/CelloSerenity/dolphin-ios/releases) | No |
| [Henge](https://github.com/Leviidev/Henge) | Yes |
| [Husk](https://github.com/Leviidev/Husk) | Yes |
| [Manic EMU](https://github.com/Manic-EMU/ManicEMU) | No |
| [tctiSH](https://github.com/tctiSH/tctiSH) | Yes |
| [Utsuro](https://github.com/Leviidev/Utsuro) | Yes |
