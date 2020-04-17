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

ListaConectados miLista;

typedef struct {
	int socket [100];
	int num;
} ListaSockets;

ListaSockets sockLista;

int ret;

MYSQL *conn;

char peticion[512];
char respuesta[512];
int count;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;//ACCESO EXLUYENTE

int addLista (char nombre[20], int socket){
	if (miLista.num == 100)
		return -1;
	else {
		pthread_mutex_lock(&mutex); 
		strcpy(miLista.conectados[miLista.num].nombre, nombre);
		miLista.conectados[miLista.num].socket = socket;
		miLista.num++;
		pthread_mutex_unlock(&mutex); 
		return 0;
	}
}
int addSock (int socket){
	if (sockLista.num == 100)
		return -1;
	else {
		pthread_mutex_lock(&mutex); 
		sockLista.socket[sockLista.num] = socket;
		sockLista.num++;
		pthread_mutex_unlock(&mutex); 
		return 0;
	}
}
int DamePos(char nombre[20]){
	int i=0;
	int found = 0;
	while (( i<miLista.num) && !found){
		if ( strcmp(miLista.conectados[i].nombre, nombre)==0)
			found = 1;
		if (!found)
			i++;
	}
	if (found)
		return i;
	else
		return -1;
}
int delLista (char nombre[20]){
	int pos = DamePos(nombre);
	if (pos == -1)
		return -1;
	else {
		int i;
		pthread_mutex_lock(&mutex);
		for(i=pos;i<miLista.num-1;i++){
			strcpy(miLista.conectados[i].nombre, miLista.conectados[i+1].nombre);
			miLista.conectados[i].socket = miLista.conectados[i+1].socket;
		}
		miLista.num--;
		pthread_mutex_unlock(&mutex); 
		return 0;
	}
}
int delSock (char nombre[20]){
	int numsock = DamePos(nombre);
	if (numsock ==-1)
		return -1;
	else{
		int i;
		for(i=numsock;i<sockLista.num-1;i++){
			sockLista.socket[i]=sockLista.socket[i+1];
		}
		sockLista.num--;
		return 0;
	}
}

int DameSocket (char nombre[20]){
	int i=0;
	int found = 0;
	while (( i<miLista.num) && !found){
		if ( strcmp(miLista.conectados[i].nombre, nombre)==0)
			found = 1;
		if (!found)
			i++;
	}
	if (found)
		return miLista.conectados[i].socket;
	else
		return -1;
}

