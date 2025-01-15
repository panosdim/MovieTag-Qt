cmake_minimum_required(VERSION 3.19)
project(MovieTag LANGUAGES CXX)

set(CMAKE_AUTORCC ON)

# Add Network to the components list
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

# Add pkg-config support
find_package(PkgConfig REQUIRED)

# Find TagLib using pkg-config
pkg_check_modules(TAGLIB REQUIRED IMPORTED_TARGET taglib)

qt_standard_project_setup()

# Path to the config file in the source directory
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/config.ini")
# Path to the build directory where the file will be copied
set(DEST_DIR "${CMAKE_BINARY_DIR}")
# Copy the config file into the build directory
configure_file(${CONFIG_FILE} ${DEST_DIR}/config.ini COPYONLY)

qt_add_executable(MovieTag
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    movieitemwidget.h movieitemwidget.cpp
    resources.qrc
    tmdbclient.h tmdbclient.cpp
    mediatagwriter.h mediatagwriter.cpp
)

target_link_libraries(MovieTag
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Network
        PkgConfig::TAGLIB
)

# Enable C++17 (or later) if necessary
set_target_properties(MovieTag PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

include(GNUInstallDirs)
install(TARGETS MovieTag
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET MovieTag
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
