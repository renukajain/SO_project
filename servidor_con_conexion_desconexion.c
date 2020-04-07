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

int addLista (ListaConectados *lista, char nombre[20], int socket){
	if (lista->num == 100)
		return -1;
	else {
		strcpy(lista->conectados[lista->num].nombre, nombre);
		lista->conectados[lista->num].socket = socket;
		lista->num++;
		return 0;
	}
}

int DameSocket (ListaConectados *lista, char nombre[20]){
	int i=0;
	int found = 0;
	while (( i<lista->num) && !found){
		if ( strcmp(lista->conectados[i].nombre, nombre)==0)
			found = 1;
		if (!found)
			i++;
	}
	if (found)
		return lista->conectados[i].socket;
	else
		return -1;
}

void DameLista (ListaConectados *lista, char respuesta[512]){
	int i=0;
	if (lista->num == 0)
		sprintf (respuesta, "no hay conectados");
	else {
		strcpy (respuesta, "lista de conectados: ");
		while (i<lista->num){
			sprintf (respuesta, "%s%s, ", respuesta, lista->conectados[i].nombre);
			printf ("conectado %d %s\n", i+1, lista->conectados[i].nombre);
			i++;
		}
	}
}

void HacerConsulta(char consulta[200], char respuesta[200], MYSQL_RES *resultado, MYSQL_ROW row, MYSQL *conn){
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
	ListaConectados miLista;
	miLista.num =0;
	
	int sock_conn;
	int *s;
	s=(int *) socket;
	sock_conn = *s;
	
	int ret;
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	
	char peticion[512];
	char respuesta[512];
	
	MYSQL *conn;
	int err;
	
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
	
	while (terminar ==0){
		char nombre[20];
		char passw[20];
		int edad;
		char consulta[200];
		char pet[200];
		int id;
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
		if (codigo ==0) //petici?n de desconexi?n
			terminar=1;
		else if(codigo ==1){	//peticion de darse de alta
			p = strtok( NULL, "/");
			strcpy (nombre, p); 
			p = strtok( NULL, "/");
			strcpy(passw,p);
			p = strtok( NULL, "/");
			edad = atoi(p);
			//Miramos cuantos usuarios hay en la BBDD
			err = mysql_query(conn, "SELECT username FROM Jugadores");
			if (err!=0) {
				printf ("Error al consultar datos de la base %u %s\n",mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			resultado = mysql_store_result(conn); 
			row = mysql_fetch_row(resultado);
			id = 1;
			int found = 0;
			while (row != NULL){
				for(int i =0;i<strlen(nombre);i++)
					nombre[i] =toupper(nombre[i]);
				for(int i =0;i<strlen(row[0]);i++)
					row[0][i]= toupper(row[0][i]);
				if (strcmp(nombre, row[0]) == 0) //si coinsideix el nom amb user existent
					found = 1;
				id++;
				row = mysql_fetch_row(resultado);
			}
			if (!found){//en cas que user no es troba a BBDD l'afegim
				char consulta[200];
				strcpy (consulta, "INSERT INTO Jugadores VALUES (");
				sprintf(pet,"%s%d,'%s','%s', %d, 0);",consulta,id,nombre,passw,edad);
				printf("Nombre : %s , Passw: %s , id: %d\nConsulta: %s\n",nombre,passw,id,pet);						
				err = mysql_query(conn, pet);
				if (err!=0) {
					printf ("Error al introducir datos la base %u %s\n", mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				sprintf (respuesta,"Bienvenido %s, estas dado de alta con id: %d\n",nombre,id);
			}else 
				sprintf (respuesta, "Existe usuario con este nombre");
		}
		else if (codigo ==2){// iniciar sesion
			p = strtok( NULL, "/");
			strcpy (nombre, p); //Tenemos la contraseￃﾱa
			p = strtok( NULL, "/");
			strcpy(passw, p);
			sprintf (consulta,"SELECT psswrd FROM Jugadores WHERE username = '%s'", nombre); 
			err=mysql_query (conn, consulta); 
			if (err!=0) {
				sprintf (respuesta, "Error al consultar datos de la base %u %s\n",mysql_errno(conn), mysql_error(conn));
				exit (1);
			}
			resultado = mysql_store_result (conn); 
			row = mysql_fetch_row (resultado);
			if (row == NULL)
				sprintf (respuesta,"No existe usuario\n");
			else{
				if(strcmp(row[0],passw)==0){
					strcpy (respuesta,"Bienvenido!\n");
					int add = addLista (&miLista, nombre, sock_conn);
					if (add == -1)
						printf("lista llena\n");
					else 
						printf("a￱adido\n");
					
					int socket = DameSocket (&miLista, nombre);
					if (socket != -1)
						printf("Socket de %s es %d\n", nombre, socket);
					else 
						printf("user no esta en la lista\n");
				}else
				   strcpy (respuesta,"id o contraseￃﾱa erronia\n");
			}
		}
		else if (codigo == 3){ //consulta 1
			p = strtok(NULL, "/");
			id =  atoi (p);
			sprintf (respuesta, "Nombre de los jugadores menores: ");
			sprintf (consulta,"SELECT Jugadores.username FROM Jugadores,Participacion WHERE Participacion.Partida = %d ", id);
			strcat(consulta, "and Participacion.Jugador = Jugadores.Id and Jugadores.age<18;"); 
			HacerConsulta(consulta, respuesta, resultado, row, conn);
		}
		else if (codigo == 4){ //consulta 2
			p = strtok(NULL, "/");
			strcpy(nombre, p);
			sprintf (respuesta, "Nombre de las ciudades: ");
			sprintf (consulta,"SELECT Partidas.Ciudad FROM Jugadores,Participacion,Partidas WHERE Jugadores.username = '%s' ", nombre);
			strcat(consulta, "and Participacion.Jugador = Jugadores.Id and Partidas.Id = Participacion.Partida;"); 
			HacerConsulta(consulta, respuesta, resultado, row, conn);
		}
		else if (codigo == 5){ //consulta 3
			p = strtok(NULL, "/");
			strcpy(nombre, p);
			sprintf (respuesta, "Nombre de los jugadores que han perdido: ");
			sprintf (consulta,"SELECT Jugadores.username FROM Jugadores,Participacion,Partidas WHERE Partidas.winner = '%s' and", nombre);
			strcat (consulta, " Participacion.Partida = Partidas.Id and Jugadores.Id = Participacion.Jugador and Jugadores.username!='");
			sprintf (consulta, "%s%s';",consulta, nombre); 
			HacerConsulta(consulta, respuesta, resultado, row, conn);
		}
		else if (codigo == 6){//lista de conectados
			DameLista (&miLista, respuesta);
		}
		else
				 printf("no hay consulta");
		
		if (codigo !=0){ // enviamos la respuesta al cliente
			printf ("Respuesta: %s\n", respuesta);
			write (sock_conn,respuesta, strlen(respuesta));
		}
	}
	close(sock_conn); // Se acabo el servicio para este cliente
	mysql_close (conn);
}

int main(int argc, char *argv[]){ 
	struct sockaddr_in serv_adr;
	int sock_conn, sock_listen;
	// INICIALITZACIONS
	// Obrim el socket
	if ((sock_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		printf("Error creant socket");
	// Fem el bind al port
	memset(&serv_adr, 0, sizeof(serv_adr));// inicialitza a zero serv_addr
	serv_adr.sin_family = AF_INET;
	
	// asocia el socket a cualquiera de las IP de la m?quina. 
	//htonl formatea el numero que recibe al formato necesario
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(9000);// establecemos el puerto de escucha
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	
	if (listen(sock_listen, 10) < 0)
		printf("Error en el Listen");
	int i;
	int sockets[100];
	pthread_t thread[100];
	for (i=0;i<5;i++){	// Bucle infinito
		printf ("Escuchando\n");
		sock_conn = accept(sock_listen, NULL, NULL);//socket que usaremos para este cliente
		printf ("He recibido conexion\n");
		sockets[i] = sock_conn;
		pthread_create (&thread[i], NULL, AtenderCliente, &sockets[i]);
		
	}
	exit(0);
}