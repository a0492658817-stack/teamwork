cmake_minimum_required(VERSION 3.16)

project(teamwork LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

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
    target_include_directories(teamwork PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/winbgim
    )

    target_link_libraries(teamwork PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/winbgim/libbgi.a
        gdi32
        comdlg32
        uuid
        oleaut32
        ole32
    )
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