#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "../include/claves.h"

// Función auxiliar para imprimir el resultado de las pruebas
void print_test(const char *test_name, bool condition) {
    static int test_num = 1;
    printf("TEST %d: %-40s: %s\n", test_num++, test_name, condition ? "ÉXITO" : "ERROR");
}

int main() {
    printf("INICIANDO PLAN DE PRUEBAS EXHAUSTIVO\n\n");

    // 1. Inicialización del sistema
    print_test("Inicializar sistema (destroy)", destroy() == 0);

    // Datos de prueba
    char *key1 = "clave_1";
    char *v1_1 = "primer valor de prueba";
    float v2_1[] = {1.1, 2.2, 3.3};
    struct Paquete v3_1 = {10, 20, 30};

    // 2. Prueba de inserción (set_value)
    print_test("Insertar clave_1 válida", set_value(key1, v1_1, 3, v2_1, v3_1) == 0);

    // 3. Prueba de existencia (exist)
    print_test("Comprobar existencia de clave_1", exist(key1) == 1);
    print_test("Comprobar no existencia de clave_falsa", exist("falsa") == 0);

    // 4. Prueba de recuperación de datos (get_value)
    char res_v1[256];
    int res_n2;
    float res_v2[32];
    struct Paquete res_v3;
    
    int err_get = get_value(key1, res_v1, &res_n2, res_v2, &res_v3);
    bool data_ok = (err_get == 0 && strcmp(res_v1, v1_1) == 0 && res_n2 == 3 && res_v2[0] == 1.1f);
    print_test("Recuperar y verificar datos de clave_1", data_ok);

    // 5. Prueba de error: Inserción de clave duplicada
    print_test("Error al insertar clave duplicada", set_value(key1, "otro", 1, v2_1, v3_1) == -1);

    // 6. Prueba de error: Límites de N_value2 (1-32)
    print_test("Error N_value2 fuera de rango (>32)", set_value("key2", "v1", 33, v2_1, v3_1) == -1);
    print_test("Error N_value2 fuera de rango (<1)", set_value("key2", "v1", 0, v2_1, v3_1) == -1);

    // 7. Prueba de modificación (modify_value)
    char *new_v1 = "valor modificado";
    v3_1.x = 999;
    print_test("Modificar clave_1", modify_value(key1, new_v1, 3, v2_1, v3_1) == 0);
    
    get_value(key1, res_v1, &res_n2, res_v2, &res_v3);
    print_test("Verificar modificación (v3.x == 999)", res_v3.x == 999);

    // 8. Prueba de error: Modificar clave inexistente
    print_test("Error al modificar clave inexistente", modify_value("no_existe", "v1", 1, v2_1, v3_1) == -1);

    // 9. Prueba de borrado (delete_key)
    print_test("Borrar clave_1", delete_key(key1) == 0);
    print_test("Verificar que clave_1 ya no existe", exist(key1) == 0);
    print_test("Error al borrar clave ya borrada", delete_key(key1) == -1);

    // 10. Prueba de stress: inserción múltiple (sin límite de elementos)
    bool stress_ok = true;
    for (int i = 0; i < 100; i++) {
        char k[16]; // Suficiente para "key_999" y el nulo
        snprintf(k, sizeof(k), "key_%d", i);
        if (set_value(k, "val", 1, v2_1, v3_1) != 0) stress_ok = false;
    }
    print_test("Insertar 100 elementos (sin límites)", stress_ok);

    // 11. Destrucción total
    destroy();
    print_test("Verificar destroy (clave key_50 no existe)", exist("key_50") == 0);

    printf("\n--- PLAN DE PRUEBAS FINALIZADO ---\n");
    return 0;
}