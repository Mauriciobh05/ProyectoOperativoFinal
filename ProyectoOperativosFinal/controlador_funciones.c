/******************************************************
* Fecha 11/11/2025
* Pontificia Universidad Javeriana
* Profesor: J. Corredor, PhD
* Autor(es): Alejandro Beltran, Mauricio Beltran & Andres Diaz
* Materia: Sistemas opertivos
* Temas: Proyecto controlador_funciones.c
*
* Descripción:
* Este archivo implementa la lógica principal del controlador del sistema
* de reservas. Aquí se desarrolla el funcionamiento del reloj simulado,
* la gestión de solicitudes de los agentes, el procesamiento de mensajes,
* la asignación, reprogramación o negación de reservas y el control del
* aforo del parque en tiempo real. También incluye la generación del
* reporte final, el cierre ordenado de los agentes y la limpieza de los
* recursos del sistema. Este módulo constituye el núcleo operativo de la
* simulación, coordinando la interacción entre hilos, pipes y estructuras
* de datos.
******************************************************/
#include "controlador_funciones.h"

// Funcion ejecutada por el hilo que controla el avance del reloj
void* reloj(void* arg) {
    (void)arg;
    while (hora_actual <= hora_fin) { //Bucle hasta que la hora pase de la del fin
        sleep(seg_por_hora); //Que pase una hora simulada
        avanzar_hora(); //Actualiza las rservas y si esta ocupado
    }
    simulacion_terminada = 1; //Indica que termino la simulacion
    imprimir_reporte(); //Muestra un reporte final de la simulacion
    terminar_agentes(); //Le indica todos los agentes que se termino la simulacion
    limpiar_recursos(); //limpia los recursos y borra el pipe del controlador
    pthread_exit(NULL); //Termina el hilo
}

// Funcion ejecutada por el hilo que gestiona las solicitudes de los agentes
void* gestor_solicitudes(void* arg) {
    (void)arg;
    char buffer[MAX_BUFFER]; //Buffer de lectura
    while (!simulacion_terminada) { //Mientras no termine
        int n = read(fd_pipe_entrada, buffer, sizeof(buffer) - 1); //Lee los mensajes del pipe
        if (n > 0) {
            buffer[n] = '\0'; 
            procesar_mensaje(buffer); //LLama a la funcion que procesa los datos
        }
    }
    pthread_exit(NULL); 
}

// Avanza la simulacion en una hora, se actualizan las rservas para esa nueva hora
void avanzar_hora() {
    printf("Hora actual: %d\n", hora_actual);

    int saliendo = 0;
    char familias_saliendo[512] = "";
    if (hora_actual - 2 >= 7) { //Salen las rservas de las ultimas dos horas
        for (int i = 0; i < parque[hora_actual - 2].num_reservas; i++) {
            Reserva* r = &parque[hora_actual - 2].reservas[i];
            if (r->hora_inicio == hora_actual - 2) { //Si estaban en las ultimas dos horas
                parque[hora_actual - 2].ocupacion -= r->personas; //Se restan del aforo
                if (hora_actual - 1 <= 19)
                    parque[hora_actual - 1].ocupacion -= r->personas;
                saliendo += r->personas;
                strcat(familias_saliendo, r->familia);
                strcat(familias_saliendo, ", ");
            }
        }
        //Quita la coma si solo hay una familia
        if (strlen(familias_saliendo) > 0)
            familias_saliendo[strlen(familias_saliendo) - 2] = '\0';
    }

    int entrando = 0; //Inicializa el contador de personas entrando
    char familias_entrando[512] = ""; //Donde van los nombres de las familias que entraron
    for (int i = 0; i < parque[hora_actual].num_reservas; i++) { //Recoore las reservas
        Reserva* r = &parque[hora_actual].reservas[i]; //Va rserva por reserva
        if (r->hora_inicio == hora_actual) { //Si es la hora de su reserva entran
            parque[hora_actual].ocupacion += r->personas; //Aumenta el aforo
            if (hora_actual + 1 <= 19) //En la siguiente hora ya que estan durante dos horas
                parque[hora_actual + 1].ocupacion += r->personas;
            entrando += r->personas; //Actualiza las personas entrando
            strcat(familias_entrando, r->familia); //Agrega el nombre de la familia
            strcat(familias_entrando, ", ");
        }
    }
    //Elimina la , si solo hay una familia
    if (strlen(familias_entrando) > 0)
        familias_entrando[strlen(familias_entrando) - 2] = '\0';
    //Muestra cuantos salieron y entraron
    printf("Salen: %d personas (%s)\n", saliendo,
           strlen(familias_saliendo) > 0 ? familias_saliendo : "ninguna");
    printf("Entran: %d personas (%s)\n", entrando,
           strlen(familias_entrando) > 0 ? familias_entrando : "ninguna");
    //Avanza la hora en uno
    hora_actual++;
}

