set(TARGET_NAME pico-satelite)

add_executable(${TARGET_NAME}
        main.c
        sequence.hpp
        uart2rs232c.hpp
        )

target_link_libraries(${TARGET_NAME} PRIVATE
        pico_stdlib
        pico_multicore
        hardware_spi
        hardware_dma
        ETHERNET_FILES
        IOLIBRARY_FILES
        LOOPBACK_FILES 
        )

pico_enable_stdio_usb(${TARGET_NAME} 1)
pico_enable_stdio_uart(${TARGET_NAME} 0)

pico_add_extra_outputs(${TARGET_NAME})