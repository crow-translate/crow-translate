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
    GIT_TAG 9cdd27e33b754c81454ca02328b42ab4149d86f4
)

FetchContent_MakeAvailable(SingleApplication QTaskbarControl QHotkey QOnlineTranslator)

if(WIN32)
    FetchContent_Declare(QGitTag
        GIT_REPOSITORY https://github.com/crow-translate/QGitTag
        GIT_TAG 4ad07810469db88aaa5681ed1ace184652c01bfd
    )

    FetchContent_MakeAvailable(QGitTag)
endif()