// Procesa los mensajes recibidos desde los agentes
void procesar_mensaje(char* msg) {
    //Variables en donde se guardan los datos del mensaje
    char tipo[20], p1[MAX_NOMBRE], p2[MAX_NOMBRE], p3[MAX_NOMBRE], p4[MAX_NOMBRE];
    int n = sscanf(msg, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]", tipo, p1, p2, p3, p4);
    //El mensaje indica que hay un nuevo agente
    if (strcmp(tipo, "REGISTRO") == 0 && n >= 3) {
        registrar_agente(p1, p2); //Lo registra
    } else if (strcmp(tipo, "SOLICITUD") == 0 && n >= 4) { //El mensaje es una solicitud de reserva
        int hora = atoi(p2); //Convierte la entrada a entero
        int personas = atoi(p3); //Convierte el numero de personas a entero
        char respuesta[MAX_BUFFER] = ""; //Buffer para la respuesta
        //Indica que recibio la solicitud
        printf("Recibida solicitud de %s: familia %s, hora %d, %d personas\n",
               p4, p1, hora, personas);
        //Llama la funcion de rservas
        intentar_reserva(p1, hora, personas, p4, respuesta);

        char pipe_agente[MAX_NOMBRE] = ""; //Variable para el nombre del pipe del agente
        for (int i = 0; i < num_agentes; i++) { //Recorre los agentes
            if (strcmp(agentes[i].nombre, p4) == 0) { //Si es el que envio la solicitud
                strcpy(pipe_agente, agentes[i].pipe_respuesta); //Obtiene una copia del nombre
                break;
            }
        }

        if (strlen(pipe_agente) > 0) //Si lo encontro le envia la respuesta
            enviar_respuesta(pipe_agente, respuesta);
        else //En caso contrario muestra un error
            fprintf(stderr, "Error: no se encontró el agente %s para enviar respuesta.\n", p4);
    }
}

// Registra un nuevo agente en el sistema
//Registra un nuevo agente si todavia no ha alcanzado el maximo de agentes posibles
int registrar_agente(char* nombre, char* pipe_resp) {
    if (num_agentes >= MAX_AGENTES) return 0;
    for (int i = 0; i < num_agentes; i++) //Recorre a los agntes para ver que no quede duplicado
        if (strcmp(agentes[i].nombre, nombre) == 0) return 0;

    strcpy(agentes[num_agentes].nombre, nombre); //Copia el nombre del agente al arreglo
    strcpy(agentes[num_agentes].pipe_respuesta, pipe_resp); //Guarda el nombre del pipe usado para respuestas
    agentes[num_agentes].activo = 1; //Indica que el agente esta activo
    num_agentes++; //Aumenta el numero de agentes
    //Le confirma que quedo registrado y le envia la hora actual
    char msg[50];
    sprintf(msg, "HORA|%d", hora_actual);
    enviar_respuesta(pipe_resp, msg);
    return 1;
}

