target_sources(${SERVER_NAME} PRIVATE
    server_main.cpp
    ${GAME_SCRIPTS}
    ${SCHEMA_SCRIPT_HINT_FILES}
    ${LUAU_TYPES_HINT_FILES})
target_link_libraries(${SERVER_NAME} PRIVATE ${COMMON_NAME})


