# Crea una libreria statica chiamata dht che include il file dht.c
add_library(dht STATIC
    dht.c
)

# Genera l'header PIO per il sensore
pico_generate_pio_header(dht ${CMAKE_CURRENT_LIST_DIR}/dht.pio)

# Aggiungi la directory ./include tra gli header pubblici
target_include_directories(dht
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

# Linka le librerie hardware richieste
target_link_libraries(dht
    PUBLIC
    hardware_clocks
    hardware_dma
    hardware_pio
    pico_stdlib
)
