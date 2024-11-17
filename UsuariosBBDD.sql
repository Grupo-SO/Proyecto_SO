DROP DATABASE IF EXIST Usuarios;
CREATE DATABASE M3_Usuarios;

USE M3_Usuarios;

CREATE TABLE Usuarios (
	id int primary key not null auto_increment, 
	usuario TEXT not null, 
	contrasena TEXT not null
)ENGINE=InnoDB;

INSERT INTO Usuarios VALUES (4896, 'Ivan', 'madrid');
INSERT INTO Usuarios VALUES (3712, 'Oscar', 'zaragoza');
INSERT INTO Usuarios VALUES (7733, 'Adri', 'barcelona');
INSERT INTO Usuarios VALUES (0691, 'Arnau', 'bilbao');
