/******************************************************
* Fecha 11/11/2025
* Pontificia Universidad Javeriana
* Profesor: J. Corredor, PhD
* Autor(es): Alejandro Beltran, Mauricio Beltran & Andres Diaz
* Materia: Sistemas opertivos
* Temas: Proyecto controlador.c
*
* Descripción:
*
* Este archivo implementa el programa principal del controlador del
* sistema de reservas. Aquí se procesan los argumentos recibidos por
* línea de comandos, se inicializan las estructuras del parque, se crea
* el pipe de comunicación y se configuran las variables globales que
* serán utilizadas durante toda la simulación. Además, se lanzan los
* hilos encargados del reloj y de la gestión de solicitudes, coordinando
* su ejecución mediante la sincronización correspondiente. Este módulo
* actúa como punto de inicio y núcleo organizador del sistema,
* asegurando que todos los componentes funcionen de manera integrada.
******************************************************/

#include <stdio.h> //Libreria para mostrar informacion por pantalla
#include <stdlib.h> //Libreria de memoria dinamica
#include <string.h> //libreria para cadenas de caracteres
#include <unistd.h> //Libreria para funciones relacionadas con posix
#include <fcntl.h> // Libreria para constantes y flags
#include <sys/stat.h> //Libreria para archivos y permisos
#include <sys/types.h> //Libreria para syscalls
#include <pthread.h> // Libreria para tipos de datos usados en llamadas al sistema
#include <signal.h> // Libreria para manejo de senales
#include <errno.h> //Libreria para manejo de errores
#include <time.h> // Libreria para funciones relacionadas con el tiempo

#include "controlador_funciones.h"
//Variables globales
//Indica la hora actual, la de inicio y fin con las que se obtiene el tiempo transcurrido
//La cantidad de segundos de las horas simuladas y el maximo de personas
int hora_actual, hora_inicio, hora_fin, seg_por_hora, aforo_max;
char pipe_entrada[MAX_NOMBRE]; //Nombre del pipe
HoraParque parque[MAX_HORAS]; //Estructura que tiene la informacion de las horas
int fd_pipe_entrada; //Descriptor del pipe

int solicitudes_aceptadas = 0; //Cuenta cuantas solicitudes han sido aprobadas
int solicitudes_reprogramadas = 0; //Cuenta cuantas solicitudes han sido reprogramadas
int solicitudes_negadas = 0; //Cuenta cuantas solicitudes han sido negadas

Agente agentes[MAX_AGENTES]; //Lista de los agentes
int num_agentes = 0; //Guarda cuantos agentes estan conectados

pthread_t hilo_reloj, hilo_gestor; //Identifica los hilos que va a usar
volatile int simulacion_terminada = 0; //Indica el fin de la simulacion

int main(int argc, char* argv[]) {
    //Revisa los argumentos recibidos
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-i") == 0) { i++; hora_inicio = atoi(argv[i]); } //Hora inicial
        else if (strcmp(argv[i], "-f") == 0) { i++; hora_fin = atoi(argv[i]); } //Hora final
        else if (strcmp(argv[i], "-s") == 0) { i++; seg_por_hora = atoi(argv[i]); } //Segundos por hora
        else if (strcmp(argv[i], "-t") == 0) { i++; aforo_max = atoi(argv[i]); } //Aforo maximo
        else if (strcmp(argv[i], "-p") == 0) { i++; strcpy(pipe_entrada, argv[i]); } //Nombre del pipe
        i++;
    }
    //Verifica que las horas si sean en horarios de atencion y valores positivos
    if (hora_inicio < 7 || hora_inicio > 19 || hora_fin < 7 || hora_fin > 19 ||
        hora_inicio > hora_fin || seg_por_hora <= 0 || aforo_max <= 0) {
        fprintf(stderr, "Error: parámetros inválidos.\n");
        exit(1);
    }

    hora_actual = hora_inicio; //Obtiene la hora actual
    //Inicializa las variables
    for (int h = 7; h <= 19; h++) {
        parque[h].hora = h; //Asigna la hora
        parque[h].ocupacion = 0; //Inicializa la ocupacion en 0
        parque[h].num_reservas = 0; //Inicializa las reservas en 0
    }

    if (mkfifo(pipe_entrada, 0666) == -1 && errno != EEXIST) {
        perror("mkfifo pipe_entrada"); //Muestra un error si no puede crear el fifo
        exit(1);
    }

    fd_pipe_entrada = open(pipe_entrada, O_RDONLY);
    if (fd_pipe_entrada == -1) {
        perror("open pipe_entrada"); //Muestra un error si no puede abrir el pipe
        exit(1);
    }

    if (pthread_create(&hilo_reloj, NULL, reloj, NULL) != 0) {
        perror("pthread_create reloj"); //Crea el hilo que va cambiando la hora
        exit(1);
    }
    if (pthread_create(&hilo_gestor, NULL, gestor_solicitudes, NULL) != 0) {
        perror("pthread_create gestor"); //Crea el hilo que maneja las solicitudes
        exit(1);
    }

    pthread_join(hilo_reloj, NULL); //Espera a que el hilo de las horas termine
    pthread_join(hilo_gestor, NULL); //Espera a que el hilo que maneja las solicitudes termine

    return 0;
}

/******************************************************
* CONCLUSIÓN
*
* Este archivo cumple el rol fundamental de manejar la
* simulación completa del sistema de reservas. Desde la
* lectura y validación de parámetros hasta la inicialización
* del parque y la creación de los hilos, su función es
* garantizar que todos los elementos del proyecto operen en
* coherencia.
*
* La correcta configuración del entorno, incluyendo pipes,
* estructuras de horas, límites de aforo y tiempos de 
* simulación, permite que los módulos de gestión y reloj
* trabajen de manera estable y sincronizada. Además, la
* creación de hilos independientes asegura concurrencia real,
* permitiendo que el controlador responda dinámicamente a las
* solicitudes de los agentes.
*
* Finalmente, este archivo establece la base operativa
* necesaria para que el sistema funcione de manera ordenada,
* modular y eficiente, logrando así los objetivos del proyecto
* en un entorno concurrente.
******************************************************/

