/*
 Código del servidor del juego
 Editado por última vez: 20/10/2024
 Autores: Adria Sancho, Ivan de Dios, Arnau Torrent y Oscar Rigol
*/

#include <mysql.h>   // Para trabajar con MySQL
#include <string.h>  // Para manipular cadenas de caracteres
#include <stdlib.h>  // Para funciones generales de utilidad
#include <stdio.h>   // Para funciones de entrada/salida
#include <unistd.h>  // Para funciones POSIX (como `read`, `write`, etc.)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>  // Necesario para trabajar con hilos
#include <errno.h>  // Para manejar errores


/*==============================================================================================================================*/
/*====================================================== Variables globales ====================================================*/

#define MAX_USERS 100
#define MAX_NOTIFICACIONES 100
#define MAX_JUGADORES_POR_PARTIDA 3
#define MAX_PARTIDAS 100
#define MAX_MATCHMAKING_QUEUE 100

int sock_listen;

/*============================================================== Estructuras ===============================================================*/

typedef struct {
    int socket;
    char nombre_usuario[50];
	int partida_id;                                                                 // ID de la partida, -1 si no está en una
} UsuarioInfo;

typedef struct {
    int id;
    int jugadores[MAX_JUGADORES_POR_PARTIDA];                                       // Sockets de los jugadores
    char nombres[MAX_JUGADORES_POR_PARTIDA][50];                                    // Nombres de los jugadores
    int num_jugadores;
} Partida;

typedef struct {
    int socket;
    char nombre_usuario[50];
} UsuarioMatchmaking;

typedef struct {
    char mensaje[512];
} Notificacion;

/*============================================================== Listas ===============================================================*/

UsuarioInfo usuarios_conectados[MAX_USERS];
int num_usuarios_conectados = 0;  

Partida partidas_activas[MAX_PARTIDAS];
int num_partidas_activas = 0;

UsuarioMatchmaking cola_matchmaking[MAX_MATCHMAKING_QUEUE];
int num_matchmaking = 0;

Notificacion cola_notificaciones[MAX_NOTIFICACIONES];                               //Notificaciones pendientes, en orden
int num_notificaciones = 0;

/*============================================================== Mutexes ===============================================================*/

pthread_mutex_t partidas_mutex = PTHREAD_MUTEX_INITIALIZER;                         // Mutex para la lista de partidas

pthread_mutex_t matchmaking_mutex = PTHREAD_MUTEX_INITIALIZER;                      // Mutex para la cola de matchmaking

pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER; 						    //Mutex para aceptar conexiones 
pthread_cond_t accept_cond = PTHREAD_COND_INITIALIZER;								//Variable de condicion para activar hilos

pthread_mutex_t lista_mutex = PTHREAD_MUTEX_INITIALIZER; 						    //Mutex para la lista de usuarios conectados     

pthread_mutex_t notificacion_mutex = PTHREAD_MUTEX_INITIALIZER;                     //Mutex para la cola de notificaciones
pthread_cond_t notificacion_cond = PTHREAD_COND_INITIALIZER;                        //Variable de condicion para activar hilos


/*==============================================================================================================================*/
/*=================================================== Funciones auxiliares =====================================================*/

/*=================================================== MySQL =====================================================*/
void conectarMySQL(MYSQL** conn) 
{                                                                                   // Función para conectar a MySQL
    *conn = mysql_init(NULL);
    if (*conn == NULL) {
        printf("Error al crear la conexión: %u %s\n", mysql_errno(*conn), mysql_error(*conn));
        exit(1);
    }
}

void entrarBD(MYSQL* conn) 
{														                            // Función para entrar en la base de datos  
    if (mysql_real_connect(conn, "shiva2.upc.es", "root", "mysql", "M3_UsuariosBBDD", 0, NULL, 0) == NULL) {
        printf("Error al inicializar la conexión: %u %s\n", mysql_errno(conn), mysql_error(conn));
        exit(1);
    }
}

/*=================================================== Notificaciones =====================================================*/

void encolar_notificacion() 
{
    pthread_mutex_lock(&notificacion_mutex);  
    if (num_notificaciones < MAX_NOTIFICACIONES) {
		strcpy(cola_notificaciones[num_notificaciones].mensaje, "UPDATE_LIST");                 //La notificación actualizara la lista de usuarios
        num_notificaciones++;
		pthread_cond_signal(&notificacion_cond);                                                // Despertar a un hilo en espera
    }
    else {
        printf("Cola de notificaciones llena. No se puede encolar la notificación.\n");
    }
    pthread_mutex_unlock(&notificacion_mutex);
}
/*=================================================================================================================*/
/*=================================================== Funciones para partidas =====================================================*/

// Función para crear una nueva partida
int crear_partida() {
    pthread_mutex_lock(&partidas_mutex);
    if (num_partidas_activas >= MAX_PARTIDAS) {
        pthread_mutex_unlock(&partidas_mutex);
        return -1; // No hay espacio para nuevas partidas
    }

    Partida nueva_partida;
    nueva_partida.id = num_partidas_activas + 1; // Simple incremento, puede mejorarse
    nueva_partida.num_jugadores = 0;

    partidas_activas[num_partidas_activas] = nueva_partida;
    num_partidas_activas++;
    pthread_mutex_unlock(&partidas_mutex);
    printf("partida creada");
    return nueva_partida.id;
}