// Envia una respuesta a un agente mediante su pipe especifico
void enviar_respuesta(char* pipe_resp, char* msg) {
    int fd = open(pipe_resp, O_WRONLY); //Abre el pipe
    if (fd != -1) { //Si logra abrirlo escribe un mensaje
        write(fd, msg, strlen(msg));
        close(fd); //Cierra el pipe
    }
}

// Intenta reservar basado en la disponibilidad del parque
int intentar_reserva(char* familia, int hora, int personas, char* agente, char* resp) {
    if (personas > aforo_max) { //Revisa que todavia se pueden meter mas personas
        sprintf(resp, "NEGADA|%s", familia); //Como ya esta lleno la niega
        solicitudes_negadas++; //Aumenta el contador de solicitudes negadas
        return 4;
    }
    //Si la hora ya paso lo re agenda
    if (hora < hora_actual) {
        int nueva; //para la nueva hora
        if (buscar_bloque_libre(personas, &nueva)) { //Busca dos horas libres
            Reserva r = { "", nueva, personas, "" }; //Crea una nueva reserva
            strcpy(r.familia, familia); //Pone en la nueva rserva el nombre de la familia
            strcpy(r.agente, agente); //Pone en la nueva rserva el nombre del agente
            parque[nueva].reservas[parque[nueva].num_reservas++] = r; //Anade la nueva rserva
            if (nueva + 1 <= 19) //Como las reservas son dos horas, la siguiente
                parque[nueva + 1].reservas[parque[nueva + 1].num_reservas++] = r; 
            parque[nueva].ocupacion += personas; //Aumenta la ocupacion
            if (nueva + 1 <= 19)
                parque[nueva + 1].ocupacion += personas;
            sprintf(resp, "REPROGRAMADA|%s|%d", familia, nueva);
            solicitudes_reprogramadas++; //Incrementa el contador de reprogramadas
            return 2;
        } else {
            sprintf(resp, "NEGADA_EXT|%s", familia); //Mensaje de rechazo
            solicitudes_negadas++; //Aumenta el contador de rechazos
            return 3;
        }
    }

    if (hora + 1 > hora_fin) { //Si la hora no esta dentro dle horario de atencion
        sprintf(resp, "NEGADA|%s", familia); //La rechaza, aumenta el contador y muetsra el mensaje de error
        solicitudes_negadas++;
        return 4;
    }
    //Si hay espacio a la hora que pidieron
    if (parque[hora].ocupacion + personas <= aforo_max &&
        parque[hora + 1].ocupacion + personas <= aforo_max) {
        Reserva r = { "", hora, personas, "" }; //Crea la resrva
        strcpy(r.familia, familia); //Anade a la familia
        strcpy(r.agente, agente); //Anade al agente
        parque[hora].reservas[parque[hora].num_reservas++] = r;
        if (hora + 1 <= 19) //Tambien lo anade para la segunda hora de la reserva
            parque[hora + 1].reservas[parque[hora + 1].num_reservas++] = r; 
        parque[hora].ocupacion += personas; //Actualiza la ocupacion en esa hora
        if (hora + 1 <= 19)
            parque[hora + 1].ocupacion += personas; //Actualiza la ocupacion en esa hora
        sprintf(resp, "OK|%s|%d", familia, hora);
        solicitudes_aceptadas++; //Aumenta el contador de solicitudes esperadas
        return 1;
    } else {
        int nueva;
        if (buscar_bloque_libre(personas, &nueva)) { //Busca otras horas
            Reserva r = { "", nueva, personas, "" }; //Crea una nueva rserva
            strcpy(r.familia, familia); //Anade a la familia
            strcpy(r.agente, agente); //Anade al agente
            parque[nueva].reservas[parque[nueva].num_reservas++] = r; //Guarda la reserva
            if (nueva + 1 <= 19)
                parque[nueva + 1].reservas[parque[nueva + 1].num_reservas++] = r;
            parque[nueva].ocupacion += personas; //Actualiza la ocupacion
            if (nueva + 1 <= 19)
                parque[nueva + 1].ocupacion += personas;
            sprintf(resp, "REPROGRAMADA|%s|%d", familia, nueva);
            solicitudes_reprogramadas++; //Aumenta las reprogamadas
            return 2;
        } else { //Si no lo logro, la rechaza
            sprintf(resp, "NEGADA|%s", familia);
            solicitudes_negadas++; //Aumenta el contador de negadas
            return 4;
        }
    }
}

