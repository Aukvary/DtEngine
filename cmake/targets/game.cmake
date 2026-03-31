set(GAME_SOURCES
        GameScripts/DeleteMe.c
        GameScripts/DrawSpriteSystem.c
        GameScripts/Sprite.c
)

add_library(GameLibStatic STATIC ${GAME_SOURCES})
add_library(GameLibShared SHARED ${GAME_SOURCES})

set_target_properties(GameLibStatic PROPERTIES
        OUTPUT_NAME "GameLib"
        ARCHIVE_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIR}"
)
set_target_properties(GameLibShared PROPERTIES
        OUTPUT_NAME "GameLib"
        LIBRARY_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
)

target_compile_definitions(GameLibShared PRIVATE -DEDITOR)
target_include_directories(GameLibShared PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Editor")

add_executable(Game Game/main_game.c ${GAME_SOURCES})
set_target_properties(Game PROPERTIES
        OUTPUT_NAME "Game"
        RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIR}"
)
target_include_directories(Game PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Editor")

target_include_directories(GameLibStatic PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Core"
)
target_include_directories(GameLibShared PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Core"
)
target_include_directories(Game PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Core"
        "${CMAKE_CURRENT_SOURCE_DIR}/Game"
)