// Función para encontrar una partida disponible para añadir un jugador
int asignar_a_partida(int sock_conn, char* nombre_usuario) {
    pthread_mutex_lock(&partidas_mutex);
    for (int i = 0; i < num_partidas_activas; i++) {
        if (partidas_activas[i].num_jugadores < MAX_JUGADORES_POR_PARTIDA) {
            partidas_activas[i].jugadores[partidas_activas[i].num_jugadores] = sock_conn;
            strncpy(partidas_activas[i].nombres[partidas_activas[i].num_jugadores], nombre_usuario, 49);
            partidas_activas[i].nombres[partidas_activas[i].num_jugadores][49] = '\0';
            partidas_activas[i].num_jugadores++;

            // Si la partida está completa, iniciar la partida
            if (partidas_activas[i].num_jugadores == MAX_JUGADORES_POR_PARTIDA) {
                // Notificar a los jugadores que la partida ha comenzado
                for (int j = 0; j < MAX_JUGADORES_POR_PARTIDA; j++) {
                    char mensaje_inicio[] = "PARTIDA_INICIADA";
                    write(partidas_activas[i].jugadores[j], mensaje_inicio, strlen(mensaje_inicio));
                }
            }

            pthread_mutex_unlock(&partidas_mutex);
            return partidas_activas[i].id;
        }
    }
    pthread_mutex_unlock(&partidas_mutex);
    return -1; // No hay partidas disponibles
}

// Función para eliminar una partida (después de que finalice)
void eliminar_partida(int id) {
    pthread_mutex_lock(&partidas_mutex);
    for (int i = 0; i < num_partidas_activas; i++) {
        if (partidas_activas[i].id == id) {
            // Mover la última partida a la posición actual para mantener la lista compacta
            partidas_activas[i] = partidas_activas[num_partidas_activas - 1];
            num_partidas_activas--;
            break;
        }
    }
    pthread_mutex_unlock(&partidas_mutex);
}

/*==============================================================================================================================*/
/*============================================= Funciones manejo entrada y salida de usuarios ================================================*/

void anadir_usuario(int sock_conn, char* usuario)                                               // Función para añadir un usuario a la lista de conectados
{
    pthread_mutex_lock(&lista_mutex);

    if (num_usuarios_conectados < MAX_USERS) {
        usuarios_conectados[num_usuarios_conectados].socket = sock_conn;
        strncpy(usuarios_conectados[num_usuarios_conectados].nombre_usuario, usuario, sizeof(usuarios_conectados[num_usuarios_conectados].nombre_usuario) - 1);
        usuarios_conectados[num_usuarios_conectados].nombre_usuario[sizeof(usuarios_conectados[num_usuarios_conectados].nombre_usuario) - 1] = '\0';
        usuarios_conectados[num_usuarios_conectados].partida_id = -1;                           // No está en ninguna partida
        num_usuarios_conectados++;

        printf("Jugador conectado: %s\n", usuario);
        printf("Número de jugadores conectados: %d\n", num_usuarios_conectados);

        pthread_mutex_unlock(&lista_mutex);

		encolar_notificacion();                                                                 // Enviar notificación de actualización de lista            
        printf("Notificacion de actualización de lista\n");
    }
    else {
        pthread_mutex_unlock(&lista_mutex);
		printf("Lista de usuarios llena. No se puede añadir más usuarios.\n");
    }
}

void quitar_usuario(int sock_fd, char* nombre_usuario) 
{
    pthread_mutex_lock(&lista_mutex);

    for (int i = 0; i < num_usuarios_conectados; i++) {
        if (usuarios_conectados[i].socket == sock_fd) {
            strcpy(nombre_usuario, usuarios_conectados[i].nombre_usuario);

			for (int j = i; j < num_usuarios_conectados - 1; j++) {                             // Mover los usuarios restantes
                usuarios_conectados[j] = usuarios_conectados[j + 1];
            }
            num_usuarios_conectados--;

            printf("Jugador desconectado: %s\n", nombre_usuario);

            pthread_mutex_unlock(&lista_mutex);

			encolar_notificacion();                                                             // Enviar notificación de actualización de lista    
            printf("Notificacion de actualizacion de lista");
            return;
        }
    }
    pthread_mutex_unlock(&lista_mutex);
}

/*==============================================================================================================================*/
/*============================================= Funciones-query preestablecidas ================================================*/

