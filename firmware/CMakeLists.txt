cmake_minimum_required(VERSION 3.15)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
# Disable linking with the standard C library (libc)
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")
set(CMAKE_C_FLAGS "-nostdlib")
set(CMAKE_CXX_FLAGS "-nostdlib")
# Define the base toolchain path
set(TOOLCHAIN "/opt/local/gcc-arm-none-eabi-5_4-2016q3/bin")

# Set the compilers and tools using the TOOLCHAIN variable
set(CMAKE_C_COMPILER "${TOOLCHAIN}/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN}/arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN}/arm-none-eabi-gcc")
set(CMAKE_AR "${TOOLCHAIN}/arm-none-eabi-ar")
set(CMAKE_OBJCOPY "${TOOLCHAIN}/arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN}/arm-none-eabi-objdump")
set(SIZE "${TOOLCHAIN}/arm-none-eabi-size")


project(nando_fw)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

# Toolchain and directories
set(BOOT_DIR bootloader)
set(APP_DIR programmer)
set(OBJ_DIR ${CMAKE_BINARY_DIR}/obj)
set(GEN_NAME nando_fw)

set(MCPU cortex-m3)

# Flags
#set(SPL_FLAGS -DSTM32F10X_HD -DCONFIG_BOOTLOADER)
set(CFLAGS -g -Wall -Werror -Og)
#list(APPEND CFLAGS ${INCLUDES} -MMD -MP -ffunction-sections -fdata-sections -mcpu=cortex-m3 -mthumb ${SPL_FLAGS})

# Common compile options
add_compile_options(-mcpu=${MCPU} -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# Uncomment for hardware floating point
# add_compile_definitions(ARM_MATH_CM3;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
# add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
# add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)


# Uncomment for software floating point
# add_compile_options(-mfloat-abi=soft)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

# Set the library path for SPL (Standard Peripheral Library)
set(LIB_DIR "${CMAKE_SOURCE_DIR}/libs")

# Define the paths for SPL headers and configuration file
set(SPL_PATH "${LIB_DIR}/spl")
set(SPL_DEVICE_SUPPORT_PATH "${SPL_PATH}/CMSIS/CM3/DeviceSupport/ST/STM32F10x")
set(SPL_CORE_SUPPORT "${SPL_PATH}/CMSIS/CM3/CoreSupport")
set(SPL_PERIPH_PATH "${SPL_PATH}/STM32F10x_StdPeriph_Driver")
set(SPL_USB_PATH "${SPL_PATH}/STM32_USB-FS-Device_Driver")
set(SPL_CONFIG_FILE "${SPL_PATH}/stm32f10x_conf.h")

# Additional include directories
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}")
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(USB_DIR "${CMAKE_SOURCE_DIR}/usb_cdc")

# Add all necessary include directories
include_directories(
        ${INCLUDE_DIR}              # Project-specific include directory
        ${SPL_CORE_SUPPORT}         # SPL Core support headers
        ${SPL_DEVICE_SUPPORT_PATH}  # Device support headers (STM32F10x)
        ${SPL_PATH}                 # SPL general headers
        ${SPL_PERIPH_PATH}/inc       # Peripheral driver headers
        ${SPL_USB_PATH}/inc          # USB driver headers
        ${SRC_DIR}                  # Source-specific includes
        ${USB_DIR}                  # USB CDC directory
)

add_definitions(-include ${SPL_CONFIG_FILE})
# Define preprocessor macros for STM32F103VC-based project
add_definitions(
        -DUSE_HAL_DRIVER          # Enable STM32 HAL support
        -DSTM32F103xx             # Target STM32F103 series (includes STM32F103VC)
        -DCORE_CM3                # Targeting ARM Cortex-M3 core
        -DDEBUG                   # Optional: Enable debugging code (use in #ifdef DEBUG)
)


