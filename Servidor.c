/*
 Codigo del servidor del juego
 Editado por ultima vez: 5/10/2024
 Autores: Adria Sancho, Ivan del Rey, Arnau Torrent y Oscar Rigol
*/

#include "cabecera.h"

/*==============================================================================================================================*/
/*============================================ Declaracion de constantes y variables ===========================================*/

// Variables globales
int sock_conn, sock_listen, ret;												
struct sockaddr_in serv_adr;
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

// Funcion para cambiar la contraseña del usuario
void buscarUserAndChangePassBBDD(){
	conectarMySQL();
	entrarBD();
	int encontrado = 0;
	int err;
	err=mysql_query (conn, "SELECT * FROM Usuarios;");
	
	if (err!=0){
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result (conn);
	row = mysql_fetch_row (resultado);
	
	if (row == NULL){
		printf ("No se han obtenido datos en la consulta\n");
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
		// Usuario encontrado, procedemos a actualizar la contraseña
		char query[512];
		snprintf(query, sizeof(query), "UPDATE Usuarios SET contrasena='%s' WHERE usuario='%s';", contrasena, usuario);
		
		// Ejecutamos la consulta de actualización
		err = mysql_query(conn, query);
		if (err != 0){
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
	else {
		// Enviar "ERROR" al cliente si no son válidas
		char respuesta[] = "ERROR";
		printf("Usuario no encontrado, ERROR\n");
		write(sock_conn, respuesta, strlen(respuesta));
	}
	
	// Liberamos el resultado
	mysql_free_result(resultado);	
}

// Funcion para comprobar que el usuario que intenta iniciar sesion este registrado y sus credenciales sean correctas
void buscarUserAndPassBBDD(){
	conectarMySQL();
	entrarBD();
	int encontrado = 0;
	int err;
	err=mysql_query (conn, "SELECT * FROM Usuarios;");
	
	if (err!=0){
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result (conn);
	row = mysql_fetch_row (resultado);
	
	if (row == NULL){
		printf ("No se han obtenido datos en la consulta\n");
		return;
	}
	
	while (row !=NULL && encontrado==0){
		userDB = row[1];
		passDB = row[2];
		printf("Comparando Usuario: %s con %s y Contraseña: %s con %s\n", userDB, usuario, passDB, contrasena);
		if ((strcmp(userDB, usuario) ==0) && (strcmp(passDB,contrasena)==0)){
			encontrado = 1;
			printf("Usuario encontrado: %s\n", userDB);
		}
		row = mysql_fetch_row(resultado);
	}

	if (encontrado == 1) {
		// Enviar "OK" al cliente si las credenciales son válidas
		char respuesta[] = "OK";
		printf("OK\n");
		write(sock_conn, respuesta, strlen(respuesta));
	} 
	else {
		// Enviar "ERROR" al cliente si no son válidas
		char respuesta[] = "ERROR";
		printf("Usuario no encontrado, ERROR\n");
		write(sock_conn, respuesta, strlen(respuesta));
	}
	
	// Liberamos el resultado
	mysql_free_result(resultado);	
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
	
	// Insertamos el usuario y la contraseña predeterminados
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
	
	// Preparamos la consulta SQL para insertar el usuario y la contraseña
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
		// Enviar "OK" al cliente si las credenciales son válidas
		char respuesta[] = "OK";
		write(sock_conn, respuesta, strlen(respuesta));
	} 
	else {
		// Enviar "ERROR" al cliente si no son válidas
		char respuesta[] = "ERROR";
		printf("Usuario no encontrado, ERROR\n");
		write(sock_conn, respuesta, strlen(respuesta));
	}
}

// Funcion para contar cuantos usuarios hay en la base de datos
int contarUsuariosBBDD() {
	int err;
	int totalUsuarios = 0;
	
	// Realizamos la consulta SQL para contar el número total de filas en la tabla Usuarios
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
			printf("El número total de usuarios es: %d\n", totalUsuarios);
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
	
	// Extraemos el usuario y la contraseña del mensaje
	usuario = strtok(mensaje + 11, "/");									
	contrasena = strtok(NULL, "/");
	
	if (usuario && contrasena){
		printf("Usuario: %s, Contraseña: %s\n", usuario, contrasena);
		printf("Autenticando usuario: %s\n", usuario);
		buscarUserAndPassBBDD();
	}
}

// Mensaje para cambiar la contraseña del usuario
void mensaje_cambiarPass(char*mensaje){
	
	// Extraemos el usuario y la contraseña del mensaje
	usuario = strtok(mensaje + 13, "/");									
	contrasena = strtok(NULL, "/");
	
	if (usuario && contrasena){
		printf("Usuario: %s, Contraseña: %s\n", usuario, contrasena);
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

// Mensaje para añadir un usuario
void mensaje_addUser(char*mensaje){
	
	// Extraemos el usuario y la contraseña del mensaje
	usuario = strtok(mensaje + 9, "/");									
	contrasena = strtok(NULL, "/");
	
	if (usuario && contrasena){
		printf("Usuario: %s, Contraseña: %s\n", usuario, contrasena);
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
	// Mensaje para cambiar la contraseña del usuario
	else if (strncmp(mensaje, "CAMBIAR_PASS/", 13) == 0){
		mensaje_cambiarPass(mensaje);
	}
	// Mensaje para añadir un usuario
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

/*==============================================================================================================================*/
/*======================================== Funciones para la comunicacion con el cliente =======================================*/

// Funcion para inicializar el socket
void inicializarSocket(){
	
	// Si no podemos abrir el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0){				
		printf("Error creando socket");											
		exit(1);	
	}
	
	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(50000);
	
	if (bind(sock_listen, (struct sockaddr *)&serv_adr, sizeof(serv_adr)) < 0){
		printf("Error en el bind");
		exit(1);
	}
	
	// En caso de que la cola de peticiones pendientes sea mayor que 2
	if (listen(sock_listen, 2) < 0){						
		printf("Error en el listen");
	}
	printf("Servidor iniciado correctamente. Escuchando en puerto 50000\n");
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
	} 
	else {
		printf("Error al leer el mensaje del cliente.\n");
	}
}

// Funcion para establecer conexion con el cliente
void establecerConexion(){
	printf("Esperando conexion...\n");
	
	// A la escucha de connexiones while true, infinitamente
	while (1){
		sock_conn = accept(sock_listen, NULL, NULL);							
		if (sock_conn < 0) {
			printf("Error aceptando conexion.\n");
			
			// Seguimos escuchando aunque la connexion falle
			continue;  															
		}
		printf("Conexion aceptada.\n");
		obtener_mensajeCliente();												
		
		//Cerrar conexion tras recibir el mesaje, el servidor seguira escuchando
		//close(sock_conn); 						
		//printf("Conexion cerrada.\n");
	}
	close(sock_listen);  														
}

/*==============================================================================================================================*/
/*============================================================ MAIN ============================================================*/

int main(int argc, char *argv[]){
	
	inicializarSocket();
	establecerConexion();

	return 0;																	
}
