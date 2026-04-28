cmake_minimum_required(VERSION 3.20)

project(TreasureLooter
    VERSION 0.1.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################# compile 3rdlibs #####################

include(cmake/add_3rdlib.cmake)
add_subdirectory(3rdlibs)

################# options #####################
option(TL_ENABLE_PROFILE "enable profile use tracy v0.13.1" OFF)

################ generate project path config file ################
configure_file(project_path_generator.xml ${CMAKE_CURRENT_SOURCE_DIR}/project_path.xml)

################ compile schema parser ################
set(NEED_SCHEMA_PARSER ON)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(NEED_SCHEMA_PARSER OFF)
endif()

message(STATUS "schema parser gating: CMAKE_SYSTEM_NAME='${CMAKE_SYSTEM_NAME}', ANDROID='${ANDROID}', NEED_SCHEMA_PARSER='${NEED_SCHEMA_PARSER}'")

if (NEED_SCHEMA_PARSER)
    add_subdirectory(schema)
endif()

################ run schema parser ################
file(GLOB_RECURSE SCHEMAS schema/schema/*)
file(GLOB_RECURSE MUSTACHES ./schema/schema_parser/mustaches/*.mustache)
file(GLOB_RECURSE SCHEMA_PARSER_TOOL_SOURCES
    CONFIGURE_DEPENDS
    schema/schema_parser/*.cpp
    schema/schema_parser/*.hpp)
set(SCHEMA_OUTPUT_PATH "schema_generate/schema")
set(SCHEMA_DECLARE_OUTPUT_FILES)
set(SCHEMA_SERD_OUTPUT_FILES)
set(SCHEMA_DISPLAY_OUTPUT_FILES)
foreach(schema ${SCHEMAS})
    cmake_path(GET schema FILENAME schema_filename)
    cmake_path(REPLACE_EXTENSION schema_filename ".hpp" OUTPUT_VARIABLE schema_generate_hpp)
    cmake_path(REPLACE_EXTENSION schema_filename ".cpp" OUTPUT_VARIABLE schema_generate_cpp)
    set(schema_final_path "${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}")
    list(APPEND
            SCHEMA_DECLARE_OUTPUT_FILES
            "${schema_final_path}/${schema_generate_hpp}")
    list(APPEND
            SCHEMA_SERD_OUTPUT_FILES
            "${schema_final_path}/serialize/${schema_generate_hpp}"
            "${schema_final_path}/serialize/${schema_generate_cpp}")
    list(APPEND
            SCHEMA_DISPLAY_OUTPUT_FILES
            "${schema_final_path}/display/${schema_generate_hpp}"
            "${schema_final_path}/display/${schema_generate_cpp}")
    list(APPEND
            SCHEMA_BINDING_OUTPUT_FILES
            "${schema_final_path}/binding/${schema_generate_hpp}"
            "${schema_final_path}/binding/${schema_generate_cpp}")

endforeach()

set(SCHEMA_ASSET_INFO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}/asset_info.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}/asset_info.cpp)


list(APPEND SCHEMA_SERD_OUTPUT_FILES "${schema_final_path}/serialize/serialize.hpp")
list(APPEND SCHEMA_DISPLAY_OUTPUT_FILES "${schema_final_path}/display/display.hpp")
list(APPEND SCHEMA_BINDING_OUTPUT_FILES "${schema_final_path}/binding/binding.hpp" "${schema_final_path}/binding/binding.cpp")
set(SCHEMA_SCRIPT_HINT_INC "${schema_final_path}/schema_types.luau.inc")
set(LUAU_TYPES_HINT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts/type_hints")
set(LUAU_SCHEMA_TYPES_HINT_FILE "${LUAU_TYPES_HINT_DIR}/tl_schema_types.luau")

if (NEED_SCHEMA_PARSER)
    message(STATUS "schema parser will be built, and the generated files will be placed in ${SCHEMA_OUTPUT_PATH}")

    add_custom_command(OUTPUT
        ${SCHEMA_DECLARE_OUTPUT_FILES}
        ${SCHEMA_SERD_OUTPUT_FILES}
        ${SCHEMA_DISPLAY_OUTPUT_FILES}
        ${SCHEMA_BINDING_OUTPUT_FILES}
        ${SCHEMA_ASSET_INFO_FILES}
        ${SCHEMA_SCRIPT_HINT_INC}
        COMMAND $<TARGET_FILE:schema_parser> ./schema/schema --output-dir=${SCHEMA_OUTPUT_PATH} --src-prefix="./"
        DEPENDS $<TARGET_FILE:schema_parser> ${SCHEMAS} ${MUSTACHES} ${SCHEMA_PARSER_TOOL_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "parsing schemas...")

    add_custom_command(OUTPUT ${LUAU_SCHEMA_TYPES_HINT_FILE}
        COMMAND ${CMAKE_COMMAND}
            -DINC=${SCHEMA_SCRIPT_HINT_INC}
            -DOUT=${LUAU_SCHEMA_TYPES_HINT_FILE}
            -DRETURN_VALUE=TL_Schema
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrap_luau_hint_inc.cmake
        DEPENDS ${SCHEMA_SCRIPT_HINT_INC}
        COMMENT "generate tl_schema_types.luau")

    add_custom_target(mobile_preprocess ALL
        DEPENDS ${LUAU_SCHEMA_TYPES_HINT_FILE}
        COMMENT "generate luau type hints")
endif()

################ build common ################
set(COMMON_NAME common)
add_subdirectory(common)

################ build client ################
set(CLIENT_NAME client)
add_subdirectory(client)

################ build game ################
set(GAME_NAME ${PROJECT_NAME})

if (ANDROID)
    set(GAME_NAME main)
endif()

add_subdirectory(game_entry)

################ build server ################
if (NOT ANDROID)
    set(SERVER_NAME server)
    add_subdirectory(server)
    add_subdirectory(server_entry)
endif()

################ build editor ################
if (NOT ANDROID)
    add_subdirectory(tools)
endif()

################ install #######################
install(TARGETS ${GAME_NAME} DESTINATION .)
install(DIRECTORY assets DESTINATION .)
install(DIRECTORY scripts DESTINATION .)

################ package ##################
# include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
include(CPack)
