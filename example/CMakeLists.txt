add_executable(example
    main.cpp
    test.qrc
)

target_link_libraries(example
PRIVATE
    kdtoolbars
)
