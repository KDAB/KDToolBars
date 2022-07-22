find_package(Qt5 COMPONENTS Test REQUIRED)

add_executable(tst_toolbar
    tst_toolbar.cpp
)

target_link_libraries(tst_toolbar
PRIVATE
    kdtoolbars
    Qt5::Widgets
    Qt5::Test
)
