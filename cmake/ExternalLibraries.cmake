include(FetchContent)

set(QAPPLICATION_CLASS QApplication)
FetchContent_Declare(SingleApplication
    GIT_REPOSITORY https://github.com/itay-grudev/SingleApplication
    GIT_TAG f3de0114f45c7ff073f29cf0822df5161bb5c3b3
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
    GIT_TAG 64e3e8876d571cb7bd6a5b0e71dd73b1aaf213a0
)

FetchContent_MakeAvailable(SingleApplication QTaskbarControl QHotkey QOnlineTranslator)

if(WIN32)
    FetchContent_Declare(QGitTag
        GIT_REPOSITORY https://github.com/crow-translate/QGitTag
        GIT_TAG 1.0.6
    )

    FetchContent_Declare(We10X
        GIT_REPOSITORY https://github.com/yeyushengfan258/We10X-icon-theme
        GIT_TAG 7cc84b4b62b0fdd39ce716ba928fb3222f4b931a
    )

    FetchContent_MakeAvailable(QGitTag We10X)
    FetchContent_GetProperties(We10X SOURCE_DIR We10X_SOURCE_DIR)
endif()
