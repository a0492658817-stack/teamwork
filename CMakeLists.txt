cmake_minimum_required(VERSION 3.16)

# 專案支援 C 與 CXX (C++)，因為 WinBGIm 核心需要 C++ 環境
project(teamwork LANGUAGES C CXX)

# 標準設定
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 設定輸出的執行檔與庫路徑
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# -------------------------------------------------------------------
# 1. 核心邏輯庫 (團隊共用的遊戲邏輯)
# -------------------------------------------------------------------
add_library(teamwork_core STATIC src/game.c)
target_include_directories(teamwork_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# -------------------------------------------------------------------
# 2. 主程式執行檔
# -------------------------------------------------------------------
add_executable(teamwork src/main.c src/app.c)
target_link_libraries(teamwork PRIVATE teamwork_core)

# -------------------------------------------------------------------
# 3. Windows 平台：WinBGIm 渲染配置
# -------------------------------------------------------------------
if(WIN32)
    # 定義路徑變數：支援組員的 third_party 資料夾與個人手動補齊的路徑
    set(WINBGIM_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/winbgim)
    set(MANUAL_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(MANUAL_LIB ${CMAKE_CURRENT_SOURCE_DIR}/lib/libbgi.a)

    target_sources(teamwork PRIVATE src/platform_windows.c)
    
    # [關鍵] 強制將 platform_windows.c 視為 C++ 編譯，以支援 <sstream> 等 C++ 標頭檔
    set_source_files_properties(src/platform_windows.c PROPERTIES LANGUAGE CXX)

    # 判定順序：優先檢查手動放置的路徑，再檢查 third_party
    if(EXISTS ${MANUAL_LIB} AND EXISTS ${MANUAL_INCLUDE}/graphics.h)
        # 情況 A：檔案手動放在 include/ 和 lib/ 下 (個人開發環境)
        message(STATUS "GUI: Using manual WinBGIm paths (include/lib).")
        target_compile_definitions(teamwork PRIVATE TEAMWORK_USE_WINBGIM=1)
        target_include_directories(teamwork PRIVATE ${MANUAL_INCLUDE})
        target_link_libraries(teamwork PRIVATE ${MANUAL_LIB} gdi32 comdlg32 uuid oleaut32 ole32)
        
    elseif(EXISTS ${WINBGIM_ROOT}/graphics.h)
        # 情況 B：使用組員配置的 third_party 資料夾 (團隊標準環境)
        message(STATUS "GUI: Using WinBGIm from third_party folder.")
        target_compile_definitions(teamwork PRIVATE TEAMWORK_USE_WINBGIM=1)
        target_include_directories(teamwork PRIVATE ${WINBGIM_ROOT})
        
        if(MSVC AND EXISTS ${WINBGIM_ROOT}/bgi.lib)
            target_link_libraries(teamwork PRIVATE ${WINBGIM_ROOT}/bgi.lib gdi32 comdlg32 uuid oleaut32 ole32)
        elseif(EXISTS ${WINBGIM_ROOT}/libbgi.a)
            target_link_libraries(teamwork PRIVATE ${WINBGIM_ROOT}/libbgi.a gdi32 comdlg32 uuid oleaut32 ole32)
        endif()
    else()
        # 情況 C：找不到 WinBGIm，降級為 Console 模式
        message(WARNING "WinBGIm library not found. Building in Console mode.")
    endif()

    # MinGW 執行時需要的 DLL。Code::Blocks 常自動帶 PATH，VS Code 不一定。
    # 直接複製到可執行檔旁，避免在不同 IDE 下執行行為不一致。
    if(MINGW)
        get_filename_component(MINGW_BIN_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)
        foreach(RUNTIME_DLL libstdc++-6.dll libgcc_s_dw2-1.dll libwinpthread-1.dll)
            if(EXISTS ${MINGW_BIN_DIR}/${RUNTIME_DLL})
                add_custom_command(TARGET teamwork POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${MINGW_BIN_DIR}/${RUNTIME_DLL}
                    $<TARGET_FILE_DIR:teamwork>/${RUNTIME_DLL})
            endif()
        endforeach()
    endif()

# -------------------------------------------------------------------
# 4. Apple 平台：SDL2 渲染配置
# -------------------------------------------------------------------
elseif(APPLE)
    find_package(SDL2 QUIET)
    if(SDL2_FOUND)
        target_sources(teamwork PRIVATE src/platform_sdl.c)
        target_compile_definitions(teamwork PRIVATE TEAMWORK_USE_SDL2=1)
        target_link_libraries(teamwork PRIVATE SDL2::SDL2)
    else()
        target_sources(teamwork PRIVATE src/platform_console.c)
    endif()
else()
    # 其他平台 (如 Linux) 預設使用 Console
    target_sources(teamwork PRIVATE src/platform_console.c)
endif()

# -------------------------------------------------------------------
# 5. 單元測試設定
# -------------------------------------------------------------------
option(TEAMWORK_BUILD_TESTS "Build unit tests" ON)
if(TEAMWORK_BUILD_TESTS)
    enable_testing()
    add_executable(teamwork_tests tests/game_tests.c)
    target_link_libraries(teamwork_tests PRIVATE teamwork_core)
    add_test(NAME teamwork_tests COMMAND teamwork_tests)
endif()