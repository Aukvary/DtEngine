cmake_minimum_required(VERSION 3.16)
project(Double_Trouble C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(PKG_CONFIG_PATH C:/msys64/clang64/lib/pkgconfig;C:/msys64/mingw64/lib/pkgconfig)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(LINUX TRUE)
endif ()

if (LINUX)
    set(CORE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/builds/linux/core")
    set(EDITOR_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/builds/linux/editor")
    set(GAME_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/builds/linux/game")
else ()
    set(CORE_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/builds/windows/core")
    set(EDITOR_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/builds/windows/editor")
    set(GAME_OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/builds/windows/game")
endif ()

set(RAYLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(RAYLIB_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib")

if (WIN32)
    set(RAYLIB_LIB_FILE "${RAYLIB_LIB_DIR}/windows/libraylib.a")
else ()
    set(RAYLIB_LIB_FILE "${RAYLIB_LIB_DIR}/linux/libraylib.a")
endif ()

set(CORE_SOURCES
        Core/Collections/Vec.c
        Core/Collections/Iterator.c
        Core/Ecs/DtEcsManager.c
        Core/Ecs/Entity.c
        Core/Ecs/EcsPool.c
        Core/Ecs/Systems.c
        Core/Ecs/TagPool.c
        Core/Ecs/ComponentPool.c
        Core/Ecs/ComponentRegister.c
        Core/Ecs/ComponentHandler.c
        Core/Ecs/UpdateHandler.c
        Core/Ecs/UpdateRegister.c
        Core/Ecs/DrawHandler.c
        Core/scheduler/SceneLoader.c
        Core/Collections/RbTree.c
        Core/scheduler/Environment.c
        Core/scheduler/TypeParse.c
)

set(TEST_SOURCES
        CoreTest/main_tests.c
        CoreTest/Tests/TestRegisterAll.c
        CoreTest/Tests/TestCreateRemoveEntity.c
        CoreTest/Tests/TestParentChildrenRelations.c
        CoreTest/Tests/TestFilter.c
        CoreTest/Tests/TestPools.c
        CoreTest/Tests/TestComponentRegister.c
        CoreTest/Tests/TestSystemsRegister.c
)

set(GAME_SOURCES
        GameScripts/DeleteMe.c
)

set(EDITOR_SOURCES
        Editor/DeleteMe.c
)

add_library(DtEngine_Objects OBJECT ${CORE_SOURCES})
target_compile_options(DtEngine_Objects PRIVATE -fPIC)

add_library(DtEngine STATIC $<TARGET_OBJECTS:DtEngine_Objects>)

set_target_properties(DtEngine PROPERTIES
        OUTPUT_NAME "DtEngine"
        ARCHIVE_OUTPUT_DIRECTORY "${CORE_OUTPUT_DIR}"
        POSITION_INDEPENDENT_CODE ON
)

add_executable(DtEngineTest
        ${TEST_SOURCES}
)

set_target_properties(DtEngineTest PROPERTIES
        OUTPUT_NAME "DtEngineTest"
        RUNTIME_OUTPUT_DIRECTORY "${CORE_OUTPUT_DIR}"
)

add_library(GameLibStatic STATIC ${GAME_SOURCES})
add_library(GameLibShared SHARED ${GAME_SOURCES})
target_compile_definitions(GameLibShared PRIVATE -DEDITOR)

if (WIN32)
    set_target_properties(GameLibStatic PROPERTIES
            OUTPUT_NAME "GameLib"
            ARCHIVE_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIR}"
    )
    set_target_properties(GameLibShared PROPERTIES
            OUTPUT_NAME "GameLib"
            RUNTIME_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
            ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
    )
    target_compile_definitions(GameLibShared PRIVATE -DGAMELIB_EXPORTS)
else ()
    set_target_properties(GameLibStatic PROPERTIES
            OUTPUT_NAME "GameLib"
            ARCHIVE_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIR}"
    )
    set_target_properties(GameLibShared PROPERTIES
            OUTPUT_NAME "GameLib"
            LIBRARY_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"

    )
endif ()

add_executable(Editor
        Editor/main_editor.c
        ${EDITOR_SOURCES}
)
target_compile_definitions(Editor PRIVATE -DEDITOR)

set_target_properties(Editor PROPERTIES
        OUTPUT_NAME "Editor"
        RUNTIME_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
)
add_library(EditorLib
        STATIC
        ${EDITOR_SOURCES}
)

target_compile_definitions(EditorLib PRIVATE -DEDITOR)
target_compile_definitions(EditorLib PRIVATE -DEDITOR_LIB)

if (WIN32)
    set_target_properties(EditorLib PROPERTIES
            OUTPUT_NAME "EditorLibStatic"
            ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
    )
else ()
    set_target_properties(EditorLib PROPERTIES
            OUTPUT_NAME "EditorLib"
            ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
    )
endif ()

add_executable(Game
        Game/main_game.c
        ${GAME_SOURCES}
)

set_target_properties(Game PROPERTIES
        OUTPUT_NAME "Game"
        RUNTIME_OUTPUT_DIRECTORY "${GAME_OUTPUT_DIR}"
)

# ============ Include директории ============
foreach (TARGET DtEngine_Objects DtEngine DtEngineTest GameLibStatic GameLibShared Editor Game)
    target_include_directories(${TARGET} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/Core"
    )
endforeach ()

foreach (TARGET GameLibShared Editor)
    target_include_directories(${TARGET} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/Editor"
    )
endforeach ()

target_include_directories(Game PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Game"
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CJSON REQUIRED libcjson)
pkg_check_modules(RAYLIB REQUIRED raylib)

foreach (TARGET DtEngine DtEngineTest GameLibStatic GameLibShared EditorLib Editor Game)
    if (TARGET ${TARGET})
        target_include_directories(${TARGET} PRIVATE
                ${CJSON_INCLUDE_DIRS}
                ${RAYLIB_INCLUDE_DIRS}
        )

        target_link_libraries(${TARGET} PRIVATE
                ${CJSON_LINK_LIBRARIES}
                ${RAYLIB_LINK_LIBRARIES}
        )

        if (WIN32)
            target_link_libraries(${TARGET} PRIVATE
                    winmm
                    gdi32
            )
        else ()
            target_link_libraries(${TARGET} PRIVATE
                    m
            )
        endif ()
    endif ()
endforeach ()

if (TARGET Editor)
    target_compile_definitions(Editor PRIVATE
            -DRAYLIB_NUKLEAR_IMPLEMENTATION
    )

    target_include_directories(Editor PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/Editor
    )
endif ()


# ============ Линковка зависимостей ============
foreach (TARGET DtEngineTest GameLibStatic GameLibShared Editor EditorLib)
    target_link_libraries(${TARGET} PRIVATE $<TARGET_OBJECTS:DtEngine_Objects>)
endforeach ()
target_link_libraries(GameLibShared PRIVATE EditorLib)
target_link_libraries(Game PUBLIC GameLibStatic)