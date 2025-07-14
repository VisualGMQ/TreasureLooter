cmake_minimum_required(VERSION 3.20)

project(TreasureLooter
    VERSION 0.0.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################# compile 3rdlibs #####################

include(cmake/add_3rdlib.cmake)
add_subdirectory(3rdlibs)

################ compile schema parser ################
if (NOT ANDROID)
    add_subdirectory(schema_parser)
endif()

################ run schema parser ################
file(GLOB_RECURSE SCHEMAS ./schema/*)
file(GLOB_RECURSE MUSTACHES ./schema_parser/mustaches/*.mustache)
set(SCHEMA_OUTPUT_PATH "schema_generate/schema")
set(SCHEMA_OUTPUT_FILES)
foreach(schema ${SCHEMAS})
    cmake_path(GET schema FILENAME schema_filename)
    cmake_path(REPLACE_EXTENSION schema_filename ".hpp" OUTPUT_VARIABLE schema_generate_hpp)
    cmake_path(REPLACE_EXTENSION schema_filename ".cpp" OUTPUT_VARIABLE schema_generate_cpp)
    set(schema_final_path "${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}")
    list(APPEND
        SCHEMA_OUTPUT_FILES
        "${schema_final_path}/${schema_generate_hpp}"
        "${schema_final_path}/serialize/${schema_generate_hpp}"
        "${schema_final_path}/serialize/${schema_generate_cpp}")
endforeach()

if (NOT ANDROID)
    add_custom_target(run_schema_parser
        COMMAND $<TARGET_FILE:schema_parser> ./schema --output-dir=${SCHEMA_OUTPUT_PATH} --src-prefix="./"
        DEPENDS $<TARGET_FILE:schema_parser> ${SCHEMAS} ${MUSTACHES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "parsing schemas...")

    add_custom_command(OUTPUT ${SCHEMA_OUTPUT_FILES}
        COMMAND $<TARGET_FILE:schema_parser> ./schema --output-dir=${SCHEMA_OUTPUT_PATH} --src-prefix="./"
        DEPENDS $<TARGET_FILE:schema_parser> ${SCHEMAS} ${MUSTACHES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "parsing schemas...")
endif()

################ compile game ######################

file(GLOB_RECURSE SRC ./src/*.cpp ./src/*.c)
file(GLOB_RECURSE HEADER ./include/*.hpp ./include/*.h)
file(GLOB_RECURSE ASSETS ./assets/*)

set(GAME_NAME ${PROJECT_NAME})
if (ANDROID)
    set(GAME_NAME main)
    add_library(${GAME_NAME} SHARED)
else()
    if (EMSCRIPTEN)
        set(GAME_NAME index)
    endif()
    add_executable(${GAME_NAME})
endif()

if(EMSCRIPTEN)
    message("emscripten build")
    set(CMAKE_EXECUTABLE_SUFFIX .html)
    set_target_properties(${GAME_NAME}
        PROPERTIES
            LINK_FLAGS "--preload-file=assets -s ALLOW_MEMORY_GROWTH=1")

    add_custom_command(TARGET ${GAME_NAME}
        PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
        COMMENT "copy assets file to build path"
        VERBATIM
    )
endif()

target_link_libraries(${GAME_NAME} PRIVATE SDL3::SDL3 spdlog::spdlog stb_image imgui)
target_sources(${GAME_NAME} PRIVATE ${SRC} ${HEADER} ${ASSETS})
target_include_directories(${GAME_NAME} PRIVATE include)
target_compile_features(${GAME_NAME} PRIVATE cxx_std_17)
target_compile_definitions(${GAME_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)

################ compile tools ######################
if(NOT ANDROID)
    add_subdirectory(tools)
endif()

################ instal #######################
install(TARGETS ${GAME_NAME} DESTINATION .)
install(DIRECTORY assets DESTINATION .)

################ package ##################
# include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
include(CPack)