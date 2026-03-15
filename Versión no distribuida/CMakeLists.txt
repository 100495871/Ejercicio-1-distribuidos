set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(INC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)

file(MAKE_DIRECTORY ${BIN_DIR})

include_directories(${INC_DIR})

add_library(claves_local SHARED ${SRC_DIR}/claves.c)
target_link_libraries(claves_local pthread)

add_executable(app_cliente ${SRC_DIR}/app_cliente.c)
target_link_libraries(app_cliente claves_local pthread)

set_target_properties(app_cliente PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    BUILD_RPATH ${BIN_DIR}
)

set_target_properties(claves_local PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BIN_DIR}
)
