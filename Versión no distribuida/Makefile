# Variables para el compilador y banderas
CC = gcc
CFLAGS = -Wall -g -I./include -fPIC
LDFLAGS = -L. -lpthread

# Archivos a generar
LIBS = libclaves.so
BINS = app_cliente

# Regla principal: construye todo
all: $(LIBS) $(BINS)

# Regla para crear la biblioteca dinámica libclaves.so
# Requiere claves.o y se enlaza con pthread para la atomicidad [3, 4]
libclaves.so: claves.o
	$(CC) -shared -o libclaves.so claves.o -lpthread

# Regla para compilar el objeto de la biblioteca con código independiente de posición
claves.o: src/claves.c
	$(CC) $(CFLAGS) -c src/claves.c -o claves.o

# Regla para compilar el cliente de pruebas
# Se enlaza con -lclaves para usar tu biblioteca dinámica [3]
app_cliente: src/app_cliente.c libclaves.so
	$(CC) src/app_cliente.c -o app_cliente $(CFLAGS) -L. -lclaves -lpthread

# Regla para limpiar archivos temporales y ejecutables
clean:
	rm -f *.o *.so $(BINS)