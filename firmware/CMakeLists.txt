cmake_minimum_required(VERSION 3.17)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
# Disable linking with the standard C library (libc)
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")
set(CMAKE_C_FLAGS "-nostdlib")
set(CMAKE_CXX_FLAGS "-nostdlib")
# Define the base toolchain path
set(TOOLCHAIN "/opt/local/gcc-arm-none-eabi-4_9-2015q1/bin")

# Set the compilers and tools using the TOOLCHAIN variable
set(CMAKE_C_COMPILER "${TOOLCHAIN}/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN}/arm-none-eabi-g++")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN}/arm-none-eabi-gcc")
set(CMAKE_AR "${TOOLCHAIN}/arm-none-eabi-ar")
set(CMAKE_OBJCOPY "${TOOLCHAIN}/arm-none-eabi-objcopy")
set(CMAKE_OBJDUMP "${TOOLCHAIN}/arm-none-eabi-objdump")
set(SIZE "${TOOLCHAIN}/arm-none-eabi-size")
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

project(nando_fw C ASM)
#set(CMAKE_ASM_FLAGS "${CFLAGS} -x assembler-with-cpp")


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

# Toolchain and directories
set(BOOT_DIR bootloader)
set(APP_DIR programmer)
set(GEN_NAME nando_fw)

set(BOOT_PROG bootloader_fw)
set(APP_FW_1_PROG app_fw_1)
set(APP_FW_2_PROG app_fw_2)

set(MCPU cortex-m3)

# Flags
set(CFLAGS -g -Wall -Werror -Og)

# Common compile options
add_compile_options(-DSTM32F10X_HD -DCONFIG_BOOTLOADER ${CFLAGS})
add_compile_options(-mcpu=${MCPU} -mthumb -mthumb-interwork -MMD -MP)
add_compile_options(-ffunction-sections -fdata-sections)

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
set(USB_DIR "${CMAKE_SOURCE_DIR}/usb_cdc")

