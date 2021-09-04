cmake_minimum_required(VERSION 3.0)

project(IMGUI LANGUAGES CXX)

set(CMAKE_DEBUG_POSTFIX "_debug")

set(IMGUI ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

set(IMGUI_SOURCES
	${IMGUI}/imgui.h
	${IMGUI}/imgui.cpp
	${IMGUI}/imgui_demo.cpp
	${IMGUI}/imgui_draw.cpp
	${IMGUI}/imgui_internal.h
	${IMGUI}/imgui_tables.cpp
	${IMGUI}/imgui_widgets.cpp
	${IMGUI}/imstb_rectpack.h
	${IMGUI}/imstb_textedit.h
	${IMGUI}/imstb_truetype.h
	${IMGUI}/backends/imgui_impl_glfw.h
	${IMGUI}/backends/imgui_impl_glfw.cpp
	${IMGUI}/backends/imgui_impl_opengl3.h
	${IMGUI}/backends/imgui_impl_opengl3.cpp
)

source_group(TREE ${IMGUI} FILES ${IMGUI_SOURCES})

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
		LINKER_LANGUAGE CXX,C
        COMPILE_DEFINITIONS "IMGUI_DEFINE_MATH_OPERATORS"
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

find_package(GLFW3 REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/imgui
)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw)