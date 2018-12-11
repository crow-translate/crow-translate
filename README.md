# ![Crow Translate logo](./dist/unix/generic/hicolor/72x72/apps/crow-translate.png) Crow Translate

[![GitHub (pre-)release](https://img.shields.io/github/release/Shatur95/CrowTranslate/all.svg)](https://github.com/Shatur95/CrowTranslate/releases)
[![Codacy grade](https://img.shields.io/codacy/grade/b28c6646bb324ffb98092f63a9b5896e.svg)](https://app.codacy.com/project/Shatur95/crow-translate/dashboard)

**Crow Translate** is a simple and lightweight translator programmed in **C++ / Qt** that allows to translate and speak text using Google, Yandex and Bing translate API.
You may also be interested in my library [QOnlineTranslator](https://github.com/Shatur95/QOnlineTranslator "A library for Qt5 that provides free usage of Google, Yandex and Bing translate API.") used in this project. 

## Content

-   [Screenshots](#screenshots)
-   [Features](#features)
-   [Default keyboard shortcuts](#default-keyboard-shortcuts)
-   [CLI commands](#cli-commands)
-   [Dependencies](#dependencies)
-   [Third-party](#third-party)
-   [Installation](#installation)
-   [Localization](#localization)

## Screenshots

### Linux

<p align="center">
  <img src="https://github.com/crow-translate/crow-translate.github.io/raw/master/static/img/screenshots/linux-plasma/main.png" alt="Main screenshot"/>
  <img src="https://github.com/crow-translate/crow-translate.github.io/raw/master/static/img/screenshots/linux-plasma/popup.png" width="250px" height="140px" alt="Popup screenshot"/>
  <img src="https://github.com/crow-translate/crow-translate.github.io/raw/master/static/img/screenshots/linux-plasma/sentense.png" width="250px" height="140px" alt="Sentense screenshot"/>
  <img src="https://github.com/crow-translate/crow-translate.github.io/raw/master/static/img/screenshots/linux-plasma/settings.png" width="250px" height="140px" alt="Settings screenshot"/>
</p>
<p align="center">
  <sub><b>Screenshots</b>: OS: <a href="https://www.archlinux.org">Arch Linux</a> | Desktop environment: <a href="https://www.kde.org/plasma-desktop">Plasma</a> | Theme: Breeze | Icons: <a href="https://github.com/PapirusDevelopmentTeam/papirus-icon-theme">Papirus</a> | Dock: <a href="https://github.com/psifidotos/Latte-Dock">Latte Dock</a> | Wallpaper: <a href="https://alpha.wallhaven.cc/wallpaper/288991">Wallhaven</a></sub>
</p>

### Windows

<p align="center">
  <img src="https://github.com/crow-translate/crow-translate.github.io/raw/master/static/img/screenshots/windows/main.png" alt="Main screenshot"/>
  <img src="https://github.com/crow-translate/crow-translate.github.io/raw/master/static/img/screenshots/windows/popup.png" width="250px" height="140px" alt="Popup screenshot"/>
  <img src="https://github.com/crow-translate/crow-translate.github.io/raw/master/static/img/screenshots/windows/sentense.png" width="250px" height="140px" alt="Sentense screenshot"/>
  <img src="https://github.com/crow-translate/crow-translate.github.io/raw/master/static/img/screenshots/windows/settings.png" width="250px" height="140px" alt="Settings screenshot"/>
</p>
<p align="center">
  <sub><b>Screenshots</b>: OS: <a href="https://www.microsoft.com/en-us/software-download/windows10">Windows 10</a> | Theme: <a href="https://www.deviantart.com/sagorpirbd/art/Windows-8-Themes-for-Win10-Final-556761647">Windows 8</a> | Wallpaper: Default Windows 10 flower</sub>
</p>


## Features

-   Translate and speak text in any application that supports text selection
-   Support 117 different languages
-   Low memory consumption (~20MB)
-   Highly customizable shortcuts
-   Command-line interface with rich options
-   Available for Linux and Windows

## Default keyboard shortcuts

You can change them in the settings. Some key sequences may not be available due to OS limitations.

### Global

| Key                                             | Description             |
| ----------------------------------------------- | ----------------------- |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>E</kbd> | Translate selected text |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>S</kbd> | Speak selected text     |
| <kbd>Ctrl</kbd> + <kbd>Alt</kbd> + <kbd>C</kbd> | Show main window        |

### In main window

| Key                                               | Description                       |
| ------------------------------------------------- | --------------------------------- |
| <kbd>Ctrl</kbd> + <kbd>Return</kbd>               | Translate                         |
| <kbd>Ctrl</kbd> + <kbd>Q</kbd>                    | Close window                      |
| <kbd>Ctrl</kbd> + <kbd>S</kbd>                    | Play / pause source text speaking |
| <kbd>Ctrl</kbd> + <kbd>G</kbd>                    | Stop source text speaking         |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>S</kbd> | Play / pause translation speaking |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>D</kbd> | Stop translation speaking         |
| <kbd>Ctrl</kbd> + <kbd>Shift</kbd> + <kbd>C</kbd> | Copy translation to clipboard     |

## CLI commands

The program also has a console interface.

**Usage:** `crow [options] text`

| Option                     | Description                                                                                    |
| -------------------------- | ---------------------------------------------------------------------------------------------- |
| `-h, --help`               | Display help                                                                                   |
| `-v, --version`            | Display version information                                                                    |
| `-s, --source <code>`      | Specify the source language (by default, engine will try to determine the language on its own) |
| `-t, --translation <code>` | Specify the translation language(s), joined by '+' (by default, the system language is used)   |
| `-l, --locale <code>`      | Specify the translator language (by default, the system language is used)                      |
| `-e, --engine <engine>`    | Specify the translator engine ('google', 'yandex' or 'bing'), Google is used by default        |
| `-p, --speak-translation`  | Speak the translation                                                                          |
| `-u, --speak-source`       | Speak the source                                                                               |
| `-a, --audio-only`         | Print text only for speaking when using `--speak-translation` or `--speak-source`.             |
| `-f, --file`               | Read source text from files. Arguments will be interpreted as file paths.                      |
| `-i, --stdin`              | Add stdin data to source text.                                                                 |

**Note:** If you do not pass startup arguments to the program, the GUI starts.

## Dependencies

**Arch Linux:** qt5-base qt5-multimedia qt5-x11extras gst-plugins-good openssl

**Debian:** gstreamer1.0-fluendo-mp3, qtgstreamer-plugins-qt5, gstreamer1.0-plugins-good, gstreamer1.0-alsa, gstreamer1.0-pulseaudio, libqt5multimedia5-plugins

## Third-party

### Libraries

This project uses the following third-party libraries:

-   [QOnlineTranslator](https://github.com/Shatur95/QOnlineTranslator) - my library that provides free usage of Google, Yandex and Bing translate API for Qt5.
-   [QHotkey](https://github.com/Skycoder42/QHotkey) - A global shortcut/hotkey for Desktop Qt-Applications.
-   [SingleApplication](https://github.com/itay-grudev/SingleApplication) - A simple single instance application for Qt.

Therefore, if you want to clone this project, you need to use the `--recursive` option: 

```bash
git clone --recursive git@github.com:Shatur95/CrowTranslate.git
```

or you can initialize these modules later:

```bash
git clone git@github.com:Shatur95/CrowTranslate.git
git submodule init
git submodule update
```

### Icons

Only Linux supports icon theming. Windows use [Papirus](https://github.com/PapirusDevelopmentTeam/papirus-icon-theme "Free and open source SVG icon theme") icons.

[FlagKit](https://github.com/madebybowtie/FlagKit "Beautiful flag icons for usage in apps and on the web") icons are used for flags.

## Installation

To install a stable version, go to the [releases](https://github.com/Shatur95/CrowTranslate/releases) page. The instructions below will help you install Crow Translate with the latest commits.

### Automatic script

You can use the automatic script that builds **Crow Translate** and creates a package for your distribution:

```bash
cd dist/unix
./create-package.sh
```

Than you can install it as usual. The script will tell you where the package will be after the making. Currently, only **Arch Linux**, **Debian** and their derivatives are supported.

### Arch Linux and derivatives

You can install [crow-translate-git](https://aur.archlinux.org/packages/crow-translate-git "A simple and lightweight translator that allows to translate and speak text using the Google, Yandex and Bing translate API") from AUR.

### Manual building

You can build **Crow Translate** by using the following commands:

```bash
qmake
make
make clean
```

You will then get a binary named `crow`.

## Localization

To help with localization you can use [Transefix](https://www.transifex.com/crow-translate/crow-translate) or [Qt Linguist](https://doc.qt.io/Qt-5/linguist-translators.html) directly (translation files are located in `data/translations` repository folder).
