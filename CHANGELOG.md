# Changelog

All notable changes to this project will be documented in this file. This project adheres to [Semantic Versioning](http://semver.org/spec/v2.0.0.html).

## [Unreleased](https://github.com/crow-translate/crow-translate/tree/HEAD)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/2.2.3...HEAD)

**Added**

-   The ability to disable embedded global shortcuts.
-   D-Bus API

**Changed**

-   Fix crash on Wayland (still no support for hotkeys and selection).
-   Use a simpler and more compatible way to show notifications.

## [2.2.3](https://github.com/crow-translate/crow-translate/tree/2.2.3) (2019-11-04)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/2.2.2...2.2.3)

**Added**

-   Polish localization.
-   French localization.
-   Portable mode.

**Changed**

-   Fix wrong names in errors.
-   Fix email address link in the About section.
-   Use Brazil flag instead of Portugal.
-   Windows: Save application installer to disk by blocks.

## [2.2.2](https://github.com/crow-translate/crow-translate/tree/2.2.2) (2019-10-11)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/2.2.1...2.2.2)

**Added**

-   Clear button.

**Changed**

-   Fix voice and emotion settings.

## [2.2.1](https://github.com/crow-translate/crow-translate/tree/2.2.1) (2019-09-11)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/2.2.0...2.2.1)

**Changed**

-   Focus input field when showing the main window.
-   Disable player buttons if no text to speak.
-   Always speak translation on its language.

## [2.2.0](https://github.com/crow-translate/crow-translate/tree/2.2.0) (2019-08-31)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/2.1.0...2.2.0)

**Added**

-   Abort button.
-   Simplified Chinese localization.
-   Text speaking indication in the taskbar.

**Changed**

-   Cancel previous request automatically if a new one was requested.
-   Remove 300ms delay for automatic translation when changing source text.
-   Sort languages in comboboxes alphabetically.
-   Fix date check for leap years.
-   Rework CLI interface.
-   Use source / translation fields to control selection speaking.
-   Use one shortcut to stop all text playback.
-   Fix double translation if the translation button is pressed faster.
-   Minor performance improvements.
-   Windows: Update Papirus icon theme.

## [2.1.0](https://github.com/crow-translate/crow-translate/tree/2.1.0) (2018-12-08)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/2.0.1...2.1.0)

**Added**

-   Bing support.

**Changed**

-   Select the first supported engine if the selected language is not supported by the current one.
-   Do not request / analyze parts of the translation that are not displayed.
-   Show a message if the application is already running.
-   Fix main window activation on some window systems.
-   Windows: Update changelog parser.

## [2.0.1](https://github.com/crow-translate/crow-translate/tree/2.0.1) (2018-11-26)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/2.0.0...2.0.1)

**Changed**

-   Make pop-up window active explicity for some window systems.
-   Use different channels to display messages.
-   Fix selected language on search.
-   Fix size of auto language buttons in pop-up window.
-   Fix pop-up window engine selection.
-   Google: Fix parsing for text with non-breaking space.
-   Linux: Create autorun file from desktop file.
-   Windows: Load Qt library translations.
-   Windows: Improve getting selection.

## [2.0.0](https://github.com/crow-translate/crow-translate/tree/2.0.0) (2018-11-17)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/1.0.3...2.0.0)

**Added**

-   Yandex support.
-   Speech synthesis settings.
-   New convenient languages menu.
-   An option for CLI to read source text from file.
-   An option for CLI to read stdin.
-   Placeholders.
-   Hotkey to copy translated selection.
-   Linux: autorun file creation error message.

**Changed**

-   Fix display of languages in the terminal.
-   Show message if not specified `--speak-source` or `--speak-translation` options in `--audio-only` mode.
-   Change the CLI option "translator" to "locale".
-   Move the functionality of the "Copy to source" button to the "Swap languages" button.
-   Select first row in settings by default.
-   Change tab order in main window.
-   Minor optimizations.
-   Windows: Improve updater.

## [1.0.3](https://github.com/crow-translate/crow-translate/tree/1.0.3) (2018-08-05)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/1.0.2...1.0.3)

**Added**

-   A button to remove shortcuts in settings.

**Changed**

-   Improve appearance of shortcuts settings.
-   Close window shortcut now works in pop-up window.
-   Slightly increase settings window height.
-   Prevent crashes when converting for non-existent languages or codes.
-   Remove languages from "Auto" buttons ​​on error.

## [1.0.2](https://github.com/crow-translate/crow-translate/tree/1.0.2) (2018-07-04)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/1.0.1...1.0.2)

**Added**

-   A global shortcut to play translation of seleted text.
-   Minor optimizations.

**Changed**

-   Clear the translation field if the source field is empty.
-   Fix wrong proxy combobox behavior.
-   Windows: fix missing tray icon.
-   Separate source and selection players.
-   Windows: show the application name instead of the description in the list of running processes.

## [1.0.1](https://github.com/crow-translate/crow-translate/tree/1.0.1) (2018-07-02)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/1.0.0...1.0.1)

**Added**

-   Button style settings for pop-up window.

**Changed**

-   Improve the prevention of a pop-up window outside the screen.
-   Fix wrong layout properties in settings.
-   Fix the inactive translation button after a network error.
-   Windows: Fix new changelog parsing for Updater.

## [1.0.0](https://github.com/crow-translate/crow-translate/tree/1.0.0) (2018-07-01)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/0.9.9...1.0.0)

**Added**

-   Word gender support for translation options.
-   Definition support.
-   Support for SOCKS5 proxies.
-   Ability to set a custom system tray icon.
-   Flag icons for languages.
-   Display of network error in the pop-up window.
-   Options to select buttons style.
-   Ability to set pop-up window size.
-   A global shortcut to stop playing the selected text.
-   A button to copy translation to the source field.
-   Minor visual design improvements.
-   Windows: Auto-update.

**Changed**

-   Redesign application icon.
-   Fix the incorrect behavior of adding language button.

## [0.9.9](https://github.com/crow-translate/crow-translate/tree/0.9.9) (2018-06-04)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/0.9.8...0.9.9)

**Added**

-   Ability to stop and pause the text.
-   Menu item "Translation" in the settings.
-   Splitting queries to parts if they exceed the limit.
-   Ability to disable transliteration and translation options.

**Changed**

-   Redesign the console interface.
-   Rework the shortcuts menu.
-   Rewrite list of languages in accordance with the official Google documentation.
-   Fix wrong interpretation of some special characters.
-   Fix crash when there is no internet connection.
-   Fix wrong button sizes.
-   Fix the re-translation when translating by the keyboard shortcut.
-   Fix disabling of the keyboard shortcut for translating selected text after translation in the main window.
-   Fix wrong behaviour for "Automatically translate" checkbox.

## [0.9.8](https://github.com/crow-translate/crow-translate/tree/0.9.8) (2018-05-19)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/0.9.7...0.9.8)

**Added**

-   Proxy settings.
-   Button and shortcut for copying only translation.
-   The preferred languages feature.
-   More buttons in the pop-up window.
-   Saving of the main window size state.
-   Major performance optimizations.
-   Windows: Icons for the tray context menu.

**Changed**

-   Redesign interface of the settings menu.
-   Fix pop-up window appearing outside the screen.
-   Fix reactivation for the translation hotkey.
-   Fix the bug when an empty field was displayed instead of translation when the hotkey was pressed.
-   Fix wrong interpretation of some symbols.
-   Fix automatic translation in the main window after inserting a new language.
-   Linux: Change icon names for better compatibility.
-   Windows: Improve icon look.
-   "Automatic detection" removed from the context menu of languages ​​as useless.

## [0.9.7](https://github.com/crow-translate/crow-translate/tree/0.9.7) (2018-04-23)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/0.9.6...0.9.7)

**Added**

-   The selection of the previous language if the source language and the target language are the same.
-   Displaying detected languages on the language auto-detection buttons.
-   "Automatically translate" checkbox.
-   Tooltips for buttons.
-   Perfomance improvements.
-   Linux: Icon for the "Show Window" action in the system tray.

**Changed**

-   Redesigned settings appearance.
-   Fix the crash when changing the application language.
-   Fix memory leak with system tray menu.
-   Fix incorrect formatting of the translation text.

## [0.9.6](https://github.com/crow-translate/crow-translate/tree/0.9.6) (2018-04-05)

[Full Changelog](https://github.com/crow-translate/crow-translate/compare/0.9.5...0.9.6)

**Added**

-   Close window shortcut.
-   Minor optimizations.
-   Linux: Icons to tray context menu.

**Changed**

-   New default Windows compatible keyboard shortcuts.
-   Fix memory leak after closing settings.
-   Fix language detection when saying selected text.
-   Windows: Fix the obtaining of selected text.

## [0.9.5](https://github.com/crow-translate/crow-translate/tree/0.9.5) (2018-04-05)

**Added**

-   Display options for translation and transcriptions with formatting.
-   Minor optimizations.

**Changed**

-   Improve translation parsing.
-   Improve network error output.
-   Rework behavior of the button "Reset settings".
-   Now list of languages is sorted alphabetically.
-   Windows: Fix reproduction of Cyrillic characters as "?".

## 0.9.0 (2018-03-14)

_Initial release_
