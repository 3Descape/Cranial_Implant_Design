cmake_minimum_required(VERSION 3.15)
project(nrrd LANGUAGES CXX VERSION 1.0)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(${PROJECT_NAME} INTERFACE)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 11
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)

set(NRRD ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/nrrd)

# target_sources(${PROJECT_NAME} INTERFACE
#     ${NRRD}/NRRD/nrrd_image_view.hxx
# 	${NRRD}/NRRD/nrrd_image.hxx
# 	${NRRD}/NRRD/nrrd_metaio.hxx
# 	${NRRD}/NRRD/nrrd.hxx
#     ${NRRD}/GetSet/BaseTypes.hxx
# 	${NRRD}/GetSet/StringConversion.hxx
# 	${NRRD}/GetSet/StringType.hxx
# 	${NRRD}/GetSet/StringUtil.hxx
# )

# set(NRRD_SOURCES)
# get_target_property(NRRD_SOURCES ${PROJECT_NAME} INTERFACE_SOURCES)
# source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${NRRD_SOURCES})
# add_custom_target("${PROJECT_NAME}_INTERFACE" SOURCES ${NRRD_SOURCES})

target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${NRRD}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

find_package(Boost REQUIRED)
target_link_libraries(${PROJECT_NAME} INTERFACE Boost::boost)

install(TARGETS ${PROJECT_NAME}
        EXPORT nrrdTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY ${NRRD}/GetSet ${NRRD}/NRRD DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT nrrdTargets
    FILE nrrdTargets.cmake
    NAMESPACE nrrd::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nrrd)

write_basic_package_version_file(
    nrrdConfigVersion.cmake
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion)

install(FILES nrrdConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/nrrdConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/nrrd)