// Busca un bloque libre para reprogramar una reserva
int buscar_bloque_libre(int personas, int* nueva_hora) {
    for (int h = hora_actual; h <= hora_fin - 1 && h + 1 <= 19; h++) { //Recorre las horas
        if (parque[h].ocupacion + personas <= aforo_max && //Si hay una y la siguiente libres la asigna
            parque[h + 1].ocupacion + personas <= aforo_max) {
            *nueva_hora = h;
            return 1;
        }
    }
    return 0;
}

//Muestra un reporte final de la simulacion
void imprimir_reporte() {
    //Variables
    int max_p = 0, min_p = 100000; //Maximo y mino de personas
    int horas_pico[13], horas_valle[13], np = 0, nv = 0; //Para hora pico y mas vacia

    for (int h = 7; h <= 19; h++) { //Recorre todas las horas
        int p = parque[h].ocupacion; //Obtiene la ocupacion
        if (p > max_p) { max_p = p; np = 0; horas_pico[np++] = h; }
        else if (p == max_p) horas_pico[np++] = h; //Si es mayor o igual la anade a pico
        if (p < min_p) { min_p = p; nv = 0; horas_valle[nv++] = h; }
        else if (p == min_p) horas_valle[nv++] = h; //Si es menor o igual la anade a pico
    }

    printf("Horas pico: ");
    for (int i = 0; i < np; i++) printf("%d ", horas_pico[i]);
    printf("(%d personas)\n", max_p);
    printf("Horas valle: "); //Muestra las horas pico
    for (int i = 0; i < nv; i++) printf("%d ", horas_valle[i]); //Muestra las horas valle
    printf("(%d personas)\n", min_p);
    printf("Solicitudes negadas: %d\n", solicitudes_negadas); //Muestra cuantas solicitudes fueron negadas
    printf("Solicitudes aceptadas en su hora: %d\n", solicitudes_aceptadas); //Muestra cuantas solicitudes fueron aceptadas
    printf("Solicitudes re-programadas: %d\n", solicitudes_reprogramadas);  //Muestra cuantas solicitudes fueron reprogramadas
}

//Le indica todos los agentes que se termino la simulacion
void terminar_agentes() {
    for (int i = 0; i < num_agentes; i++) //Pasa por los agentes
        if (agentes[i].activo) //Si esta activo, le dice que termine
            enviar_respuesta(agentes[i].pipe_respuesta, "TERMINAR");
}
//limpia los recursos y borra el pipe del controlador
void limpiar_recursos() {
    close(fd_pipe_entrada); //Ciera el pipe del controlador
    unlink(pipe_entrada); //elimina el fifo
}

/******************************************************
* CONCLUSIÓN
*
* Este archivo genera el motor del controlador,
* ejecutando las funciones críticas que permiten la
* simulación completa del parque. El uso de hilos
* independientes para el reloj y la gestión de solicitudes
* garantiza concurrencia realista y evita bloqueos entre
* las distintas entidades del sistema.
*
* La implementación del algoritmo de reservas, con validación
* de aforo, detección de conflictos y búsqueda de bloques
* disponibles, asegura un manejo eficiente y coherente de las
* solicitudes. Igualmente, la comunicación mediante pipes
* permite una interacción clara y ordenada entre agentes y
* controlador.
*
* Finalmente, el reporte generado al concluir la simulación
* ofrece una visión global del comportamiento del sistema,
* evidenciando horas pico, horas valle y estadísticas clave
* de aceptación, reprogramación y negación de reservas.
* Con esto, se cumple exitosamente el objetivo del proyecto:
* modelar un entorno concurrente estable, sincronizado y
* funcional.
******************************************************/
