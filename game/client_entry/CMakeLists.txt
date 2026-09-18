if (ANDROID)
    add_library(${GAME_CLIENT_NAME} SHARED)
else()
    add_executable(${GAME_CLIENT_NAME})
endif()

file(GLOB_RECURSE GAME_SCRIPTS ../scripts/client/*.lua)

target_sources(${GAME_CLIENT_NAME} PRIVATE
    client_main.cpp
    ${GAME_SCRIPTS}
    ${SCHEMA_SCRIPT_HINT_FILES}
    ${LUA_TYPES_HINT_FILES})
target_link_libraries(${GAME_CLIENT_NAME} PRIVATE ${CLIENT_MODULE_NAME})


