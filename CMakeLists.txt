cmake_minimum_required(VERSION 3.20)

project("AgriVision_Sensor_Device")
enable_language(C ASM)

message("Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(cpu_PARAMS
    ${CPU_FLAGS}
)

set(common_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/LED/hal_led.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/LCD/HAL_LCD.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/ST7735S/HAL_ST7735S.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/GPIO/mcal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/RCC/MCAL_RCC.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/USART/MCAL_USART.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/SPI/MCAL_SPI.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/Interrupt/MCAL_Interrupt.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/Core/system_stm32f4xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/Core/startup_stm32f401xx.S
)

set(tx_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/main_tft_os.c
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/beso_os.c
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/beso_login.c
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/beso_calc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/beso_snake.c
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/beso_car.c
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/beso_flappy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/KEYPAD/HAL_KEYPAD.c
    ${common_SRCS}
)

set(rx_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/APP/main_spi_rx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/KEYPAD/HAL_KEYPAD.c
    ${common_SRCS}
)

set(include_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/LIBS
    ${CMAKE_CURRENT_SOURCE_DIR}/APP
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/LED
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/LCD
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/ST7735S
    ${CMAKE_CURRENT_SOURCE_DIR}/HAL/KEYPAD
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/GPIO
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/RCC
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/USART
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/SPI
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/Interrupt
    ${CMAKE_CURRENT_SOURCE_DIR}/MCAL/Core
)

set(symbols_c_SYMB
    RTE_DEVICE_STARTUP_STM32F4XX
)

set(linker_script_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/stm32f401xc_flash.ld
)

set(compiler_OPTS
    ${SECURE_FLAGS}
)

set(linker_OPTS)

add_executable(${CMAKE_PROJECT_NAME} ${tx_SRCS})
add_executable(${CMAKE_PROJECT_NAME}_RX EXCLUDE_FROM_ALL ${rx_SRCS})

set(app_targets
    ${CMAKE_PROJECT_NAME}
    ${CMAKE_PROJECT_NAME}_RX
)

foreach(app_target IN LISTS app_targets)
    target_include_directories(${app_target} PRIVATE
        ${include_DIRS}
    )

    target_compile_definitions(${app_target} PRIVATE
        ${symbols_c_SYMB}
        $<$<CONFIG:Debug>:DEBUG>
    )

    target_compile_options(${app_target} PRIVATE
        ${cpu_PARAMS}
        ${compiler_OPTS}
        -fstack-usage
        -Wall
        -Wextra
        -Wpedantic
        -Wno-unused-parameter
        $<$<COMPILE_LANGUAGE:ASM>:-x assembler-with-cpp -MMD -MP>
        $<$<CONFIG:Debug>:-O0 -g3 -ggdb>
        $<$<CONFIG:Release>:-Os>
    )

    target_link_options(${app_target} PRIVATE
        -T${linker_script_SRC}
        ${cpu_PARAMS}
        ${linker_OPTS}
        -Wl,-Map=${app_target}.map
        --specs=nosys.specs
        -Wl,--start-group
        -lc
        -lm
        -Wl,--end-group
        -Wl,--print-memory-usage
    )

    if(CMSIS_Dsecure STREQUAL "Secure")
        target_link_options(${app_target} PRIVATE
            -Wl,--cmse-implib
            -Wl,--out-implib=secure_nsclib.o
        )
    endif()

    set_target_properties(${app_target} PROPERTIES LINK_DEPENDS ${linker_script_SRC})

    add_custom_command(TARGET ${app_target} POST_BUILD
        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${app_target}>
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${app_target}> ${app_target}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${app_target}> ${app_target}.bin
    )
endforeach()
