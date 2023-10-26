cmake_minimum_required(VERSION 3.15)

set(CMAKE_POLICY_DEFAULT_CMP0148 OLD)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include(${CMAKE_CURRENT_SOURCE_DIR}/../src/glad/cmake/CMakeLists.txt)
glad_add_library(glad STATIC LOCATION ${CMAKE_INSTALL_PREFIX} API gl:core=4.6)

install(TARGETS glad
    EXPORT gladTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT gladTargets
    FILE gladTargets.cmake
    NAMESPACE glad::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/glad)

write_basic_package_version_file(
    gladConfigVersion.cmake
    VERSION 1.0
    COMPATIBILITY AnyNewerVersion)

install(FILES gladConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/gladConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/glad)