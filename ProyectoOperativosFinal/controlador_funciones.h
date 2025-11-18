/******************************************************
* Fecha 11/11/2025
* Pontificia Universidad Javeriana
* Profesor: J. Corredor, PhD
* Autor(es): Alejandro Beltran, Mauricio Beltran & Andres Diaz
* Materia: Sistemas opertivos
* Temas: Proyecto controlador_funciones.h
*
* Descripción:
* Este archivo contiene las definiciones de estructuras, constantes,
* variables globales y prototipos de funciones necesarios para el
* funcionamiento del controlador del sistema de reservas del parque.
* Aquí se organiza la lógica base de la simulación: manejo del reloj,
* registro de agentes, procesamiento de solicitudes, control de aforo,
* reprogramación de reservas y comunicación mediante pipes. Su propósito
* es centralizar los elementos fundamentales que permiten la correcta
* interacción entre los hilos y módulos del proyecto.
******************************************************/
#ifndef CONTROLADOR_FUNCIONES_H
#define CONTROLADOR_FUNCIONES_H

#include <stdio.h> //Libreria para mostrar informacion por pantalla
#include <stdlib.h> //Libreria de memoria dinamica
#include <string.h> //libreria para cadenas de caracteres
#include <unistd.h> //Libreria para funciones relacionadas con posix
#include <fcntl.h> // Libreria para constantes y flags
#include <sys/stat.h> //Libreria para archivos y permisos
#include <sys/types.h> //Libreria para syscalls
#include <pthread.h> // Libreria para tipos de datos usados en llamadas al sistema
#include <errno.h> //Libreria para manejo de errores

#define MAX_HORAS 20 //Cantidad maxima de horas que maneja
#define MAX_RESERVAS_POR_HORA 50 //Cantidad maxima de reservas en una hora
#define MAX_AGENTES 20 //Cantidad maxima de agentes que soporta
#define MAX_NOMBRE 50 //Cantidad maxima de caracteres en un nombre
#define MAX_BUFFER 256 //Cantidad maxima de caracteres para el buffer (lectura y escritura)

typedef struct {
    char familia[MAX_NOMBRE]; //Nombre de la familia que realizo la reserva
    int hora_inicio; //Hora de inicio
    int personas; //Cantidad de integrantes de la familia
    char agente[MAX_NOMBRE]; //Nombre del agente
} Reserva;

typedef struct {
    int hora; //Entero donde se guarda la hora
    int ocupacion; //Cantidad de persona en esa hora
    Reserva reservas[MAX_RESERVAS_POR_HORA]; //Arreglo de rservas para esa hora
    int num_reservas;
} HoraParque;

typedef struct {
    char nombre[MAX_NOMBRE]; //Nombre del agente
    char pipe_respuesta[MAX_NOMBRE]; //Nombre del pipe para comunicarse
    int activo; //Si esta activo (1) o no (0)
} Agente;

// Variables globales externas
//Indica la hora actual, la de inicio y fin con las que se obtiene el tiempo transcurrido
//La cantidad de segundos de las horas simuladas y el maximo de personas
extern int hora_actual, hora_inicio, hora_fin, seg_por_hora, aforo_max;
extern char pipe_entrada[MAX_NOMBRE]; //Nombre del pipe
extern HoraParque parque[MAX_HORAS]; //Arreglo que tiene la informacion de las horas
extern int fd_pipe_entrada; //Descriptor del pipe
//Cuenta cuantas solicitudes han sido aprobadas, reprogramadas o negadas
extern int solicitudes_aceptadas, solicitudes_reprogramadas, solicitudes_negadas;
extern Agente agentes[MAX_AGENTES]; //Lista de los agentes
extern int num_agentes; //Guarda cuantos agentes estan conectados
extern volatile int simulacion_terminada; //Indica el fin de la simulacion

// Prototipos
// Funcion que controla el avance del reloj
void* reloj(void* arg);
// Funcion que gestiona las solicitudes de los agentes
void* gestor_solicitudes(void* arg);
// Avanza la simulacion en una hora, se actualizan las rservas para esa nueva hora
void avanzar_hora(void);
// Procesa los mensajes recibidos desde los agentes
void procesar_mensaje(char* msg);
// Registra un nuevo agente en el sistema
int registrar_agente(char* nombre, char* pipe_resp);
// Envia una respuesta a un agente mediante su pipe especifico
void enviar_respuesta(char* pipe_resp, char* msg);
// Intenta reservar basado en la disponibilidad del parque
int intentar_reserva(char* familia, int hora, int personas, char* agente, char* resp);
// Busca un bloque libre para reprogramar una reserva
int buscar_bloque_libre(int personas, int* nueva_hora);
//Muestra un reporte final de la simulacion
void imprimir_reporte(void);
//Le indica todos los agentes que se termino la simulacion
void terminar_agentes(void);
//limpia los recursos y borra el pipe del controlador
void limpiar_recursos(void);

#endif 

/******************************************************
* CONCLUSIÓN
* 
* Este archivo centraliza todas las estructuras de datos
* y prototipos necesarios para el funcionamiento del 
* controlador. Su organización modular permite separar 
* claramente la gestión del reloj, las reservas, la 
* comunicación por pipes y la sincronización entre hilos.
* 
* La definición explícita de constantes, tipos y variables 
* globales facilita la integración entre los diferentes
* componentes del proyecto y asegura un manejo ordenado
* de la simulación. Gracias a esta arquitectura, el 
* controlador puede coordinar múltiples agentes, validar 
* aforos, reprogramar reservas y mantener la consistencia 
* del sistema en tiempo real.
******************************************************/

