include(FetchContent)

set(QAPPLICATION_CLASS QApplication)
FetchContent_Declare(SingleApplication
    GIT_REPOSITORY https://github.com/itay-grudev/SingleApplication
    GIT_TAG 5d837262f42a446719754fd8ea6f1373b2bc0fd0
)

FetchContent_Declare(QTaskbarControl
    GIT_REPOSITORY https://github.com/Skycoder42/QTaskbarControl
    GIT_TAG 2.0.2
)

option(QHOTKEY_INSTALL OFF)
FetchContent_Declare(QHotkey
    GIT_REPOSITORY https://github.com/Skycoder42/QHotkey
    GIT_TAG 53745ba936ec3eef4653ecffbfb50e66c6722fd9
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
