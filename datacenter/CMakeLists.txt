pico_sdk_init()

add_subdirectory(dht)

add_executable(datacenter
    main.c
)

pico_enable_stdio_usb(datacenter 1)
pico_enable_stdio_uart(datacenter 0)

target_link_libraries(datacenter PRIVATE
    pico_stdlib
    common_utils
    hardware_uart
    hardware_irq
    FreeRTOS-Kernel FreeRTOS-Kernel-Heap4
    microros
    dht
)

target_compile_definitions(datacenter PRIVATE
    PICO_UART_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_ENABLE_CRLF_SUPPORT=0
    PICO_STDIO_DEFAULT_CRLF=0
)

set_target_properties(datacenter PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_compile_options(datacenter PRIVATE -Wall)

pico_add_extra_outputs(datacenter)