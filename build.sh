#!/bin/bash

echo "CONSTRUYENDO EJERCICIO 1"
echo ""

echo "Creando directorio build..."
mkdir -p build
echo "Ejecutando cmake..."
cmake -B build > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: cmake falló"
    exit 1
fi
cd build
echo "Compilando..."
make > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: compilación falló"
    exit 1
fi

cd ..

echo ""
echo "CONSTRUCCIÓN COMPLETADA"
echo ""
echo "Estructura generada:"
echo "Ejercicio-1-dsitribuidos"
echo "|-- Versión no distribuida/"
echo "|   |-- build/"
echo "|   |   |-- app_cliente"
echo "|   |   |-- libclaves_local.so"
echo "|   |-- ..."
echo "|"
echo "|-- Versión distribuida/"
echo "    |-- build/"
echo "    |   |--  app_cliente_dist"
echo "    |   |-- servidor"
echo "    |   |-- libclaves_dist.so"
echo "    |   |-- libproxyclaves.so"
echo "    |-- ..."
echo ""
# 5. Instrucciones de ejecución
echo "[5/5] Para ejecutar:"
echo ""
echo "  Versión NO distribuida:"
echo "    ./build/no_distribuida/app_cliente"
echo ""
echo "  Versión DISTRIBUIDA:"
echo "    Terminal 1: ./build/distribuida/servidor"
echo "    Terminal 2: ./build/distribuida/app_cliente_dist"
echo ""
echo "  Limpiar: rm -rf build/*"
echo "========================================="
