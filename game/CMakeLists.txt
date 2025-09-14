cmake_minimum_required(VERSION 3.20)

project(TreasureLooter
    VERSION 0.0.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################# compile 3rdlibs #####################

include(cmake/add_3rdlib.cmake)
add_subdirectory(3rdlibs)

################ generate project path config file ################
configure_file(project_path_generator.xml ${CMAKE_CURRENT_SOURCE_DIR}/project_path.xml)

################ compile schema parser ################
set(NEED_SCHEMA_PARSER ON)

if (ANDROID)
    set(NEED_SCHEMA_PARSER OFF)
endif()

if (NEED_SCHEMA_PARSER)
    add_subdirectory(schema)
endif()

################ run schema parser ################
file(GLOB_RECURSE SCHEMAS schema/schema/*)
file(GLOB_RECURSE MUSTACHES ./schema/schema_parser/mustaches/*.mustache)
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
endforeach()

set(SCHEMA_ASSET_INFO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}/asset_info.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}/asset_info.cpp)


list(APPEND SCHEMA_SERD_OUTPUT_FILES "${schema_final_path}/serialize/serialize.hpp")
list(APPEND SCHEMA_DISPLAY_OUTPUT_FILES "${schema_final_path}/display/display.hpp")

if (NEED_SCHEMA_PARSER)
    add_custom_target(run_schema_parser
            COMMAND $<TARGET_FILE:schema_parser> ./schema/schema --output-dir=${SCHEMA_OUTPUT_PATH} --src-prefix="./"
            DEPENDS $<TARGET_FILE:schema_parser> ${SCHEMAS} ${MUSTACHES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "parsing schemas...")

    add_custom_command(OUTPUT ${SCHEMA_DECLARE_OUTPUT_FILES} ${SCHEMA_SERD_OUTPUT_FILES} ${SCHEMA_DISPLAY_OUTPUT_FILES} ${SCHEMA_ASSET_INFO_FILES}
            COMMAND $<TARGET_FILE:schema_parser> ./schema/schema --output-dir=${SCHEMA_OUTPUT_PATH} --src-prefix="./"
            DEPENDS $<TARGET_FILE:schema_parser> ${SCHEMAS} ${MUSTACHES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "parsing schemas...")
endif()

################ run protoc ################
file(GLOB_RECURSE PROTO_FILES protobuf/*.proto)
set(PROTO_OUTPUT_PATH "proto-generate/proto")
set(PROTO_DECLARE_OUTPUT_FILES "")
set(PROTO_IMPL_OUTPUT_FILES "")
foreach(proto ${PROTO_FILES})
    cmake_path(GET proto FILENAME proto_filename)
    cmake_path(REPLACE_EXTENSION proto_filename ".pb.h" OUTPUT_VARIABLE proto_generate_hpp)
    cmake_path(REPLACE_EXTENSION proto_filename ".pb.cc" OUTPUT_VARIABLE proto_generate_cpp)
    set(proto_final_path "${CMAKE_CURRENT_SOURCE_DIR}/${PROTO_OUTPUT_PATH}")
    list(APPEND
            PROTO_DECLARE_OUTPUT_FILES
            "${proto_final_path}/${proto_generate_hpp}")
    list(APPEND
            PROTO_IMPL_OUTPUT_FILES
            "${proto_final_path}/${proto_generate_cpp}")
endforeach()

add_custom_command(OUTPUT ${PROTO_DECLARE_OUTPUT_FILES} ${PROTO_IMPL_OUTPUT_FILES}
            COMMAND $<TARGET_FILE:protobuf::protoc> --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protobuf --cpp_out=${PROTO_OUTPUT_PATH} ${PROTO_FILES}
            DEPENDS $<TARGET_FILE:protobuf::protoc> ${PROTO_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "parsing protobuf files...")

################ build engine ################
set(ENGINE_NAME engine)
file(GLOB_RECURSE ENGINE_SRC engine/src/*.cpp engine/src/*.c)
file(GLOB_RECURSE ENGINE_HEADER engine/include/*.hpp engine/include/*.h)
file(GLOB_RECURSE ASSETS assets/*)

add_library(${ENGINE_NAME} STATIC)

target_sources(${ENGINE_NAME}
    PRIVATE
        ${ENGINE_SRC} ${ENGINE_HEADER}
        ${ASSETS}
        ${SCHEMAS} ${SCHEMA_DECLARE_OUTPUT_FILES} ${SCHEMA_SERD_OUTPUT_FILES} ${SCHEMA_ASSET_INFO_FILES}
        ${PROTO_DECLARE_OUTPUT_FILES} ${PROTO_IMPL_OUTPUT_FILES})
target_include_directories(${ENGINE_NAME} PUBLIC engine/include schema_generate proto-generate)
target_compile_features(${ENGINE_NAME} PRIVATE cxx_std_17)
target_link_libraries(${ENGINE_NAME} PUBLIC SDL3::SDL3 spdlog::spdlog stb_image imgui RapidXML stduuid tmxlite SDL3_ttf::SDL3_ttf enet protobuf::libprotobuf)
target_compile_definitions(${ENGINE_NAME}
    PUBLIC $<$<CONFIG:Debug>:TL_DEBUG>
    PRIVATE _CRT_SECURE_NO_WARNINGS)

################ build game ################
set(GAME_NAME ${PROJECT_NAME})

if (ANDROID)
    set(GAME_NAME main)
    add_library(${GAME_NAME} SHARED)
else()
    add_executable(${GAME_NAME})
endif()

target_sources(${GAME_NAME} PRIVATE main_entry/game_main.cpp)
target_link_libraries(${GAME_NAME} PRIVATE ${ENGINE_NAME})

################ build editor ################
if (NOT ANDROID)
    file(GLOB_RECURSE EDITOR_SRC editor/*.hpp editor/*.cpp)
    add_executable(TreasureLooter-Editor)
    target_sources(TreasureLooter-Editor PRIVATE ${EDITOR_SRC} ${SCHEMA_DISPLAY_OUTPUT_FILES})
    target_link_libraries(TreasureLooter-Editor PRIVATE ${ENGINE_NAME})
endif()

################ build server ################
if (NOT ANDROID)
    file(GLOB_RECURSE SERVER_SRC server/*.hpp server/*.cpp)
    add_executable(TreasureLooter-Server)
    target_sources(TreasureLooter-Server PRIVATE ${SERVER_SRC})
    target_link_libraries(TreasureLooter-Server PRIVATE ${ENGINE_NAME})
endif()

################ install #######################
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
