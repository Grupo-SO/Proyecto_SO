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
#define MAX_JUGADORES_POR_PARTIDA 2
#define MAX_PARTIDAS 100
#define MAX_MATCHMAKING_QUEUE 100
#define TCP_PORT 50012
#define UDP_PORT 50013

int sock_listen;
int sock_listen_udp;

/*============================================================== Estructuras ===============================================================*/

typedef struct {
    int socket;
    struct sockaddr_in udp_addr;
    int udp_flag;
    char nombre_usuario[50];
	int partida_id;     // ID de la partida, -1 si no está en una
} UsuarioInfo;

typedef struct {
    int id;
    int jugadores[MAX_JUGADORES_POR_PARTIDA]; // Sockets de los jugadores
    char nombres[MAX_JUGADORES_POR_PARTIDA][50];   // Nombres de los jugadores
    int sizes[MAX_JUGADORES_POR_PARTIDA];
    int num_jugadores;
                                                                                    
    char historialNombres[MAX_JUGADORES_POR_PARTIDA][50];  // array para TODOS los que pasaron por la partida
    int historialSizes[MAX_JUGADORES_POR_PARTIDA];
    int total_jugadores;  // cuántos hay en el historial

	int en_juego;  // 1 si la partida está en curso, 0 si no
} Partida;


typedef struct {
    char mensaje[512];
} Notificacion;


/*============================================================== Listas ===============================================================*/

UsuarioInfo usuarios_conectados[MAX_USERS];
int num_usuarios_conectados = 0;  

Partida partidas_activas[MAX_PARTIDAS];
int num_partidas_activas = 0;

int cola_matchmaking[MAX_MATCHMAKING_QUEUE];
int num_matchmaking = 0;

Notificacion cola_notificaciones[MAX_NOTIFICACIONES];   //Notificaciones pendientes, en orden
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

/*==============================================================================================================================*/
/*============================================= Funciones manejo entrada y salida de usuarios ================================================*/

void registrar_udp_addr(char* username, struct sockaddr_in* addr) {
    pthread_mutex_lock(&lista_mutex);
    for (int i = 0; i < num_usuarios_conectados; i++) {
        if (strcmp(usuarios_conectados[i].nombre_usuario, username) == 0) {
            usuarios_conectados[i].udp_addr = *addr;
            usuarios_conectados[i].udp_flag = 1;
            //printf("Registrada direccion UDP para %s: %s:%d\n", username, inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
            printf("Registrada direccion UDP de: %s\n", username);
            break;
        }
    }
    pthread_mutex_unlock(&lista_mutex);
}

