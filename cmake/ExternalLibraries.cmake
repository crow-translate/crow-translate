include(FetchContent)

set(QAPPLICATION_CLASS QApplication)
FetchContent_Declare(SingleApplication
    GIT_REPOSITORY https://github.com/itay-grudev/SingleApplication
    GIT_TAG v3.2.0
)

FetchContent_Declare(QTaskbarControl
    GIT_REPOSITORY https://github.com/Skycoder42/QTaskbarControl
    GIT_TAG 2.0.2
)

option(QHOTKEY_INSTALL OFF)
FetchContent_Declare(QHotkey
    GIT_REPOSITORY https://github.com/Skycoder42/QHotkey
    GIT_TAG 1.4.2
)

FetchContent_Declare(QOnlineTranslator
    GIT_REPOSITORY https://github.com/crow-translate/QOnlineTranslator
    GIT_TAG df89083d2f680a8f856b1df00b8846f995cf1fae
)

FetchContent_Declare(CircleFlags
    GIT_REPOSITORY https://github.com/HatScripts/circle-flags
    GIT_TAG v2.3.0
)

FetchContent_Declare(FluentIconTheme
    GIT_REPOSITORY https://github.com/vinceliuice/Fluent-icon-theme
    GIT_TAG 2021-08-15
)

FetchContent_MakeAvailable(SingleApplication QTaskbarControl QHotkey QOnlineTranslator CircleFlags FluentIconTheme)
FetchContent_GetProperties(CircleFlags SOURCE_DIR CircleFlags_SOURCE_DIR)
FetchContent_GetProperties(FluentIconTheme SOURCE_DIR FluentIconTheme_SOURCE_DIR)

if(WIN32)
    FetchContent_Declare(QGitTag
        GIT_REPOSITORY https://github.com/crow-translate/QGitTag
        GIT_TAG 1.0.7
    )

    FetchContent_MakeAvailable(QGitTag)
endif()
