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

#define port 50073
#define mysql_direc "shiva2.upc.es"
#define mysql_user "root"
#define mysql_pass "mysql"
#define mysql_ddbb "T8_BBDDJuego"
#define maxConectados 100
#define maxPartidas 25

typedef struct {
	char nombre [20];
	int socket;
} Conectado;

typedef struct {
	Conectado conectados [maxConectados];
	int num;
} ListaConectados;

typedef struct {
	Conectado jugador [2];
	int id;
	int numj;
	int llena;
} Partida;

typedef struct {
	Partida partida [maxPartidas];
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
		if (miLista.conectados[i].socket == socket)
			return i;
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
void DameLista (char respuesta[512]){
	int i=0;
	if (miLista.num == 0)
		sprintf (respuesta, "-1");
	else {
		for (int i=0;i<miLista.num;i++)
			sprintf (respuesta, "%s, %s\n ", respuesta, miLista.conectados[i].nombre);
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
		err = mysql_query(conn, consulta);
		if (err!=0) {
			printf ("Error al introducir datos la base %u %s\n", mysql_errno(conn), mysql_error(conn));
			exit (1);
		}
		return id;
	}
	else 
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

int dameSocket(char nombre[20]){
	int i=0;
	while (i<miLista.num){
		if (strcmp(miLista.conectados[i].nombre,nombre)==0)
			return miLista.conectados[i].socket;
		i++;
	}
}

int InsertarPartida(char anfitrion[20], char invitados[100])
{
	int cont = 1;
	char jugador2[20];
	char jugador3[20];

	char *v = strtok(invitados, ",");

	if (v != NULL)
	{
		cont++;
		strcpy(jugador2, v);
		v = strtok(NULL, ",");
	}
	if (v != NULL)
	{
		cont++;
		strcpy(jugador3, v);
		v = strtok(NULL, ",");
	}
	for (int i = 0; i < maxPartidas; i++)
	{
		if (miTabla.partida[i].llena == 0)
		{
			strcpy(miTabla.partida[i].jugador[0].nombre, anfitrion);
			miTabla.partida[i].jugador[0].socket = dameSocket(anfitrion);
			
			if (cont > 1)
			{
				strcpy(miTabla.partida[i].jugador[1].nombre, jugador2);
				miTabla.partida[i].jugador[1].socket = dameSocket(jugador2);
			}
			if (cont > 2)
			{
				strcpy(miTabla.partida[i].jugador[2].nombre, jugador3);
				miTabla.partida[i].jugador[2].socket = dameSocket(jugador3);			
			}
			miTabla.partida[i].llena = 1;
			miTabla.partida[i].numj = cont;
			miTabla.partida[i].id = i;
			
			return i;
		}
	}
	return -1;
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

void Consulta1(int id, char respuesta[200]){
	char consulta[512];
	sprintf (consulta,"SELECT Jugadores.username FROM Jugadores,Participacion WHERE Participacion.Partida = %d ", id);
	strcat(consulta, "and Participacion.Jugador = Jugadores.Id and Jugadores.age<18;"); 
	HacerConsulta(consulta, respuesta);
}
void Consulta2(char nombre[20], char respuesta[200]){
	char consulta[512];
	sprintf (consulta,"SELECT Partidas.Ciudad FROM Jugadores,Participacion,Partidas WHERE Jugadores.username = '%s' ", nombre);
	strcat(consulta, "and Participacion.Jugador = Jugadores.Id and Partidas.Id = Participacion.Partida;"); 
	HacerConsulta(consulta, respuesta);
}
void Consulta3(char nombre[20], char respuesta[200]){
	char consulta[512];
	sprintf (consulta,"SELECT Jugadores.username FROM Jugadores,Participacion,Partidas WHERE Partidas.winner = '%s' and", nombre);
	strcat (consulta, " Participacion.Partida = Partidas.Id and Jugadores.Id = Participacion.Jugador and Jugadores.username!='");
	sprintf (consulta, "%s%s';",consulta, nombre);
	HacerConsulta(consulta, respuesta);
}


void hacerInvitacion(char nombre[20],char nombre_anf[20]){
	char respuesta[200];
	int sock_inv= dameSocket(nombre);
	sprintf(respuesta,"9/%s",nombre_anf);
	write(sock_inv, respuesta, strlen(respuesta));
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
	conn = mysql_real_connect (conn, "mysql_direc","mysql_user", "mysql_pass","mysql_ddbb", 0, NULL, 0);
	if (conn==NULL)
	{
		printf ("Error al inicializar la conexion: %u %s\n", mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
	int terminar =0;
	// Entramos en un bucle para atender todas las peticiones de este cliente hasta que se desconecte
	while (terminar ==0){
		char nombre[20];
		char passw[20];
		int edad, id;
		char conectado[20];
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
			int del = delLista(sock_conn);
			if (del == -1)
				printf("error en desconexion\n");
			else{
				printf("%s se ha desconectado\n", conectado);
				char noms[200];
				strcpy(noms, "6/");//codiigo lista conectados
				DameLista (noms);
				for(int j=0;j<miLista.num;j++)
					write (miLista.conectados[j].socket,noms, strlen(noms));
			}
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
			if (id == -1) //error
				sprintf(respuesta, "1/-1");
			else 
				sprintf (respuesta,"1/0/%d/%s",id,nombre);
		}
		else if (codigo ==2){ //iniciar sesion
			p = strtok( NULL, "/");
			strcpy (conectado, p);
			p = strtok( NULL, "/");
			strcpy(passw, p);
			int res = logIN(conectado, passw);
			if(res == 0){
				int add = addNameLista (conectado, sock_conn);
				if (add ==0){
					sprintf (respuesta, "2/0");
					printf("Anadid@. Socket de %s es %d\n", conectado, sock_conn);
					char noms[200];
					strcpy(noms, "6/");//codiigo lista conectados
					DameLista (noms);
					for(int j=0;j<miLista.num;j++)
						write (miLista.conectados[j].socket,noms, strlen(noms));
				}
			}
			else if(res == -1)
			   sprintf (respuesta, "2/-2"); //contra erronia
			else
				sprintf (respuesta, "2/-3"); //no existe user
		}
		else if (codigo == 3){ //consulta 1
			p = strtok(NULL, "/");
			id =  atoi (p);
			char noms[200];
			Consulta1(id, noms);
			sprintf(respuesta,"3/%s", noms);
		}
		else if (codigo == 4){ //consulta 2
			p = strtok(NULL, "/");
			strcpy(nombre, p);
			char noms[200];
			Consulta2(nombre, noms);
			sprintf(respuesta,"4/%s", noms);
		}
		else if (codigo == 5){ //consulta 3
			p = strtok(NULL, "/");
			strcpy(nombre, p);
			char noms[200];
			Consulta3(nombre, noms);
			sprintf(respuesta,"5/%s", noms);
		}
		else if (codigo == 8){ //lista de sockets
			char noms[200];
			strcpy(noms, "8/");
			DameListaSockets (noms);
			strcpy(respuesta, noms);
		}
		else if (codigo == 9) //Solicitud de invitacion
		{
			/*
			char invitados[100];
			char invitacion[20];
			p = strtok(NULL, "/");
			int num =  atoi (p);
			for(int j =0; j<num;j++){
				p = strtok( NULL, ",");
				strcpy(invitados,p);
				hacerInvitacion(invitados, conectado);
			}*/

			//esto es de aaron
			char invitados[100];
			p = strtok(NULL, "/");
			strcpy(invitados, p);
			
			int posp = InsertarPartida(conectado, invitados);
			
			if (posp == -1)
				printf("Error: No hay partidas disponibles\n");
			else
			{
				printf("Partida agregada. ID de partida: %d\n", posp);
				
				char invitacion[100];
				int numjugadores = miTabla.partida[posp].numj;
				
				sprintf(invitacion, "9/%d,%s,%s", posp, conectado, invitados);
				for (int i = 0; i < numjugadores; i++)
				{
					if (miTabla.partida[posp].jugador[i].socket != sock_conn)
						write(miTabla.partida[posp].jugador[i].socket, invitacion, strlen(invitacion));
				}
				printf("Invitaciones enviadas. Esperando confirmaciones...\n");
			}
		}

		/*else if(codigo==10){
		p = strtok( NULL, "/");
		
		if(strcmp("s",p)==0)
			
		
		}	*/	
			
			
		else
				 printf("No hay consulta");
		
		if ((codigo==1)||(codigo==2)||(codigo==3)||(codigo==4)||(codigo==5)||(codigo==8)){ // enviamos la respuesta al cliente
			printf ("Respuesta: %s\n", respuesta);
			write (sock_conn, respuesta, strlen(respuesta));
		}
		if ((codigo!=0)&&(codigo!=7)&&(codigo!=8)&&(codigo!=6)){
			pthread_mutex_lock(&mutex); //no interrumpir
			count++;//modificacion estructura compartida
			pthread_mutex_unlock(&mutex); //se puede interrumpir
			char noms[200];
			sprintf(noms, "7/%d", count);//codigo numero de peticiones
			for(int j=0;j<miLista.num;j++)
				write (miLista.conectados[j].socket, noms, strlen(noms));
		}
	}
	close(sock_conn); // Se acabo el servicio para este cliente
	if (miLista.num == 0)
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
	serv_adr.sin_port = htons(port);// establecemos el puerto de escucha
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind\n");
	
	if (listen(sock_listen, 10) < 0) //
		printf("Error en el Listen\n");
	count =0;
	int i=0;
	pthread_t thread[100];
	for (;;){	// 
		printf ("Escuchando\n");
		sock_conn = accept(sock_listen, NULL, NULL);//socket que usaremos para este cliente
		printf ("He recibido conexion sock %d\n", sock_conn);
		if (miLista.num==100)
			printf("lista llena sock\n");
		else{
			miLista.conectados[miLista.num].socket=sock_conn;
			miLista.num++;
			pthread_create (&thread[i], NULL, AtenderCliente, &miLista.conectados[miLista.num-1].socket);
			i++;
		}
	}
	//mysql_close (conn);
	//exit(0);
}
