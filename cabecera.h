#ifndef CABECERA_H
#define CABECERA_H

#include <mysql.h>   // Para trabajar con MySQL
#include <string.h>  // Para manipular cadenas de caracteres
#include <stdlib.h>  // Para funciones generales de utilidad
#include <stdio.h>   // Para funciones de entrada/salida
#include <unistd.h>  // Para funciones POSIX (como `read`, `write`, etc.)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>  // Necesario para trabajar con hilos


// Declaración de variables globales con `extern`
extern char *usuario;
extern char *contrasena;

extern int sock_conn, sock_listen, ret;

// Declaración de las funciones que quieres usar en servidor.c
void conectarMySQL();
void entrarBD();
void buscarUserAndChangePassBBDD();
void crearAndBorrarBBDD();
void usarBBDD();
void crearTablaUsuarios();
int contarUsuariosBBDD();

#endif

