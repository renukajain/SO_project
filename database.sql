DROP DATABASE IF EXISTS Juego;
CREATE DATABASE Juego;
USE Juego;

CREATE TABLE Jugadores (
	Id INTEGER PRIMARY KEY NOT NULL,
	username TEXT NOT NULL,
	psswrd TEXT NOT NULL,
	age INTEGER NOT NULL,
	scores INTEGER
)ENGINE = InnoDB;

CREATE TABLE Partidas (
	Id INTEGER PRIMARY KEY NOT NULL,
	Ciudad TEXT NOT NULL,
	fecha INTEGER NOT NULL,
	hora INTEGER NOT NULL,
	minutes FLOAT NOT NULL,
	winner TEXT NOT NULL
)ENGINE = InnoDB;

CREATE TABLE Participacion (
	Jugador INTEGER,
	Partida INTEGER,
	Posicion INTEGER NOT NULL,
	FOREIGN KEY (Jugador) REFERENCES Jugadores(Id),
	FOREIGN KEY (Partida) REFERENCES Partidas(Id),
PRIMARY KEY(Jugador, Partida)
)ENGINE = InnoDB;

INSERT INTO Jugadores(Id, username, psswrd, age, scores) VALUES(1,'Juan', 'pass1', 16, 1234);
INSERT INTO Jugadores(Id, username, psswrd, age, scores) VALUES(2,'Maria', 'pass2', 19, 5678);
INSERT INTO Jugadores(Id, username, psswrd, age, scores) VALUES(3,'Pedro', 'pass3', 3, 9012);
INSERT INTO Jugadores(Id, username, psswrd, age, scores) VALUES(4,'Luis', 'pass4', 40, 3456);
INSERT INTO Jugadores(Id, username, psswrd, age, scores) VALUES(5,'Julia', 'pass5', 50, 7890);

INSERT INTO Partidas(Id, Ciudad, fecha, hora, minutes, winner) VALUES(1,'Barcelona', 20200320, 1416, 11.03, 'Juan');
INSERT INTO Partidas(Id, Ciudad, fecha, hora, minutes, winner) VALUES(2,'Madrid', 20200319, 1630, 15.4, 'Luis');
INSERT INTO Partidas(Id, Ciudad, fecha, hora, minutes, winner) VALUES(3,'Sevilla', 10100318, 1320, 14.7, 'Maria');

INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(1,1,1);--
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(1,2,2);
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(2,2,3);
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(2,3,1);
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(3,2,4);
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(3,1,2);--
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(4,2,1);
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(4,3,2);
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(5,1,3);--
INSERT INTO Participacion(Jugador, Partida, Posicion) VALUES(5,3,3);