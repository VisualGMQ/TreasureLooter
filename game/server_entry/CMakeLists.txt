add_executable(${GAME_SERVER_NAME})

file(GLOB_RECURSE SERVER_SCRIPTS ../scripts/server/*.lua)

target_sources(${GAME_SERVER_NAME} PRIVATE
    server_main.cpp
    ${SERVER_SCRIPTS}
    ${SCHEMA_SCRIPT_HINT_FILES}
    ${LUA_TYPES_HINT_FILES})
target_link_libraries(${GAME_SERVER_NAME} PRIVATE ${SERVER_MODULE_NAME})


