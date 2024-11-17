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

typedef struct {
    int socket;
    char nombre_usuario[50];  // Nombre del usuario
} UsuarioInfo;

typedef struct {
    char mensaje[512];
} Notificacion;

UsuarioInfo usuarios_conectados[MAX_USERS];
int num_usuarios_conectados = 0;  // Contador de usuarios conectados

//Mutex para la lista de usuarios conectados para que no acceda a la lista mas de un hilo a la vez
pthread_mutex_t lista_mutex = PTHREAD_MUTEX_INITIALIZER; 

//Variables para la cola de notificaciones
Notificacion cola_notificaciones[MAX_NOTIFICACIONES];  //Vector de notificaciones
int num_notificaciones = 0;  //Contador de notificaciones
pthread_mutex_t notificacion_mutex = PTHREAD_MUTEX_INITIALIZER; //Mutex para la cola de notificaciones
pthread_cond_t notificacion_cond = PTHREAD_COND_INITIALIZER; //Variable de condicion para activar hilos
//Mutex bloquea el acceso y la condicion despierta a un hilo en espera

/*==============================================================================================================================*/
/*=================================================== Funciones auxiliares =====================================================*/

// Función para conectar a MySQL
void conectarMySQL(MYSQL** conn) {
    *conn = mysql_init(NULL);
    if (*conn == NULL) {
        printf("Error al crear la conexión: %u %s\n", mysql_errno(*conn), mysql_error(*conn));
        exit(1);
    }
}

// Función para seleccionar la base de datos
void entrarBD(MYSQL* conn) {
    if (mysql_real_connect(conn, "shiva2.upc.es", "root", "mysql", "M3_UsuariosBBDD", 0, NULL, 0) == NULL) {
        printf("Error al inicializar la conexión: %u %s\n", mysql_errno(conn), mysql_error(conn));
        exit(1);
    }
}

/*==============================================================================================================================*/
/*======================================== Funciones para manejo de usuarios conectados ========================================*/

void encolar_notificacion() {
	// Bloquear el mutex para que no accedan a la cola de notificaciones más de un hilo a la vez
    pthread_mutex_lock(&notificacion_mutex);  
    if (num_notificaciones < MAX_NOTIFICACIONES) {
        strcpy(cola_notificaciones[num_notificaciones].mensaje, "UPDATE_LIST");
        num_notificaciones++;
		pthread_cond_signal(&notificacion_cond);  // Despertar a un hilo en espera
    }
    else {
        // Cola llena, manejar error si es necesario
        printf("Cola de notificaciones llena. No se puede encolar la notificación.\n");
    }
    pthread_mutex_unlock(&notificacion_mutex);
}


// Función para añadir un usuario a la lista de conectados
void anadir_usuario(int sock_conn, char* usuario) {
    pthread_mutex_lock(&lista_mutex);

    if (num_usuarios_conectados < MAX_USERS) {
        usuarios_conectados[num_usuarios_conectados].socket = sock_conn;
        strncpy(usuarios_conectados[num_usuarios_conectados].nombre_usuario, usuario, sizeof(usuarios_conectados[num_usuarios_conectados].nombre_usuario) - 1);
        usuarios_conectados[num_usuarios_conectados].nombre_usuario[sizeof(usuarios_conectados[num_usuarios_conectados].nombre_usuario) - 1] = '\0';
        num_usuarios_conectados++;

        printf("Jugador conectado: %s\n", usuario);
        printf("Número de jugadores conectados: %d\n", num_usuarios_conectados);

        pthread_mutex_unlock(&lista_mutex);

        // Enviar lista actualizada de usuarios
        encolar_notificacion();
        printf("Notificacion de actualización de lista\n");
    }
    else {
        pthread_mutex_unlock(&lista_mutex);
		printf("Lista de usuarios llena. No se puede añadir más usuarios.\n");
    }
}

