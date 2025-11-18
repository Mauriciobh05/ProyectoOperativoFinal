/******************************************************
* Fecha 11/11/2025
* Pontificia Universidad Javeriana
* Profesor: J. Corredor, PhD
* Autor(es): Alejandro Beltran, Mauricio Beltran & Andres Diaz
* Materia: Sistemas opertivos
* Temas: Proyecto agente.c
*
* Descripción:
* Este archivo implementa el programa principal del agente dentro del
* sistema de reservas. Su función es coordinar todas las operaciones
* necesarias para que el agente pueda interactuar correctamente con el
* controlador: interpretar argumentos de entrada, crear su pipe propio,
* registrarse ante el controlador, recibir la hora inicial de la
* simulación y procesar las solicitudes de reserva presentes en el
* archivo asignado. Finalmente, gestiona el cierre de los pipes y la
* liberación de recursos utilizados. Este módulo constituye la entrada
* ejecutable del agente y organiza su flujo completo de ejecución.
******************************************************/

#include <stdio.h> //Libreria para mostrar informacion por pantalla
#include <stdlib.h> //Libreria de memoria dinamica
#include <string.h> //libreria para cadenas de caracteres
#include <unistd.h> //Libreria para funciones relacionadas con posix
#include <fcntl.h> // Libreria para constantes y flags para open
#include <sys/stat.h> //Libreria para archivos y permisos
#include <sys/types.h> //Libreria para syscalls
#include <errno.h> //Libreria para manejo de errores

#include "agente_funciones.h"   

#define MAX_NOMBRE 50 //Tamano maximo para el nombre
#define MAX_BUFFER 256 //Tamano maximo para el buffer

int main(int argc, char* argv[]) {
    char nombre_agente[MAX_NOMBRE] = ""; //Nombre del agente
    char archivo_solicitudes[MAX_NOMBRE] = ""; //Nombre del archivo con las solicitudes
    char pipe_entrada[MAX_NOMBRE] = ""; //Nombre del pipe de entrada
    char pipe_propio[MAX_NOMBRE] = ""; //Nombre del pipe del agente

    // Recibe los argumentos, guarda el nombre del agente, el nombre del archivo de las solicitudes y el nombre del pipe
    parsear_argumentos(argc, argv, nombre_agente, archivo_solicitudes, pipe_entrada);
    // Crea un pipe para que el controlador envia los datos
    crear_pipe_propio(pipe_propio, nombre_agente);
    // Le indica al controlador un mensaje con el nombre del agente y el pipe a utilizar
    int fd_entrada = registrar_agente_controlador(nombre_agente, pipe_propio, pipe_entrada);
    //Abre el pipe ya sea para lectura o escritura
    int fd_propio = abrir_pipe_propio(pipe_propio);
    //recibe la hora inicial y verifica que el mensaje sea para ese agente
    int hora_actual = recibir_hora_inicial(fd_propio, nombre_agente);
    //Recibe todos los datos de la solictud y espera las respuestas que les va a devolver
    procesar_solicitudes(fd_entrada, fd_propio, archivo_solicitudes, nombre_agente, hora_actual);
    // Cierre los pipes que esten abiertos y los elimina de ser necesario
    cerrar_y_limpiar(fd_entrada, fd_propio, pipe_propio, nombre_agente);

    return 0;
}

/******************************************************
* CONCLUSIÓN
*
* Este archivo implementa el flujo principal de ejecución
* del agente, integrando todas las funciones necesarias
* para su comunicación con el controlador. Desde el
* procesamiento de argumentos hasta el cierre de recursos,
* el agente opera de manera estructurada y autónoma,
* garantizando que cada solicitud sea enviada y procesada
* correctamente.
*
* Al coordinar la creación del pipe propio, el registro,
* la recepción de la hora inicial y el envío de solicitudes,
* el agente cumple su rol dentro del sistema concurrente,
* funcionando como un componente activo y sincronizado con
* el controlador.
*
* En conjunto, este archivo asegura un funcionamiento
* ordenado, modular y eficiente del agente, permitiendo
* ejecutar la simulación de manera confiable y conforme a
* los objetivos del proyecto.
******************************************************/

