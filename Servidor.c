/*
 Codigo del servidor del juego
 Editado por ultima vez: 5/10/2024
 Autores: Adria Sancho, Ivan de Dios, Arnau Torrent y Oscar Rigol
*/

#include "cabecera.h"

void *AtenderCliente(void*socket){
	//Definir el mutex
	//pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

	/*==============================================================================================================================*/
	/*============================================ Declaracion de constantes y variables ===========================================*/

	// Variables globales
	int sock_conn;
	int *s;
	s= (int *) socket;
	sock_conn= *s;
	int socket_conn = * (int *) socket;
	int terminar = 0;	
												
	char buff[512];
	char *usuario = NULL;
	char *contrasena = NULL;
	char *userDB;
	char *passDB;

	// Definicion de variables para la connexion con la base de datos
	MYSQL *conn;
	MYSQL_RES *resultado;
	MYSQL_ROW row;

	/*==============================================================================================================================*/
	/*============================================== Funciones de connexion con MYSQL ==============================================*/

	// Funcion para crear una connexion con MYSQL
	void conectarMySQL(){ 
		conn = mysql_init(NULL);
		if (conn==NULL) {
			printf ("Error al crear la conexion: %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
	}

	// Funcion para iniciar la connexion con MYSQL y entrar a la base de datos "Usuarios"
	void entrarBD(){
		conn = mysql_real_connect (conn, "localhost", "root", "mysql", "Usuarios",0, NULL, 0);
		if (conn==NULL) {
			printf ("Error al inicializar la conexion: %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
	}

	/*==============================================================================================================================*/
	/*============================================= Funciones-query preestablecidas ================================================*/

	// Funcion para cambiar la contrase a del usuario
	void buscarUserAndChangePassBBDD(){
		conectarMySQL();
		entrarBD();

		// Bloquear el mutex antes de acceder a la base de datos
		//pthread_mutex_lock(&db_mutex);

		int encontrado = 0;
		int err;
		err=mysql_query (conn, "SELECT * FROM Usuarios;");
		
		if (err != 0) {
			printf("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
			//pthread_mutex_unlock(&db_mutex); // Liberar el mutex en caso de error
			exit(1);
		}
		resultado = mysql_store_result (conn);
		row = mysql_fetch_row (resultado);
		
		if (row == NULL) {
			printf("No se han obtenido datos en la consulta\n");
			//pthread_mutex_unlock(&db_mutex); // Liberar el mutex si no hay datos
			return;
		}
		
		while (row !=NULL && encontrado==0){
			userDB = row[1];
			printf("Comparando Usuario: %s con %s\n", userDB, usuario);
			
			if ((strcmp(userDB, usuario) ==0)){
				encontrado = 1;
				printf("Usuario encontrado: %s\n", userDB);
			}
			row = mysql_fetch_row(resultado);
		}
		
		if (encontrado == 1){
			// Usuario encontrado, procedemos a actualizar la contrase a
			char query[512];
			snprintf(query, sizeof(query), "UPDATE Usuarios SET contrasena='%s' WHERE usuario='%s';", contrasena, usuario);
			
			// Ejecutamos la consulta de actualizaci n
			err = mysql_query(conn, query);
			if (err != 0){
				printf("Error al actualizar la contrase a: %u %s\n", mysql_errno(conn), mysql_error(conn));
				char respuesta[] = "ERROR_ACTUALIZACION";
				write(sock_conn, respuesta, strlen(respuesta));
			} 
			else {
				printf("Contrase a actualizada para el usuario: %s\n", usuario);
				char respuesta[] = "OK";
				write(sock_conn, respuesta, strlen(respuesta));
			}
		} 
		else {
			// Enviar "ERROR" al cliente si no son v lidas
			char respuesta[] = "ERROR";
			printf("Usuario no encontrado, ERROR\n");
			write(sock_conn, respuesta, strlen(respuesta));
		}
		
		// Liberamos el resultado
		mysql_free_result(resultado);	
		//pthread_mutex_unlock(&db_mutex); // Liberar el mutex al finalizar la operación
	}

	// Funcion para comprobar que el usuario que intenta iniciar sesion este registrado y sus credenciales sean correctas
	void buscarUserAndPassBBDD(){
		conectarMySQL();
		entrarBD();

		// Bloquear el mutex antes de acceder a la base de datos
		//pthread_mutex_lock(&db_mutex);

		int encontrado = 0;
		int err;
		err=mysql_query (conn, "SELECT * FROM Usuarios;");
		
		if (err != 0) {
			printf("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
			//pthread_mutex_unlock(&db_mutex); // Liberar el mutex en caso de error
			exit(1);
		}
		resultado = mysql_store_result (conn);
		row = mysql_fetch_row (resultado);
		
		if (row == NULL) {
			printf("No se han obtenido datos en la consulta\n");
			//pthread_mutex_unlock(&db_mutex); // Liberar el mutex si no hay datos
			return;
		}
		
		while (row !=NULL && encontrado==0){
			userDB = row[1];
			passDB = row[2];
			printf("Comparando Usuario: %s con %s y Contrase a: %s con %s\n", userDB, usuario, passDB, contrasena);
			if ((strcmp(userDB, usuario) ==0) && (strcmp(passDB,contrasena)==0)){
				encontrado = 1;
				printf("Usuario encontrado: %s\n", userDB);
			}
			row = mysql_fetch_row(resultado);
		}

		if (encontrado == 1) {
			// Enviar "OK" al cliente si las credenciales son v lidas
			char respuesta[] = "OK";
			printf("OK\n");
			write(sock_conn, respuesta, strlen(respuesta));
		} 
		else {
			// Enviar "ERROR" al cliente si no son v lidas
			char respuesta[] = "ERROR";
			printf("Usuario no encontrado, ERROR\n");
			write(sock_conn, respuesta, strlen(respuesta));
		}
		
		// Liberamos el resultado
		mysql_free_result(resultado);	
		//pthread_mutex_unlock(&db_mutex); // Liberar el mutex al finalizar la operación
	}

	// Funcion para eliminar y crear la base de datos
	void crearAndBorrarBBDD(){
		int err;
		
		// Intentamos eliminar la base de datos si existe
		mysql_query(conn, "DROP DATABASE IF EXISTS Usuarios;");
		
		// Intentamos crear la base de datos
		err = mysql_query(conn, "CREATE DATABASE Usuarios;");
		if (err != 0){
			printf("Error al crear la base de datos: %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		} 
		else{
			printf("Base de datos creada correctamente.\n");
		}
		
		// Seleccionamos la base de datos
		err = mysql_query(conn, "USE Usuarios;");
		if (err != 0){
			printf("Error al usar la base de datos: %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		}
		
		// Creamos la tabla de usuarios
		err = mysql_query(conn, "CREATE TABLE Usuarios (id int primary key not null auto_increment, usuario TEXT not null, contrasena TEXT not null);");
		if (err != 0){
			printf("Error al crear la tabla Usuarios: %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		}
		
		// Insertamos el usuario y la contrase a predeterminados
		char query[512];
		snprintf(query, sizeof(query), "INSERT INTO Usuarios (usuario, contrasena) VALUES ('root', 'root')");
		
		err = mysql_query(conn, query);
		if (err != 0){
			printf("Error al insertar el usuario predeterminado %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		} 
		else{
			printf("Usuario predeterminado insertado corrctamente: root, root\n");
		}
	}

	// Funcion para insertar un nuevo usuario en la base de datos
	void insertarUsuario(){
		int encontrado = 0;
		char query[512];
		int err;
		
		// Preparamos la consulta SQL para insertar el usuario y la contrase a
		snprintf(query, sizeof(query), "INSERT INTO Usuarios (usuario, contrasena) VALUES ('%s', '%s')", usuario, contrasena);
		
		// Ejecutamos la consulta
		err = mysql_query(conn, query);
		
		if (err != 0){
			printf("Error al insertar el usuario %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		} 
		else {
			// Confirmamos que el usuario fue insertado correctamente
			printf("Usuario '%s' insertado correctamente.\n", usuario);
			encontrado = 1;
		}
		
		if (encontrado == 1){
			// Enviar "OK" al cliente si las credenciales son v lidas
			char respuesta[] = "OK";
			write(sock_conn, respuesta, strlen(respuesta));
		} 
		else {
			// Enviar "ERROR" al cliente si no son v lidas
			char respuesta[] = "ERROR";
			printf("Usuario no encontrado, ERROR\n");
			write(sock_conn, respuesta, strlen(respuesta));
		}
	}

	// Funcion para contar cuantos usuarios hay en la base de datos
	int contarUsuariosBBDD() {
		int err;
		int totalUsuarios = 0;
		
		// Realizamos la consulta SQL para contar el n mero total de filas en la tabla Usuarios
		err = mysql_query(conn, "SELECT COUNT(*) FROM Usuarios;");
		
		if (err != 0){
			printf("Error al contar los usuarios en la base de datos %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit(1);
		} 
		else {
			// Obtenemos el resultado de la consulta
			resultado = mysql_store_result(conn);
			if (resultado == NULL){
				printf("Error al obtener el resultado de la consulta %u %s\n", mysql_errno(conn), mysql_error(conn));
				exit(1);
			}
			
			// Obtenemos la primera fila (el resultado del COUNT)
			row = mysql_fetch_row(resultado);
			if (row != NULL){
				totalUsuarios = atoi(row[0]);
				printf("El n mero total de usuarios es: %d\n", totalUsuarios);
			}
			
			// Liberamos el resultado
			mysql_free_result(resultado);
		}
		return totalUsuarios;
	}

	/*==============================================================================================================================*/
	/*============================== Funciones para procesar los mensajes recibidos desde el cliente ===============================*/

	// Mensaje para crear la base de datos
	void mensaje_crearBD(){
		printf("Recibido mensaje para crear la base de datos\n");
		conectarMySQL();
		entrarBD();
		crearAndBorrarBBDD();
		
		// Enviamos la confirmacion al cliente
		char respuesta[] = "OK";												
		write(sock_conn, respuesta, strlen(respuesta));	
	}

	// Mensaje para contar el numero de ususarios en la base de datos
	void mensaje_contarUsers(){
		printf("Recibido mensaje para contar usuarios\n");
		conectarMySQL();
		entrarBD();
		int totalUsuarios = contarUsuariosBBDD();
			
		// Preparar respuesta con el numero de usuarios
		char respuesta[512];
		snprintf(respuesta, sizeof(respuesta), "TOTAL_USERS:%d", totalUsuarios);												
		write(sock_conn, respuesta, strlen(respuesta));
	}

	// Mensaje para autenticar el usuario
	void mensaje_autenticar(char* mensaje){
		
		// Extraemos el usuario y la contrase a del mensaje
		usuario = strtok(mensaje + 11, "/");									
		contrasena = strtok(NULL, "/");
		
		if (usuario && contrasena){
			printf("Usuario: %s, Contrase a: %s\n", usuario, contrasena);
			printf("Autenticando usuario: %s\n", usuario);
			buscarUserAndPassBBDD();
		}
	}

	// Mensaje para cambiar la contrase a del usuario
	void mensaje_cambiarPass(char*mensaje){
		
		// Extraemos el usuario y la contrase a del mensaje
		usuario = strtok(mensaje + 13, "/");									
		contrasena = strtok(NULL, "/");
		
		if (usuario && contrasena){
			printf("Usuario: %s, Contrase a: %s\n", usuario, contrasena);
			printf("Autenticando usuario: %s\n", usuario);
			buscarUserAndChangePassBBDD();
		}
		// En caso de error:
		else{
			printf("No se puede extraer usuario y contrasena.\n");
			char respuesta[] = "ERROR";												
			write(sock_conn, respuesta, strlen(respuesta));						
		}
	}

	// Mensaje para a adir un usuario
	void mensaje_addUser(char*mensaje){
		
		// Extraemos el usuario y la contrase a del mensaje
		usuario = strtok(mensaje + 9, "/");									
		contrasena = strtok(NULL, "/");
		
		if (usuario && contrasena){
			printf("Usuario: %s, Contrase a: %s\n", usuario, contrasena);
			printf("Creando usuario: %s\n", usuario);
			conectarMySQL();
			entrarBD();
			insertarUsuario();
		}
		// En caso de error:
		else{
			printf("No se puede crear el usuario.\n");
			char respuesta[] = "ERROR";												
			write(sock_conn, respuesta, strlen(respuesta));						
		}
	}

	/*==============================================================================================================================*/
	/*========================== Funcion "switch" para interpretar los mensajes recibidos desde el cliente =========================*/

	void procesar_mensaje(char* mensaje){
		
		// Mensaje para crear la base de datos
		if (strcmp(mensaje, "CREAR_BD") == 0){										
			mensaje_crearBD();
		}
		// Mensaje para contar el numero de ususarios en la base de datos
		else if (strcmp(mensaje, "CONTAR_USERS") == 0){									
			mensaje_contarUsers();
		}
		// Mensaje para autenticar el usuario
		else if (strncmp(mensaje, "AUTENTICAR/", 11) == 0){
			mensaje_autenticar(mensaje);
		}
		// Mensaje para cambiar la contrase a del usuario
		else if (strncmp(mensaje, "CAMBIAR_PASS/", 13) == 0){
			mensaje_cambiarPass(mensaje);
		}
		// Mensaje para a adir un usuario
		else if (strncmp(mensaje, "ADD_USER/", 9) == 0){
			mensaje_addUser(mensaje);
		}
		/*
		// En caso de comando no reconocido
		else {
			char respuesta[] = "COMANDO_NO_RECONOCIDO";  							
			write(sock_conn, respuesta, strlen(respuesta));
			printf("Comando no reconocido: %s\n", mensaje);
		}
		*/
	}
	
	// Funcion para obtener el mensaje del cliente
	void obtener_mensajeCliente(){
		char buff[512];
		int ret;
		
		while ((ret = read(sock_conn, buff, sizeof(buff))) > 0){
			buff[ret] = '\0';
			printf("Mensaje recibido: %s\n", buff);
			
			// Interpretar el mensaje y procesarlo en la funcion "switch"
			procesar_mensaje(buff);  
		}
		
		if (ret == 0) {
			printf("El cliente ha cerrado la conexion.\n");
			terminar = 1;
		} 
		else {
			printf("Error al leer el mensaje del cliente.\n");
		}
	}
	
	while(terminar == 0){
		obtener_mensajeCliente(sock_conn); // Maneja el mensaje del cliente
	}
	
	// Se acabo el servicio para este cliente
	close(sock_conn); 
}
/*==============================================================================================================================*/
/*======================================== Funciones para la comunicacion con el cliente =======================================*/


/*==============================================================================================================================*/
/*============================================================ MAIN ============================================================*/

int main(int argc, char *argv[]){
	int sock_conn, sock_listen;
	struct sockaddr_in serv_adr;
	
		
	// Si no podemos abrir el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0){				
		printf("Error creando socket");											
		exit(1);	
	}
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(20518);
	
	if (bind(sock_listen, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) < 0){
		printf("Error en el bind");
		exit(1);
	}
	
	// En caso de que la cola de peticiones pendientes sea mayor que 2
	if (listen(sock_listen, 2) < 0){						
		printf("Error en el listen");
	}
	printf("Servidor iniciado correctamente. Escuchando en puerto 20518\n");
	
	printf("Esperando conexión...\n");
	int sockets[100];
	pthread_t thread;
	
	// A la escucha de conexiones infinitamente
	for (int i = 0; i > -1 ;i++) {
		sock_conn = accept(sock_listen, NULL, NULL);
		
		// Mostrar mensaje de conexión aceptada con el número de conexión
		printf("Conexión aceptada %d.\n");
		sockets[i] = sock_conn;
		
		// Crear un nuevo hilo para manejar al cliente
		pthread_create (&thread, NULL, AtenderCliente,&sockets[i]);	
	}
}
