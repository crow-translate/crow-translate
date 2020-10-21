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
    GIT_TAG 1.4.0
)

FetchContent_MakeAvailable(SingleApplication QTaskbarControl QHotkey QOnlineTranslator)

if(WIN32)
    FetchContent_Declare(QGitTag
        GIT_REPOSITORY https://github.com/crow-translate/QGitTag
        GIT_TAG 1.0.6
    )

    FetchContent_MakeAvailable(QGitTag)
endif()