# Define linker scripts
set(LINKER_SCRIPT_BOOT ${CMAKE_SOURCE_DIR}/${BOOT_DIR}/stm32_flash.ld)
set(LINKER_SCRIPT_1 ${CMAKE_SOURCE_DIR}/${APP_DIR}/stm32_flash_1.ld)
set(LINKER_SCRIPT_2 ${CMAKE_SOURCE_DIR}/${APP_DIR}/stm32_flash_2.ld)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=${MCPU} -mthumb -mthumb-interwork)


# Build bootloader
file(GLOB_RECURSE BOOTLOADER_SOURCES ${BOOT_DIR}/*.c ${BOOT_DIR}/*.s)
add_executable(bootloader_fw.elf ${BOOTLOADER_SOURCES})
target_link_options(bootloader_fw.elf PRIVATE -T ${LINKER_SCRIPT_BOOT})
set(HEX_FILE_BOOT ${CMAKE_BINARY_DIR}/bootloader_fw.hex)
set(BIN_FILE_BOOT ${CMAKE_BINARY_DIR}/bootloader_fw.bin)

add_custom_command(TARGET bootloader_fw.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --pad-to 0x08004000 -Oihex $<TARGET_FILE:bootloader_fw.elf> ${HEX_FILE_BOOT}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:bootloader_fw.elf> ${BIN_FILE_BOOT}
        COMMAND ${CMAKE_OBJDUMP} -St $<TARGET_FILE:bootloader_fw.elf> > ${CMAKE_BINARY_DIR}/bootloader_fw.lst
        COMMAND ${SIZE} $<TARGET_FILE:bootloader_fw.elf>
        COMMENT "Building ${HEX_FILE_BOOT}
Building ${BIN_FILE_BOOT}
Generating list file and displaying size"
        )
target_compile_definitions(bootloader_fw.elf PRIVATE STM32F10X_MD)

# Build app1
file(GLOB_RECURSE LIB_SOURCES ${CMAKE_SOURCE_DIR}/libs/*.c ${CMAKE_SOURCE_DIR}/libs/*.s)
file(GLOB_RECURSE APP1_SOURCES ${APP_DIR}/*.c ${APP_DIR}/*.s ${LIB_SOURCES})

add_executable(app_fw_1.elf ${APP1_SOURCES})
target_compile_definitions(app_fw_1.elf PRIVATE STM32F10X_MD)
target_link_options(app_fw_1.elf PRIVATE -T ${LINKER_SCRIPT_1})
set(HEX_FILE_1 ${CMAKE_BINARY_DIR}/app_fw_1.hex)
set(BIN_FILE_1 ${CMAKE_BINARY_DIR}/app_fw_1.bin)

add_custom_command(TARGET app_fw_1.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --pad-to 0x08022000 -Oihex $<TARGET_FILE:app_fw_1.elf> ${HEX_FILE_1}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:app_fw_1.elf> ${BIN_FILE_1}
        COMMAND ${CMAKE_OBJDUMP} -St $<TARGET_FILE:app_fw_1.elf> > ${CMAKE_BINARY_DIR}/app_fw_1.lst
        COMMAND ${SIZE} $<TARGET_FILE:app_fw_1.elf>
        COMMENT "Building ${HEX_FILE_1}
Building ${BIN_FILE_1}
Generating list file and displaying size"
        )

# Build app2
add_executable(app_fw_2.elf ${APP1_SOURCES})
target_compile_definitions(app_fw_2.elf PRIVATE STM32F10X_MD)
target_link_options(app_fw_2.elf PRIVATE -T ${LINKER_SCRIPT_2})
set(HEX_FILE_2 ${CMAKE_BINARY_DIR}/app_fw_2.hex)
set(BIN_FILE_2 ${CMAKE_BINARY_DIR}/app_fw_2.bin)

add_custom_command(TARGET app_fw_2.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --pad-to 0x08040000 -Oihex $<TARGET_FILE:app_fw_2.elf> ${HEX_FILE_2}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:app_fw_2.elf> ${BIN_FILE_2}
        COMMAND ${CMAKE_OBJDUMP} -St $<TARGET_FILE:app_fw_2.elf> > ${CMAKE_BINARY_DIR}/app_fw_2.lst
        COMMAND ${SIZE} $<TARGET_FILE:app_fw_2.elf>
        COMMENT "Building ${HEX_FILE_2}
Building ${BIN_FILE_2}
Generating list file and displaying size"
        )

#add_executable(${GEN_NAME} ${APP1_SOURCES})
#target_compile_definitions(${GEN_NAME} PRIVATE STM32F10X_MD)
#add_custom_command(
#        OUTPUT ${GEN_NAME}.elf
#        DEPENDS bootloader_fw.elf app_fw_1.elf app_fw_2.elf
#        COMMAND ${CMAKE_OBJCOPY} --input-target=elf32-little --output-target=elf32-little ${CMAKE_BINARY_DIR}/bootloader_fw.elf ${GEN_NAME}.elf
#        COMMAND ${CMAKE_OBJCOPY} --input-target=elf32-little --output-target=elf32-little ${CMAKE_BINARY_DIR}/app_fw_1.elf app_fw_1.tmp
#        COMMAND ${CMAKE_OBJCOPY} --input-target=elf32-little --output-target=elf32-little ${CMAKE_BINARY_DIR}/app_fw_2.elf app_fw_2.tmp
#        COMMAND ${CMAKE_COMMAND} -E append app_fw_1.tmp ${GEN_NAME}.elf
#        COMMAND ${CMAKE_COMMAND} -E append app_fw_2.tmp ${GEN_NAME}.elf
#        COMMAND ${SIZE} ${GEN_NAME}.elf
#        COMMENT "Combining ELF files into ${GEN_NAME}.elf"

add_custom_target(
        ${GEN_NAME} ALL
        DEPENDS bootloader_fw.hex app_fw_1.hex app_fw_2.hex
#        COMMAND ${CMAKE_COMMAND} -E copy bootloader_fw.hex ${OBJ_DIR}/bootloader_fw.hex
#        COMMAND ${CMAKE_COMMAND} -E copy app_fw_1.hex ${OBJ_DIR}
#        COMMAND ${CMAKE_COMMAND} -E copy app_fw_2.hex ${OBJ_DIR}
        COMMAND sed -i '' '$$d' bootloader_fw.hex
        COMMAND sed -i '' '$$d' app_fw_1.hex
        COMMAND cat bootloader_fw.hex app_fw_1.hex app_fw_2.hex > ${GEN_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -I ihex -O elf32-littlearm ${GEN_NAME}.hex ${GEN_NAME}.elf
        COMMAND ${CMAKE_OBJCOPY} -I ihex -O binary ${GEN_NAME}.hex ${GEN_NAME}.bin
        COMMAND ${SIZE} ${GEN_NAME}.elf
        COMMENT "Combining HEX files into ${GEN_NAME}.hex and generating ${GEN_NAME}.bin and ${GEN_NAME}.elf"
        )

## Step 3: Optionally copy the combined ELF file or perform other operations
#add_custom_command(TARGET ${GEN_NAME} POST_BUILD
#        COMMAND ${CMAKE_OBJCOPY} -O binary ${GEN_NAME}.elf ${GEN_NAME}.bin
#        COMMENT "Copying combined ELF to the binary directory and generating .bin file"
#        )

# Erase and program target
add_custom_target(erase
        COMMAND st-flash erase
        COMMENT "Erasing the flash memory"
        )

add_custom_target(program
        COMMAND st-flash write ${GEN_NAME}.bin 0x08000000
        COMMENT "Programming the flash memory"
        )

# Clean all build artifacts
add_custom_target(clean_all
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
        COMMAND ${CMAKE_COMMAND} -E rm -rf
        COMMENT "Cleaning all build artifacts"
        )

# Ensure dependencies
add_dependencies(app_fw_1.elf bootloader_fw.elf)
add_dependencies(app_fw_2.elf app_fw_1.elf)
add_dependencies(${GEN_NAME} app_fw_2.elf)
