#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>
#include <pthread.h>
#include <ctype.h>

typedef struct {
	char nombre [20];
	int socket;
} Conectado;

typedef struct {
	Conectado conectados [100];
	int num;
} ListaConectados;

typedef struct {
	Conectado jugador [10]; //invitados a partida (el primero es el creadorde la partida)
	int id; //identifiador partida
	int numj; //numero de jugadores
	int accept; //para comprobar si todos han aceptado
	int matriz[11][11];
} Partida;

typedef struct {
	Partida partida [100];
	int nump;
} ListaPartidas;

ListaConectados miLista;
ListaPartidas miTabla;

MYSQL *conn;

char peticion[512];
char respuesta[512];
int count, ret;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//ACCESO EXLUYENTE

int DamePos(int socket){
	int i=0;
	while (i<miLista.num){
		if (miLista.conectados[i].socket ==socket)
			return i;
		i++;
	}
	return -1;
}
int damePosTabla(int id){
	int i=0;
	while (i<miTabla.nump){
		if (miTabla.partida[i].id ==id)
			return i;
		i++;
	}
	return -1;
}
//int dameFicha(int idP, char nom){
//	int i=0;
/*	while (i<miTabla.partida[idP].numj){*/
/*		if (strcmp(miTabla.partida[idP].jugador[i].nombre, nom) ==0)*/
/*			return i;*/
/*		i++;*/
/*	}*/
/*	return -1;*/
/*}*/
int DameSock(char nombre[200]){
	int i=0;
	while (i<miLista.num){
		if (strcmp(miLista.conectados[i].nombre, nombre) ==0)
			return miLista.conectados[i].socket;
		i++;
	}
	return -1;
}
int addNameLista (char nombre[20], int socket){
	int pos =DamePos(socket);
	if (pos==-1)
		return -1;
	else{
		pthread_mutex_lock(&mutex); 
		strcpy(miLista.conectados[pos].nombre, nombre);
		pthread_mutex_unlock(&mutex);
		return 0;
	}
}

int delLista (int socket){
	int pos = DamePos(socket);
	if (pos == -1)
		return -1;
	pthread_mutex_lock(&mutex);
	for(int i=pos;i<miLista.num-1;i++){
		strcpy(miLista.conectados[i].nombre, miLista.conectados[i+1].nombre);
		miLista.conectados[i].socket = miLista.conectados[i+1].socket;
	}
	miLista.num--;
	pthread_mutex_unlock(&mutex); 
	return 0;
}
int delPartida (int id){
	int pos = damePosTabla(id);
	if (pos == -1)
		return -1;
	pthread_mutex_lock(&mutex);
	for(int i=pos;i<miTabla.nump-1;i++)
		miTabla.partida[i] = miTabla.partida[i+1];
	miTabla.nump--;
	pthread_mutex_unlock(&mutex); 
	return 0;
}
void DameLista (char respuesta[512]){
	int i=0;
	if (miLista.num == 0)
		sprintf (respuesta, "-1");
	else {
		for (int i=0;i<miLista.num;i++)
			sprintf (respuesta, "%s%s\n ", respuesta, miLista.conectados[i].nombre);
	}
}
void DameListaSockets (char respuesta[512]){
	int i=0;
	if (miLista.num == 0)
		sprintf (respuesta, "-1");
	else {
		for (int i=0;i<miLista.num;i++)
			sprintf (respuesta, "%s%d, ", respuesta, miLista.conectados[i].socket);
	}
}
int signIN(char nombre[20], char passw[20], int edad){
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	char consulta[512];
	int gap = 0;
	int err = mysql_query(conn, "SELECT username,id FROM Jugadores");
	if (err!=0) {
		printf ("Error al consultar datos de la base %u %s\n",mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result(conn); 
	row = mysql_fetch_row(resultado);
	int id = 1;
	int found = 0;
	while ((row != NULL)&&(gap==0)&&(found==0)){
		for(int i =0;i<strlen(nombre);nombre[i] =toupper(nombre[i]), i++);//pasamos todo a mayusculas
		for(int i =0;i<strlen(row[0]);row[0][i]= toupper(row[0][i]), i++);
		if (strcmp(nombre, row[0]) == 0) //si coinsideix el nom amb user existent
			found = 1;
		if (id != atoi(row[1]))
			gap = 1;
		else 
			id++; //incrementa contador
		row = mysql_fetch_row(resultado);
	}
	if (!found){//en cas que user no es troba a BBDD l'afegim
		sprintf(consulta,"INSERT INTO Jugadores VALUES (%d,'%s','%s', %d, 0);",id,nombre,passw,edad);
		puts(consulta);	
		err = mysql_query(conn, consulta);
		if (err!=0) {
			printf ("Error al introducir datos la base %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
		return id;
	}else 
		return -1;
}
int logIN(char nombre[20], char passw[20]){
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	char consulta[512];
	sprintf (consulta,"SELECT psswrd FROM Jugadores WHERE username = '%s'", nombre); 
	int err=mysql_query (conn, consulta); 
	if (err!=0) {
		sprintf (respuesta, "Error al consultar datos de la base %u %s\n",mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result (conn); 
	row = mysql_fetch_row (resultado);
	if (row == NULL)
		return -2; // no existe user 
	else{
		if(strcmp(row[0],passw)!=0){ //no coinsideix contrasenya
			return -1;
		}else
		   return 0; 
	}
}

int unsubscribe(char nombre[20]){
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	char consulta[512];
	sprintf (consulta,"DELETE FROM Jugadores WHERE Jugadores.username = '%s';", nombre); 
	int err=mysql_query (conn, consulta); 
	if (err!=0) {
		return -1;
		sprintf (respuesta, "Error al consultar datos de la base %u %s\n",mysql_errno(conn), mysql_error(conn));
		exit (1);
	}else
		return 0;
}

void HacerConsulta(char consulta[512], char respuesta[200]){
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	int err=mysql_query (conn, consulta); 
	if (err!=0) {
		printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
		strcpy (respuesta, "-2"); // error en consulta
		exit (1);
	}
	resultado = mysql_store_result (conn); 
	row = mysql_fetch_row (resultado);
	if (row == NULL)
		strcpy (respuesta, "-1"); //no hay resultados 
	else{
		strcpy (respuesta, " ");
		while(row!=NULL){
			sprintf(respuesta, "%s%s, ", respuesta, row[0]);
			row = mysql_fetch_row (resultado);
		}
	}
}