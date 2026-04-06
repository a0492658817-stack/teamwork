cmake_minimum_required(VERSION 3.16)

project(teamwork LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_library(teamwork_core STATIC
    src/game.c
)

target_include_directories(teamwork_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(teamwork
    src/main.c
)

target_link_libraries(teamwork PRIVATE teamwork_core)

if(WIN32)
    set(WINBGIM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/winbgim)
    set(WINBGIM_LIB ${WINBGIM_ROOT}/libbgi.a)
    set(WINBGIM_LIB_MSVC ${WINBGIM_ROOT}/bgi.lib)

    if(EXISTS ${WINBGIM_ROOT}/graphics.h)
        if(MSVC)
            if(EXISTS ${WINBGIM_LIB_MSVC})
                enable_language(CXX)
                target_compile_definitions(teamwork PRIVATE TEAMWORK_USE_WINBGIM=1)
                set_source_files_properties(src/main.c PROPERTIES LANGUAGE CXX)

                target_include_directories(teamwork PRIVATE
                    ${WINBGIM_ROOT}
                )

                target_link_libraries(teamwork PRIVATE
                    ${WINBGIM_LIB_MSVC}
                    gdi32
                    comdlg32
                    uuid
                    oleaut32
                    ole32
                )
            else()
                message(WARNING "WinBGIm sources were found, but MSVC requires bgi.lib. Install/use MinGW generator or provide third_party/winbgim/bgi.lib. Building without WinBGIm GUI.")
            endif()
        elseif(EXISTS ${WINBGIM_LIB})
            enable_language(CXX)
            target_compile_definitions(teamwork PRIVATE TEAMWORK_USE_WINBGIM=1)
            set_source_files_properties(src/main.c PROPERTIES LANGUAGE CXX)

            target_include_directories(teamwork PRIVATE
                ${WINBGIM_ROOT}
            )

            target_link_libraries(teamwork PRIVATE
                ${WINBGIM_LIB}
                gdi32
                comdlg32
                uuid
                oleaut32
                ole32
            )
        else()
            message(WARNING "WinBGIm header found but libbgi.a is missing. Building teamwork without WinBGIm GUI.")
        endif()
    else()
        message(WARNING "WinBGIm not found in third_party/winbgim. Building teamwork without WinBGIm GUI.")
    endif()
elseif(APPLE)
    find_package(SDL2 QUIET)

    if(SDL2_FOUND)
        target_compile_definitions(teamwork PRIVATE TEAMWORK_USE_SDL2=1)

        if(TARGET SDL2::SDL2)
            target_link_libraries(teamwork PRIVATE SDL2::SDL2)
        else()
            target_include_directories(teamwork PRIVATE ${SDL2_INCLUDE_DIRS})
            target_link_libraries(teamwork PRIVATE ${SDL2_LIBRARIES})
        endif()
    else()
        message(WARNING "SDL2 not found on macOS. Install SDL2 (e.g. brew install sdl2) to enable GUI rendering.")
    endif()
endif()

option(TEAMWORK_BUILD_TESTS "Build unit tests" ON)
if(TEAMWORK_BUILD_TESTS)
    enable_testing()

    add_executable(teamwork_tests
        tests/game_tests.c
    )

    target_link_libraries(teamwork_tests PRIVATE teamwork_core)

    add_test(NAME teamwork_tests COMMAND teamwork_tests)
endif()