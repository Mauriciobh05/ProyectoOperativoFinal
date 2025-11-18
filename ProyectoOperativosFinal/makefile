# ******************************************************
# Fecha: 11/11/2025
# Pontificia Universidad Javeriana
# Profesor: J. Corredor, PhD
# Autor(es): Alejandro Beltran, Mauricio Beltran & Andres Diaz
# Materia: Sistemas Operativos
# Temas: Makefile
#
# Descripción:
# Este Makefile automatiza el proceso de compilación de los dos módulos
# principales del proyecto: el controlador y los agentes. Define el
# compilador, los flags necesarios para habilitar POSIX y manejo de hilos,
# así como las reglas para generar los ejecutables y limpiar archivos
# temporales. Su propósito es garantizar una compilación ordenada,
# reproducible y eficiente del sistema completo de reservas.
# ******************************************************
#Indica que se compila con gcc
CC = gcc
#Flags para mostrar adeveretencias y habilitar posix
CFLAGS = -Wall -Wextra -pthread
#Que se quiere compilar tanto el agente como el controlador
TARGETS = controlador agente
#Que compile todos los objetivos
all: $(TARGETS)
#Adicional al principal le incluye sus funciones a controlador
controlador: controlador.c controlador_funciones.c
	$(CC) $(CFLAGS) -o controlador controlador.c controlador_funciones.c
#Adicional al principal le incluye sus funciones a agente
agente: agente.c agente_funciones.c
	$(CC) $(CFLAGS) -o agente agente.c agente_funciones.c
#Elimina los ejecutables y residuos de pipe y/o fifo
clean:
	rm -f $(TARGETS) Pipe* *.fifo

.PHONY: all clean

# ******************************************************
# CONCLUSIÓN
#
# Este Makefile facilita la gestión del proyecto al automatizar las
# tareas de compilación y limpieza de archivos auxiliares. Gracias a su
# estructura clara, permite recompilar rápidamente tanto el controlador
# como los agentes, asegurando que todos los módulos se construyan con
# los flags y dependencias adecuadas.
#
# Además, la inclusión de la regla 'clean' ayuda a mantener el entorno
# de trabajo libre de ejecutables y pipes residuales, garantizando que
# la simulación pueda reiniciarse sin conflictos. De esta manera, el
# Makefile contribuye de forma esencial a la organización y correcta
# ejecución del sistema desarrollado.
# ******************************************************
