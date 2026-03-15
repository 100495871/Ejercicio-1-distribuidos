#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "../include/claves.h"
#include "../include/mensajes.h"

// Definir las señales que pueden hacer que el programa termine
typedef const char *señal[2];
volatile int running = 1;

// Lista de hilos activos
typedef struct {
    pthread_t *hilos;
    int num_hilos;
    int capacidad;
    pthread_mutex_t mutex;
} ThreadList;

ThreadList lista_hilos = {NULL, 0, 0, PTHREAD_MUTEX_INITIALIZER};
mqd_t server_q;

void init_thread_list() {
    lista_hilos.capacidad = 10;
    lista_hilos.hilos = malloc(lista_hilos.capacidad * sizeof(pthread_t));
    lista_hilos.num_hilos = 0;
}

void add_thread(pthread_t thread) {
    pthread_mutex_lock(&lista_hilos.mutex);
    
    if (lista_hilos.num_hilos >= lista_hilos.capacidad) {
        lista_hilos.capacidad *= 2;
        lista_hilos.hilos = realloc(lista_hilos.hilos, 
                                    lista_hilos.capacidad * sizeof(pthread_t));
    }
    
    lista_hilos.hilos[lista_hilos.num_hilos++] = thread;
    
    pthread_mutex_unlock(&lista_hilos.mutex);
}

void remove_thread(pthread_t thread) {
    pthread_mutex_lock(&lista_hilos.mutex);
    
    for (int i = 0; i < lista_hilos.num_hilos; i++) {
        if (pthread_equal(lista_hilos.hilos[i], thread)) {
            lista_hilos.hilos[i] = lista_hilos.hilos[--lista_hilos.num_hilos];
            break;
        }
    }
    
    pthread_mutex_unlock(&lista_hilos.mutex);
}

void esperar_hilos() {
    printf("Esperando a que terminen %d hilos...\n", lista_hilos.num_hilos);
    
    while (lista_hilos.num_hilos > 0) {
        pthread_mutex_lock(&lista_hilos.mutex);
        if (lista_hilos.num_hilos > 0) {
            pthread_t hilo = lista_hilos.hilos[0];
            pthread_mutex_unlock(&lista_hilos.mutex);
            
            pthread_join(hilo, NULL);
            // El hilo se quita solo de la lista al terminar
        } else {
            pthread_mutex_unlock(&lista_hilos.mutex);
        }
    }
    
    free(lista_hilos.hilos);
    lista_hilos.hilos = NULL;
    lista_hilos.capacidad = 0;
}

void signal_handler(int sig) {
    señal posible_señal = {"Sigterm", "Sigint"}; 
    printf("\nRecibida señal %s, cerrando servidor...\n", posible_señal[sig-1]);
    
    // Cerrar la cola para que mq_receive termine con error
    mq_close(server_q);
    
    // Esperar a que terminen los hilos
    esperar_hilos();
    
    // Eliminar cola
    mq_unlink(SERVER_QUEUE);
    
    printf("Servidor cerrado.\n");
    exit(0);
}

void* handle_request(void* arg) {
    pthread_t self = pthread_self();
    RequestMessage* req = (RequestMessage*)arg;
    ResponseMessage resp;
    char client_queue_name[64];
    mqd_t client_q;
    
    snprintf(client_queue_name, sizeof(client_queue_name), 
             "%s%d", CLIENT_QUEUE_PREFIX, req->client_id);
    
    client_q = mq_open(client_queue_name, O_WRONLY);
    
    if (client_q == (mqd_t)-1) {
        perror("Error abriendo cola del cliente");
        free(req);
        remove_thread(self);
        return NULL;
    }
    
    // Ejecutar la operación correspondiente
    switch (req->op) {
        case OP_DESTROY:
            resp.result = destroy();
            break;
            
        case OP_SET_VALUE:
            resp.result = set_value(req->key, req->value1, req->N_value2, 
                                   req->V_value2, req->value3);
            break;
            
        case OP_GET_VALUE:
            resp.result = get_value(req->key, resp.value1, &resp.N_value2, 
                                   resp.V_value2, &resp.value3);
            break;
            
        case OP_MODIFY_VALUE:
            resp.result = modify_value(req->key, req->value1, req->N_value2, 
                                      req->V_value2, req->value3);
            break;
            
        case OP_DELETE_KEY:
            resp.result = delete_key(req->key);
            break;
            
        case OP_EXIST:
            resp.result = exist(req->key);
            break;
            
        default:
            resp.result = -1;
    }
    
    // Enviar respuesta
    if (mq_send(client_q, (char*)&resp, sizeof(ResponseMessage), 0) == -1) {
        perror("Error enviando respuesta");
    }
    
    mq_close(client_q);
    free(req);
    
    remove_thread(self);
    return NULL;
}

int main(int argc, char* argv[]) {
    struct mq_attr attr = {0, MAX_MESSAGES, sizeof(RequestMessage), 0};
    
    init_thread_list();
    
    // Manejar señales
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    
    // Eliminar cola si existe de ejecuciones previas
    mq_unlink(SERVER_QUEUE);
    
    // Crear cola del servidor
    server_q = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (server_q == (mqd_t)-1) {
        perror("Error creando cola del servidor");
        exit(1);
    }
    
    printf("Servidor MQ iniciado. Esperando peticiones en %s...\n", SERVER_QUEUE);
    
    while (1) {
        RequestMessage* req = malloc(sizeof(RequestMessage));
        if (!req) {
            perror("malloc");
            continue;
        }
        
        // Esto se bloquea hasta que llega un mensaje o alguien cierra la cola
        ssize_t bytes = mq_receive(server_q, (char*)req, sizeof(RequestMessage), NULL);
        
        if (bytes == -1) {
            // Si la cola se cerró por señal, salimos
            perror("mq_receive");
            free(req);
            break;
        }
        
        printf("Recibida operación %d del cliente %d\n", req->op, req->client_id);
        
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_request, req) != 0) {
            perror("Error creando thread");
            free(req);
            continue;
        }
        
        add_thread(thread);
    }
    
    // En teoría nunca se llega aquí porque signal_handler hace exit(0)
    return 0;
}