# Add all necessary include directories
include_directories(
        ${SPL_PATH}
        ${SPL_CORE_SUPPORT}         # SPL Core support headers
        ${SPL_DEVICE_SUPPORT_PATH}  # Device support headers (STM32F10x)
        ${SPL_PERIPH_PATH}/inc       # Peripheral driver headers
        ${SPL_USB_PATH}/inc          # USB driver headers
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


# Define the library name
set(SPL_LIB stm32f10x)

file(GLOB_RECURSE LIB_SRCS
        ${SPL_PATH}/*.c
        )



# Specify the static library to be built
add_library(${SPL_LIB} STATIC ${LIB_SRCS} ${SPL_PATH}/startup_stm32f10x_hd.s)
target_compile_options(${SPL_LIB} PRIVATE -ffreestanding -nostdlib)

# Define linker scripts
set(LINKER_SCRIPT_BOOT ${CMAKE_SOURCE_DIR}/${BOOT_DIR}/stm32_flash.ld)
set(LINKER_SCRIPT_1 ${CMAKE_SOURCE_DIR}/${APP_DIR}/stm32_flash_1.ld)
set(LINKER_SCRIPT_2 ${CMAKE_SOURCE_DIR}/${APP_DIR}/stm32_flash_2.ld)


add_link_options(-Wl,-gc-sections,-Map=${CMAKE_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=${MCPU} -mthumb -mthumb-interwork)
#
# Build bootloader
file(GLOB_RECURSE BOOTLOADER_SOURCES ${BOOT_DIR}/*.c)
add_executable(${BOOT_PROG}.elf ${BOOTLOADER_SOURCES} ${SPL_PATH}/init.c)
target_link_libraries(${BOOT_PROG}.elf PRIVATE -T${LINKER_SCRIPT_BOOT} ${SPL_LIB})
add_custom_command(TARGET ${BOOT_PROG}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --pad-to 0x08004000 -O ihex $<TARGET_FILE:${BOOT_PROG}.elf> ${CMAKE_BINARY_DIR}/${BOOT_PROG}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${BOOT_PROG}.elf> ${CMAKE_BINARY_DIR}/${BOOT_PROG}.bin
        COMMAND ${CMAKE_OBJDUMP} -St $<TARGET_FILE:${BOOT_PROG}.elf> > ${CMAKE_BINARY_DIR}/${BOOT_PROG}.lst
        COMMAND ${SIZE} $<TARGET_FILE:${BOOT_PROG}.elf>
        COMMENT "Building ${BOOT_PROG}.hex
Building ${BOOT_PROG}.bin
Generating list file and displaying size"
        )

# Build app1
file(GLOB_RECURSE APP1_SOURCES ${APP_DIR}/*.c ${USB_DIR}/*.c)

add_executable(${APP_FW_1_PROG}.elf ${APP1_SOURCES} ${SPL_PATH}/init.c)
target_link_libraries(${APP_FW_1_PROG}.elf PRIVATE -T ${LINKER_SCRIPT_1} ${SPL_LIB})


add_custom_command(TARGET ${APP_FW_1_PROG}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --pad-to 0x08022000 -O ihex $<TARGET_FILE:${APP_FW_1_PROG}.elf> ${CMAKE_BINARY_DIR}/${APP_FW_1_PROG}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${APP_FW_1_PROG}.elf> ${CMAKE_BINARY_DIR}/${APP_FW_1_PROG}.bin
        COMMAND ${CMAKE_OBJDUMP} -St $<TARGET_FILE:${APP_FW_1_PROG}.elf> > ${CMAKE_BINARY_DIR}/${APP_FW_1_PROG}.lst
        COMMAND ${SIZE} $<TARGET_FILE:${APP_FW_1_PROG}.elf>
        COMMENT "Building ${HEX_FILE_1}
Building ${BIN_FILE_1}
Generating list file and displaying size"
        )

# Build app2
add_executable(${APP_FW_2_PROG}.elf ${APP1_SOURCES} ${SPL_PATH}/init.c)
target_link_libraries(${APP_FW_2_PROG}.elf PRIVATE -T ${LINKER_SCRIPT_2} ${SPL_LIB})

add_custom_command(TARGET ${APP_FW_2_PROG}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --pad-to 0x08040000 -O ihex $<TARGET_FILE:${APP_FW_2_PROG}.elf> ${CMAKE_BINARY_DIR}/${APP_FW_2_PROG}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${APP_FW_2_PROG}.elf> ${CMAKE_BINARY_DIR}/${APP_FW_2_PROG}.bin
        COMMAND ${CMAKE_OBJDUMP} -St $<TARGET_FILE:${APP_FW_2_PROG}.elf> > ${CMAKE_BINARY_DIR}/${APP_FW_2_PROG}.lst
        COMMAND ${SIZE} $<TARGET_FILE:${APP_FW_2_PROG}.elf>
        COMMENT "Building ${HEX_FILE_2}
Building ${BIN_FILE_2}
Generating list file and displaying size"
        )
add_custom_target(
        ${GEN_NAME} ALL
        DEPENDS ${BOOT_PROG}.hex ${APP_FW_1_PROG}.hex ${APP_FW_2_PROG}.hex
        COMMAND sed -i '' '$$d' ${BOOT_PROG}.hex
        COMMAND sed -i '' '$$d' ${APP_FW_1_PROG}.hex
        COMMAND cat ${BOOT_PROG}.hex ${APP_FW_1_PROG}.hex ${APP_FW_2_PROG}.hex > ${GEN_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -I ihex -O elf32-littlearm ${GEN_NAME}.hex ${GEN_NAME}.elf
        COMMAND ${CMAKE_OBJCOPY} -I ihex -O binary ${GEN_NAME}.hex ${GEN_NAME}.bin
        COMMAND ${SIZE} ${GEN_NAME}.elf
        COMMENT "Combining HEX files into ${GEN_NAME}.hex and generating ${GEN_NAME}.bin and ${GEN_NAME}.elf"
        )


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
add_dependencies(${APP_FW_1_PROG}.elf ${BOOT_PROG}.elf)
add_dependencies(${APP_FW_2_PROG}.elf ${APP_FW_1_PROG}.elf)
add_dependencies(${GEN_NAME} ${APP_FW_2_PROG}.elf)