void quitar_usuario(int sock_fd, char* nombre_usuario) {
    pthread_mutex_lock(&lista_mutex);

    for (int i = 0; i < num_usuarios_conectados; i++) {
        if (usuarios_conectados[i].socket == sock_fd) {
            // Copiar el nombre del usuario para la notificación
            strcpy(nombre_usuario, usuarios_conectados[i].nombre_usuario);

            // Mover los elementos hacia la izquierda
            for (int j = i; j < num_usuarios_conectados - 1; j++) {
                usuarios_conectados[j] = usuarios_conectados[j + 1];
            }
            num_usuarios_conectados--;

            printf("Jugador desconectado: %s\n", nombre_usuario);

            pthread_mutex_unlock(&lista_mutex);

            // Enviar lista actualizada de usuarios
			encolar_notificacion();
            printf("Notificacion de actualizacion de lista");
            return;
        }
    }

    pthread_mutex_unlock(&lista_mutex); // Asegúrate de desbloquear el mutex si no se encuentra el usuario
}


// Función para el hilo de notificaciones
void* procesar_notificaciones(void* arg) {
    while (1) {
        pthread_mutex_lock(&notificacion_mutex);
        while (num_notificaciones == 0) {
            pthread_cond_wait(&notificacion_cond, &notificacion_mutex);
        }
        // Extraer la notificación
        Notificacion notificacion = cola_notificaciones[0];
        // Mover las notificaciones restantes
        for (int i = 0; i < num_notificaciones - 1; i++) {
            cola_notificaciones[i] = cola_notificaciones[i + 1];
        }
        num_notificaciones--;
        pthread_mutex_unlock(&notificacion_mutex);

        // Verificar el tipo de notificación
        if (strcmp(notificacion.mensaje, "UPDATE_LIST") == 0) {
            // Construir la lista completa de usuarios
            char lista_completa[4096]; // Ajusta el tamaño según sea necesario
            strcpy(lista_completa, "USUARIOS_CONECTADOS/");

            pthread_mutex_lock(&lista_mutex);
            for (int i = 0; i < num_usuarios_conectados; i++) {
                strcat(lista_completa, usuarios_conectados[i].nombre_usuario);
                if (i < num_usuarios_conectados - 1) {
                    strcat(lista_completa, "\n"); // Separador de usuarios
                }
            }
            pthread_mutex_unlock(&lista_mutex);

            // Enviar la lista completa a todos los clientes
            pthread_mutex_lock(&lista_mutex);
            for (int i = 0; i < num_usuarios_conectados; i++) {
                int sock_conn_i = usuarios_conectados[i].socket;
                write(sock_conn_i, lista_completa, strlen(lista_completa));
            }
            pthread_mutex_unlock(&lista_mutex);

            printf("Lista completa de usuarios enviada a todos los clientes.\n");
        }

        // Puedes manejar otros tipos de notificaciones aquí en el futuro
    }
    return NULL;
}


/*==============================================================================================================================*/
/*============================================= Funciones-query preestablecidas ================================================*/

