set(KDTOOLBARS_SRC
    toolbar.cpp
    toolbar.h
    toolbarseparator.cpp
    toolbarseparator.h
    toolbarlayout.cpp
    toolbarlayout.h
    toolbartraylayout.cpp
    toolbartraylayout.h
    toolbarcontainerlayout.cpp
    toolbarcontainerlayout.h
    mainwindow.cpp
    mainwindow.h
)

add_library(kdtoolbars ${KDTOOLBARS_SRC})

target_link_libraries(kdtoolbars
PUBLIC
    Qt5::Widgets
    Qt5::WidgetsPrivate
)

target_include_directories(kdtoolbars PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
