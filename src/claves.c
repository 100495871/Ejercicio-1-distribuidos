#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "../include/claves.h"

struct Nodo {
    char clave[256];
    char valor1[256];
    int N_valor2;
    float vector_v2[32];
    struct Paquete valor3;
    struct Nodo *siguiente;
};

struct Nodo *lista_tuplas = NULL;
pthread_mutex_t operando = PTHREAD_MUTEX_INITIALIZER;

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

    if (N_value2 > 32 || N_value2 < 1) {
        perror("Error en N_value2");
        return -1;
    }

    if (exist(key)) {
        perror("La llave ya existe");
        return -1;
    }
    struct Nodo *nuevo_nodo = (struct Nodo *)malloc(sizeof(struct Nodo));
    if (nuevo_nodo == NULL) {
        perror("Error en malloc");
        return -1;
    }
    strcpy(nuevo_nodo->clave, key);
    strcpy(nuevo_nodo->valor1, value1);
    nuevo_nodo->N_valor2 = N_value2;
    memcpy(nuevo_nodo->vector_v2, V_value2, N_value2 * sizeof(float));
    nuevo_nodo->valor3 = value3;
    nuevo_nodo->siguiente = lista_tuplas;
    pthread_mutex_lock(&operando);
    lista_tuplas = nuevo_nodo;
    pthread_mutex_unlock(&operando);
    return 0;
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {    
    pthread_mutex_lock(&operando);
    struct Nodo *actual = lista_tuplas;
    while (actual != NULL) {
        if (strcmp(actual->clave, key) == 0) {
            strcpy(value1, actual->valor1);
            *N_value2 = actual->N_valor2;
            memcpy(V_value2, actual->vector_v2, actual->N_valor2 * sizeof(float));
            *value3 = actual->valor3;   
            pthread_mutex_unlock(&operando);
            return 0;
        }
        actual = actual->siguiente;
    }
    pthread_mutex_unlock(&operando);
    return -1;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    if (N_value2 > 32 || N_value2 < 1) {
        perror("Error en N_value2");
        return -1;
    }
    pthread_mutex_lock(&operando);
    struct Nodo *actual = lista_tuplas;
    while (actual != NULL) {
        if (strcmp(actual->clave, key) == 0) {
            strcpy(actual->valor1, value1);
            actual->N_valor2 = N_value2;
            memcpy(actual->vector_v2, V_value2, N_value2 * sizeof(float));
            actual->valor3 = value3;
            pthread_mutex_unlock(&operando);
            return 0;
        }
        actual = actual->siguiente;
    }
    pthread_mutex_unlock(&operando);
    return -1;
}

int delete_key(char *key) {
    pthread_mutex_lock(&operando);
    struct Nodo *actual = lista_tuplas;
    struct Nodo *anterior = NULL;
    while (actual != NULL) {
        if (strcmp(actual->clave, key) == 0) {
            if (anterior!= NULL) {
                anterior->siguiente = actual->siguiente;
            } else {
                lista_tuplas = actual->siguiente;
            }
            free(actual);
            pthread_mutex_unlock(&operando);
            return 0;
        }
        actual = actual->siguiente;
    }
    pthread_mutex_unlock(&operando);
    return -1;
}

int exist(char *key) {
    pthread_mutex_lock(&operando);
    struct Nodo *actual = lista_tuplas;
    while (actual != NULL) {
        if (strcmp(actual->clave, key) == 0) {
            pthread_mutex_unlock(&operando);
            return 1;
        }
        actual = actual->siguiente;
    }
    pthread_mutex_unlock(&operando);
    return 0;
}
