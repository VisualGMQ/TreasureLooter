cmake_minimum_required(VERSION 3.20)

project(TreasureLooter
    VERSION 0.1.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################# compile 3rdlibs #####################

include(cmake/add_3rdlib.cmake)
add_subdirectory(3rdlibs)

################# options #####################
option(TL_ENABLE_PROFILE "enable profile use tracy" OFF)

################ generate project path config file ################
configure_file(project_path_generator.xml ${CMAKE_CURRENT_SOURCE_DIR}/project_path.xml)

################ compile schema parser ################
set(NEED_SCHEMA_PARSER ON)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set(NEED_SCHEMA_PARSER OFF)
endif()

if (NEED_SCHEMA_PARSER)
    add_subdirectory(schema)
endif()

################ run schema parser ################
file(GLOB_RECURSE SCHEMAS schema/schema/*)
file(GLOB_RECURSE MUSTACHES ./schema/schema_parser/mustaches/*.mustache)
set(SCHEMA_OUTPUT_ROOT_PATH "schema_generate")
set(SCHEMA_OUTPUT_PATH "${SCHEMA_OUTPUT_ROOT_PATH}/schema")
set(SCHEMA_PROTOBUF_OUTPUT_PATH "${SCHEMA_OUTPUT_ROOT_PATH}/proto")

set(SCHEMA_DECLARE_OUTPUT_FILES)
set(SCHEMA_SERD_OUTPUT_FILES)
set(SCHEMA_DISPLAY_OUTPUT_FILES)
set(SCHEMA_PROTO_OUTPUT_FILES)
set(PROTOBUF_OUTPUT_FILES)
foreach(schema ${SCHEMAS})
    cmake_path(GET schema FILENAME schema_filename)
    cmake_path(REPLACE_EXTENSION schema_filename ".hpp" OUTPUT_VARIABLE schema_generate_hpp)
    cmake_path(REPLACE_EXTENSION schema_filename ".cpp" OUTPUT_VARIABLE schema_generate_cpp)
    cmake_path(REPLACE_EXTENSION schema_filename ".proto" OUTPUT_VARIABLE schema_generate_proto)
    set(schema_final_path "${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}")
    list(APPEND
            SCHEMA_DECLARE_OUTPUT_FILES
            "${schema_final_path}/${schema_generate_hpp}")
    list(APPEND
            SCHEMA_PROTO_OUTPUT_FILES
            "${schema_final_path}/proto/${schema_generate_proto}")
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

    cmake_path(REPLACE_EXTENSION schema_filename ".pb.h" OUTPUT_VARIABLE protobuf_generate_hpp)
    cmake_path(REPLACE_EXTENSION schema_filename ".pb.cc" OUTPUT_VARIABLE protobuf_generate_cpp)
    set(protobuf_final_path "${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_PROTOBUF_OUTPUT_PATH}")
    list(APPEND
            PROTOBUF_OUTPUT_FILES
            "${protobuf_final_path}/${protobuf_generate_hpp}"
            "${protobuf_final_path}/${protobuf_generate_cpp}")
endforeach()

set(SCHEMA_ASSET_INFO_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}/asset_info.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/${SCHEMA_OUTPUT_PATH}/asset_info.cpp)


list(APPEND SCHEMA_SERD_OUTPUT_FILES "${schema_final_path}/serialize/serialize.hpp")
list(APPEND SCHEMA_DISPLAY_OUTPUT_FILES "${schema_final_path}/display/display.hpp")
list(APPEND SCHEMA_BINDING_OUTPUT_FILES "${schema_final_path}/binding/binding.hpp" "${schema_final_path}/binding/binding.cpp")
list(APPEND PROTOBUF_OUTPUT_FILES  "${protobuf_final_path}/all_proto.pb.h" "${protobuf_final_path}/all_proto.pb.cc")
set(PROTOBUF_NETMSG_DISPATCH_FILES "${schema_final_path}/proto/net_msg_dispatch.hpp" "${schema_final_path}/proto/net_msg_dispatch.cpp")
set(PROTOBUF_SCRIPT_BINDING_FILES "${schema_final_path}/proto/proto_binding.hpp" "${schema_final_path}/proto/proto_binding.cpp"
                                  "${schema_final_path}/proto/proto_event_binding.hpp" "${schema_final_path}/proto/proto_event_binding.cpp"
                                  "${schema_final_path}/proto/proto_convert.hpp" "${schema_final_path}/proto/proto_convert.cpp")
set(SCHEMA_SCRIPT_HINT_INC "${schema_final_path}/schema_types.luau.inc")
set(PROTO_SCRIPT_HINT_INC "${schema_final_path}/proto_types.luau.inc")
set(LUAU_TYPES_HINT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/scripts/type_hints")
set(LUAU_SCHEMA_TYPES_HINT_FILE "${LUAU_TYPES_HINT_DIR}/tl_schema_types.luau")
set(LUAU_PROTO_TYPES_HINT_FILE "${LUAU_TYPES_HINT_DIR}/tl_proto_types.luau")

if (NEED_SCHEMA_PARSER)
    add_custom_command(OUTPUT
        ${SCHEMA_DECLARE_OUTPUT_FILES}
        ${SCHEMA_SERD_OUTPUT_FILES}
        ${SCHEMA_DISPLAY_OUTPUT_FILES}
        ${SCHEMA_BINDING_OUTPUT_FILES}
        ${SCHEMA_ASSET_INFO_FILES}
        ${SCHEMA_SCRIPT_HINT_INC}
        ${PROTO_SCRIPT_HINT_INC}
        ${SCHEMA_PROTO_OUTPUT_FILES}
        ${PROTOBUF_NETMSG_DISPATCH_FILES}
        ${PROTOBUF_SCRIPT_BINDING_FILES}
        COMMAND $<TARGET_FILE:schema_parser> ./schema/schema --output-dir=${SCHEMA_OUTPUT_ROOT_PATH} --src-prefix="./"
        DEPENDS $<TARGET_FILE:schema_parser> ${SCHEMAS} ${MUSTACHES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "=== parsing schemas...")

    add_custom_command(OUTPUT ${PROTOBUF_OUTPUT_FILES}
            COMMAND $<TARGET_FILE:protoc> --proto_path=${SCHEMA_OUTPUT_PATH}/proto --cpp_out=${SCHEMA_PROTOBUF_OUTPUT_PATH} ${SCHEMA_OUTPUT_PATH}/proto/*.proto
            DEPENDS $<TARGET_FILE:protoc> ${SCHEMA_PROTO_OUTPUT_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "=== generate protobuf files...")

    add_custom_command(OUTPUT ${LUAU_SCHEMA_TYPES_HINT_FILE}
        COMMAND ${CMAKE_COMMAND}
            -DINC=${SCHEMA_SCRIPT_HINT_INC}
            -DOUT=${LUAU_SCHEMA_TYPES_HINT_FILE}
            -DRETURN_VALUE=TL_Schema
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrap_luau_hint_inc.cmake
        DEPENDS ${SCHEMA_SCRIPT_HINT_INC}
        COMMENT "=== generate tl_schema_types.luau")

    add_custom_command(OUTPUT ${LUAU_PROTO_TYPES_HINT_FILE}
        COMMAND ${CMAKE_COMMAND}
            -DINC=${PROTO_SCRIPT_HINT_INC}
            -DOUT=${LUAU_PROTO_TYPES_HINT_FILE}
            -DRETURN_VALUE=TL_Proto
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/wrap_luau_hint_inc.cmake
        DEPENDS ${PROTO_SCRIPT_HINT_INC}
        COMMENT "=== generate tl_proto_types.luau")

    add_custom_target(schema_preprocess ALL
        DEPENDS ${LUAU_SCHEMA_TYPES_HINT_FILE} ${LUAU_PROTO_TYPES_HINT_FILE} ${PROTOBUF_OUTPUT_FILES}
        COMMENT "=== generate luau type hints")
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

add_subdirectory(client_entry)

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
