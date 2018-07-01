# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](http://keepachangelog.com/en/1.0.0/)
and this project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased]
*No unreleased changes*

## [1.0.0] - 2018-07-01
### Added
- Word gender support for translation options.
- Definition support.
- Support for SOCKS5 proxies.
- Ability to set a custom system tray icon.
- Auto-update for Windows.
- Flag icons for languages.
- Display of network error in the pop-up window.
- Options to select buttons style.
- Ability to set pop-up window size.
- A global shortcut to stop playing the selected text.
- A button to copy translation to the source field.
- Minor visual design improvements.

### Changed
- Redesign application icon.
- Fix the incorrect behavior of adding language button.

## [0.9.9] - 2018-06-04
### Added
- Ability to stop and pause the text.
- Menu item "Translation" in the settings.
- Splitting queries to parts if they exceed the limit.
- Ability to disable transliteration and translation options.

### Changed
- Redesign the console interface.
- Rework the shortcuts menu.
- Rewrite list of languages in accordance with the official Google documentation.
- Fix wrong interpretation of some special characters.
- Fix crash when there is no internet connection.
- Fix wrong button sizes.
- Fix the re-translation when translating by the keyboard shortcut.
- Fix disabling of the keyboard shortcut for translating selected text after translation in the main window.
- Fix wrong behaviour for "Automatically translate" checkbox.

## [0.9.8] - 2018-05-19
### Added
- Proxy settings.
- Button and shortcut for copying only translation.
- The preferred languages feature.
- More buttons in the pop-up window.
- Saving of the main window size state.
- Major performance optimizations.
- Icons for the tray context menu for Windows.

### Changed
- Improve icon look for Windows.
- Change icon names for better compatibility for Linux.
- Redesign interface of the settings menu.
- Fix pop-up window appearing outside the screen.
- Fix reactivation for the translation hotkey.
- Fix the bug when an empty field was displayed instead of translation when the hotkey was pressed.
- Fix wrong interpretation of some symbols.
- Fix automatic translation in the main window after inserting a new language.

### Removed
- "Automatic detection" from the context menu of languages ​​as useless.

## [0.9.7] - 2018-04-23
### Added
- The selection of the previous language if the source language and the target language are the same.
- Displaying detected languages on the language auto-detection buttons.
- "Automatically translate" checkbox.
- Tooltips for buttons.
- Icon for the "Show Window" action in the system tray for Linux.
- Perfomance improvements.

### Changed
- Redesigned settings appearance.
- Fix the crash when changing the application language.
- Fix memory leak with system tray menu.
- Fix incorrect formatting of the translation text.

## [0.9.6] - 2018-04-04
### Added
- Icons to tray context menu for Linux.
- Close window shortcut.
- Minor optimizations.

### Changed
- New default Windows compatible keyboard shortcuts.
- Fix memory leak after closing settings.
- Fix language detection when saying selected text.
- Fix the obtaining of selected text on Windows.

## [0.9.5] - 2018-03-25
### Added
- Display options for translation and transcriptions with formatting.
- Minor optimizations.

### Changed
- Improve translation parsing.
- Improve network error output.
- Rework behavior of the button "Reset settings".
- Now list of languages is sorted alphabetically.
- Fix bug on Windows, when Cyrillic characters are reproduced as "?".

## [0.9.5] - 2018-03-14
*Initial release*
