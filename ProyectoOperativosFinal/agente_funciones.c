/******************************************************
* Fecha 11/11/2025
* Pontificia Universidad Javeriana
* Profesor: J. Corredor, PhD
* Autor(es): Alejandro Beltran, Mauricio Beltran & Andres Diaz
* Materia: Sistemas opertivos
* Temas: Proyecto agente_funciones.c
*
* Descripción:
*
* Este archivo implementa todas las funciones necesarias para el
* funcionamiento del agente dentro del sistema de reservas. Aquí se
* desarrollan los procesos de lectura y validación de argumentos,
* creación del pipe propio del agente, registro ante el controlador,
* recepción de la hora inicial, procesamiento de solicitudes desde un
* archivo CSV y manejo de las respuestas enviadas por el controlador.
* Además, incluye las rutinas encargadas de cerrar correctamente los
* pipes y limpiar los recursos utilizados. Este módulo constituye el
* comportamiento operativo del agente, permitiendo su comunicación
* bidireccional y sincronizada con el controlador durante toda la
* simulación.
******************************************************/

#include "agente_funciones.h" //Donde se encuentran los prototipos
// Recibe los argumentos, guarda el nombre del agente, el nombre del archivo de las solicitudes y el nombre del pipe
void parsear_argumentos(int argc, char* argv[], char* nombre, char* archivo, char* pipe) {
    int i = 1; //Variable para moverse entre los argumentos
    while (i < argc) {
        if (strcmp(argv[i], "-s") == 0) { i++; strcpy(nombre, argv[i]); } //Guarda el nombre del agente que recibio como parametro
        else if (strcmp(argv[i], "-a") == 0) { i++; strcpy(archivo, argv[i]); } //Guarda el nombre del archivo con las solicitudes que recibio como parametro
        else if (strcmp(argv[i], "-p") == 0) { i++; strcpy(pipe, argv[i]); } //Guarda el nombre del pipe del controlador que recibio como parametro
        i++;
    }
    //Revisa si tiene los 3 parametros (nombre agente, nombre archivo de solicitudes y el nombre del pipe)
    if (strlen(nombre) == 0 || strlen(archivo) == 0 || strlen(pipe) == 0) {
        fprintf(stderr, "Error: faltan parámetros.\n");
        exit(1);
    }
}
// Crea un pipe para que el controlador envia los datos
void crear_pipe_propio(char* pipe_propio, char* nombre_agente) {
    sprintf(pipe_propio, "Pipe%s", nombre_agente); //Le da un nombre al pipe
    if (mkfifo(pipe_propio, 0666) == -1 && errno != EEXIST) { //Crea el FIFO solo si aun no existe
        perror("mkfifo pipe_propio"); //Muestra un mensaje de error si no logra crearlo
        exit(1);
    }
}
// Le indica al controlador un mensaje con el nombre del agente y el pipe a utilizar
int registrar_agente_controlador(char* nombre_agente, char* pipe_propio, char* pipe_entrada) {
    int fd_entrada = open(pipe_entrada, O_WRONLY); //abre el pipe para escritura
    if (fd_entrada == -1) {
        perror("open pipe_entrada"); //Si no logra abrirlo muestra el mensaje de error
        exit(1);
    }
    char msg_registro[256]; //Espacio para el mensaje de registro
    sprintf(msg_registro, "REGISTRO|%s|%s", nombre_agente, pipe_propio); //Crea el mensaje
    write(fd_entrada, msg_registro, strlen(msg_registro)); //Lo escribe para el controlador
    return fd_entrada; //Retorna el descriptor (la primera parte del mensaje), para ubicarlo
}
//Abre el pipe ya sea para lectura o escritura
int abrir_pipe_propio(char* pipe_propio) {
    int fd_propio = open(pipe_propio, O_RDONLY); //abre el pipe del agente
    if (fd_propio == -1) {
        perror("open pipe_propio"); //Muestra un mensaje de error si no logra abrirlo
        exit(1);
    }
    return fd_propio; //retorna el descriptor
}
//recibe la hora inicial y verifica que el mensaje sea para ese agente
int recibir_hora_inicial(int fd_propio, char* nombre_agente) {
    char buffer[MAX_BUFFER]; //Tamano maximo del buffer que se usa para leer el mensaje
    int n = read(fd_propio, buffer, sizeof(buffer) - 1); //Le indica que el pipe esta en lectura
    int hora_actual = 0; //Inicializa la variable en donde se guarda la hora
    if (n > 0) {
        buffer[n] = '\0';
        sscanf(buffer, "HORA|%d", &hora_actual); //Obtiene la hora
        printf("Agente %s registrado. Hora actual: %d\n", nombre_agente, hora_actual);
    } else {
        fprintf(stderr, "Error: no se recibió hora inicial del controlador.\n"); //Muestra un mensaje de error si no pudo obtenerla
    }
    return hora_actual;
}
//Recibe todos los datos de la solictud y espera las respuestas que les va a devolver
void procesar_solicitudes(int fd_entrada, int fd_propio, char* archivo_solicitudes, char* nombre_agente, int hora_actual) {
    FILE* archivo = fopen(archivo_solicitudes, "r"); //Abre el archivo con las solicitudes
    if (!archivo) {
        perror("fopen archivo_solicitudes"); //Muestra un error sino logra abrirlo
        exit(1);
    }
    char linea[256]; //buffer para el contenido del csv
    char buffer[MAX_BUFFER]; //buffer para respuestas
    while (fgets(linea, sizeof(linea), archivo)) { //hasta que no lo lea todo no para
        char familia[MAX_NOMBRE], hora_str[10], personas_str[10]; 
        if (sscanf(linea, "%[^,],%[^,],%s", familia, hora_str, personas_str) != 3) continue; //Revisa la estructura de la solicitud
        int hora = atoi(hora_str); //Hace que la hora pase a ser un entero
        int personas = atoi(personas_str); //Cantidad de personas a entero
        if (hora < hora_actual) { //Si la hora de la solicitud es menor a la actual, ignora la solicitud
            printf("Solicitud ignorada: %s, hora %d (anterior a %d)\n", familia, hora, hora_actual);
            continue;
        }
        char msg_solicitud[256];
        sprintf(msg_solicitud, "SOLICITUD|%s|%d|%d|%s", familia, hora, personas, nombre_agente);
        write(fd_entrada, msg_solicitud, strlen(msg_solicitud)); //Se la envia al controlador

        int n = read(fd_propio, buffer, sizeof(buffer) - 1); //Espera a que haya una respuesta que leer
        if (n > 0) {
            buffer[n] = '\0';
            printf("Respuesta: %s\n", buffer); //Muestra la respuesta por pantalla
        }
        sleep(2); //Pausa
    }
    fclose(archivo); //Cierra el permiso para leer archivos
}
// Cierre los pipes que esten abiertos y los elimina de ser necesario
void cerrar_y_limpiar(int fd_entrada, int fd_propio, char* pipe_propio, char* nombre_agente) {
    close(fd_entrada); //Cierra el pipe hacia el controlador
    close(fd_propio); //Cierra su pipe
    unlink(pipe_propio); //Elimina el FIFO
    printf("Agente %s termina.\n", nombre_agente); //Mensaje confirmando que lo termino
}

/******************************************************
* CONCLUSIÓN
*
* Este archivo implementa la lógica completa del agente,
* permitiendo su participación activa en el sistema de
* reservas. A través del manejo correcto de pipes, lectura
* de archivos CSV, validación de solicitudes y recepción
* de respuestas, el agente logra comunicarse de forma
* ordenada y confiable con el controlador.
*
* La modularidad presentada aquí permite que cada agente
* funcione de manera independiente, garantizando la
* concurrencia y evitando interferencias entre ellos.
* Asimismo, los procesos de registro, envío de mensajes,
* espera de respuestas y limpieza final aseguran un flujo
* de ejecución claro y estable durante toda la simulación.
*
* Con este diseño, el agente cumple adecuadamente su rol
* de intermediario entre los usuarios y el controlador,
* contribuyendo al funcionamiento armónico del sistema.
******************************************************/