void buscarUserAndPassBBDD(MYSQL* conn, int sock_conn, char* usuario, char* contrasena) {                       //Autenticar usuario
    int encontrado = 0; 
    int err;
    MYSQL_RES* resultado;
    MYSQL_ROW row;

    char query[512];
    snprintf(query, sizeof(query), "SELECT * FROM Usuarios WHERE usuario='%s' AND contrasena='%s';", usuario, contrasena);
    err = mysql_query(conn, query);

    if (err != 0) {
        printf("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
        exit(1);
    }

    resultado = mysql_store_result(conn);
    row = mysql_fetch_row(resultado);

    if (row != NULL) {
        encontrado = 1;
    }

    if (encontrado == 1) {
        char respuesta[] = "OK";
        write(sock_conn, respuesta, strlen(respuesta));

        // Añadir usuario a la lista de conectados
        anadir_usuario(sock_conn, usuario);
		printf("Usuario autenticado: %s\n", usuario);
    }
    else {
        char respuesta[] = "ERROR";
        write(sock_conn, respuesta, strlen(respuesta));
    }

    mysql_free_result(resultado);
}

void buscarUserAndChangePassBBDD(MYSQL* conn, int sock_conn, char* usuario, char* nueva_contrasena) {               // Función para cambiar la contraseña del usuario
    int err;
    char query[512];
    snprintf(query, sizeof(query), "UPDATE Usuarios SET contrasena='%s' WHERE usuario='%s';", nueva_contrasena, usuario);

    err = mysql_query(conn, query);
    if (err != 0) {
        printf("Error al actualizar la contraseña: %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR_ACTUALIZACION";
        write(sock_conn, respuesta, strlen(respuesta));
    }
    else {
        printf("Contraseña actualizada para el usuario: %s\n", usuario);
        char respuesta[] = "OK";
        write(sock_conn, respuesta, strlen(respuesta));
    }
}

void insertarUsuario(MYSQL* conn, int sock_conn, char* usuario, char* contrasena) 
{                                                                                           // Función para insertar un nuevo usuario en la base de datos
    int err;
    char query[512];
    snprintf(query, sizeof(query), "INSERT INTO Usuarios (usuario, contrasena) VALUES ('%s', '%s');", usuario, contrasena);

    err = mysql_query(conn, query);
    if (err != 0) {
        printf("Error al insertar el usuario %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR";
        write(sock_conn, respuesta, strlen(respuesta));
    }
    else {
        printf("Usuario '%s' insertado correctamente.\n", usuario);
        char respuesta[] = "OK";
        write(sock_conn, respuesta, strlen(respuesta));
    }
}

void crearAndBorrarBBDD(MYSQL* conn, int sock_conn) 
    {																                        // Función para crear y borrar la base de datos        
    int err;

    err = mysql_query(conn, "DROP DATABASE IF EXISTS M3_UsuariosBBDD;");
    if (err != 0) {
        printf("Error al eliminar la base de datos: %u %s\n", mysql_errno(conn), mysql_error(conn));
    }
    else {
        printf("Base de datos eliminada correctamente.\n");
    }

    err = mysql_query(conn, "CREATE DATABASE M3_UsuariosBBDD;");
    if (err != 0) {
        printf("Error al crear la base de datos: %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR_CREAR_BD";
        write(sock_conn, respuesta, strlen(respuesta));
        return;
    }
    else {
        printf("Base de datos creada correctamente.\n");
    }

    err = mysql_query(conn, "USE M3_UsuariosBBDD;");
    if (err != 0) {
        printf("Error al usar la base de datos: %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR_USAR_BD";
        write(sock_conn, respuesta, strlen(respuesta));
        return;
    }

    err = mysql_query(conn, "CREATE TABLE Usuarios (id INT PRIMARY KEY NOT NULL AUTO_INCREMENT, usuario TEXT NOT NULL, contrasena TEXT NOT NULL);");
    if (err != 0) {
        printf("Error al crear la tabla Usuarios: %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR_CREAR_TABLA";
        write(sock_conn, respuesta, strlen(respuesta));
        return;
    }
    else {
        printf("Tabla Usuarios creada correctamente.\n");
    }

    err = mysql_query(conn, "INSERT INTO Usuarios (usuario, contrasena) VALUES ('root', 'root');");
    if (err != 0) {
        printf("Error al insertar el usuario predeterminado %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR_INSERTAR_USUARIO";
        write(sock_conn, respuesta, strlen(respuesta));
        return;
    }
    else {
        printf("Usuario predeterminado insertado correctamente: root, root\n");
    }

    char respuesta[] = "OK";
    write(sock_conn, respuesta, strlen(respuesta));
}

void contarUsuariosBBDD(MYSQL* conn, int sock_conn) 
{																                    // Función para contar el número de usuarios en la base de datos
    int err;
    MYSQL_RES* resultado;
    MYSQL_ROW row;
    int totalUsuarios = 0;

    err = mysql_query(conn, "SELECT COUNT(*) FROM Usuarios;");
    if (err != 0) {
        printf("Error al contar los usuarios en la base de datos %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR_CONTAR_USUARIOS";
        write(sock_conn, respuesta, strlen(respuesta));
        return;
    }
    else {
        resultado = mysql_store_result(conn);
        if (resultado == NULL) {
            printf("Error al obtener el resultado de la consulta %u %s\n", mysql_errno(conn), mysql_error(conn));
            char respuesta[] = "ERROR_RESULTADO";
            write(sock_conn, respuesta, strlen(respuesta));
            return;
        }

        row = mysql_fetch_row(resultado);
        if (row != NULL) {
            totalUsuarios = atoi(row[0]);
            printf("El número total de usuarios es: %d\n", totalUsuarios);
        }
        mysql_free_result(resultado);
    }

    char respuesta[512];
    snprintf(respuesta, sizeof(respuesta), "TOTAL_USERS:%d", totalUsuarios);
    write(sock_conn, respuesta, strlen(respuesta));
}

int encontrar_socket_por_usuario(const char* nombre_usuario) 
{																						   // Función para encontrar el socket de un usuario por su nombre  
    for (int i = 0; i < num_usuarios_conectados; i++) {
        if (strcmp(usuarios_conectados[i].nombre_usuario, nombre_usuario) == 0) {
            printf("%s\n", usuarios_conectados[i].nombre_usuario);
            return usuarios_conectados[i].socket;

        }
    }
    return -1; 
}


/*==============================================================================================================================*/
/*============================== Funciones para procesar los mensajes recibidos desde el cliente ===============================*/

void procesar_mensaje(int sock_conn, char* mensaje, char* usuario) 
{
    MYSQL* conn;
    conectarMySQL(&conn);

    if (strcmp(mensaje, "CREAR_BD") == 0) {
        printf("Recibido mensaje para crear la base de datos\n");
        crearAndBorrarBBDD(conn, sock_conn);
    }

    else if (strcmp(mensaje, "CONTAR_USERS") == 0) 
    {
        printf("Recibido mensaje para contar usuarios\n");
        entrarBD(conn);
        contarUsuariosBBDD(conn, sock_conn);
    }

    else if (strncmp(mensaje, "AUTENTICAR/", 11) == 0) 
    {
        char* usuario_msg = strtok(mensaje + 11, "/");
        char* contrasena = strtok(NULL, "/");

        if (usuario_msg && contrasena) {
            strcpy(usuario, usuario_msg);  
            entrarBD(conn);
            buscarUserAndPassBBDD(conn, sock_conn, usuario, contrasena);
        }
    }

    else if (strncmp(mensaje, "CAMBIAR_PASS/", 13) == 0) 
    {
        char* usuario_msg = strtok(mensaje + 13, "/");
        char* nueva_contrasena = strtok(NULL, "/");

        if (usuario_msg && nueva_contrasena) {
            entrarBD(conn);
            buscarUserAndChangePassBBDD(conn, sock_conn, usuario_msg, nueva_contrasena);
        }
    }

    else if (strncmp(mensaje, "ADD_USER/", 9) == 0) {
        char* usuario_msg = strtok(mensaje + 9, "/");
        char* contrasena = strtok(NULL, "/");

        if (usuario_msg && contrasena) {
            entrarBD(conn);
            insertarUsuario(conn, sock_conn, usuario_msg, contrasena);
        }
    }


    else if (strncmp(mensaje, "INVITAR/", 8) == 0) {
        char* target_username = strtok(mensaje + 8, "/");
        char* source_username = strtok(NULL, "/");

        if (target_username && source_username) 
        {
            pthread_mutex_lock(&lista_mutex);
            int target_socket = encontrar_socket_por_usuario(target_username);
            pthread_mutex_unlock(&lista_mutex);

            if (target_socket != -1) {
                // Comprobar si la partida del source ya tiene 2 jugadores
                int source_partida = -1;
                pthread_mutex_lock(&lista_mutex);
                for (int i = 0; i < num_usuarios_conectados; i++) {
                    if (usuarios_conectados[i].socket == sock_conn) {
                        source_partida = usuarios_conectados[i].partida_id;
                        break;
                    }
                }
                pthread_mutex_unlock(&lista_mutex);
                if (source_partida != -1) {
                    // Obtener el número de jugadores en la partida
                    pthread_mutex_lock(&partidas_mutex);
                    int num_jugadores = 0;
                    for (int i = 0; i < num_partidas_activas; i++) {
                        if (partidas_activas[i].id == source_partida) {
                            num_jugadores = partidas_activas[i].num_jugadores;
                            break;
                        }
                    }
                    pthread_mutex_unlock(&partidas_mutex);

                    if (num_jugadores >= MAX_JUGADORES_POR_PARTIDA) {
                        char error_msg[] = "ERROR_INVITACION: Tu partida ya está llena.";
                        write(sock_conn, error_msg, strlen(error_msg));
                        printf("Error: Partida del usuario %s ya está llena.\n", source_username);
                        return;
                    }
                }

                // Enviar invitación al usuario objetivo
                char invitacion[256];
                snprintf(invitacion, sizeof(invitacion), "INVITACION:%s", source_username);
                write(target_socket, invitacion, strlen(invitacion));
                printf("Enviada invitación de %s a %s\n", source_username, target_username);
			}
			else {
				// Enviar error al usuario que invita
				char error_msg[256];
				snprintf(error_msg, sizeof(error_msg), "ERROR_INVITACION:%s no está conectado.", target_username);
				write(sock_conn, error_msg, strlen(error_msg));
				printf("Error: %s no está conectado.\n", target_username);
			}
        }
    }
    else if (strncmp(mensaje, "INVITACION_ACEPTADA/", 20) == 0) {
        char* source_username = strtok(mensaje + 20, "/");
        char* target_username = strtok(NULL, "/");

        if (source_username && target_username) {
            printf("Usuario '%s' ha aceptado la invitación de '%s'.\n", target_username, source_username);

            // Encontrar el socket del usuario que invitó
            pthread_mutex_lock(&lista_mutex);
            int source_socket = encontrar_socket_por_usuario(source_username);
            pthread_mutex_unlock(&lista_mutex);

            if (source_socket != -1) {
                // Obtener la partida del source_username
                pthread_mutex_lock(&lista_mutex);
                int source_partida = -1;
                for (int i = 0; i < num_usuarios_conectados; i++) {
                    if (strcmp(usuarios_conectados[i].nombre_usuario, source_username) == 0) {
                        source_partida = usuarios_conectados[i].partida_id;
                        break;
                    }
                }
                pthread_mutex_unlock(&lista_mutex);

                printf("Usuario '%s' está en partida ID: %d\n", source_username, source_partida);

                if (source_partida == -1) {
                    // Crear una nueva partida si el source no tiene una
                    int nueva_partida_id = crear_partida();
					
                    if (nueva_partida_id == -1) {
                        char error_msg[] = "ERROR_PARTIDA: No se pudo crear una nueva partida.\n";
                        write(sock_conn, error_msg, strlen(error_msg));
                        printf("Error: No se pudo crear una nueva partida.\n");
                        return;
                    }

                    // Asignar el source a la nueva partida
                    pthread_mutex_lock(&lista_mutex);
                    
                    for (int i = 0; i < num_usuarios_conectados; i++) {
                        if (usuarios_conectados[i].socket == source_socket) {
                            usuarios_conectados[i].partida_id = nueva_partida_id;
                            printf("Asignando usuario '%s' a nueva partida ID: %d\n", target_username, nueva_partida_id);
                            break;
                        }
                    }
                    pthread_mutex_unlock(&lista_mutex);
                    source_partida = nueva_partida_id;
                }
                 // Añadir el source_username a la nueva partida
                pthread_mutex_lock(&partidas_mutex);
                for (int i = 0; i < num_partidas_activas; i++) {
                    if (partidas_activas[i].id == source_partida) {
                        partidas_activas[i].jugadores[partidas_activas[i].num_jugadores] = source_socket;
                        strncpy(partidas_activas[i].nombres[partidas_activas[i].num_jugadores], source_username, 49);
                        partidas_activas[i].nombres[partidas_activas[i].num_jugadores][49] = '\0';
                        partidas_activas[i].num_jugadores++;

                        printf("Añadiendo usuario '%s' a partida ID: %d\n", source_username, source_partida);
                        printf("Número de jugadores en la partida ID %d: %d\n", source_partida, partidas_activas[i].num_jugadores);

                        // Notificar al source user que ha sido asignado a la partida
                        char mensaje_partida[] = "HAS_SIDO_ASIGNADO_A_PARTIDA\n";
                        write(source_socket, mensaje_partida, strlen(mensaje_partida));
                        printf("Enviando mensaje 'HAS_SIDO_ASIGNADO_A_PARTIDA' a socket %d\n", source_socket);

                        // Si la partida está completa, iniciar la partida
                        if (partidas_activas[i].num_jugadores == MAX_JUGADORES_POR_PARTIDA) {
                            for (int j = 0; j < MAX_JUGADORES_POR_PARTIDA; j++) {
                                char mensaje_inicio[] = "PARTIDA_INICIADA\n";
                                write(partidas_activas[i].jugadores[j], mensaje_inicio, strlen(mensaje_inicio));
                                printf("Enviando mensaje 'PARTIDA_INICIADA' a socket %d\n", partidas_activas[i].jugadores[j]);
                            }
                        }

                        break;
                    }
                }
                pthread_mutex_unlock(&partidas_mutex);
            

                 // Añadir el source_username a la nueva partida
                pthread_mutex_lock(&partidas_mutex);
                for (int i = 0; i < num_partidas_activas; i++) {
                    if (partidas_activas[i].id == source_partida) {
                        partidas_activas[i].jugadores[partidas_activas[i].num_jugadores] = source_socket;
                        strncpy(partidas_activas[i].nombres[partidas_activas[i].num_jugadores], source_username, 49);
                        partidas_activas[i].nombres[partidas_activas[i].num_jugadores][49] = '\0';
                        partidas_activas[i].num_jugadores++;

                        printf("Añadiendo usuario '%s' a partida ID: %d\n", source_username, source_partida);
                        printf("Número de jugadores en la partida ID %d: %d\n", source_partida, partidas_activas[i].num_jugadores);

                        // Notificar al source user que ha sido asignado a la partida
                        char mensaje_partida[] = "HAS_SIDO_ASIGNADO_A_PARTIDA\n";
                        write(source_socket, mensaje_partida, strlen(mensaje_partida));
                        printf("Enviando mensaje 'HAS_SIDO_ASIGNADO_A_PARTIDA' a socket %d\n", source_socket);

                        // Si la partida está completa, iniciar la partida
                        if (partidas_activas[i].num_jugadores == MAX_JUGADORES_POR_PARTIDA) {
                            for (int j = 0; j < MAX_JUGADORES_POR_PARTIDA; j++) {
                                char mensaje_inicio[] = "PARTIDA_INICIADA\n";
                                write(partidas_activas[i].jugadores[j], mensaje_inicio, strlen(mensaje_inicio));
                                printf("Enviando mensaje 'PARTIDA_INICIADA' a socket %d\n", partidas_activas[i].jugadores[j]);
                            }
                        }

                        break;
                    }
                }
                pthread_mutex_unlock(&partidas_mutex);
            
                

                // Añadir al target_username a la partida del source_username
                pthread_mutex_lock(&partidas_mutex);
                for (int i = 0; i < num_partidas_activas; i++) {
                    if (partidas_activas[i].id == source_partida) {
                        if (partidas_activas[i].num_jugadores < MAX_JUGADORES_POR_PARTIDA) {
                            partidas_activas[i].jugadores[partidas_activas[i].num_jugadores] = sock_conn;
                            strncpy(partidas_activas[i].nombres[partidas_activas[i].num_jugadores], target_username, 49);
                            partidas_activas[i].nombres[partidas_activas[i].num_jugadores][49] = '\0';
                            partidas_activas[i].num_jugadores++;

                            printf("Añadiendo usuario '%s' a partida ID: %d\n", target_username, source_partida);

                            // Actualizar la estructura del usuario para reflejar la partida
                            pthread_mutex_lock(&lista_mutex);
                            for (int j = 0; j < num_usuarios_conectados; j++) {
                                if (strcmp(usuarios_conectados[j].nombre_usuario, target_username) == 0) {
                                    usuarios_conectados[j].partida_id = source_partida;
                                    printf("Usuario '%s' asignado a partida ID: %d\n", target_username, source_partida);
                                    break;
                                }
                            }
                            pthread_mutex_unlock(&lista_mutex);

                            // Notificar al jugador que ha sido añadido a la partida
                            char mensaje_partida[] = "HAS_SIDO_ASIGNADO_A_PARTIDA\n";
                            write(sock_conn, mensaje_partida, strlen(mensaje_partida));
                            printf("Enviando mensaje 'HAS_SIDO_ASIGNADO_A_PARTIDA' a socket %d\n", sock_conn);

                            // Si la partida está completa, iniciar la partida
                            if (partidas_activas[i].num_jugadores == MAX_JUGADORES_POR_PARTIDA) {
                                for (int j = 0; j < MAX_JUGADORES_POR_PARTIDA; j++) {
                                    char mensaje_inicio[] = "PARTIDA_INICIADA\n";
                                    write(partidas_activas[i].jugadores[j], mensaje_inicio, strlen(mensaje_inicio));
                                    printf("Enviando mensaje 'PARTIDA_INICIADA' a socket %d\n", partidas_activas[i].jugadores[j]);
                                }
                            }

                            break;
                        }
                        else {
                            // La partida ya está llena
                            char error_msg[] = "ERROR_PARTIDA: La partida ya está llena.\n";
                            write(sock_conn, error_msg, strlen(error_msg));
                            printf("Error: La partida ID %d ya está llena. No se puede añadir al usuario '%s'.\n", source_partida, target_username);
                        }
                    }
                }
                pthread_mutex_unlock(&partidas_mutex);

                // Enviar notificación de aceptación al source_username
                char aceptada_msg[256];
                snprintf(aceptada_msg, sizeof(aceptada_msg), "INVITACION_ACEPTADA:%s\n", target_username);
                write(source_socket, aceptada_msg, strlen(aceptada_msg));
                printf("Enviando 'INVITACION_ACEPTADA:%s' a socket %d\n", target_username, source_socket);
            }
            else {
                char respuesta[] = "ERROR_INVITACION_ACEPTADA: Usuario que invita no está conectado.\n";
                write(sock_conn, respuesta, strlen(respuesta));
                printf("Error: Usuario '%s' que invitó no está conectado.\n", source_username);
            }
        }
        else {
            printf("Formato inválido para INVITACION_ACEPTADA: %s\n", mensaje);
            char respuesta[] = "ERROR_FORMATO_INVITACION_ACEPTADA\n";
            write(sock_conn, respuesta, strlen(respuesta));
        }
    }

    // Mensaje para rechazar la invitación
    else if (strncmp(mensaje, "INVITACION_RECHAZADA/", 21) == 0) {
        char* source_username = strtok(mensaje + 21, "/");
        char* target_username = strtok(NULL, "/");

        if (source_username && target_username) {
            // Encontrar el socket del usuario que invitó
            pthread_mutex_lock(&lista_mutex);
            int source_socket = encontrar_socket_por_usuario(source_username);
            pthread_mutex_unlock(&lista_mutex);

            if (source_socket != -1) {
                // Enviar notificación de rechazo al usuario que invitó
                char rechazada_msg[256];
                snprintf(rechazada_msg, sizeof(rechazada_msg), "INVITACION_RECHAZADA:%s", target_username);
                write(source_socket, rechazada_msg, strlen(rechazada_msg));
                printf("Invitación rechazada por %s para %s\n", target_username, source_username);
            }
        }
    }
    else if (strncmp(mensaje, "CHAT/", 5) == 0) {
        char* usuario_chat = strtok(mensaje + 5, "/");
        char* texto_chat = strtok(NULL, "/");

        if (usuario_chat && texto_chat) {
            printf("Recibido mensaje de chat de '%s': %s\n", usuario_chat, texto_chat);

            // Obtener la partida del usuario
            int partida_id = -1;
            pthread_mutex_lock(&lista_mutex);
            for (int i = 0; i < num_usuarios_conectados; i++) {
                if (strcmp(usuarios_conectados[i].nombre_usuario, usuario_chat) == 0) {
                    partida_id = usuarios_conectados[i].partida_id;
                    break;
                }
            }
            pthread_mutex_unlock(&lista_mutex);

            printf("Usuario '%s' está en partida ID: %d\n", usuario_chat, partida_id);

            if (partida_id != -1) {
                // Encontrar la partida y enviar el mensaje a todos los jugadores
                pthread_mutex_lock(&partidas_mutex);
                for (int i = 0; i < num_partidas_activas; i++) {
                    if (partidas_activas[i].id == partida_id) {
                        char mensaje_chat[512];
                        snprintf(mensaje_chat, sizeof(mensaje_chat), "CHAT/%s/%s\n", usuario_chat, texto_chat);
                        printf("Reenviando mensaje de chat a partida ID %d: %s", partida_id, mensaje_chat);

                        for (int j = 0; j < partidas_activas[i].num_jugadores; j++) {
                            int target_sock = partidas_activas[i].jugadores[j];
                            int bytes_written = write(target_sock, mensaje_chat, strlen(mensaje_chat));
                            if (bytes_written < 0) {
                                printf("Error al enviar mensaje de chat a socket %d: %s\n", target_sock, strerror(errno));
                            }
                            else {
                                printf("Mensaje de chat enviado a socket %d\n", target_sock);
                            }
                        }
                        break;
                    }
                }
                pthread_mutex_unlock(&partidas_mutex);
            }
            else {
                // El usuario no está en ninguna partida, enviar error
                char error_msg[] = "ERROR_CHAT: No estás en ninguna partida.\n";
                write(sock_conn, error_msg, strlen(error_msg));
                printf("Usuario '%s' no está en ninguna partida. Enviando mensaje de error.\n", usuario_chat);
            }
        }
        else {
            printf("Formato inválido para CHAT: %s\n", mensaje);
            char respuesta[] = "ERROR_FORMATO_CHAT\n";
            write(sock_conn, respuesta, strlen(respuesta));
        }
    }

    // Mensaje para buscar una partida automáticamente
    else if (strcmp(mensaje, "BUSCAR_PARTIDA") == 0) {
        // Añadir al usuario a la cola de matchmaking
        pthread_mutex_lock(&matchmaking_mutex);
        if (num_matchmaking < MAX_MATCHMAKING_QUEUE) {
            cola_matchmaking[num_matchmaking].socket = sock_conn;
            strncpy(cola_matchmaking[num_matchmaking].nombre_usuario, usuario, 49);
            cola_matchmaking[num_matchmaking].nombre_usuario[49] = '\0';
            num_matchmaking++;
            pthread_mutex_unlock(&matchmaking_mutex);
            printf("Usuario %s añadido a la cola de matchmaking.\n", usuario);

            // Intentar asignar a una partida si hay suficientes jugadores
            pthread_mutex_lock(&matchmaking_mutex);
            if (num_matchmaking >= 3) {
                // Crear una nueva partida
                int nueva_partida_id = crear_partida();
                if (nueva_partida_id != -1) {
                    // Asignar los 3 primeros jugadores de la cola a la nueva partida
                    pthread_mutex_lock(&lista_mutex);
                    for (int i = 0; i < 3; i++) {
                        int socket = cola_matchmaking[0].socket;
                        char* nombre = cola_matchmaking[0].nombre_usuario;

                        // Asignar al usuario a la partida
                        for (int j = 0; j < num_usuarios_conectados; j++) {
                            if (usuarios_conectados[j].socket == socket) {
                                usuarios_conectados[j].partida_id = nueva_partida_id;
                                break;
                            }
                        }

                        // Añadir al jugador a la partida
                        pthread_mutex_lock(&partidas_mutex);
                        for (int p = 0; p < num_partidas_activas; p++) {
                            if (partidas_activas[p].id == nueva_partida_id) {
                                partidas_activas[p].jugadores[partidas_activas[p].num_jugadores] = socket;
                                strncpy(partidas_activas[p].nombres[partidas_activas[p].num_jugadores], nombre, 49);
                                partidas_activas[p].nombres[partidas_activas[p].num_jugadores][49] = '\0';
                                partidas_activas[p].num_jugadores++;

                                // Notificar al jugador que ha sido asignado a la partida
                                char mensaje_asignacion[] = "HAS_SIDO_ASIGNADO_A_PARTIDA";
                                write(socket, mensaje_asignacion, strlen(mensaje_asignacion));

                                // Si la partida está completa, notificar a todos los jugadores
                                if (partidas_activas[p].num_jugadores == MAX_JUGADORES_POR_PARTIDA) {
                                    for (int m = 0; m < MAX_JUGADORES_POR_PARTIDA; m++) {
                                        char mensaje_inicio[] = "PARTIDA_INICIADA";
                                        write(partidas_activas[p].jugadores[m], mensaje_inicio, strlen(mensaje_inicio));
                                    }
                                }

                                break;
                            }
                        }
                        pthread_mutex_unlock(&partidas_mutex);

                        // Eliminar al usuario de la cola de matchmaking
                        for (int k = 0; k < num_matchmaking - 1; k++) {
                            cola_matchmaking[k] = cola_matchmaking[k + 1];
                        }
                        num_matchmaking--;
                    }
                    printf("Se ha creado la partida %d con 3 jugadores de la cola de matchmaking.\n", nueva_partida_id);
                }
            }
            else {
                pthread_mutex_unlock(&matchmaking_mutex);
            }
        }
        else {
            pthread_mutex_unlock(&matchmaking_mutex);
            char error_msg[] = "ERROR_MATCHMAKING: La cola de matchmaking está llena.";
            write(sock_conn, error_msg, strlen(error_msg));
        }
	}
    else {
        char respuesta[] = "COMANDO_NO_RECONOCIDO";
        write(sock_conn, respuesta, strlen(respuesta));
        printf("Comando no reconocido: %s\n", mensaje);
    }

    mysql_close(conn);
}

/*===================================================================================================================================================================*/
/*============================================================ Hilos Atender_cliente Manejar_conexiones Notificaciones y Matchmaking============================================================*/
void* AtenderCliente(void* arg) 
{
    int sock_conn = *(int*)arg;
    int terminar = 0;
    char buff[512];
    char usuario[50] = "";  

    while (!terminar) 
    {
		int ret = read(sock_conn, buff, sizeof(buff) - 1);                                  //Leer el mensaje del cliente
        if (ret > 0) 
        {
            buff[ret] = '\0';
            printf("Mensaje recibido: %s\n", buff);

			procesar_mensaje(sock_conn, buff, usuario);                                      //Procesar el mensaje recibido
        }
		else if (ret == 0)                                                                   //El cliente ha cerrado la conexión
        {
            if (strlen(usuario) > 0) {
                quitar_usuario(sock_conn, usuario);
            }
            terminar = 1;
        }
        else 
        {
			printf("Error al leer el mensaje del cliente: %s\n", strerror(errno));           //Error al leer el mensaje del cliente
            if (strlen(usuario) > 0) {
                quitar_usuario(sock_conn, usuario);
            }
            terminar = 1;
        }
    }
    close(sock_conn);                                                                         // Se acabó el servicio para este cliente
    pthread_exit(NULL);
}

void* AceptarConexiones(void* arg) 
{
    int sock_conn;
    int sockets[100];
    pthread_t thread;
    int i = 0;

    while (1) {
		sock_conn = accept(sock_listen, NULL, NULL);										   //Aceptar la conexión
        if (sock_conn < 0) {
            printf("Error en el accept\n");
            continue;
        }

        printf("Conexión aceptada\n");
        sockets[i] = sock_conn;

		pthread_create(&thread, NULL, AtenderCliente, &sockets[i]);							    //Crear un hilo para atender al cliente    
		pthread_detach(thread);                                                                 //Libera los recursos del hilo al finalizar

        i++;
		if (i >= 100) {                                                                         //Reiniciar el contador de sockets
            i = 0;
        }
    }
    pthread_exit(NULL);
}

void* procesar_notificaciones(void* arg) 
{                                                                                               // Función para el hilo de notificaciones
    while (1) {
        pthread_mutex_lock(&notificacion_mutex);
        while (num_notificaciones == 0) {                                                       // Esperar a que haya notificaciones 
            pthread_cond_wait(&notificacion_cond, &notificacion_mutex);                         // Bloquear el hilo hasta que se despierte
        }

        Notificacion notificacion = cola_notificaciones[0];									    // Obtener la primera notificación
        for (int i = 0; i < num_notificaciones - 1; i++) {  								    // Mover las notificaciones restantes
            cola_notificaciones[i] = cola_notificaciones[i + 1];
        }
        num_notificaciones--;																	//Decrementar el contador de notificaciones    
        pthread_mutex_unlock(&notificacion_mutex);

        if (strcmp(notificacion.mensaje, "UPDATE_LIST") == 0) {
            char connectedUsers[1000];
            strcpy(connectedUsers, "USUARIOS_CONECTADOS/");

            pthread_mutex_lock(&lista_mutex);
            for (int i = 0; i < num_usuarios_conectados; i++) {                                  //Construir la lista de usuarios conectados
                strcat(connectedUsers, usuarios_conectados[i].nombre_usuario);
                if (i < num_usuarios_conectados - 1) {
                    strcat(connectedUsers, "\n"); // Separador de usuarios
                }
            }
            pthread_mutex_unlock(&lista_mutex);

            pthread_mutex_lock(&lista_mutex);
            for (int i = 0; i < num_usuarios_conectados; i++) {     			                 //Enviar la lista de usuarios conectados a todos los clientes     
                int sock_conn_i = usuarios_conectados[i].socket;
                write(sock_conn_i, connectedUsers, strlen(connectedUsers));
            }
            pthread_mutex_unlock(&lista_mutex);

            printf("Lista completa de usuarios enviada a todos los clientes.\n");
        }
    }
    return NULL;
}


void* gestionar_matchmaking(void* arg) {
    while (1) {
        pthread_mutex_lock(&matchmaking_mutex);
        if (num_matchmaking >= 3) {
            // Crear una nueva partida
            int nueva_partida_id = crear_partida();
            if (nueva_partida_id != -1) {
                // Asignar los 3 primeros jugadores de la cola a la nueva partida
                pthread_mutex_lock(&lista_mutex);
                for (int i = 0; i < 3; i++) {
                    int socket = cola_matchmaking[0].socket;
                    char* nombre = cola_matchmaking[0].nombre_usuario;

                    // Asignar al usuario a la partida
                    for (int j = 0; j < num_usuarios_conectados; j++) {
                        if (usuarios_conectados[j].socket == socket) {
                            usuarios_conectados[j].partida_id = nueva_partida_id;
                            break;
                        }
                    }

                    // Añadir al jugador a la partida
                    pthread_mutex_lock(&partidas_mutex);
                    for (int p = 0; p < num_partidas_activas; p++) {
                        if (partidas_activas[p].id == nueva_partida_id) {
                            partidas_activas[p].jugadores[partidas_activas[p].num_jugadores] = socket;
                            strncpy(partidas_activas[p].nombres[partidas_activas[p].num_jugadores], nombre, 49);
                            partidas_activas[p].nombres[partidas_activas[p].num_jugadores][49] = '\0';
                            partidas_activas[p].num_jugadores++;

                            // Notificar al jugador que ha sido asignado a la partida
                            char mensaje_asignacion[] = "HAS_SIDO_ASIGNADO_A_PARTIDA";
                            write(socket, mensaje_asignacion, strlen(mensaje_asignacion));

                            // Si la partida está completa, notificar a todos los jugadores
                            if (partidas_activas[p].num_jugadores == MAX_JUGADORES_POR_PARTIDA) {
                                for (int m = 0; m < MAX_JUGADORES_POR_PARTIDA; m++) {
                                    char mensaje_inicio[] = "PARTIDA_INICIADA";
                                    write(partidas_activas[p].jugadores[m], mensaje_inicio, strlen(mensaje_inicio));
                                }
                            }

                            break;
                        }
                    }
                    pthread_mutex_unlock(&partidas_mutex);

                    // Eliminar al usuario de la cola de matchmaking
                    for (int k = 0; k < num_matchmaking - 1; k++) {
                        cola_matchmaking[k] = cola_matchmaking[k + 1];
                    }
                    num_matchmaking--;
                }
                pthread_mutex_unlock(&lista_mutex);
                printf("Se ha creado la partida %d con 3 jugadores de la cola de matchmaking.\n", nueva_partida_id);
            }
        }
        pthread_mutex_unlock(&matchmaking_mutex);
        sleep(5); // Esperar 5 segundos antes de revisar nuevamente
    }
    return NULL;
}


/*==============================================================================================================================*/
/*============================================================ MAIN ============================================================*/

int main(int argc, char* argv[]) 
{
	struct sockaddr_in serv_adr;                                                        // Dirección del servidor    

	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {	                        // Crear el socket
        printf("Error creando socket\n");
        exit(1);
    }

	memset(&serv_adr, 0, sizeof(serv_adr));                                             // Inicializar la estructura de dirección
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(50000);  // Puerto de escucha

	if (bind(sock_listen, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) < 0) {         // Asociar el socket a la dirección
        printf("Error en el bind\n");
        exit(1);
    }

	if (listen(sock_listen, 2) < 0) {   									            // Escuchar en el socket      
        printf("Error en el listen\n");
        exit(1);
    }

    printf("Servidor iniciado correctamente. Escuchando en puerto 50003\n");

    pthread_t thread_accept;                                                            // Hilo para aceptar conexiones
    pthread_create(&thread_accept, NULL, AceptarConexiones, NULL);
    pthread_detach(thread_accept);                                                      

	pthread_t notificacion_thread;                                                      // Hilo para procesar notificaciones
    pthread_create(&notificacion_thread, NULL, procesar_notificaciones, NULL);
	pthread_detach(notificacion_thread);     

	pthread_t matchmaking_thread;       							                    // Hilo para el matchmaking         
    pthread_create(&matchmaking_thread, NULL, gestionar_matchmaking, NULL);
    pthread_detach(matchmaking_thread);


    while (1) 
    {
        sleep(10);                                              // Hilo principal duerme, puede hacer otras cosas
    }

    close(sock_listen);                                         // Nunca se llega aquí, pero por buenas prácticas
    return 0;
}
