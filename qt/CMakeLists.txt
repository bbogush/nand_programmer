cmake_minimum_required(VERSION 3.17)
project(nand_programmer)

# Set the CMAKE_PREFIX_PATH to find Qt5
set(CMAKE_PREFIX_PATH "/opt/local/libexec/qt5/lib/cmake/Qt5" ${CMAKE_PREFIX_PATH})

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)

# Find Qt5 Widgets
find_package(Qt5 REQUIRED COMPONENTS Widgets SerialPort)

# Enable Qt's AUTOMOC (meta-object compiler) feature
set(CMAKE_AUTOMOC ON)
# Enable AUTOMOC (to process Qt's meta-object code) and AUTOUIC (to process .ui files)
set(CMAKE_AUTOUIC ON)

# Include directories (Qt headers should be found automatically)
include_directories(.)

# Set Boost root directory
set(BOOST_ROOT "/opt/local")
set(BOOST_INCLUDEDIR "/opt/local/include")
set(BOOST_LIBRARYDIR "/opt/local/lib")
find_package(Boost REQUIRED COMPONENTS system thread)

# If you use Boost headers
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR} ${Qt5Core_INCLUDE_DIRS} ${Qt5SerialPort_INCLUDE_DIRS})
file(GLOB APP_SOURCES ${CMAKE_SOURCE_DIR}/*.c ${CMAKE_SOURCE_DIR}/*.cpp ${CMAKE_SOURCE_DIR}/*.ui)
if (APPLE)
    add_executable(nand_programmer
            MACOSX_BUNDLE  # Specify this flag to create a macOS app bundle
            ${APP_SOURCES}
            )
else ()
    add_executable(nand_programmer
            ${APP_SOURCES}
            )
endif ()
# Copy CSV files into the build directory
file(COPY ${CMAKE_SOURCE_DIR}/nando_parallel_chip_db.csv DESTINATION ${CMAKE_BINARY_DIR}/)
file(COPY ${CMAKE_SOURCE_DIR}/nando_spi_chip_db.csv DESTINATION ${CMAKE_BINARY_DIR}/)

# Link with Qt5 Widgets
target_link_libraries(nand_programmer Qt5::Widgets Qt5::SerialPort Boost::system Boost::thread)


if (APPLE)
    set_target_properties(nand_programmer PROPERTIES
            MACOSX_BUNDLE_ICON_FILE app_icon.icns  # Optional: Set app icon (replace with your .icns file)
            MACOSX_BUNDLE_GUI_IDENTIFIER com.example.nandprogrammer
            MACOSX_BUNDLE_BUNDLE_VERSION 1.0
            MACOSX_BUNDLE_SHORT_VERSION_STRING 1.0.0
            )
    set(MACDEPLOYQT_PATH "/opt/local/libexec/qt5/bin/macdeployqt")
    # Copy the file to the MacOS directory in the .app bundle
    file(COPY ${CMAKE_SOURCE_DIR}/nando_parallel_chip_db.csv
            DESTINATION ${CMAKE_BINARY_DIR}/nand_programmer.app/Contents/MacOS)

    # Copy the file to the MacOS directory in the .app bundle
    file(COPY ${CMAKE_SOURCE_DIR}/nando_spi_chip_db.csv
            DESTINATION ${CMAKE_BINARY_DIR}/nand_programmer.app/Contents/MacOS)


    # Set the path to your icon file
    set(MACOS_APP_ICON ${CMAKE_SOURCE_DIR}/img/app_icon.icns)

    file(COPY ${MACOS_APP_ICON} DESTINATION ${CMAKE_BINARY_DIR}/nand_programmer.app/Contents/Resources)


    # Add the post-build command to run macdeployqt
    add_custom_command(TARGET nand_programmer POST_BUILD
            COMMAND ${MACDEPLOYQT_PATH} ${CMAKE_BINARY_DIR}/nand_programmer.app
            COMMENT "Running macdeployqt to bundle Qt libraries"
            )
endif ()
