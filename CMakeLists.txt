cmake_minimum_required(VERSION 3.20)

project(TreasureLooter
    VERSION 0.0.1)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################# compile 3rdlibs #####################

include(cmake/add_3rdlib.cmake)
add_subdirectory(3rdlibs)

################# compile schema #####################

set(SCHEMA_COMPILE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/schema/generate)
file(MAKE_DIRECTORY ${SCHEMA_COMPILE_OUTPUT_PATH})
file(GLOB_RECURSE SCHEMAS ./schema/*.capnp)

macro(compile_capn_schema filename)
    add_custom_command(OUTPUT ${SCHEMA_COMPILE_OUTPUT_PATH}/${filename}.h ${SCHEMA_COMPILE_OUTPUT_PATH}/${filename}.c++
        COMMAND $<TARGET_FILE:capnp_tool> compile --output=$<TARGET_FILE:capnpc_cpp>:${SCHEMA_COMPILE_OUTPUT_PATH} schema/${filename} --src-prefix=schema -Ischema
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS $<TARGET_FILE:capnp_tool> $<TARGET_FILE:capnpc_cpp> schema/${filename}
        COMMENT "compiling schema: schema/${filename} ..."
        VERBATIM
    )
endmacro()

set(SCHEMA_OUTPUTS "")
foreach(schema ${SCHEMAS})
    cmake_path(GET schema FILENAME schema_pure_filename)
    compile_capn_schema(${schema_pure_filename})
    set(OUTPUT_HEADER_FILE ${SCHEMA_COMPILE_OUTPUT_PATH}/${schema_pure_filename}.h)
    set(OUTPUT_SRC_FILE ${SCHEMA_COMPILE_OUTPUT_PATH}/${schema_pure_filename}.c++)
    list(APPEND SCHEMA_OUTPUTS ${OUTPUT_HEADER_FILE} ${OUTPUT_SRC_FILE})
endforeach()

################# capn id generator target #####################

add_custom_target(capn_gen_id ${capnp} id)

################ compile game ######################

file(GLOB_RECURSE SRC ./src/*.cpp ./src/*.c)
file(GLOB_RECURSE HEADER ./include/*.hpp ./include/*.h)
file(GLOB_RECURSE ASSETS ./assets/*)

set(GAME_NAME ${PROJECT_NAME})
add_executable(${GAME_NAME})

target_sources(${GAME_NAME} PRIVATE ${SRC} ${HEADER} ${ASSETS} ${SCHEMA_OUTPUTS} ${SCHEMAS})
target_include_directories(${GAME_NAME} PRIVATE include schema)
target_compile_features(${GAME_NAME} PRIVATE cxx_std_17)
target_link_libraries(${GAME_NAME} PRIVATE SDL3::SDL3 spdlog::spdlog stb_image imgui CapnProto::capnp CapnProto::kj CapnProto::capnp-json)