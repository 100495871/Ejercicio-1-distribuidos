set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

file(MAKE_DIRECTORY ${BIN_DIR})

include_directories(${INC_DIR})

find_package(Threads REQUIRED)

# Nombres diferentes para evitar conflictos
add_library(claves_dist SHARED ${SRC_DIR}/claves.c)
target_link_libraries(claves_dist Threads::Threads)

add_library(proxyclaves SHARED ${SRC_DIR}/proxy-mq.c)
target_link_libraries(proxyclaves Threads::Threads rt)

add_executable(app_cliente_dist ${SRC_DIR}/app_cliente.c)
target_link_libraries(app_cliente_dist proxyclaves Threads::Threads rt)

add_executable(servidor_mq ${SRC_DIR}/servidor-mq.c)
target_link_libraries(servidor_mq claves_dist Threads::Threads rt)

set_target_properties(app_cliente_dist PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    BUILD_RPATH ${BIN_DIR}
)

set_target_properties(servidor_mq PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    BUILD_RPATH ${BIN_DIR}
)

set_target_properties(claves_dist PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}
)

set_target_properties(proxyclaves PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}
)