void DameLista (char respuesta[512]){
	int i=0;
	if (miLista.num == 0)
		sprintf (respuesta, "no hay conectados");
	else {
		for (int i=0;i<miLista.num;i++)
			sprintf (respuesta, "%s%s, ", respuesta, miLista.conectados[i].nombre);
	}
}
void DameListaSock (char respuesta[512]){
	if (sockLista.num == 0)
		sprintf (respuesta, "no hay conectados");
	else {
		for (int i=0;i<sockLista.num;i++)
			sprintf (respuesta, "%s%d, ", respuesta, sockLista.socket[i]);
	}
}
int signIN(char nombre[20], char passw[20], int edad){
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	char consulta[512];
	int err = mysql_query(conn, "SELECT username FROM Jugadores");
	if (err!=0) {
		printf ("Error al consultar datos de la base %u %s\n",mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result(conn); 
	row = mysql_fetch_row(resultado);
	int id = 1;
	int found = 0;
	while (row != NULL){
		for(int i =0;i<strlen(nombre);i++)
			nombre[i] =toupper(nombre[i]);
		for(int i =0;i<strlen(row[0]);i++)
			row[0][i]= toupper(row[0][i]);
			if (strcmp(nombre, row[0]) == 0) //si coinsideix el nom amb user existent
				found = 1;
			id++; //incrementa contador
			row = mysql_fetch_row(resultado);
	}
	if (!found){//en cas que user no es troba a BBDD l'afegim
		sprintf(consulta,"INSERT INTO Jugadores VALUES (%d,'%s','%s', %d, 0);",id,nombre,passw,edad);
		puts(consulta);	
		pthread_mutex_lock(&mutex); 
		err = mysql_query(conn, consulta);
		pthread_mutex_unlock(&mutex); 
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
void Consulta1(int id, char respuesta[200]){
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	char consulta[512];
	sprintf (respuesta, " ");
	sprintf (consulta,"SELECT Jugadores.username FROM Jugadores,Participacion WHERE Participacion.Partida = %d ", id);
	strcat(consulta, "and Participacion.Jugador = Jugadores.Id and Jugadores.age<18;"); 
	int err=mysql_query (conn, consulta); 
	if (err!=0) {
		sprintf (respuesta, "Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result (conn); 
	row = mysql_fetch_row (resultado);
	if (row == NULL)
		sprintf (respuesta, "Error en consulta");
	else{
		while(row!=NULL){
			sprintf(respuesta, "%s%s, ", respuesta, row[0]);
			row = mysql_fetch_row (resultado);
		}
	}
}
void Consulta2(char nombre[20], char respuesta[200]){
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	char consulta[512];
	sprintf (respuesta, " ");
	sprintf (consulta,"SELECT Partidas.Ciudad FROM Jugadores,Participacion,Partidas WHERE Jugadores.username = '%s' ", nombre);
	strcat(consulta, "and Participacion.Jugador = Jugadores.Id and Partidas.Id = Participacion.Partida;"); 
	int err=mysql_query (conn, consulta); 
	if (err!=0) {
		sprintf (respuesta, "Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result (conn); 
	row = mysql_fetch_row (resultado);
	if (row == NULL)
		sprintf (respuesta, "Error en consulta");
	else{
		while(row!=NULL){
			sprintf(respuesta, "%s%s, ", respuesta, row[0]);
			row = mysql_fetch_row (resultado);
		}
	}
}
void Consulta3(char nombre[20], char respuesta[200]){
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	char consulta[512];
	sprintf (respuesta, " ");
	sprintf (consulta,"SELECT Jugadores.username FROM Jugadores,Participacion,Partidas WHERE Partidas.winner = '%s' and", nombre);
	strcat (consulta, " Participacion.Partida = Partidas.Id and Jugadores.Id = Participacion.Jugador and Jugadores.username!='");
	sprintf (consulta, "%s%s';",consulta, nombre);
	int err=mysql_query (conn, consulta); 
	if (err!=0) {
		sprintf (respuesta, "Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	resultado = mysql_store_result (conn); 
	row = mysql_fetch_row (resultado);
	if (row == NULL)
		sprintf (respuesta, "Error en consulta");
	else{
		while(row!=NULL){
			sprintf(respuesta, "%s%s, ", respuesta, row[0]);
			row = mysql_fetch_row (resultado);
		}
	}
}

void *AtenderCliente(void *socket){
	int sock_conn;
	int *s;
	s=(int *) socket;
	sock_conn = *s;
	
	//Creamos una conexion al servidor MYSQL 
	conn = mysql_init(NULL);
	if (conn==NULL) {
		printf ("Error al crear la conexion: %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	conn = mysql_real_connect (conn, "localhost","root", "mysql","Juego", 0, NULL, 0);
	if (conn==NULL)
	{
		printf ("Error al inicializar la conexion: %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	int terminar =0;
	// Entramos en un bucle para atender todas las peticiones de este cliente hasta que se desconecte
	char conectado[20];
	while (terminar ==0){
		char nombre[20];
		char passw[20];
		
		int edad, id;
		// Ahora recibimos la petici?n
		ret=read(sock_conn,peticion, sizeof(peticion));
		printf ("Recibido\n");
		
		// Tenemos que a?adirle la marca de fin de string 
		// para que no escriba lo que hay despues en el buffer
		peticion[ret]='\0';
		printf ("Peticion: %s\n",peticion);
		
		char *p = strtok( peticion, "/");
		int codigo =  atoi (p); //obtenemos codigo del servicio
		printf("codigo %d\n",codigo);
		if (codigo ==0){ //petici?n de desconexi?n
			terminar=1;
			int del = delLista(conectado);
			int delS = delSock(conectado);
			if (del == -1)
				printf("error en desconexion\n");
			else
				printf("%s se ha desconectado\n", conectado);
		}
		else if(codigo ==1){ //darse de alta
			p = strtok( NULL, "/");
			strcpy (nombre, p); 
			p = strtok( NULL, "/");
			strcpy(passw,p);
			p = strtok( NULL, "/");
			edad = atoi(p);
			//Miramos cuantos usuarios hay en la BBDD
			id = signIN(nombre, passw, edad);
			if (id == -1)
				strcpy(respuesta, "Existe usuario con este nombre");
			else 
				sprintf (respuesta,"Bienvenido %s, estas dado de alta con id: %d\n",nombre,id);
		}
		else if (codigo ==2){ //iniciar sesion
			p = strtok( NULL, "/");
			strcpy (nombre, p);
			p = strtok( NULL, "/");
			strcpy(passw, p);
			int res = logIN(nombre, passw);
			if(res == 0){
				strcpy (respuesta, "ﾡBienvenod@!");
				strcpy(conectado, nombre);
				int add = addLista (nombre, sockLista.socket[sockLista.num-1]);
				if (add == -1)
					printf("lista llena\n");
				else
					printf("Anadid@. Socket de %s es %d\n", nombre, DameSocket (nombre));
			}
			else if(res == -1)
			   strcpy (respuesta,"id o contrasenya erronia\n");
			else
				strcpy (respuesta,"No existe usuario\n");
		}
		else if (codigo == 3){ //consulta 1
			char noms[200];
			p = strtok(NULL, "/");
			id =  atoi (p);
			Consulta1(id, noms);
			strcpy(respuesta, noms);
		}
		else if (codigo == 4){ //consulta 2
			char noms[200];
			p = strtok(NULL, "/");
			strcpy(nombre, p);
			Consulta2(nombre, noms);
			strcpy(respuesta, noms);
		}
		else if (codigo == 5){ //consulta 3
			char noms[200];
			p = strtok(NULL, "/");
			strcpy(nombre, p);
			Consulta3(nombre, noms);
			strcpy(respuesta, noms);
		}
		else if (codigo == 6){ //lista de conectados
			char noms[200];
			strcpy(noms, " ");
			DameLista (noms);
			strcpy(respuesta, noms);
		}
		else if (codigo ==7){ //numero de peticiones
			sprintf(respuesta, "%d",count); 
		}
		else if (codigo == 8){ //lista de sockets
			char noms[200];
			strcpy(noms, " ");
			DameListaSock (noms);
			strcpy(respuesta, noms);
		}
		else
				 printf("no hay consulta");
		
		if (codigo !=0){ // enviamos la respuesta al cliente
			printf ("Respuesta: %s\n", respuesta);
			write (sock_conn,respuesta, strlen(respuesta));
		}
		if ((codigo!=0)&&(codigo!=7)){
			pthread_mutex_lock(&mutex); //no interrumpir
			count++;//modificacion estructura compartida
			pthread_mutex_unlock(&mutex); //se puede interrumpir
		}
	}
	close(sock_conn); // Se acabo el servicio para este cliente
	mysql_close (conn);
}

int main(int argc, char *argv[]){ 
	miLista.num =0;
	struct sockaddr_in serv_adr;
	int sock_conn, sock_listen;
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket\n");
	// Fem el bind al port
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(9050);// establecemos el puerto de escucha
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind\n");
	
	if (listen(sock_listen, 10) < 0) //
		printf("Error en el Listen\n");
	count =0;
	int i=0;
	sockLista.num=0;
	pthread_t thread[100];
	for (;;){	// 
		printf ("Escuchando\n");
		sock_conn = accept(sock_listen, NULL, NULL);//socket que usaremos para este cliente
		printf ("He recibido conexion sock %d\n", sock_conn);
		int sock = addSock(sock_conn);
		if (sock==-1)
			printf("lista llena sock\n");
		else
			pthread_create (&thread[sockLista.num-1], NULL, AtenderCliente, &sockLista.socket[sockLista.num-1]);
		i++;
	}
	exit(0);
}