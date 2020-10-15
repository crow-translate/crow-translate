include(FetchContent)

set(QAPPLICATION_CLASS QApplication)
FetchContent_Declare(SingleApplication
    GIT_REPOSITORY https://github.com/Shatur95/SingleApplication
    GIT_TAG fix-clang-tidy-warnings
)

FetchContent_Declare(QTaskbarControl
    GIT_REPOSITORY https://github.com/Shatur95/QTaskbarControl
    GIT_TAG fix-clang-tidy-warnings
)

option(QHOTKEY_INSTALL OFF)
FetchContent_Declare(QHotkey
    GIT_REPOSITORY https://github.com/Shatur95/QHotkey
    GIT_TAG fix-clang-tidy-warnings
)

FetchContent_Declare(QOnlineTranslator
    GIT_REPOSITORY https://github.com/crow-translate/QOnlineTranslator
    GIT_TAG 0ad1e2eddd1ff6d59b5f32da763def54d863a71a
)

FetchContent_MakeAvailable(SingleApplication QTaskbarControl QHotkey QOnlineTranslator)

if(WIN32)
    FetchContent_Declare(QGitTag
        GIT_REPOSITORY https://github.com/crow-translate/QGitTag
        GIT_TAG f1b2950791e35f1f25bd143e4c5f832d4cb2e498
    )

    FetchContent_MakeAvailable(QGitTag)
endif()
