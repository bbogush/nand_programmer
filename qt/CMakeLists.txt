cmake_minimum_required(VERSION 3.15)
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
include_directories(${Qt5Core_INCLUDE_DIRS} ${Qt5SerialPort_INCLUDE_DIRS})

if (APPLE)
add_executable(nand_programmer
        MACOSX_BUNDLE  # Specify this flag to create a macOS app bundle
        about_dialog.cpp
        about_dialog.ui
        about_dialog.h
        chip_db.cpp
        chip_db.h
        chip_info.cpp
        chip_info.h
        clickcombobox.cpp
        clickcombobox.h
        cmd.h
        dataviewer.cpp
        dataviewer.h
        err.cpp
        err.h
        firmware_update_dialog.cpp
        firmware_update_dialog.h
        firmware_update_dialog.ui
        logger.cpp
        logger.h
        main.cpp
        main_window.cpp
        main_window.h
        parallel_chip_db.cpp
        parallel_chip_db.h
        parallel_chip_db_dialog.cpp
        parallel_chip_db_dialog.h
        parallel_chip_db_table_model.cpp
        parallel_chip_db_table_model.h
        parallel_chip_info.cpp
        parallel_chip_info.h
        programmer.cpp
        programmer.h
        reader.cpp
        reader.h
        serial_port.cpp
        serial_port.h
        settings.h
        settings_programmer_dialog.cpp
        settings_programmer_dialog.h
        settings_programmer_dialog.ui
        spi_chip_db.cpp
        spi_chip_db.h
        spi_chip_db_dialog.cpp
        spi_chip_db_dialog.h
        spi_chip_db_table_model.cpp
        spi_chip_db_table_model.h
        spi_chip_info.cpp
        spi_chip_info.h
        sync_buffer.h
        version.h
        writer.cpp
        writer.h
        )
else()
    add_executable(nand_programmer
            about_dialog.cpp
            about_dialog.ui
            about_dialog.h
            chip_db.cpp
            chip_db.h
            chip_info.cpp
            chip_info.h
            clickcombobox.cpp
            clickcombobox.h
            cmd.h
            dataviewer.cpp
            dataviewer.h
            err.cpp
            err.h
            firmware_update_dialog.cpp
            firmware_update_dialog.h
            firmware_update_dialog.ui
            logger.cpp
            logger.h
            main.cpp
            main_window.cpp
            main_window.h
            parallel_chip_db.cpp
            parallel_chip_db.h
            parallel_chip_db_dialog.cpp
            parallel_chip_db_dialog.h
            parallel_chip_db_table_model.cpp
            parallel_chip_db_table_model.h
            parallel_chip_info.cpp
            parallel_chip_info.h
            programmer.cpp
            programmer.h
            reader.cpp
            reader.h
            serial_port.cpp
            serial_port.h
            settings.h
            settings_programmer_dialog.cpp
            settings_programmer_dialog.h
            settings_programmer_dialog.ui
            spi_chip_db.cpp
            spi_chip_db.h
            spi_chip_db_dialog.cpp
            spi_chip_db_dialog.h
            spi_chip_db_table_model.cpp
            spi_chip_db_table_model.h
            spi_chip_info.cpp
            spi_chip_info.h
            sync_buffer.h
            version.h
            writer.cpp
            writer.h
            )
endif()
# Copy CSV files into the build directory
file(COPY ${CMAKE_SOURCE_DIR}/nando_parallel_chip_db.csv DESTINATION ${CMAKE_BINARY_DIR}/)
file(COPY ${CMAKE_SOURCE_DIR}/nando_spi_chip_db.csv DESTINATION ${CMAKE_BINARY_DIR}/)

# Link with Qt5 Widgets
target_link_libraries(nand_programmer Qt5::Widgets Qt5::SerialPort Boost::system Boost::thread)


if(APPLE)
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


#    # Set the path to your icon file
#    set(MACOS_ICON ${CMAKE_SOURCE_DIR}/img/app_icon.ico)
#
#    # Set the .app bundle properties
#    set(MACOS_APP_ICON ${MACOS_ICON})
#
#    # Add the icon to your app bundle
#    set_target_properties(nand_programmer PROPERTIES
#            MACOSX_BUNDLE_ICON_FILE app_icon.ico
#            )

    # Copy the icon to the .app bundle directory (inside Contents/Resources)
#    file(COPY ${MACOS_APP_ICON} DESTINATION ${CMAKE_BINARY_DIR}/nand_programmer.app/Contents/Resources)



    # Add the post-build command to run macdeployqt
    add_custom_command(TARGET nand_programmer POST_BUILD
            COMMAND ${MACDEPLOYQT_PATH} ${CMAKE_BINARY_DIR}/nand_programmer.app
            COMMENT "Running macdeployqt to bundle Qt libraries"
            )
endif()
