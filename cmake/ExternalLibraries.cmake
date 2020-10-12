include(FetchContent)

set(QAPPLICATION_CLASS QApplication)
FetchContent_Declare(SingleApplication
    GIT_REPOSITORY https://github.com/itay-grudev/SingleApplication
    GIT_TAG v3.1.5
)

FetchContent_Declare(QTaskbarControl
    GIT_REPOSITORY https://github.com/Skycoder42/QTaskbarControl
    GIT_TAG 56a0aea84365969f13553cdcf28a8c01d71b6183
)

option(QHOTKEY_INSTALL OFF)
FetchContent_Declare(QHotkey
    GIT_REPOSITORY https://github.com/Skycoder42/QHotkey
    GIT_TAG 1.4.1
)

FetchContent_Declare(QOnlineTranslator
    GIT_REPOSITORY https://github.com/crow-translate/QOnlineTranslator
    GIT_TAG eb2ebc210bb23f1ba12274cf00a7b3a287b05e87
)

FetchContent_MakeAvailable(SingleApplication QTaskbarControl QHotkey QOnlineTranslator)

if(WIN32)
    FetchContent_Declare(QGitTag
        GIT_REPOSITORY https://github.com/crow-translate/QGitTag
        GIT_TAG 1.0.5
    )

    FetchContent_MakeAvailable(QGitTag)
endif()
