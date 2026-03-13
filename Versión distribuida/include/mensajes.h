#ifndef MENSAJES_H
#define MENSAJES_H
#include "claves.h"

// Nombres de las colas POSIX
#define SERVER_QUEUE  "/servidor_mq"
#define CLIENT_QUEUE_PREFIX "/cliente_mq_"

// Tipos de operaciones
typedef enum {
    OP_DESTROY = 1,
    OP_SET_VALUE,
    OP_GET_VALUE,
    OP_MODIFY_VALUE,
    OP_DELETE_KEY,
    OP_EXIST
} OperationType;

// Estructura base de petición del cliente al servidor
typedef struct {
    OperationType op;           // Tipo de operación
    int client_id;              // ID único del cliente (para nombre de cola respuesta)
    char key[256];              // Clave
    char value1[256];           // Valor 1
    int N_value2;               // Dimensión del vector
    float V_value2[32];         // Vector de floats
    struct Paquete value3;      // Estructura Paquete
} RequestMessage;

// Estructura de respuesta del servidor al cliente
typedef struct {
    int result;                 // Resultado de la operación (0, -1, o -2)
    char value1[256];           // Valor 1 (para get)
    int N_value2;               // Dimensión (para get)
    float V_value2[32];         // Vector (para get)
    struct Paquete value3;      // Paquete (para get)
} ResponseMessage;

// Tamaños máximos de las colas
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(RequestMessage)

#endif