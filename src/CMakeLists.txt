# This file is part of KDToolBars.
#
# SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
#
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
#
# Contact KDAB at <info@kdab.com> for commercial licensing options.
#
set(KDTOOLBARS_SOURCES
    toolbar.cpp
    toolbarseparator.cpp
    toolbarseparator.h
    toolbarlayout.cpp
    toolbarlayout.h
    toolbartraylayout.cpp
    toolbartraylayout.h
    toolbarcontainerlayout.cpp
    toolbarcontainerlayout.h
    mainwindow.cpp
    toolbarcustomizationdialog.cpp
    toolbarcustomizationdialog.h
)

set(KDTOOLBARS_INSTALLABLE_HEADERS toolbar.h mainwindow.h kdtoolbars_export.h)

set(KDTOOLBARS_RESOURCES kdtoolbars_resources.qrc)

add_library(
    kdtoolbars ${KDTOOLBARS_LIBRARY_MODE} ${KDTOOLBARS_SOURCES} ${KDTOOLBARS_INSTALLABLE_HEADERS}
               ${KDTOOLBARS_RESOURCES}
)

target_link_libraries(
    kdtoolbars
    PUBLIC Qt::Widgets
    PRIVATE Qt::WidgetsPrivate
)

target_include_directories(
    kdtoolbars PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/fwd_headers>
                      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(KDToolBars_STATIC)
    target_compile_definitions(kdtoolbars PUBLIC KDTOOLBARS_STATICLIB)
else()
    target_compile_definitions(kdtoolbars PRIVATE BUILDING_KDTOOLBARS_LIBRARY)
endif()

# Enable -Werror
if(KDToolBars_WERROR)
    if(NOT MSVC OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(kdtoolbars PRIVATE -Werror -Wundef -Wno-error=deprecated-declarations)
    else()
        target_compile_options(kdtoolbars PRIVATE /WX)
        add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
    endif()
endif()

foreach(file ${KDTOOLBARS_INSTALLABLE_HEADERS})
    get_filename_component(dir ${file} DIRECTORY)
    install(FILES ${file} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/kdtoolbars/${dir})
endforeach()

install(
    TARGETS kdtoolbars
    EXPORT kdtoolbarsTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT kdtoolbarsTargets
    FILE KDToolBarsTargets.cmake
    NAMESPACE KDAB::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDToolBars
)

include(CMakePackageConfigHelpers)

configure_package_config_file(
    KDToolBarsConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/KDToolBarsConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDToolBars
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/KDToolBarsConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/KDToolBarsConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/KDToolBarsConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/KDToolBars
)
