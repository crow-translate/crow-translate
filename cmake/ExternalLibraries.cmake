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
    GIT_TAG 1.4.5
)

FetchContent_Declare(CircleFlags
    GIT_REPOSITORY https://github.com/HatScripts/circle-flags
    GIT_TAG v2.1.0
)

FetchContent_Declare(We10X
    GIT_REPOSITORY https://github.com/yeyushengfan258/We10X-icon-theme
    GIT_TAG bd2c68482a06d38b2641503af1ca127b9e6540db
)

FetchContent_MakeAvailable(SingleApplication QTaskbarControl QHotkey QOnlineTranslator CircleFlags We10X)
FetchContent_GetProperties(CircleFlags SOURCE_DIR CircleFlags_SOURCE_DIR)
FetchContent_GetProperties(We10X SOURCE_DIR We10X_SOURCE_DIR)

if(WIN32)
    FetchContent_Declare(QGitTag
        GIT_REPOSITORY https://github.com/crow-translate/QGitTag
        GIT_TAG 1.0.6
    )

    FetchContent_MakeAvailable(QGitTag)
endif()
