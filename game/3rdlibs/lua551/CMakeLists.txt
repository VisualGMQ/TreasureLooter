cmake_minimum_required(VERSION 3.10)

project(lua)

# Platform flags mirror the Makefile platform targets:
#   Windows : luaconf.h auto-defines LUA_USE_WINDOWS (C89 + LUA_DL_DLL)
#   Linux   : LUA_USE_LINUX    (POSIX + dlopen, needs -ldl)
#   macOS   : LUA_USE_MACOSX   (POSIX + dlopen)
#   iOS     : LUA_USE_IOS      (POSIX + dlopen)
#   Android : LUA_USE_LINUX    (POSIX + dlopen)

set(LUA551_SRC_DIR "src")

# src/Makefile: CORE_O
set(LUA551_CORE_SOURCES
    ${LUA551_SRC_DIR}/lapi.c
    ${LUA551_SRC_DIR}/lcode.c
    ${LUA551_SRC_DIR}/lctype.c
    ${LUA551_SRC_DIR}/ldebug.c
    ${LUA551_SRC_DIR}/ldo.c
    ${LUA551_SRC_DIR}/ldump.c
    ${LUA551_SRC_DIR}/lfunc.c
    ${LUA551_SRC_DIR}/lgc.c
    ${LUA551_SRC_DIR}/llex.c
    ${LUA551_SRC_DIR}/lmem.c
    ${LUA551_SRC_DIR}/lobject.c
    ${LUA551_SRC_DIR}/lopcodes.c
    ${LUA551_SRC_DIR}/lparser.c
    ${LUA551_SRC_DIR}/lstate.c
    ${LUA551_SRC_DIR}/lstring.c
    ${LUA551_SRC_DIR}/ltable.c
    ${LUA551_SRC_DIR}/ltm.c
    ${LUA551_SRC_DIR}/lundump.c
    ${LUA551_SRC_DIR}/lvm.c
    ${LUA551_SRC_DIR}/lzio.c)

set(LUA551_LIB_SOURCES
    ${LUA551_SRC_DIR}/lauxlib.c
    ${LUA551_SRC_DIR}/lbaselib.c
    ${LUA551_SRC_DIR}/lcorolib.c
    ${LUA551_SRC_DIR}/ldblib.c
    ${LUA551_SRC_DIR}/liolib.c
    ${LUA551_SRC_DIR}/lmathlib.c
    ${LUA551_SRC_DIR}/loadlib.c
    ${LUA551_SRC_DIR}/loslib.c
    ${LUA551_SRC_DIR}/lstrlib.c
    ${LUA551_SRC_DIR}/ltablib.c
    ${LUA551_SRC_DIR}/lutf8lib.c
    ${LUA551_SRC_DIR}/linit.c)

add_library(lua551 STATIC ${LUA551_CORE_SOURCES} ${LUA551_LIB_SOURCES})
add_library(Lua551::Lua551 ALIAS lua551)

target_include_directories(lua551 PUBLIC ${LUA551_SRC_DIR})
target_compile_features(lua551 PRIVATE c_std_99)

set_target_properties(lua551 PROPERTIES
    POSITION_INDEPENDENT_CODE ON)

if (WIN32)
    target_compile_definitions(lua551 PRIVATE LUA_USE_WINDOWS)
elseif (CMAKE_SYSTEM_NAME STREQUAL "iOS")
    target_compile_definitions(lua551 PRIVATE LUA_USE_IOS)
elseif (APPLE)
    target_compile_definitions(lua551 PRIVATE LUA_USE_MACOSX)
elseif (ANDROID OR UNIX)
    target_compile_definitions(lua551 PRIVATE LUA_USE_LINUX)
endif()

if (NOT WIN32)
    target_link_libraries(lua551 PRIVATE m ${CMAKE_DL_LIBS})
endif()

if (MSVC)
    target_compile_definitions(lua551 PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

option(LUA551_BUILD_TOOLS "Build the lua interpreter and luac compiler" OFF)
if (LUA551_BUILD_TOOLS)
    add_executable(lua551_cli ${LUA551_SRC_DIR}/lua.c)
    set_target_properties(lua551_cli PROPERTIES OUTPUT_NAME lua)
    target_link_libraries(lua551_cli PRIVATE lua551)

    add_executable(luac551_cli ${LUA551_SRC_DIR}/luac.c)
    set_target_properties(luac551_cli PROPERTIES OUTPUT_NAME luac)
    target_link_libraries(luac551_cli PRIVATE lua551)

    # macOS defines LUA_USE_READLINE for the REPL.
    if (APPLE AND NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
        target_link_libraries(lua551_cli PRIVATE readline)
    endif()
endif()
