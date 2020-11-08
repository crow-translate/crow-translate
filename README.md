# ![Crow Translate logo](data/icons/freedesktop/128-apps-crow-translate.png) Crow Translate

[![GitHub (pre-)release](https://img.shields.io/github/release/crow-translate/crow-translate/all.svg)](https://github.com/crow-translate/crow-translate/releases)
[![Codacy grade](https://img.shields.io/codacy/grade/b28c6646bb324ffb98092f63a9b5896e.svg)](https://app.codacy.com/project/crow-translate/crow-translate/dashboard)

**Crow Translate** is a simple and lightweight translator programmed in **C++ / Qt** that allows to translate and speak text using Google, Yandex and Bing translate API.
You may also be interested in my library [QOnlineTranslator](https://github.com/crow-translate/QOnlineTranslator "A library for Qt5 that provides free usage of Google, Yandex and Bing translate API.") used in this project.

## Content

- [Screenshots](#screenshots)
- [Features](#features)
- [Default keyboard shortcuts](#default-keyboard-shortcuts)
- [CLI commands](#cli-commands)
- [D-Bus API](#d-bus-api)
- [Dependencies](#dependencies)
- [Icons](#icons)
- [Installation](#installation)
- [Building](#building)
- [Localization](#localization)

## Screenshots

### Linux KDE

<p align="center">
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/main.png" alt="Main screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/popup.png" width="256px" height="108px" alt="Popup screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/sentense.png" width="256px" height="108px" alt="Sentence screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/linux-plasma/settings.png" width="256px" height="108px" alt="Settings screenshot"/>
</p>
<p align="center">
  <sub><b>Screenshots</b>: OS: <a href="https://www.archlinux.org">Arch Linux</a> | Desktop environment: <a href="https://www.kde.org/plasma-desktop">Plasma</a> | Theme: Breeze | Icons: <a href="https://github.com/cbrnix/Newaita">Newaita</a> | Wallpaper: <a href="https://www.hdwallpapers.in/macaw_lowpoly_art_4k-wallpapers.html">Macaw</a></sub>
</p>

### Windows 10

<p align="center">
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/main.png" alt="Main screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/popup.png" width="256px" height="108px" alt="Popup screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/sentense.png" width="256px" height="108px" alt="Sentence screenshot"/>
  <img src="https://raw.githubusercontent.com/crow-translate/crow-translate.github.io/master/img/screenshots/windows/settings.png" width="256px" height="108px" alt="Settings screenshot"/>
</p>
<p align="center">
  <sub><b>Screenshots</b>: OS: <a href="https://www.microsoft.com/en-us/software-download/windows10">Windows 10</a> | Theme: default | Wallpaper: <a href="https://wallpaperscraft.com/wallpaper/needles_spruce_branch_blur_114943">WallpapersCraft</a></sub>
</p>

## Features

- Translate and speak text from screen or selection
- Support 125 different languages
- Low memory consumption (~20MB)
- Highly customizable shortcuts
- Command-line interface with rich options
- D-Bus API
- Available for Linux and Windows

## Default keyboard shortcuts

You can change them in the settings. Some key sequences may not be available due to OS limitations.

Wayland does not support global shortcuts registration, but you can use [D-Bus](#d-bus-api) to bind actions in the system settings. For desktop environments that support [additional applications actions](https://specifications.freedesktop.org/desktop-entry-spec/desktop-entry-spec-latest.html#extra-actions) (KDE, for example) you will see them predefined in the system shortcut settings. You can also use them for X11 sessions, but you need to disable global shortcuts registration in the application settings to avoid conflicts.

### Global

| Key                                             | Description                        |
| ----------------------------------------------- | ---------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>E</kbd> | Translate selected text            |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>S</kbd> | Speak selected text                |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>F</kbd> | Speak translation of selected text |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>G</kbd> | Stop speaking                      |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>C</kbd> | Show main window                   |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>O</kbd> | Translate text in screen area      |

### In main window

| Key                                               | Description                             |
| ------------------------------------------------- | --------------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Return</kbd>               | Translate                               |
| <kbd>Ctrl</kbd> + <kbd>R</kbd>                    | Swap languages                          |
| <kbd>Ctrl</kbd> + <kbd>Q</kbd>                    | Close window                            |
| <kbd>Ctrl</kbd> + <kbd>S</kbd>                    | Speak source / pause text speaking      |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>S</kbd> | Speak translation / pause text speaking |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>C</kbd> | Copy translation to clipboard           |

## CLI commands

The program also has a console interface.

**Usage:** `crow [options] text`

| Option                     | Description                                                                                    |
| -------------------------- | ---------------------------------------------------------------------------------------------- |
| `-h, --help`               | Display help                                                                                   |
| `-v, --version`            | Display version information                                                                    |
| `-c, --codes`              | Display language codes                                                                         |
| `-s, --source <code>`      | Specify the source language (by default, engine will try to determine the language on its own) |
| `-t, --translation <code>` | Specify the translation language(s), splitted by '+' (by default, the system language is used) |
| `-l, --locale <code>`      | Specify the translator language (by default, the system language is used)                      |
| `-e, --engine <engine>`    | Specify the translator engine ('google', 'yandex' or 'bing'), Google is used by default        |
| `-p, --speak-translation`  | Speak the translation                                                                          |
| `-u, --speak-source`       | Speak the source                                                                               |
| `-f, --file`               | Read source text from files. Arguments will be interpreted as file paths                       |
| `-i, --stdin`              | Add stdin data to source text                                                                  |
| `-a, --audio-only`         | Print text only for speaking when using `--speak-translation` or `--speak-source`              |
| `-b, --brief`              | Print only translations                                                                        |

**Note:** If you do not pass startup arguments to the program, the GUI starts.

## D-Bus API

    io.crow_translate.CrowTranslate
    └── /io/crow_translate/CrowTranslate/MainWindow
        |   # Global shortcuts
        ├── method void io.crow_translate.CrowTranslate.MainWindow.translateSelection();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.speakSelection();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.speakTranslatedSelection();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.stopSpeaking();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.open();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.copyTranslatedSelection();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.translateScreenArea();
        |   # Main window shortcuts
        ├── method void io.crow_translate.CrowTranslate.MainWindow.clearText();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.cancelOperation();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.swapLanguages();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.openSettings();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.setAutoTranslateEnabled(bool enabled);
        ├── method void io.crow_translate.CrowTranslate.MainWindow.copySourceText();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.copyTranslation();
        ├── method void io.crow_translate.CrowTranslate.MainWindow.copyAllTranslationInfo();
        └── method void io.crow_translate.CrowTranslate.MainWindow.quit();

For example, you can show main window using `dbus-send`:

```bash
dbus-send --type=method_call --dest=io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow io.crow_translate.CrowTranslate.MainWindow.open
```

Or via `qdbus`:

```bash
qdbus io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow io.crow_translate.CrowTranslate.MainWindow.open
# or shorter
qdbus io.crow_translate.CrowTranslate /io/crow_translate/CrowTranslate/MainWindow open
```

## Dependencies

### Required

- [CMake](https://cmake.org) 3.14+
- [Extra CMake Modules](https://github.com/KDE/extra-cmake-modules)
- [Qt](https://www.qt.io) 5.9+ with Widgets, Network, Multimedia, Concurrent, X11Extras (Linux), DBus (Linux) and WinExtras (Windows) modules
- [Tesseract](https://tesseract-ocr.github.io) 4.0+
- [Png2Ico](https://www.winterdrache.de/freeware/png2ico) or [IcoTool](https://www.nongnu.org/icoutils) for generating executable icon (Windows)

### External libraries

This project uses the following external libraries, which will be downloaded automatically at configure time using the built-in CMake [FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html) module:

- [QOnlineTranslator](https://github.com/crow-translate/QOnlineTranslator) - provides free usage of Google, Yandex and Bing translate API.
- [QGitTag](https://github.com/crow-translate/QGitTag) - uses the GitHub API to provide information about releases.
- [QHotkey](https://github.com/Skycoder42/QHotkey) - provides global shortcuts for desktop platforms.
- [QTaskbarControl](https://github.com/Skycoder42/QTaskbarControl) - allows to create a taskbar/launcher progress for all desktop platforms.
- [SingleApplication](https://github.com/itay-grudev/SingleApplication) - prevents launch of multiple application instances.

## Icons

Only Linux supports icon theming. For Windows [We10X](https://github.com/yeyushengfan258/We10X-icon-theme) icon theme is bundled.

[FlagKit](https://github.com/madebybowtie/FlagKit "Beautiful flag icons for usage in apps and on the web") icons are used for flags.

## Installation

Downloads are available on the [Releases](https://github.com/crow-translate/crow-translate/releases/latest) page. Also check out the [website](https://crow-translate.github.io/#installation) for installation from package.

## Building

### Building executable

You can build **Crow Translate** by using the following commands:

```bash
mkdir build
cd build
cmake .. # Or `cmake -D CMAKE_BUILD_TYPE=Release ..` for single-configuration generators such as Ninja or GNU Make
cmake --build . # Or `cmake --build . --config Release` for multi-config generators such as Visual Studio Generators or Xcode
```

You will then get a binary named `crow`.

### Building a package using CPack

CMake allows to create [specified package types](https://cmake.org/cmake/help/latest/manual/cpack-generators.7.html) automatically.

If you use Makefile, Ninja, or Xcode generator you can use [package](https://cmake.org/cmake/help/latest/module/CPack.html#targets-package-and-package-source) target:

```bash
mkdir build
cd build
cmake -D CMAKE_BUILD_TYPE=Release -D CPACK_GENERATOR=DEB .. # You can specify several types of packages separated by semicolons in double quotes, for example: `CPACK_GENERATOR="DEB;ZIP;NSIS"`
cmake --build . --target package
```

Or you can use [CPack](https://cmake.org/cmake/help/latest/module/CPack.html) utility for any generators:

```bash
mkdir build
cd build
cmake .. # Or `cmake -D CMAKE_BUILD_TYPE=Release ..` for single-configuration generators such as Ninja or GNU Make
cpack -G DEB # Or `cpack -G DEB -C Release` for multi-config generators such as Visual Studio Generators or Xcode
```

On Windows you need [VCPKG](https://github.com/microsoft/vcpkg) to bundle all necessary DLLs.

### Build parameters

- `WITH_PORTABLE_MODE` - Enable portable functionality. If you create file named `settings.ini` in the app folder and Crow will store the configuration in it. It also adds the “Portable Mode” option to the application settings, which does the same.

Build parameters are passed at configuration stage: `cmake -D WITH_PORTABLE_MODE ..`.

## Localization

To help with localization you can use [Crowdin](https://crowdin.com/project/crow-translate) or translate files in `data/translations` with [Qt Linguist](https://doc.qt.io/Qt-5/linguist-translators.html) directly. To add a new language, write me on the Crowdin page or copy `data/translations/crow-translate.ts` to `data/translations/crow-translate_<ISO 639-1 language code>_<ISO 3166-1 alpha-2 language code>.ts`, translate it and send a pull request.