// Función para buscar y autenticar al usuario
void buscarUserAndPassBBDD(MYSQL* conn, int sock_conn, char* usuario, char* contrasena) {
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

// Función para cambiar la contraseña del usuario
void buscarUserAndChangePassBBDD(MYSQL* conn, int sock_conn, char* usuario, char* nueva_contrasena) {
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

// Función para insertar un nuevo usuario en la base de datos
void insertarUsuario(MYSQL* conn, int sock_conn, char* usuario, char* contrasena) {
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

// Función para crear y borrar la base de datos
void crearAndBorrarBBDD(MYSQL* conn, int sock_conn) {
    int err;

    // Eliminar la base de datos si existe
    err = mysql_query(conn, "DROP DATABASE IF EXISTS M3_UsuariosBBDD;");
    if (err != 0) {
        printf("Error al eliminar la base de datos: %u %s\n", mysql_errno(conn), mysql_error(conn));
    }
    else {
        printf("Base de datos eliminada correctamente.\n");
    }

    // Crear la base de datos
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

    // Seleccionar la base de datos
    err = mysql_query(conn, "USE M3_UsuariosBBDD;");
    if (err != 0) {
        printf("Error al usar la base de datos: %u %s\n", mysql_errno(conn), mysql_error(conn));
        char respuesta[] = "ERROR_USAR_BD";
        write(sock_conn, respuesta, strlen(respuesta));
        return;
    }

    // Crear la tabla de usuarios
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

    // Insertar usuario predeterminado
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

    // Enviar confirmación al cliente
    char respuesta[] = "OK";
    write(sock_conn, respuesta, strlen(respuesta));
}

// Función para contar cuántos usuarios hay en la base de datos
void contarUsuariosBBDD(MYSQL* conn, int sock_conn) {
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

    // Preparar respuesta con el número de usuarios
    char respuesta[512];
    snprintf(respuesta, sizeof(respuesta), "TOTAL_USERS:%d", totalUsuarios);
    write(sock_conn, respuesta, strlen(respuesta));
}

// Función para encontrar el socket de un usuario por su nombre
int encontrar_socket_por_usuario(const char* nombre_usuario) {
    for (int i = 0; i < num_usuarios_conectados; i++) {
        if (strcmp(usuarios_conectados[i].nombre_usuario, nombre_usuario) == 0) {
            return usuarios_conectados[i].socket;
        }
    }
    return -1; // No encontrado
}


/*==============================================================================================================================*/
/*============================== Funciones para procesar los mensajes recibidos desde el cliente ===============================*/

// Función para procesar el mensaje recibido
void procesar_mensaje(int sock_conn, char* mensaje, char* usuario) {
    MYSQL* conn;
    conectarMySQL(&conn);

    // Mensaje para crear la base de datos
    if (strcmp(mensaje, "CREAR_BD") == 0) {
        printf("Recibido mensaje para crear la base de datos\n");
        crearAndBorrarBBDD(conn, sock_conn);
    }
    // Mensaje para contar el número de usuarios en la base de datos
    else if (strcmp(mensaje, "CONTAR_USERS") == 0) {
        printf("Recibido mensaje para contar usuarios\n");
        entrarBD(conn);
        contarUsuariosBBDD(conn, sock_conn);
    }
    // Mensaje para autenticar el usuario
    else if (strncmp(mensaje, "AUTENTICAR/", 11) == 0) {
        char* usuario_msg = strtok(mensaje + 11, "/");
        char* contrasena = strtok(NULL, "/");

        if (usuario_msg && contrasena) {
            strcpy(usuario, usuario_msg);  // Guardar el nombre de usuario
            entrarBD(conn);
            buscarUserAndPassBBDD(conn, sock_conn, usuario, contrasena);
        }
    }
    // Mensaje para cambiar la contraseña del usuario
    else if (strncmp(mensaje, "CAMBIAR_PASS/", 13) == 0) {
        char* usuario_msg = strtok(mensaje + 13, "/");
        char* nueva_contrasena = strtok(NULL, "/");

        if (usuario_msg && nueva_contrasena) {
            entrarBD(conn);
            buscarUserAndChangePassBBDD(conn, sock_conn, usuario_msg, nueva_contrasena);
        }
    }
    // Mensaje para añadir un usuario
    else if (strncmp(mensaje, "ADD_USER/", 9) == 0) {
        char* usuario_msg = strtok(mensaje + 9, "/");
        char* contrasena = strtok(NULL, "/");

        if (usuario_msg && contrasena) {
            entrarBD(conn);
            insertarUsuario(conn, sock_conn, usuario_msg, contrasena);
        }
    }

    // Mensaje para invitar a jugar
    else if (strncmp(mensaje, "INVITAR/", 8) == 0) {
        char* target_username = strtok(mensaje + 8, "/");
        char* source_username = strtok(NULL, "/");

        if (target_username && source_username) {
            // Encontrar el socket del usuario objetivo
            pthread_mutex_lock(&lista_mutex);
            int target_socket = encontrar_socket_por_usuario(target_username);
            pthread_mutex_unlock(&lista_mutex);

            if (target_socket != -1) {
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
    // Mensaje para aceptar la invitación
    else if (strncmp(mensaje, "INVITACION_ACEPTADA/", 21) == 0) {
        char* source_username = strtok(mensaje + 21, "/");
        char* target_username = strtok(NULL, "/");

        if (source_username && target_username) {
            // Encontrar el socket del usuario que invitó
            pthread_mutex_lock(&lista_mutex);
            int source_socket = encontrar_socket_por_usuario(source_username);
            pthread_mutex_unlock(&lista_mutex);

            if (source_socket != -1) {
                // Enviar notificación de aceptación al usuario que invitó
                char aceptada_msg[256];
                snprintf(aceptada_msg, sizeof(aceptada_msg), "INVITACION_ACEPTADA:%s", target_username);
                write(source_socket, aceptada_msg, strlen(aceptada_msg));
                printf("Invitación aceptada por %s para %s\n", target_username, source_username);

                // Aquí podrías implementar la lógica para iniciar una partida entre source y target
                // Por simplicidad, se deja como notificación
            }
        }
    }
    // Mensaje para rechazar la invitación
    else if (strncmp(mensaje, "INVITACION_RECHAZADA/", 22) == 0) {
        char* source_username = strtok(mensaje + 22, "/");
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

    else {
        char respuesta[] = "COMANDO_NO_RECONOCIDO";
        write(sock_conn, respuesta, strlen(respuesta));
        printf("Comando no reconocido: %s\n", mensaje);
    }

    mysql_close(conn);
}

/*==============================================================================================================================*/
/*============================================= Función para atender al cliente ================================================*/

void* AtenderCliente(void* arg) {
    int sock_conn = *(int*)arg;
    int terminar = 0;
    char buff[512];
    char usuario[50] = "";  // Nombre del usuario

    while (!terminar) {
        int ret = read(sock_conn, buff, sizeof(buff) - 1);
        if (ret > 0) {
            buff[ret] = '\0';
            printf("Mensaje recibido: %s\n", buff);

            // Procesar el mensaje
            procesar_mensaje(sock_conn, buff, usuario);
        }
        else if (ret == 0) {
            // El cliente ha cerrado la conexión
            if (strlen(usuario) > 0) {
                quitar_usuario(sock_conn, usuario);
            }
            terminar = 1;
        }
        else {
            // Ha ocurrido un error al leer del socket
            printf("Error al leer el mensaje del cliente: %s\n", strerror(errno));
            if (strlen(usuario) > 0) {
                quitar_usuario(sock_conn, usuario);
            }
            terminar = 1;
        }
    }

    // Se acabó el servicio para este cliente
    close(sock_conn);
    pthread_exit(NULL);
}


/*==============================================================================================================================*/
/*============================================================ MAIN ============================================================*/
int main(int argc, char* argv[]) {
    int sock_conn, sock_listen;
    struct sockaddr_in serv_adr;

    if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Error creando socket\n");
        exit(1);
    }

    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(50003);                           // Puerto de escucha

    if (bind(sock_listen, (struct sockaddr*)&serv_adr, sizeof(serv_adr)) < 0) {
        printf("Error en el bind\n");
        exit(1);
    }

    if (listen(sock_listen, 2) < 0) {
        printf("Error en el listen\n");
        exit(1);
    }
    printf("Servidor iniciado correctamente. Escuchando en puerto 50003\n");

    int sockets[100];
    pthread_t thread;
    pthread_t notificacion_thread;

    // Crear el hilo de notificaciones
    pthread_create(&notificacion_thread, NULL, procesar_notificaciones, NULL);

    for (int i = 0; ; i++) {
        sock_conn = accept(sock_listen, NULL, NULL);
        if (sock_conn < 0) {
            printf("Error en el accept\n");
            continue;
        }
        printf("Conexión aceptada\n");
        sockets[i] = sock_conn;

        // Crear un nuevo hilo para manejar al cliente
        pthread_create(&thread, NULL, AtenderCliente, &sockets[i]);
        pthread_detach(thread);  // Liberar recursos al terminar el hilo
    }

    // Nunca se llega aquí, pero por buenas prácticas
    close(sock_listen);
    pthread_mutex_destroy(&lista_mutex);
    pthread_mutex_destroy(&notificacion_mutex);
    pthread_cond_destroy(&notificacion_cond);

    return 0;
}