void anadir_usuario(int sock_conn, char* usuario)  // Función para añadir un usuario a la lista de conectados
{
    pthread_mutex_lock(&lista_mutex);

    if (num_usuarios_conectados < MAX_USERS) {
        usuarios_conectados[num_usuarios_conectados].socket = sock_conn;
        strncpy(usuarios_conectados[num_usuarios_conectados].nombre_usuario, usuario, sizeof(usuarios_conectados[num_usuarios_conectados].nombre_usuario) - 1);
        usuarios_conectados[num_usuarios_conectados].nombre_usuario[sizeof(usuarios_conectados[num_usuarios_conectados].nombre_usuario) - 1] = '\0';
        usuarios_conectados[num_usuarios_conectados].partida_id = -1;  // No está en ninguna partida
        num_usuarios_conectados++;

        printf("Jugador conectado: %s\n", usuario);
        printf("Número de jugadores conectados: %d\n", num_usuarios_conectados);

        pthread_mutex_unlock(&lista_mutex);

        encolar_notificacion();  // Enviar notificación de actualización de lista            
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

            for (int j = i; j < num_usuarios_conectados - 1; j++) {   // Mover los usuarios restantes
                usuarios_conectados[j] = usuarios_conectados[j + 1];
            }
            num_usuarios_conectados--;

            printf("Jugador desconectado: %s\n", nombre_usuario);

            pthread_mutex_unlock(&lista_mutex);

            encolar_notificacion();   // Enviar notificación de actualización de lista    
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

void buscarUserAndChangePassBBDD(MYSQL* conn, int sock_conn, char* usuario, char* nueva_contrasena) {  // Función para cambiar la contraseña del usuario
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
{   // Función para insertar un nuevo usuario en la base de datos
    int err;
    char query[512];
    MYSQL_RES* result;
    MYSQL_ROW row;

	sprintf(query, "SELECT usuario FROM Usuarios WHERE usuario='%s';", usuario);  //Verificar si existe el usuario
    err = mysql_query(conn, query);
    if (err != 0) {
        printf("Error al comprobar el usuario %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR";
        write(sock_conn, respuesta, strlen(respuesta));
        return;
    }

    result = mysql_store_result(conn);
    row = mysql_fetch_row(result);
    if (row != NULL) {
        // El usuario ya existe
        printf("Usuario '%s' ya existe.\n", usuario);
        char respuesta[] = "Existe";
        write(sock_conn, respuesta, strlen(respuesta));
        mysql_free_result(result);
        return;
    }
    mysql_free_result(result);

    sprintf(query, "INSERT INTO Usuarios (usuario, contrasena) VALUES ('%s', '%s');", usuario, contrasena);
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
{   // Función para crear y borrar la base de datos        
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
{	// Función para contar el número de usuarios en la base de datos
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
{	// Función para encontrar el socket de un usuario por su nombre  
    for (int i = 0; i < num_usuarios_conectados; i++) {
        if (strcmp(usuarios_conectados[i].nombre_usuario, nombre_usuario) == 0) {
            printf("%s\n", usuarios_conectados[i].nombre_usuario);
            return usuarios_conectados[i].socket;

        }
    }
    return -1;
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
    nueva_partida.en_juego = 0;                   // Inicializar el flag a 0
    nueva_partida.total_jugadores = 0;

    partidas_activas[num_partidas_activas] = nueva_partida;
    num_partidas_activas++;
    pthread_mutex_unlock(&partidas_mutex);
    printf("partida creada\n");
    return nueva_partida.id;    
}

// Función para encontrar una partida disponible para añadir un jugador
int asignar_a_partida(int sock_conn, char* nombre_usuario) {
	int partida_id = -1;

    pthread_mutex_lock(&partidas_mutex);
    for (int i = 0; i < num_partidas_activas; i++) {
        if (partidas_activas[i].num_jugadores < MAX_JUGADORES_POR_PARTIDA) {
			// Añadir al jugador a la partida
            partidas_activas[i].jugadores[partidas_activas[i].num_jugadores] = sock_conn;
            strncpy(partidas_activas[i].nombres[partidas_activas[i].num_jugadores], nombre_usuario, 49);
            //partidas_activas[i].nombres[partidas_activas[i].num_jugadores][49] = '\0';
            partidas_activas[i].num_jugadores++;

            // AÑADE también al historial:
            int t = partidas_activas[i].total_jugadores;
            strncpy(partidas_activas[i].historialNombres[t], nombre_usuario, 49);
            partidas_activas[i].historialSizes[t] = 100;  // O el tamaño inicial que quieras
            partidas_activas[i].total_jugadores++;
			partida_id = partidas_activas[i].id;
            
            // Actualizar el partida_id del usuario
            pthread_mutex_lock(&lista_mutex);
            for (int j = 0; j < num_usuarios_conectados; j++) {
                if (usuarios_conectados[j].socket == sock_conn) {
                    usuarios_conectados[j].partida_id = partida_id;
                    break;
                }
            }
            pthread_mutex_unlock(&lista_mutex);
            pthread_mutex_unlock(&partidas_mutex);
			return partida_id;
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
            // Reemplazar con la última
            partidas_activas[i] = partidas_activas[num_partidas_activas - 1];
            num_partidas_activas--;
            break;
        }
    }
    pthread_mutex_unlock(&partidas_mutex);

    // Ahora eliminar de la cola matchmaking
    pthread_mutex_lock(&matchmaking_mutex);
    for (int mm = 0; mm < num_matchmaking; mm++) {
        if (cola_matchmaking[mm] == id) {
            for (int mm2 = mm; mm2 < num_matchmaking - 1; mm2++) {
                cola_matchmaking[mm2] = cola_matchmaking[mm2 + 1];
            }
            num_matchmaking--;
            printf("Eliminada la partida %d de la cola de matchmaking.\n", id);
            break;
        }
    }
    pthread_mutex_unlock(&matchmaking_mutex);
}

void finalizar_partida_y_registrar_en_BD(int id_partida)
{
	printf("registrando en bd\n");
    pthread_mutex_lock(&partidas_mutex);
    Partida partida_final;
    int found = 0;
    for (int i = 0; i < num_partidas_activas; i++) {
        if (partidas_activas[i].id == id_partida) {
            partida_final = partidas_activas[i];
            found = 1;
            break;
        }
    }
    pthread_mutex_unlock(&partidas_mutex);

    if (!found) {
        // No existe la partida en memoria, ya fue eliminada o similar
        return;
    }

    // 1) Conectar a MySQL
    MYSQL* conn;
    conectarMySQL(&conn);
    entrarBD(conn); // "USE M3_UsuariosBBDD" (o la base que uses)

    // 2) Insertar la fila en 'partidas' para obtener su ID
	printf("insertando fecha\n");
    int err;
    err = mysql_query(conn, "INSERT INTO partidas (fecha) VALUES (NOW());");
    if (err != 0) {
        printf("Error al insertar en partidas: %u %s\n", mysql_errno(conn), mysql_error(conn));
        // Manejar error (return, etc.)
    }
    int new_partida_id = mysql_insert_id(conn);

    // 3) Para cada jugador, insertar/actualizar su registro
    for (int j = 0; j < partida_final.total_jugadores; j++) {
        char* nombre_jugador = partida_final.historialNombres[j];
        int size_final = partida_final.historialSizes[j];

        // 3.1) Buscar o insertar en la tabla 'jugadores'
        char query[1024];
        snprintf(query, sizeof(query),"SELECT id FROM jugadores WHERE nombre = '%s';", nombre_jugador);
		printf("seleccionando jugador...\n");
        err = mysql_query(conn, query);
        if (err != 0) {
            printf("Error SELECT jugadores: %u %s\n", mysql_errno(conn), mysql_error(conn));
        }
        MYSQL_RES* res = mysql_store_result(conn);
        MYSQL_ROW row = mysql_fetch_row(res);

        int jugador_id = -1;
        if (row) {
            // Ya existe
            jugador_id = atoi(row[0]);
        }
        mysql_free_result(res);

        if (jugador_id == -1) {
            // Insertar
            snprintf(query, sizeof(query),
                "INSERT INTO jugadores (nombre) VALUES ('%s');",
                nombre_jugador);
            err = mysql_query(conn, query);
            if (err != 0) {
                printf("Error INSERT jugadores: %u %s\n", mysql_errno(conn), mysql_error(conn));
            }
            jugador_id = mysql_insert_id(conn);
        }

        // 3.2) Insertar en 'partidas_jugadores'
        snprintf(query, sizeof(query),
            "INSERT INTO partidas_jugadores (partida_id, jugador_id, puntuacion) "
            "VALUES (%d, %d, %d);",
            new_partida_id, jugador_id, size_final);
        err = mysql_query(conn, query);
        if (err != 0) {
            printf("Error INSERT partidas_jugadores: %u %s\n", mysql_errno(conn), mysql_error(conn));
        }
    }

    // 4) Eliminar la partida de la lista interna
    eliminar_partida(id_partida);

    // 5) Cerrar conexión
    mysql_close(conn);
}

void salir_partida(int sock_conn, int enviar_mensaje)
{
    pthread_mutex_lock(&lista_mutex);
    int partida_id = -1;
    for (int i = 0; i < num_usuarios_conectados; i++) {
        if (usuarios_conectados[i].socket == sock_conn) {
            partida_id = usuarios_conectados[i].partida_id;
            usuarios_conectados[i].partida_id = -1;
			printf("Jugador %s saliendo de la partida %d\n", usuarios_conectados[i].nombre_usuario, partida_id);
            break;
        }
    }
    pthread_mutex_unlock(&lista_mutex);

    if (partida_id != -1) {
        pthread_mutex_lock(&partidas_mutex);
        for (int i = 0; i < num_partidas_activas; i++) {
            if (partidas_activas[i].id == partida_id) {

                // Eliminar al jugador de la partida
                for (int j = 0; j < partidas_activas[i].num_jugadores; j++) {
                    if (partidas_activas[i].jugadores[j] == sock_conn) {

                        int size_final_del_que_sale = partidas_activas[i].sizes[j];
                        char nombre_del_que_sale[50];
                        strcpy(nombre_del_que_sale, partidas_activas[i].nombres[j]);

                        // Buscamos en el historial y actualizamos
                        for (int h = 0; h < partidas_activas[i].total_jugadores; h++) {
                            if (strcmp(partidas_activas[i].historialNombres[h], nombre_del_que_sale) == 0) {
                                partidas_activas[i].historialSizes[h] = size_final_del_que_sale;
                                break;
                            }
                        }

                        // Hacer "shift" de los arrays para quitar al jugador
                        for (int k = j; k < partidas_activas[i].num_jugadores - 1; k++) {
                            partidas_activas[i].jugadores[k] = partidas_activas[i].jugadores[k + 1];
                            strcpy(partidas_activas[i].nombres[k], partidas_activas[i].nombres[k + 1]);
                            partidas_activas[i].sizes[k] = partidas_activas[i].sizes[k + 1];
                        }

                        // Decrementar número de jugadores
                        partidas_activas[i].num_jugadores--;

                        // Comprobar cuántos jugadores quedan
                        int num_actual_jugadores = partidas_activas[i].num_jugadores;

                        if (num_actual_jugadores == 0) {
                            // Partida vacía -> eliminar
                            int id_a_eliminar = partidas_activas[i].id;
                            pthread_mutex_unlock(&partidas_mutex);

                            eliminar_partida(id_a_eliminar);
                            printf("Partda abandonadda\n");

                            // Si queremos enviar PARTIDA_ABANDONADA_OK a quien se fue
                            if (enviar_mensaje) {
								
                                char respuesta[] = "PARTIDA_ABANDONADA_OK";
                                write(sock_conn, respuesta, strlen(respuesta));
                            }
                            return;
                        }
                        else if (num_actual_jugadores == 1 && partidas_activas[i].en_juego == 1) {
                            // La partida está en juego (en_juego == 1) y queda 1 jugador => Gana
                            int id_partida = partidas_activas[i].id;
                            int socket_ganador = partidas_activas[i].jugadores[0];

                            // Enviar mensaje de ganador al jugador que queda
                            //char msg_ganar[] = "PARTIDA_GANADA";
                            //write(socket_ganador, msg_ganar, strlen(msg_ganar));

                            // Registrar en BD y eliminar
                            pthread_mutex_unlock(&partidas_mutex);
                            finalizar_partida_y_registrar_en_BD(id_partida);
							printf("Partida finalizada\n");
                            return;
                            
                        }
                        // Si no cumple ninguna condición, simplemente seguimos
                        break;
                    }
                }
                break; // salir del for de i
            }
        }
        pthread_mutex_unlock(&partidas_mutex);
    }

    // Si llegamos aquí, no se ha eliminado la partida ni se ha declarado ganador
    if (enviar_mensaje) {
        char respuesta[] = "PARTIDA_ABANDONADA_OK";
        write(sock_conn, respuesta, strlen(respuesta));
    }
}





/*==============================================================================================================================*/
/*============================== Funciones para procesar los mensajes recibidos desde el cliente ===============================*/

void procesar_mensaje(int sock_conn, char* mensaje, char* usuario) 
{
    MYSQL* conn;
    conectarMySQL(&conn);

    if (strcmp(mensaje, "CREAR_BD") == 0) {
        printf("Recibido mensaje para crear la base de datos\n");
        entrarBD(conn);
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
        for (int i = 0; i < num_usuarios_conectados; i++)
        {
            if (strcmp(usuarios_conectados[i].nombre_usuario, usuario_msg) == 0) {
                printf("Usuario ya conectado\n");
                char respuesta[] = "Online";
                write(sock_conn, respuesta, strlen(respuesta));
				printf("Usuario ya conectado\n");
                return;
            }
        }
        if (usuario_msg && contrasena) {
            strcpy(usuario, usuario_msg);  
            entrarBD(conn);
            buscarUserAndPassBBDD(conn, sock_conn, usuario, contrasena);
        }
    }

    else if (strncmp(mensaje, "GET_GANADOR/", 12) == 0) {
        char* str_pid = mensaje + 12;
        int pid = atoi(str_pid);
        if (pid <= 0) return;

        entrarBD(conn);

        char query[1024];
        snprintf(query, sizeof(query),
            "SELECT j.nombre, pj.puntuacion "
            "FROM partidas_jugadores pj "
            "JOIN jugadores j ON pj.jugador_id = j.id "
            "WHERE pj.partida_id = %d "
            "ORDER BY pj.puntuacion DESC "
            "LIMIT 1;", pid);

        int err = mysql_query(conn, query);
        if (err != 0) {
            printf("Error SELECT GET_GANADOR: %u %s\n", mysql_errno(conn), mysql_error(conn));
            return;
        }
        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            printf("Error store_result GET_GANADOR: %u %s\n", mysql_errno(conn), mysql_error(conn));
            return;
        }
        MYSQL_ROW row = mysql_fetch_row(res);
        if (!row) {
            // No hay datos => no existe la partida
            mysql_free_result(res);
            return;
        }
        // row[0] => nombre, row[1] => puntuacion
        char respuesta[256];
        snprintf(respuesta, sizeof(respuesta), "GANADOR/%s/%s", row[0], row[1]);
        mysql_free_result(res);

        write(sock_conn, respuesta, strlen(respuesta));
    }


    else if (strncmp(mensaje, "GET_RESULTADOS/", 15) == 0) {
        printf("Mensaje recibido en el servidor: %s\n", mensaje);
        char* str_partida_id = mensaje + 15;
        int partida_id_consulta = atoi(str_partida_id);
        if (partida_id_consulta <= 0) {
			printf("Error en el formato del mensaje GET_RESULTADOS\n");
            return;
        }
		printf("Partida ID: %d\n", partida_id_consulta);
        entrarBD(conn);

        char query[1024];
        snprintf(query, sizeof(query),
            "SELECT j.nombre, pj.puntuacion "
            "FROM partidas_jugadores pj "
            "JOIN jugadores j ON pj.jugador_id = j.id "
            "WHERE pj.partida_id = %d;", partida_id_consulta);

        int err = mysql_query(conn, query);
		printf("Query: %s\n", query);
        if (err != 0) {
            printf("Error SELECT GET_RESULTADOS: %u %s\n", mysql_errno(conn), mysql_error(conn));
            return;
        }
        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            printf("Error store_result GET_RESULTADOS: %u %s\n", mysql_errno(conn), mysql_error(conn));
            return;
        }
		printf("Resultados de la partida %d:\n", partida_id_consulta);
        // Formato: "RESULTADOS/\nNombre1,Punt1\nNombre2,Punt2\n..."
		printf("Enviando resultados al cliente\n");
        char respuesta[4096];
        strcpy(respuesta, "RESULTADOS/");
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            // row[0] = nombre, row[1] = puntuacion
            strcat(respuesta, row[0]);
            strcat(respuesta, ",");
            strcat(respuesta, row[1]);
            strcat(respuesta, "\n");
        }
        mysql_free_result(res);
        printf("Enviando resultados al cliente: %s\n", respuesta);
        write(sock_conn, respuesta, strlen(respuesta));
    }


    else if (strncmp(mensaje, "GET_HISTORIAL/", 14) == 0) {
        // 1) Extraer nombre de usuario
        char* usuarioHist = strtok(mensaje + 14, "/");
        if (!usuarioHist) {
            // Error de formato
            return;
        }

        // 2) Conectamos a MySQL
        entrarBD(conn);

        char query[1024];
        snprintf(query, sizeof(query),
            "SELECT p.id, DATE_FORMAT(p.fecha, '%%Y-%%m-%%d %%H:%%i:%%s') as fecha "
            "FROM partidas p "
            "JOIN partidas_jugadores pj ON p.id = pj.partida_id "
            "JOIN jugadores j ON pj.jugador_id = j.id "
            "WHERE j.nombre = '%s' "
            "GROUP BY p.id "
            "ORDER BY p.fecha DESC;", usuarioHist);

        int err = mysql_query(conn, query);
        if (err != 0) {
            printf("Error al hacer SELECT historial: %u %s\n", mysql_errno(conn), mysql_error(conn));
            return;
        }

        MYSQL_RES* res = mysql_store_result(conn);
        if (!res) {
            printf("Error al obtener resultado SELECT historial: %u %s\n", mysql_errno(conn), mysql_error(conn));
            return;
        }

        // 4) Formatear la respuesta
        // Por ejemplo, un formato: "HISTORIAL/\n{id1},{fecha1}\n{id2},{fecha2}\n..."
        char respuesta[4096];
        strcpy(respuesta, "HISTORIAL/");
        MYSQL_ROW row;
        while ((row = mysql_fetch_row(res)) != NULL) {
            // row[0] = p.id, row[1] = fecha
            strcat(respuesta, row[0]);
            strcat(respuesta, ",");
            strcat(respuesta, row[1]);
            strcat(respuesta, "\n");
        }
        mysql_free_result(res);

        write(sock_conn, respuesta, strlen(respuesta));
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
                // Comprobar si el jugador que invita ya tiene una partida
                int source_partida = -1;
                printf("bbb, source username: %s\n", source_username);
                pthread_mutex_lock(&lista_mutex);
                for (int i = 0; i < num_usuarios_conectados; i++) {
                    printf("el usuario %d es %s\n", i, usuarios_conectados[i].nombre_usuario);
                    if (strcmp(usuarios_conectados[i].nombre_usuario, source_username) == 0) {
                        source_partida = usuarios_conectados[i].partida_id;;
                        printf("El usuario %s esta en la partida %d\n", source_username, source_partida);
                        break;
                    }
                }
                pthread_mutex_unlock(&lista_mutex);

                if (source_partida == -1) {
                    // Crear una nueva partida
                    int nueva_partida_id = crear_partida();
					nueva_partida_id = asignar_a_partida(sock_conn, source_username);
                    if (nueva_partida_id == -1) {
                        char error_msg[] = "ERROR_PARTIDA: No se pudo crear una nueva partida.\n";
                        write(sock_conn, error_msg, strlen(error_msg));
                        printf("Error: No se pudo crear una nueva partida.\n");
                        return;
                    }

                    source_partida = nueva_partida_id;
                }

                // Verificar si la partida está llena
                pthread_mutex_lock(&partidas_mutex);
                int partida_llena = 0;
                for (int i = 0; i < num_partidas_activas; i++) {
                    if (partidas_activas[i].id == source_partida) {
                        if (partidas_activas[i].num_jugadores >= MAX_JUGADORES_POR_PARTIDA) {
                            partida_llena = 1;
                        }
                        break;
                    }
                }
                pthread_mutex_unlock(&partidas_mutex);

				//Verificar si el usuario objetivo ya está en juego
				int target_partida_id = 0;
				int jugando = 0;
				pthread_mutex_lock(&lista_mutex);
				for (int i = 0; i < num_usuarios_conectados; i++) {
					if (strcmp(usuarios_conectados[i].nombre_usuario, target_username) == 0) {
						target_partida_id = usuarios_conectados[i].partida_id;
						break;
					}
				}
				pthread_mutex_unlock(&lista_mutex);

                if (target_partida_id != -1) {
                    pthread_mutex_lock(&partidas_mutex);
                    for (int i = 0; i < num_partidas_activas; i++) {
                        if (partidas_activas[i].id == target_partida_id) {
                            jugando = partidas_activas[i].en_juego;
                            break;
                        }
                    }
                    pthread_mutex_unlock(&partidas_mutex);
                }

                if (jugando) {
                    char sala_llena_msg[] = "JUGANDO\n";
                    write(sock_conn, sala_llena_msg, strlen(sala_llena_msg));
                    printf("Sala de partida %d está llena. No se puede invitar.\n", source_partida);
                }


                if (partida_llena) {
                    // Enviar mensaje "SALA_LLENA" al usuario que invita
                    char sala_llena_msg[] = "SALA_LLENA\n";
                    write(sock_conn, sala_llena_msg, strlen(sala_llena_msg));
                    printf("Sala de partida %d está llena. No se puede invitar.\n", source_partida);
                }
                else {
                    // Enviar invitación al usuario objetivo
                    char invitacion[256];
                    snprintf(invitacion, sizeof(invitacion), "INVITACION:%s", source_username);
                    write(target_socket, invitacion, strlen(invitacion));
                    printf("Enviada invitación de %s a %s\n", source_username, target_username);
                }
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

            pthread_mutex_lock(&lista_mutex);
            int source_socket = encontrar_socket_por_usuario(source_username);
            int target_socket = encontrar_socket_por_usuario(target_username);
            pthread_mutex_unlock(&lista_mutex);

            if (source_socket != -1 && target_socket != -1) {
                // Obtener partida del usuario que invitó
                int source_partida = -1;
                pthread_mutex_lock(&lista_mutex);
                for (int i = 0; i < num_usuarios_conectados; i++) {
                    if (strcmp(usuarios_conectados[i].nombre_usuario, source_username) == 0) {
                        source_partida = usuarios_conectados[i].partida_id;
                        break;
                    }
                }
                pthread_mutex_unlock(&lista_mutex);

                if (source_partida != -1) {

                    // Antes de agregar al jugador invitado, verificar si ya está en una partida
                    pthread_mutex_lock(&lista_mutex);
                    int target_partida = -1;
                    for (int i = 0; i < num_usuarios_conectados; i++) {
                        if (strcmp(usuarios_conectados[i].nombre_usuario, target_username) == 0) {
                            target_partida = usuarios_conectados[i].partida_id;
                            break;
                        }
                    }
                    pthread_mutex_unlock(&lista_mutex);

                    if (target_partida != -1) {
                        // Salir de la partida actual
                        salir_partida(target_socket, 0);
                        printf("Usuario '%s' salió de la partida %d para unirse a la nueva partida.\n", target_username, target_partida);
                    }

                    // Agregar al jugador invitado a la partida
                    pthread_mutex_lock(&partidas_mutex);
                    for (int i = 0; i < num_partidas_activas; i++) {
                        if (partidas_activas[i].id == source_partida) {
                            if (partidas_activas[i].num_jugadores < MAX_JUGADORES_POR_PARTIDA) {
                                partidas_activas[i].jugadores[partidas_activas[i].num_jugadores] = target_socket;
                                strncpy(partidas_activas[i].nombres[partidas_activas[i].num_jugadores], target_username, 49);
                                partidas_activas[i].nombres[partidas_activas[i].num_jugadores][49] = '\0';
                                partidas_activas[i].num_jugadores++;

                                int t = partidas_activas[i].total_jugadores;
                                strncpy(partidas_activas[i].historialNombres[t], target_username, 49);
                                partidas_activas[i].historialSizes[t] = 100; // tamaño inicial, o 80, etc.
                                partidas_activas[i].total_jugadores++;


                                // Actualizar el partida_id del jugador invitado
                                pthread_mutex_lock(&lista_mutex);
                                for (int j = 0; j < num_usuarios_conectados; j++) {
                                    if (usuarios_conectados[j].socket == target_socket) {
                                        usuarios_conectados[j].partida_id = source_partida;
                                        break;
                                    }
                                }
                                pthread_mutex_unlock(&lista_mutex);

                                // Notificar al jugador que ha sido asignado a la partida
                                char mensaje_partida[] = "HAS_SIDO_ASIGNADO_A_PARTIDA\n";
                                write(target_socket, mensaje_partida, strlen(mensaje_partida));

                                // Enviar notificación de aceptación al jugador que invitó
                                char aceptada_msg[256];
                                snprintf(aceptada_msg, sizeof(aceptada_msg), "INVITACION_ACEPTADA:%s\n", target_username);
                                write(source_socket, aceptada_msg, strlen(aceptada_msg));
                                printf("Enviando 'INVITACION_ACEPTADA:%s' a socket %d\n", target_username, source_socket);

                                break;
                            }
                            else {
                                // La partida ya está llena
                                char error_msg[] = "ERROR_PARTIDA: La partida ya está llena.\n";
                                write(target_socket, error_msg, strlen(error_msg));
                                printf("Error: La partida ID %d ya está llena. No se puede añadir al usuario '%s'.\n", source_partida, target_username);
                            }
                        }
                    }
                    pthread_mutex_unlock(&partidas_mutex);
                }
                else {
                    // El jugador que invitó no tiene una partida asignada
                    char error_msg[] = "ERROR_INVITACION_ACEPTADA: El jugador que invitó no tiene una partida.\n";
                    write(target_socket, error_msg, strlen(error_msg));
                }
            }
            else {
                char respuesta[] = "ERROR_INVITACION_ACEPTADA: Usuario que invitó no está conectado.\n";
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

    else if (strcmp(mensaje, "SALIR_PARTIDA") == 0) {
		salir_partida(sock_conn, 1);
    }

    else if (strcmp(mensaje, "SALIR_PARTIDA2") == 0) {
		printf("Recibido mensaje de salir de partida\n");
        salir_partida(sock_conn, 0);
    }

    else if (strcmp(mensaje, "ELIMINAR_CUENTA") == 0) {
        // Entrar a la BD
		salir_partida(sock_conn, 0);
        entrarBD(conn);

        // Ejecutar el DELETE en la base de datos
        char query[512];
        snprintf(query, sizeof(query), "DELETE FROM Usuarios WHERE usuario='%s';", usuario);
        int err = mysql_query(conn, query);

        if (err == 0) {
            // Cuenta eliminada correctamente
            char respuesta[] = "CUENTA_ELIMINADA_OK";
            write(sock_conn, respuesta, strlen(respuesta));

            // Quitar al usuario de la lista de conectados
            char nombre_usuario_eliminado[50];
            quitar_usuario(sock_conn, nombre_usuario_eliminado);

            // Cerrar la conexión con el cliente pues su cuenta ya no existe
            //close(sock_conn);
            //pthread_exit(NULL);
        }
        else {
            // Error al eliminar la cuenta
            char respuesta[] = "ERROR_ELIMINAR_CUENTA";
            write(sock_conn, respuesta, strlen(respuesta));
        }
    }



    else if (strcmp(mensaje, "BUSCAR_PARTIDA") == 0) {
        // Obtener la partida del usuario
        int jugador_partida = -1;
        pthread_mutex_lock(&lista_mutex);
        for (int i = 0; i < num_usuarios_conectados; i++) {
            if (usuarios_conectados[i].socket == sock_conn) {
                jugador_partida = usuarios_conectados[i].partida_id;
                break;
            }
        }
        pthread_mutex_unlock(&lista_mutex);

        if (jugador_partida == -1) {
			// El jugador no tiene partida, asignar a una existente
			int partida_id = asignar_a_partida(sock_conn, usuario);
			printf("Partida asignada: %d con usuario: %s\n", partida_id, usuario);
            // No hay existentes, crear una nueva
            if (partida_id == -1) {
                partida_id = crear_partida();
				partida_id = asignar_a_partida(sock_conn, usuario);
				printf("Partida creada: %d con usuario: %s\n", partida_id, usuario);
                if (partida_id == -1) {
                    char error_msg[] = "ERROR_PARTIDA: No se pudo crear una nueva partida.\n";
                    write(sock_conn, error_msg, strlen(error_msg));
                    printf("Error: No se pudo crear una nueva partida.\n");
                    return;
                }
            }
            jugador_partida = partida_id;
        }

        // Comprobar si la partida está completa
        int num_jugadores_en_partida = 0;
        pthread_mutex_lock(&partidas_mutex);
        for (int i = 0; i < num_partidas_activas; i++) {
            if (partidas_activas[i].id == jugador_partida) {
                num_jugadores_en_partida = partidas_activas[i].num_jugadores;
                printf("jugadores en la partida: %d\n", num_jugadores_en_partida);
                break;
            }
        }
        pthread_mutex_unlock(&partidas_mutex);

        if (num_jugadores_en_partida < MAX_JUGADORES_POR_PARTIDA) {
            // Añadir la partida a la cola de matchmaking si no está ya en la cola
            pthread_mutex_lock(&matchmaking_mutex);
            int partida_ya_en_cola = 0;
            for (int i = 0; i < num_matchmaking; i++) {
                if (cola_matchmaking[i] == jugador_partida) {
                    partida_ya_en_cola = 1;
					printf("Partida %d ya está en la cola de matchmaking.\n", jugador_partida);
                    break;
                }
            }
            if (!partida_ya_en_cola) {
                if (num_matchmaking < MAX_MATCHMAKING_QUEUE) {
                    cola_matchmaking[num_matchmaking] = jugador_partida;
                    num_matchmaking++;
                    printf("Partida %d añadida a la cola de matchmaking.\n", jugador_partida);
                }
                else {
                    char error_msg[] = "ERROR_MATCHMAKING: La cola de matchmaking está llena.";
					printf("Error: La cola de matchmaking está llena.\n");
                    write(sock_conn, error_msg, strlen(error_msg));
                }
            }
            pthread_mutex_unlock(&matchmaking_mutex);
        }
        else {
            // La partida ya está completa, iniciar la partida
            pthread_mutex_lock(&partidas_mutex);
            Partida* partida_actual = NULL;
            for (int i = 0; i < num_partidas_activas; i++) {
                if (partidas_activas[i].id == jugador_partida) {
                    partidas_activas[i].en_juego = 1; // Actualizar el flag
                    partida_actual = &partidas_activas[i];
                    break;
                }
            }
            pthread_mutex_unlock(&partidas_mutex);

            if (partida_actual != NULL) {
                char mensaje_inicio[] = "PARTIDA_INICIADA\n";
                printf("Enviando mensaje de inicio de partida a los jugadores de la partida %d\n", jugador_partida);
                for (int j = 0; j < partida_actual->num_jugadores; j++) {
                    write(partida_actual->jugadores[j], mensaje_inicio, strlen(mensaje_inicio));
                }
            }
        }
    }
    else if (strcmp(mensaje, "QUITAR_USUARIO") == 0) {
        quitar_usuario(sock_conn, usuario);
        // Aquí ya se quita el usuario de la lista de conectados.
    }


    else if (strncmp(mensaje, "UPDATE_GAME_STATE/", 18) == 0) {
        // Mensaje con formato:
        // UPDATE_GAME_STATE/nombreJugador/size,color,estado/jugadoresComidosStr/burbujasEstaticasComidasStr
        // Vamos a parsear el mensaje
        char mensaje_original[512];
        strncpy(mensaje_original, mensaje, sizeof(mensaje_original) - 1);
        mensaje_original[sizeof(mensaje_original) - 1] = '\0';


        char* raw_data = mensaje_original + strlen("UPDATE_GAME_STATE/");
        char* nombre = strtok(raw_data, "/");
        char* pos_data = strtok(NULL, "/");
        char* jugadores_comidos = strtok(NULL, "/");
        char* burbujas_comidas = strtok(NULL, "/");

        // Comprobar que todas existen, si no, el formato es incorrecto
        if (!nombre || !pos_data || !jugadores_comidos || !burbujas_comidas) {
            printf("Formato UPDATE_GAME_STATE invalido\n");
            return;
        }

        // Parsear pos_data: size,color,estado
        int size, color;
        char estado_jugador;
        if (sscanf(pos_data, "%d,%d,%c", &size, &color, &estado_jugador) != 3) {
            printf("Error al parsear pos_data en UPDATE_GAME_STATE\n");
            return;
        }

        // Encontrar la partida del jugador
        pthread_mutex_lock(&lista_mutex);
        int partida_id = -1;
        int socket_jugador = -1;
        for (int i = 0; i < num_usuarios_conectados; i++) {
            if (strcmp(usuarios_conectados[i].nombre_usuario, nombre) == 0) {
                partida_id = usuarios_conectados[i].partida_id;
                socket_jugador = usuarios_conectados[i].socket;
                break;
            }
        }
        pthread_mutex_unlock(&lista_mutex);

        if (partida_id == -1) {
            // El jugador no está en ninguna partida
            return;
        }

        // Ahora que tenemos la partida, reenviar este mensaje a todos los jugadores de la partida excepto el remitente
        pthread_mutex_lock(&partidas_mutex);
        Partida* p = NULL;
        for (int i = 0; i < num_partidas_activas; i++) {
            if (partidas_activas[i].id == partida_id) {
                p = &partidas_activas[i];
                break;
            }
        }

        if (p != NULL) {
            // Aquí podríamos actualizar datos del servidor sobre el estado del jugador si fuera necesario.
            // Por ejemplo, podríamos mantener una estructura con el estado de cada jugador, pero no está implementado.
            // De momento solo retransmitiremos el mensaje a los demás jugadores.

            for (int j = 0; j < p->num_jugadores; j++) {
                int target_sock = p->jugadores[j];
                if (target_sock != socket_jugador) {
                    // Reenviar el mismo mensaje tal cual
                    write(target_sock, mensaje, strlen(mensaje));
                    printf("Mensaje enviado : %s\n", mensaje);
                }
            }
        }
        pthread_mutex_unlock(&partidas_mutex);
    }

     mysql_close(conn);
}

void reenviar_udp(char* username, int x, int y) {
    pthread_mutex_lock(&partidas_mutex);

    // Encontrar la partida del usuario
    int partida_id = -1;
    for (int i = 0; i < num_usuarios_conectados; i++) {
        if (strcmp(usuarios_conectados[i].nombre_usuario, username) == 0) {
            partida_id = usuarios_conectados[i].partida_id;
            break;
        }
    }

    if (partida_id == -1) {
        pthread_mutex_unlock(&partidas_mutex);
        return; // Usuario no esta en ninguna partida
    }

    // Reenviar a los demas jugadores de la misma partida
    for (int i = 0; i < num_partidas_activas; i++) {
        if (partidas_activas[i].id == partida_id) {
            for (int j = 0; j < partidas_activas[i].num_jugadores; j++) {
                int target_sock = partidas_activas[i].jugadores[j];
                char target_username[50];
                strcpy(target_username, partidas_activas[i].nombres[j]);

                // Buscar la direccion UDP del jugador
                struct sockaddr_in target_udp_addr;
                int encontrado = 0;
                pthread_mutex_lock(&lista_mutex);
                for (int k = 0; k < num_usuarios_conectados; k++) {
                    if (strcmp(usuarios_conectados[k].nombre_usuario, target_username) == 0 && usuarios_conectados[k].udp_flag) {
                        target_udp_addr = usuarios_conectados[k].udp_addr;
                        encontrado = 1;
                        break;
                    }
                }
                pthread_mutex_unlock(&lista_mutex);

                if (encontrado) {
                    // Crear el mensaje
                    char mensaje[512];
                    snprintf(mensaje, sizeof(mensaje), "UPDATE_GAME_POS/%s/%d,%d", username, x, y);
                    //printf("Mensaje enviado : %s\n", mensaje);
                    sendto(sock_listen_udp, mensaje, strlen(mensaje), 0, (struct sockaddr*)&target_udp_addr, sizeof(target_udp_addr));
                }
            }
            break;
        }
    }
    pthread_mutex_unlock(&partidas_mutex);
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
		int ret = read(sock_conn, buff, sizeof(buff) - 1); //Leer el mensaje del cliente
        if (ret > 0) 
        {
            buff[ret] = '\0';
            //printf("Mensaje recibido: %s\n", buff);

			procesar_mensaje(sock_conn, buff, usuario);//Procesar el mensaje recibido
        }
		else if (ret == 0)  //El cliente ha cerrado la conexión
        {
            if (strlen(usuario) > 0) {
				salir_partida(sock_conn, 0);
                quitar_usuario(sock_conn, usuario);
				printf("Usuario %s desconectado\n", usuario);
            }
            terminar = 1;
        }
        else 
        {
			printf("Error al leer el mensaje del cliente: %s\n", strerror(errno)); //Error al leer el mensaje del cliente
            if (strlen(usuario) > 0) {
                salir_partida(sock_conn, 0);
                quitar_usuario(sock_conn, usuario);
            }
            terminar = 1;
        }
    }
	close(sock_conn);  // Se acabó el servicio para este cliente. Jugador desconectado
    pthread_exit(NULL);
}

void* AtenderClienteUDP(void* arg)
{
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    char buffer[512];

    while (1) {
        int n = recvfrom(sock_listen_udp, buffer, sizeof(buffer) - 1, 0, (struct sockaddr*)&cli_addr, &clilen);
        if (n > 0) {
            buffer[n] = '\0';

            if (strncmp(buffer, "REGISTER_UDP/", 13) == 0) {
                // Formato: "REGISTER_UDP/username"
                char* username = strtok(buffer + 13, "/");
                if (username) {
                    registrar_udp_addr(username, &cli_addr);
                }
            }
            else if (strncmp(buffer, "UPDATE_GAME_POS/", 16) == 0) {
                // Formato: "UPDATE_GAME_POS/username/x,y"
                char* username = strtok(buffer + 16, "/");
                char* pos = strtok(NULL, "/");
                if (username && pos) {
                    int x, y;
                    sscanf(pos, "%d,%d", &x, &y);

                    // Reenviar a los demas jugadores de la misma partida
                    reenviar_udp(username, x, y);
                }
            }
        }
    }
    pthread_exit(NULL);

}

void* AceptarConexiones(void* arg) 
{
    int sock_conn;
    int sockets[100];
    pthread_t thread;
    int i = 0;

    while (1) {
		sock_conn = accept(sock_listen, NULL, NULL); //Aceptar la conexión
        if (sock_conn < 0) {
            printf("Error en el accept\n");
            continue;
        }

        printf("Conexión aceptada\n");
        sockets[i] = sock_conn;

		pthread_create(&thread, NULL, AtenderCliente, &sockets[i]);	//Crear un hilo para atender al cliente    
		pthread_detach(thread); //Libera los recursos del hilo al finalizar

        i++;
		if (i >= 100) {  //Reiniciar el contador de sockets
            i = 0;
        }
    }
    pthread_exit(NULL);
}

void* procesar_notificaciones(void* arg) 
{  // Función para el hilo de notificaciones
    while (1) {
        pthread_mutex_lock(&notificacion_mutex);
        while (num_notificaciones == 0) {  // Esperar a que haya notificaciones 
            pthread_cond_wait(&notificacion_cond, &notificacion_mutex);  // Bloquear el hilo hasta que se despierte
        }

        Notificacion notificacion = cola_notificaciones[0];	 // Obtener la primera notificación
        for (int i = 0; i < num_notificaciones - 1; i++) {    // Mover las notificaciones restantes
            cola_notificaciones[i] = cola_notificaciones[i + 1];
        }
        num_notificaciones--;	//Decrementar el contador de notificaciones    
        pthread_mutex_unlock(&notificacion_mutex);

        if (strcmp(notificacion.mensaje, "UPDATE_LIST") == 0) {
            char connectedUsers[1000];
            strcpy(connectedUsers, "USUARIOS_CONECTADOS/");

            pthread_mutex_lock(&lista_mutex);
            for (int i = 0; i < num_usuarios_conectados; i++) {  //Construir la lista de usuarios conectados
                strcat(connectedUsers, usuarios_conectados[i].nombre_usuario);
                if (i < num_usuarios_conectados - 1) {
                    strcat(connectedUsers, "\n"); // Separador de usuarios
                }
            }
            pthread_mutex_unlock(&lista_mutex);

            pthread_mutex_lock(&lista_mutex);
            for (int i = 0; i < num_usuarios_conectados; i++) {  //Enviar la lista de usuarios conectados a todos los clientes     
                int sock_conn_i = usuarios_conectados[i].socket;
                write(sock_conn_i, connectedUsers, strlen(connectedUsers));
            }
            pthread_mutex_unlock(&lista_mutex);

            printf("Lista completa de usuarios enviada a todos los clientes.\n");
        }
    }
    return NULL;
}



/*==============================================================================================================================*/
/*============================================================ MAIN ============================================================*/

int main(int argc, char* argv[])
{
    // Creacion del socket TCP
    /*==================================================================================================================================*/
    struct sockaddr_in serv_adr; // Dirección del servidor    

    if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0) { // Crear el socket
        printf("Error creando socket TCP\n");
        exit(1);
    }

    memset(&serv_adr, 0, sizeof(serv_adr)); // Inicializar la estructura de dirección
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(TCP_PORT); // Puerto TCP

    if (bind(sock_listen, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) < 0) { // Asociar el socket a la dirección
        printf("Error en el bind TCP\n");
        exit(1);
    }

    if (listen(sock_listen, 2) < 0) { // Escuchar en el socket      
        printf("Error en el listen TCP\n");
        exit(1);
    }

    printf("Servidor TCP iniciado correctamente. Escuchando en puerto %d\n", TCP_PORT);

    // Creacion del socket UDP
    /*==================================================================================================================================*/

    struct sockaddr_in serv_udp_addr;

    if ((sock_listen_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) { // Crear el socket
        printf("Error creando socket UDP\n");
        exit(1);
    }

    memset(&serv_udp_addr, 0, sizeof(serv_udp_addr));
    serv_udp_addr.sin_family = AF_INET;
    serv_udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_udp_addr.sin_port = htons(UDP_PORT);  // Puerto UDP

    if (bind(sock_listen_udp, (struct sockaddr*)&serv_udp_addr, sizeof(serv_udp_addr)) < 0) {
        printf("Error en el bind UDP\n");
        exit(1);
    }

    printf("Servidor UDP iniciado correctamente. Escuchando en puerto %d\n", UDP_PORT);

    pthread_t thread_udp; // Hilo para el UDP
    pthread_create(&thread_udp, NULL, AtenderClienteUDP, NULL);
    pthread_detach(thread_udp);

    pthread_t thread_accept; // Hilo para aceptar conexiones TCP
    pthread_create(&thread_accept, NULL, AceptarConexiones, NULL);
    pthread_detach(thread_accept);

    pthread_t notificacion_thread; // Hilo para procesar notificaciones TCP
    pthread_create(&notificacion_thread, NULL, procesar_notificaciones, NULL);
    pthread_detach(notificacion_thread);

    while (1)
    {
        sleep(10); // Hilo principal duerme, puede hacer otras cosas
    }

    close(sock_listen);
    close(sock_listen_udp);
    return 0;
}

