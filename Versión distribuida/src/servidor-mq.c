#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include "../include/claves.h"
#include "../include/mensajes.h"

// Flag para terminación graceful
volatile int running = 1;

void signal_handler(int sig) {
    running = 0;
}

// Función que ejecuta cada thread para atender una petición
void* handle_request(void* arg) {
    RequestMessage* req = (RequestMessage*)arg;
    ResponseMessage resp;
    char client_queue_name[64];
    mqd_t client_q;
    
    // Preparar nombre de cola del cliente
    snprintf(client_queue_name, sizeof(client_queue_name), 
             "%s%d", CLIENT_QUEUE_PREFIX, req->client_id);
    
    // Abrir cola del cliente (no bloqueante para evitar deadlock)
    client_q = mq_open(client_queue_name, O_WRONLY);
    
    if (client_q == (mqd_t)-1) {
        perror("Error abriendo cola del cliente");
        free(req);
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
    return NULL;
}

int main(int argc, char* argv[]) {
    mqd_t server_q;
    struct mq_attr attr = {0, MAX_MESSAGES, sizeof(RequestMessage), 0};
    
    // Manejar señales para limpieza
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    // Eliminar cola si existe de ejecuciones previas
    mq_unlink(SERVER_QUEUE);
    
    // Crear cola del servidor
    server_q = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (server_q == (mqd_t)-1) {
        perror("Error creando cola del servidor");
        exit(1);
    }
    
    printf("Servidor MQ iniciado. Esperando peticiones en %s...\n", SERVER_QUEUE);
    
    while (running) {
        RequestMessage* req = malloc(sizeof(RequestMessage));
        
        // Recibir petición (bloqueante pero interrumpible)
        ssize_t bytes = mq_receive(server_q, (char*)req, sizeof(RequestMessage), NULL);
        
        if (bytes == -1) {
            if (!running) break; // Terminación por señal
            perror("Error recibiendo mensaje");
            free(req);
            continue;
        }
        
        printf("Recibida operación %d del cliente %d\n", req->op, req->client_id);
        
        // Crear thread para atender la petición concurrentemente
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_request, req) != 0) {
            perror("Error creando thread");
            free(req);
            continue;
        }
        
        // Detach para que se limpie automáticamente al terminar
        pthread_detach(thread);
    }
    
    printf("\nCerrando servidor...\n");
    mq_close(server_q);
    mq_unlink(SERVER_QUEUE);
    
    return 0;
}