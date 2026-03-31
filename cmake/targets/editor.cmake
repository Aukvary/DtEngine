set(EDITOR_SOURCES
        Editor/GameLibLink/GameLibLoader.c
        Editor/UI/Systems.c
        Editor/UI/EntityComponentsPanel.c
        Editor/UI/Inspecor.c
        Editor/UI/ManagePanel.c
        Editor/UI/MessagePanel.c
        Editor/UI/EditorApi.c
        Editor/GameLibLink/GameSystemController.c
)

add_executable(Editor Editor/main_editor.c ${EDITOR_SOURCES})
target_compile_definitions(Editor PRIVATE -DEDITOR)
set_target_properties(Editor PROPERTIES
        OUTPUT_NAME "Editor"
        RUNTIME_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
)

add_library(EditorLib STATIC ${EDITOR_SOURCES})
target_compile_definitions(EditorLib PRIVATE -DEDITOR -DEDITOR_LIB)
set_target_properties(EditorLib PROPERTIES
        OUTPUT_NAME "EditorLib"
        ARCHIVE_OUTPUT_DIRECTORY "${EDITOR_OUTPUT_DIR}"
)

target_include_directories(Editor PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Core"
        "${CMAKE_CURRENT_SOURCE_DIR}/Editor"
)
target_include_directories(EditorLib PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Core"
        "${CMAKE_CURRENT_SOURCE_DIR}/Editor"
)

if (TARGET Editor)
    target_compile_definitions(Editor PRIVATE
            -DRAYLIB_NUKLEAR_INCLUDE_DEFAULT_FONT
    )
#    if (LINUX)
#        target_link_options(Editor PUBLIC "-rdynamic")
#    endif ()
endif ()