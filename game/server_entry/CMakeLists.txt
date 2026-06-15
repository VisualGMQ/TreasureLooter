add_executable(${PROJECT_NAME}-Server)

file(GLOB_RECURSE SERVER_SCRIPTS ../scripts/server/*.luau)

target_sources(${PROJECT_NAME}-Server PRIVATE
    server_main.cpp
    ${SERVER_SCRIPTS}
    ${SCHEMA_SCRIPT_HINT_FILES}
    ${LUAU_TYPES_HINT_FILES})
target_link_libraries(${PROJECT_NAME}-Server PRIVATE ${SERVER_NAME})


