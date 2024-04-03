# KDToolBars

`KDToolBars` is a Qt toolbar library written by KDAB, suitable for replacing `QToolBar` and
implementing advanced features missing in Qt.

## Features

`KDToolBars` provides several advanced capabilities beyond what `QToolbar` provides, including:

### User Customization

- While in customization mode, `QAction`s can be dragged and dropped between toolbars
- Custom toolbars can be created and deleted by the user
- Toolbars can be reset back to their original `QAction` configuration
- Customizations can be automatically saved and restored

### Advanced Toolbar Floating

- Toolbar displays a title bar while floating
- Toolbar can be resized while floating
- Double click on toolbar title bar to redock

## Requirements

| Requirement  | Version                     | Needed                          |
| ------------ | --------------------------- | ------------------------------- |
| CMake        | minimum 3.15                | Always                          |
| C++ compiler | Qt-supported, C++17 minimum | Always                          |
| Qt           | 5 or 6                      | Always                          |
| Doxygen      | minimum 1.9.1               | Only for building documentation |

## Build & Install

`KDToolBars` uses CMake for its build system.

### CMake Options

The most basic way to build & install `KDToolBars` is like so:

```bash
$ cmake -S . -B build
$ cmake --build build
$ cmake --install build
```

> _Note: you will likely need `sudo` for the install step on Linux._

There are several options that can be configured in the first step as well:

| Option                      | Description                 | Default Value   |
| --------------------------- | --------------------------- | --------------- |
| `KDToolBars_QT6`            | Build against Qt 6          | `OFF`           |
| `KDToolBars_TESTS`          | Build tests                 | `ON`            |
| `KDToolBars_STATIC`         | Build statically            | `OFF`           |
| `KDToolBars_DOCS`           | Build the API documentation | `OFF`           |
| `KDToolBars_EXAMPLES`       | Build examples              | `ON`            |
| `KDToolBars_DEVELOPER_MODE` | Developer Mode              | `OFF`           |

An example that builds documentation might look like:

```bash
$ cmake -S . -B build -DKDToolBars_DOCS=ON
$ cmake --build build
```

It also is not a bad idea to shadow build, i.e. specify `../build-KDToolBars`
as the build directory rather than `build`:

```bash
$ cmake -S . -B ../build-KDToolBars -DKDToolBars_DOCS=ON
$ cmake --build ../build-KDToolBars
```

## Usage

Make sure to install `KDToolBars`. In a CMake project, use `find_package` and
`target_link_libraries` to find and link the library:

```cmake
find_package(KDToolBars)

# additional cmake commands
# ...
# ...

target_link_libraries(my_target KDAB::kdtoolbars)
```

Now you can get started with something like this:

```cpp
#include <kdtoolbars/mainwindow.h>

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    KDToolBars::MainWindow w;
    w.show();
    return a.exec();
}

```

## License

KDToolBars is © Klarälvdalens Datakonsult AB (KDAB) and is licensed according
to the terms of the [GPL 2.0](LICENSES/GPL-2.0-only.txt) or [GPL 3.0](LICENSES/GPL-3.0-only.txt).

Contact KDAB at <info@kdab.com> to inquire about commercial licensing.

## About KDAB

KDToolBars is supported and maintained by Klarälvdalens Datakonsult AB (KDAB).

The KDAB Group is the global No.1 software consultancy for Qt, C++ and OpenGL applications across
desktop, embedded and mobile platforms.

The KDAB Group provides consulting and mentoring for developing Qt applications from scratch and in
porting from all popular and legacy frameworks to Qt. We continue to help develop parts of Qt and
are one of the major contributors to the Qt Project. We can give advanced or standard trainings
anywhere around the globe on Qt as well as C++, OpenGL, 3D and more.

Please visit <https://www.kdab.com> to meet the people who write code like this.
