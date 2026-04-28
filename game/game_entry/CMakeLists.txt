if (ANDROID)
    add_library(${GAME_NAME} SHARED)
else()
    add_executable(${GAME_NAME})
endif()

file(GLOB_RECURSE GAME_SCRIPTS ../scripts/client/*.luau)

target_sources(${GAME_NAME} PRIVATE
    game_main.cpp
    ${GAME_SCRIPTS}
    ${SCHEMA_SCRIPT_HINT_FILES}
    ${LUAU_TYPES_HINT_FILES})
target_link_libraries(${GAME_NAME} PRIVATE ${CLIENT_NAME})


