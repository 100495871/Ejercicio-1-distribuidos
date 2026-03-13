cmake_minimum_required(VERSION 3.10)
project(EjercicioEvaluable1 C)

# Configuración del compilador
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -g")

# Incluir directorios de cabeceras
include_directories(include)

# Opciones para enlazar con pthread y rt
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# ============================================
# PARTE A: VERSIÓN NO DISTRIBUIDA
# ============================================

# Biblioteca compartida libclaves.so (versión local)
add_library(claves SHARED src/claves.c)
target_link_libraries(claves Threads::Threads)
set_target_properties(claves PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Ejecutable cliente versión NO distribuida
add_executable(app_cliente src/app_cliente.c)
target_link_libraries(app_cliente claves Threads::Threads)
# Añadir rpath para que encuentre libclaves.so en el directorio actual
set_target_properties(app_cliente PROPERTIES
    BUILD_RPATH "${CMAKE_CURRENT_BINARY_DIR}"
    INSTALL_RPATH "${CMAKE_CURRENT_BINARY_DIR}"
)

# ============================================
# PARTE B: VERSIÓN DISTRIBUIDA CON COLAS MQ
# ============================================

# Biblioteca compartida libproxyclaves.so (proxy)
add_library(proxyclaves SHARED src/proxy-mq.c)
target_link_libraries(proxyclaves Threads::Threads rt)
set_target_properties(proxyclaves PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Ejecutable cliente versión distribuida (usa la misma app_cliente.c)
add_executable(app_cliente_mq src/app_cliente.c)
target_link_libraries(app_cliente_mq proxyclaves Threads::Threads rt)
set_target_properties(app_cliente_mq PROPERTIES
    BUILD_RPATH "${CMAKE_CURRENT_BINARY_DIR}"
    INSTALL_RPATH "${CMAKE_CURRENT_BINARY_DIR}"
)

# Ejecutable del servidor MQ
add_executable(servidor_mq src/servidor-mq.c)
target_link_libraries(servidor_mq claves Threads::Threads rt)
set_target_properties(servidor_mq PROPERTIES
    BUILD_RPATH "${CMAKE_CURRENT_BINARY_DIR}"
    INSTALL_RPATH "${CMAKE_CURRENT_BINARY_DIR}"
)

# ============================================
# OBJETIVOS ADICIONALES (clean, etc.)
# ============================================

# Target para limpiar colas de mensajes POSIX
add_custom_target(clean-queues
    COMMAND rm -f /dev/mqueue/servidor_mq
    COMMAND rm -f /dev/mqueue/cliente_mq_*
    COMMENT "Limpiando colas de mensajes POSIX"
)

# Target para limpiar todo (incluye colas)
add_custom_target(distclean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND rm -f /dev/mqueue/servidor_mq
    COMMAND rm -f /dev/mqueue/cliente_mq_*
    COMMENT "Limpiando todo (incluye colas)"
)