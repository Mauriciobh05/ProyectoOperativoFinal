README — 
Proyecto Final Sistemas Operativos
Sistema de Reservas Concurrente con Procesos, Hilos y Pipes (FIFO)
Pontificia Universidad Javeriana — 2025
Profesor: J. Corredor
Autores: Mauricio Beltrán, Alejandro Beltrán & Andrés Díaz

1) DESCRIPCIÓN GENERAL

Este proyecto implementa un sistema concurrente de reservas para un parque mediante procesos, hilos y comunicación por pipes FIFO. El sistema está compuesto por dos programas principales: el controlador, que administra horarios, aforo, solicitudes y simulación del tiempo; y los agentes, que representan usuarios externos que envían solicitudes de reserva.

El controlador recibe peticiones de múltiples agentes, revisa disponibilidad, reprograma si es necesario y responde a cada agente por un pipe independiente. Además, simula el paso del tiempo usando hilos POSIX y actualiza la ocupación del parque hora tras hora. El proyecto integra conceptos fundamentales de sistemas operativos como concurrencia, IPC, syscalls, hilos, sincronización y modularidad.

2) ARQUITECTURA DEL PROYECTO

controlador.c
controlador_funciones.c
controlador_funciones.h
agente.c
agente_funciones.c
agente_funciones.h
makefile
README.md

3) CÓMO COMPILAR

Abrir una terminal en la carpeta del proyecto.

Ejecutar:

make clean
make

Esto generará los binarios "controlador" y "agente".

4) CÓMO EJECUTAR EL CONTROLADOR

Ejemplo:

./controlador -i 7 -f 19 -s 2 -t 20 -p pipeCONTROLADOR

Parámetros:
-i : hora inicial del parque
-f : hora final del parque
-s : número de segundos que dura una hora simulada
-t : aforo máximo permitido
-p : nombre del pipe principal usado para recibir solicitudes

5) CÓMO EJECUTAR UN AGENTE

Ejemplo:

./agente -s A1 -a solicitudesA1.csv -p pipeCONTROLADOR

Parámetros:
-s : nombre del agente
-a : archivo CSV con las solicitudes
-p : pipe principal del controlador

Se pueden ejecutar múltiples agentes en distintas terminales.

6) FORMATO DEL ARCHIVO CSV

Ejemplo:

Zuluaga,8,10
Garcia,9,8
Dominguez,8,4
Lopez,11,12

Formato:
Familia,Hora,Personas

FORMATO DE LOS MENSAJES ENTRE PROCESOS

Mensajes enviados al controlador:
REGISTRO|Agente|PipePropio
SOLICITUD|Familia|Hora|Personas|Agente

Mensajes enviados a los agentes:
OK
REPROGRAMADA|NuevaHora
NEGADA
NEGADA_EXT

7) CONCEPTOS DE SISTEMAS OPERATIVOS UTILIZADOS

Procesos POSIX
Hilos POSIX (pthread)
Comunicación entre procesos (IPC)
Pipes FIFO (mkfifo, open, read, write, unlink)
Syscalls del sistema operativo
Concurrencia y sincronización
Estructuras de datos compartidas
Condiciones de carrera
Simulación del tiempo con hilos
Modularidad y manejo ordenado de recursos

Este proyecto aplica todos estos conceptos en un sistema distribuido real.

8) FUNCIONAMIENTO DEL CONTROLADOR

• Crea su pipe principal.
• Acepta registros de agentes.
• Guarda información de horarios, aforo y reservas.
• Usa un hilo para avanzar la simulación del tiempo.
• Usa otro hilo para recibir y procesar solicitudes.
• Revisa disponibilidad y aprueba, reprograma o rechaza solicitudes.
• Lleva un conteo de reservas aceptadas, reprogramadas y negadas.
• Envía respuestas a cada agente mediante su pipe propio.
• Cierra y elimina los pipes al finalizar.

9) FUNCIONAMIENTO DEL AGENTE

• Lee parámetros recibidos por consola.
• Crea un pipe propio para recibir respuestas.
• Se registra ante el controlador.
• Espera la hora inicial.
• Lee solicitudes desde un archivo CSV.
• Envía cada solicitud y espera la respuesta.
• Ignora solicitudes cuya hora ya pasó.
• Cierra y elimina sus pipes al finalizar.

10) LIMPIEZA DE ARCHIVOS TEMPORALES

Para borrar ejecutables y pipes creados durante la ejecución:

make clean

11) EJEMPLO DE EJECUCIÓN COMPLETA

Terminal 1:
./controlador -i 7 -f 19 -s 2 -t 20 -p pipeCONTROLADOR

Terminal 2:
./agente -s A1 -a solicitudesA1.csv -p pipeCONTROLADOR

Terminal 3:
./agente -s A2 -a solicitudesA2.csv -p pipeCONTROLADOR

Y así sucesivamente.

12) CONCLUSIÓN DEL PROYECTO

Este proyecto demuestra el uso real de procesos, hilos e IPC para construir un sistema distribuido capaz de manejar solicitudes concurrentes. Implementa correctamente comunicación mediante pipes FIFO, simulación del tiempo, control de aforo y ejecución paralela organizada. El resultado refleja una comprensión sólida de los conceptos centrales de sistemas operativos y su aplicación práctica en un entorno modular y funcional.
