set(CORE_SOURCES
        Core/Collections/Vec.c
        Core/Collections/Iterator.c
        Core/Ecs/DtEcsManager.c
        Core/Ecs/Entity.c
        Core/Ecs/EcsPool.c
        Core/Ecs/Systems.c
        Core/Ecs/TagPool.c
        Core/Ecs/ComponentPool.c
        Core/Components/Components.c
        Core/Ecs/ComponentHandler.c
        Core/Ecs/UpdateHandler.c
        Core/Ecs/UpdateRegister.c
        Core/Ecs/DrawHandler.c
        Core/scheduler/SceneLoader.c
        Core/Collections/RbTree.c
        Core/scheduler/Environment.c
        Core/scheduler/TypeParse.c
        Core/Components/DtTransform2D.c
)

add_library(DtEngine_Objects OBJECT ${CORE_SOURCES})
target_compile_options(DtEngine_Objects PRIVATE -fPIC)

add_library(DtEngine STATIC $<TARGET_OBJECTS:DtEngine_Objects>)
set_target_properties(DtEngine PROPERTIES
        OUTPUT_NAME "DtEngine"
        ARCHIVE_OUTPUT_DIRECTORY "${CORE_OUTPUT_DIR}"
        POSITION_INDEPENDENT_CODE ON
)

target_include_directories(DtEngine_Objects PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Core"
)
target_include_directories(DtEngine PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Core"
)