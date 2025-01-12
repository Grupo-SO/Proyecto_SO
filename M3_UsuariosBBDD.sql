DROP DATABASE IF EXIST UsuariosBBDD;
CREATE DATABASE M3_UsuariosBBDD;

USE M3_UsuariosBBDD;

CREATE TABLE Usuarios (
	id int primary key not null auto_increment, 
	usuario TEXT not null, 
	contrasena TEXT not null
)ENGINE=InnoDB;

INSERT INTO Usuarios VALUES (4896, 'Ivan', 'madrid');
INSERT INTO Usuarios VALUES (3712, 'Oscar', 'zaragoza');
INSERT INTO Usuarios VALUES (7733, 'Adri', 'barcelona');
INSERT INTO Usuarios VALUES (0691, 'Arnau', 'bilbao');

CREATE TABLE partidas (
    id INT AUTO_INCREMENT PRIMARY KEY,
    fecha DATETIME NOT NULL
);

-- Tabla de jugadores
CREATE TABLE jugadores (
    id INT AUTO_INCREMENT PRIMARY KEY,
    nombre VARCHAR(100) NOT NULL
);

-- Tabla intermedia partidas_jugadores
CREATE TABLE partidas_jugadores (
    partida_id INT NOT NULL,
    jugador_id INT NOT NULL,
    puntuacion INT NOT NULL,
    PRIMARY KEY (partida_id, jugador_id),
    FOREIGN KEY (partida_id) REFERENCES partidas(id) ON DELETE CASCADE,
    FOREIGN KEY (jugador_id) REFERENCES jugadores(id) ON DELETE CASCADE
);