#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include "../include/claves.h"
#include "../include/mensajes.h"

// Variables estáticas para la comunicación
static mqd_t server_q = (mqd_t)-1;
static int client_id = 0;
static char client_queue_name[64] = {0};

// Función auxiliar para inicializar la comunicación con el servidor
static int init_communication(void) {
    if (server_q != (mqd_t)-1) {
        return 0; // Ya inicializado
    }
    
    // Generar ID único basado en PID
    client_id = getpid();
    snprintf(client_queue_name, sizeof(client_queue_name), 
             "%s%d", CLIENT_QUEUE_PREFIX, client_id);
    
    // Eliminar cola previa si existe (por si acaso)
    mq_unlink(client_queue_name);
    
    // Crear cola privada para recibir respuestas del servidor
    struct mq_attr attr = {0};
    attr.mq_maxmsg = 1;
    attr.mq_msgsize = sizeof(ResponseMessage);
    
    mqd_t client_q = mq_open(client_queue_name, O_RDONLY | O_CREAT | O_EXCL, 0666, &attr);
    if (client_q == (mqd_t)-1) {
        return -2; // Error de comunicación
    }
    mq_close(client_q); // La cerramos, la abriremos cuando necesitemos recibir
    
    // Abrir cola del servidor
    server_q = mq_open(SERVER_QUEUE, O_WRONLY);
    if (server_q == (mqd_t)-1) {
        mq_unlink(client_queue_name);
        return -2; // Error de comunicación - servidor no disponible
    }
    
    return 0;
}

// Función auxiliar para enviar petición y recibir respuesta
static int send_and_receive(RequestMessage* req, ResponseMessage* resp) {
    // Asegurar que la comunicación está inicializada
    int init_result = init_communication();
    if (init_result == -2) {
        return -2; // Error de comunicación
    }
    
    // Asignar ID del cliente
    req->client_id = client_id;
    
    // Enviar petición al servidor
    if (mq_send(server_q, (char*)req, sizeof(RequestMessage), 0) == -1) {
        return -2; // Error al enviar
    }
    
    // Abrir cola propia para recibir respuesta
    mqd_t client_q = mq_open(client_queue_name, O_RDONLY);
    if (client_q == (mqd_t)-1) {
        return -2;
    }
    
    // Recibir respuesta con timeout de 5 segundos
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 5;
    
    ssize_t bytes = mq_timedreceive(client_q, (char*)resp, sizeof(ResponseMessage), NULL, &ts);
    mq_close(client_q);
    
    if (bytes == -1) {
        return -2; // Timeout o error de recepción
    }
    
    return resp->result;
}

// Validación de parámetros según especificación del PDF
// Devuelve -1 si hay error de validación, 0 si OK
static int validate_string_length(const char* str) {
    if (str == NULL) return -1;
    if (strlen(str) > 255) return -1;
    return 0;
}

static int validate_N_value2(int N_value2) {
    if (N_value2 < 1 || N_value2 > 32) return -1;
    return 0;
}

// ============================================================
// IMPLEMENTACIÓN DE LA API - Versión Distribuida con Colas MQ
// ============================================================

int destroy(void) {
    RequestMessage req;
    ResponseMessage resp;
    
    memset(&req, 0, sizeof(RequestMessage));
    memset(&resp, 0, sizeof(ResponseMessage));
    
    req.op = OP_DESTROY;
    
    return send_and_receive(&req, &resp);
}

int set_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // Validaciones locales que devuelven -1 sin contactar al servidor
    if (validate_string_length(key) == -1) return -1;
    if (validate_string_length(value1) == -1) return -1;
    if (validate_N_value2(N_value2) == -1) return -1;
    
    RequestMessage req;
    ResponseMessage resp;
    
    memset(&req, 0, sizeof(RequestMessage));
    memset(&resp, 0, sizeof(ResponseMessage));
    
    req.op = OP_SET_VALUE;
    strncpy(req.key, key, 255);
    req.key[255] = '\0'; // Asegurar terminación
    strncpy(req.value1, value1, 255);
    req.value1[255] = '\0';
    req.N_value2 = N_value2;
    
    // Copiar vector (N_value2 ya validado entre 1-32)
    if (V_value2 != NULL) {
        memcpy(req.V_value2, V_value2, N_value2 * sizeof(float));
    }
    
    req.value3 = value3;
    
    return send_and_receive(&req, &resp);
}

int get_value(char *key, char *value1, int *N_value2, float *V_value2, struct Paquete *value3) {
    // Validación local
    if (validate_string_length(key) == -1) return -1;
    
    RequestMessage req;
    ResponseMessage resp;
    
    memset(&req, 0, sizeof(RequestMessage));
    memset(&resp, 0, sizeof(ResponseMessage));
    
    req.op = OP_GET_VALUE;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    
    int result = send_and_receive(&req, &resp);
    
    // Si la operación fue exitosa, copiar datos de vuelta
    if (result == 0) {
        if (value1 != NULL) {
            strcpy(value1, resp.value1);
        }
        if (N_value2 != NULL) {
            *N_value2 = resp.N_value2;
        }
        if (V_value2 != NULL) {
            memcpy(V_value2, resp.V_value2, resp.N_value2 * sizeof(float));
        }
        if (value3 != NULL) {
            *value3 = resp.value3;
        }
    }
    
    return result;
}

int modify_value(char *key, char *value1, int N_value2, float *V_value2, struct Paquete value3) {
    // Validaciones locales
    if (validate_string_length(key) == -1) return -1;
    if (validate_string_length(value1) == -1) return -1;
    if (validate_N_value2(N_value2) == -1) return -1;
    
    RequestMessage req;
    ResponseMessage resp;
    
    memset(&req, 0, sizeof(RequestMessage));
    memset(&resp, 0, sizeof(ResponseMessage));
    
    req.op = OP_MODIFY_VALUE;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    strncpy(req.value1, value1, 255);
    req.value1[255] = '\0';
    req.N_value2 = N_value2;
    
    if (V_value2 != NULL) {
        memcpy(req.V_value2, V_value2, N_value2 * sizeof(float));
    }
    
    req.value3 = value3;
    
    return send_and_receive(&req, &resp);
}

int delete_key(char *key) {
    // Validación local
    if (validate_string_length(key) == -1) return -1;
    
    RequestMessage req;
    ResponseMessage resp;
    
    memset(&req, 0, sizeof(RequestMessage));
    memset(&resp, 0, sizeof(ResponseMessage));
    
    req.op = OP_DELETE_KEY;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    
    return send_and_receive(&req, &resp);
}

int exist(char *key) {
    // Validación local
    if (validate_string_length(key) == -1) return -1;
    
    RequestMessage req;
    ResponseMessage resp;
    
    memset(&req, 0, sizeof(RequestMessage));
    memset(&resp, 0, sizeof(ResponseMessage));
    
    req.op = OP_EXIST;
    strncpy(req.key, key, 255);
    req.key[255] = '\0';
    
    return send_and_receive(&req, &resp);
}