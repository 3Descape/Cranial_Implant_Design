cmake_minimum_required(VERSION 3.15)
project(imgui LANGUAGES CXX VERSION 1.0)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(${PROJECT_NAME} STATIC)
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
		LINKER_LANGUAGE CXX,C
        COMPILE_DEFINITIONS "IMGUI_DEFINE_MATH_OPERATORS"
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

set(IMGUI ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui)

set(IMGUI_SOURCES
	${IMGUI}/imgui_demo.cpp
	${IMGUI}/imgui_draw.cpp
	${IMGUI}/imgui_tables.cpp
	${IMGUI}/imgui_widgets.cpp
	${IMGUI}/imgui.cpp
	${IMGUI}/backends/imgui_impl_glfw.cpp
	${IMGUI}/backends/imgui_impl_opengl3.cpp
)

target_sources(${PROJECT_NAME} PRIVATE ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${IMGUI}
    PUBLIC
        $<BUILD_INTERFACE:${IMGUI}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/imgui>)

find_package(GLFW3 REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)

install(TARGETS ${PROJECT_NAME}
    EXPORT imguiTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES
    ${IMGUI}/imgui.h
    ${IMGUI}/imconfig.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imgui)

install(FILES
    ${IMGUI}/backends/imgui_impl_glfw.h
    ${IMGUI}/backends/imgui_impl_opengl3.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imgui/backends)

install(EXPORT imguiTargets
    FILE imguiTargets.cmake
    NAMESPACE imgui::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imgui)

write_basic_package_version_file(
    imguiConfigVersion.cmake
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES imguiConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/imguiConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imgui)