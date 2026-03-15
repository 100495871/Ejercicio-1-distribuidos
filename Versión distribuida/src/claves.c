#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/claves.h"

#define MAX_STR 256
#define MAX_VEC 32

struct Nodo {
    char clave[MAX_STR];
    char valor1[MAX_STR];
    int N_valor2;
    float vector_v2[MAX_VEC];
    struct Paquete valor3;
    struct Nodo *siguiente;
};

struct Nodo *lista_tuplas = NULL;
pthread_mutex_t operando = PTHREAD_MUTEX_INITIALIZER;

// función de apoyo para buscar una clave sin gestionar el mutex
static struct Nodo* buscar_nodo_interno(const char *key) {
    struct Nodo *actual = lista_tuplas;
    while (actual != NULL) {
        if (strcmp(actual->clave, key) == 0) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

int destroy(void) {
    pthread_mutex_lock(&operando);
    struct Nodo *actual = lista_tuplas;
    while (actual != NULL) {
        struct Nodo *siguiente = actual->siguiente;
        free(actual);
        actual = siguiente;
    }
    lista_tuplas = NULL;
    pthread_mutex_unlock(&operando);
    return 0;
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // validamos que el tamaño del vector sea correcto
    if (N_value2 > MAX_VEC || N_value2 < 1) return -1;

    pthread_mutex_lock(&operando);
    
    // miramos si la llave ya está en la lista
    if (buscar_nodo_interno(key) != NULL) {
        pthread_mutex_unlock(&operando);
        return -1;
    }

    // reservamos memoria para el nuevo nodo
    struct Nodo *nuevo = (struct Nodo *)malloc(sizeof(struct Nodo));
    if (nuevo == NULL) {
        pthread_mutex_unlock(&operando);
        return -1;
    }

    // copiamos los strings y los datos al nodo
    strncpy(nuevo->clave, key, MAX_STR - 1);
    nuevo->clave[MAX_STR - 1] = '\0';
    strncpy(nuevo->valor1, value1, MAX_STR - 1);
    nuevo->valor1[MAX_STR - 1] = '\0';
    
    nuevo->N_valor2 = N_value2;
    memcpy(nuevo->vector_v2, V_value2, N_value2 * sizeof(float));
    nuevo->valor3 = value3;

    // enganchamos el nodo al principio de la lista
    nuevo->siguiente = lista_tuplas;
    lista_tuplas = nuevo;

    pthread_mutex_unlock(&operando);
    return 0;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {    
    pthread_mutex_lock(&operando);
    // buscamos el nodo y extraemos sus valores
    struct Nodo *n = buscar_nodo_interno(key);
    if (n != NULL) {
        strcpy(value1, n->valor1);
        *N_value2 = n->N_valor2;
        memcpy(V_value2, n->vector_v2, n->N_valor2 * sizeof(float));
        *value3 = n->valor3;   
        pthread_mutex_unlock(&operando);
        return 0;
    }
    pthread_mutex_unlock(&operando);
    return -1;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (N_value2 > MAX_VEC || N_value2 < 1) return -1;

    pthread_mutex_lock(&operando);
    // buscamos el nodo existente para actualizar sus campos
    struct Nodo *n = buscar_nodo_interno(key);
    if (n != NULL) {
        strncpy(n->valor1, value1, MAX_STR - 1);
        n->valor1[MAX_STR - 1] = '\0';
        n->N_valor2 = N_value2;
        memcpy(n->vector_v2, V_value2, n->N_valor2 * sizeof(float));
        n->valor3 = value3;
        pthread_mutex_unlock(&operando);
        return 0;
    }
    pthread_mutex_unlock(&operando);
    return -1;
}

int delete_key(char *key) {
    pthread_mutex_lock(&operando);
    struct Nodo *actual = lista_tuplas;
    struct Nodo *anterior = NULL;
    
    // recorremos la lista para encontrar y desconectar el nodo
    while (actual != NULL) {
        if (strcmp(actual->clave, key) == 0) {
            if (anterior != NULL) anterior->siguiente = actual->siguiente;
            else lista_tuplas = actual->siguiente;
            
            free(actual);
            pthread_mutex_unlock(&operando);
            return 0;
        }
        anterior = actual;
        actual = actual->siguiente;
    }
    pthread_mutex_unlock(&operando);
    return -1;
}

int exist(char *key) {
    pthread_mutex_lock(&operando);
    // comprobamos si el nodo existe y devolvemos el resultado
    int res = (buscar_nodo_interno(key) != NULL);
    pthread_mutex_unlock(&operando);
    return res;
}