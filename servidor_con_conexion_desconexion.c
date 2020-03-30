#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <mysql.h>



int main(int argc, char *argv[])
{ 
	int sock_conn, sock_listen, ret;
	MYSQL_RES *resultado;
	MYSQL_ROW row;
	
	MYSQL_RES *resultado2;
	MYSQL_ROW row2;
	
	MYSQL_RES *resultado3;
	MYSQL_ROW row3;
	
	struct sockaddr_in serv_adr;
	char peticion[512];
	char respuesta[512];
	
	MYSQL *conn;
	int err;
	//Creamos una conexion al servidor MYSQL 
	conn = mysql_init(NULL);
	if (conn==NULL) {
		printf ("Error al crear la conexion: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	conn = mysql_real_connect (conn, "localhost","root", "mysql","Juego", 0, NULL, 0);
	if (conn==NULL)
	{
		printf ("Error al inicializar la conexion: %u %s\n", 
				mysql_errno(conn), mysql_error(conn));
		exit (1);
	}
	
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
	// establecemos el puerto de escucha
	serv_adr.sin_port = htons(9000);
	if (bind(sock_listen, (struct sockaddr *) &serv_adr, sizeof(serv_adr)) < 0)
		printf ("Error al bind");
	
	if (listen(sock_listen, 10) < 0)
		printf("Error en el Listen");
	
	// Bucle infinito
	for (;;){
		printf ("Escuchando\n");
		
		sock_conn = accept(sock_listen, NULL, NULL);
		printf ("He recibido conexion\n");
		//sock_conn es el socket que usaremos para este cliente
		
		int terminar =0;
		// Entramos en un bucle para atender todas las peticiones de este cliente
		//hasta que se desconecte
		
		while (terminar ==0){
			char nombre[20];
			char passw[20];
			int edad;
			char consulta[200];
			char pet[200];
			int id;
			char noms[80];
			// Ahora recibimos la petici?n
			ret=read(sock_conn,peticion, sizeof(peticion));
			printf ("Recibido\n");
			
			// Tenemos que a?adirle la marca de fin de string 
			// para que no escriba lo que hay despues en el buffer
			peticion[ret]='\0';
			
			printf ("Peticion: %s\n",peticion);
			
			// vamos a ver que quieren
			char *p = strtok( peticion, "/");
			
			int codigo =  atoi (p);
			// Ya tenemos el c?digo de la petici?n
			printf("codigo %d\n",codigo);
			if (codigo ==0){ //petici?n de desconexi?n
				terminar=1;
			}
			else if(codigo ==1){	//peticion de darse de alta
				
				p = strtok( NULL, "/");
				strcpy (nombre, p); 
				p = strtok( NULL, "/");
				strcpy(passw,p);
				p = strtok( NULL, "/");
				edad = atoi(p);
				//Miramos cuantos usuarios hay en la BBDD
				sprintf(consulta,"SELECT username FROM Jugadores");
				err = mysql_query(conn, consulta);
				if (err!=0) {
					printf ("Error al consultar datos de la base %u %s\n",
							mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				
				resultado = mysql_store_result(conn); 
				row = mysql_fetch_row(resultado);
				id = 1;
				while (row != NULL){
					if (strcmp(nombre, row[0]) == 0){ //si coinsideix el nom amb user existent
						sprintf(respuesta, "ER");
					}
					id++;
					row = mysql_fetch_row(resultado);
				}
				
				if (strcmp(respuesta, "ER") != 0)//en cas que user no es troba a BBDD l'afegim
				{ 
					char consulta[200];
					strcpy (consulta, "INSERT INTO Jugadores VALUES (");
					sprintf(pet,"%s%d,'%s','%s', %d, 0);",consulta,id,nombre,passw,edad);
					printf("Nombre : %s , Passw: %s , id: %d\nConsulta: %s\n",nombre,passw,id,pet);						
					err = mysql_query(conn, pet);
					if (err!=0) {
						printf ("Error al introducir datos la base %u %s\n", 
								mysql_errno(conn), mysql_error(conn));
						exit (1);
					}
					sprintf (respuesta,"Bienvenido %s, estas dado de alta con id: %d\n",nombre,id);
				}
			}
			else if (codigo ==2){// quiere loguearse
				p = strtok( NULL, "/");
				strcpy (nombre, p); //Tenemos la contraseña
				p = strtok( NULL, "/");
				strcpy(passw, p);
				sprintf (consulta,"SELECT psswrd FROM Jugadores WHERE username = '%s'", nombre); 
				// hacemos la consulta 
				err=mysql_query (conn, consulta); 
				if (err!=0) {
					sprintf (respuesta, "Error al consultar datos de la base %u %s\n",
							mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				resultado = mysql_store_result (conn); 
				row = mysql_fetch_row (resultado);
				if (row == NULL)
					sprintf (respuesta,"No existe usuario\n");
				else
				{
					if(strcmp(row[0],passw)==0)
						strcpy (respuesta,"Bienvenido!\n");
					else
						strcpy (respuesta,"id o contraseña erronia\n");
				}
			}
			else if (codigo == 3){ //consulta 1
				p = strtok(NULL, "/");
				id =  atoi (p);
				strcpy(noms, " ");
				sprintf (consulta,"SELECT Jugador FROM Participacion WHERE Partida = %d;", id); 
				err=mysql_query (conn, consulta); 
				if (err!=0) {
					sprintf (respuesta, "Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				//recogemos el resultado de la consulta 
				resultado = mysql_store_result (conn); 
				row = mysql_fetch_row (resultado);
				if (row == NULL)
					sprintf (respuesta, "No exsite esa partida\n");
				else{
					while(row!=NULL){
						sprintf (consulta, "SELECT username FROM Jugadores WHERE age < 18 && Id = %s;", row[0]);
						err = mysql_query (conn, consulta);
						if (err!=0) {
							printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
							exit (1);
						}
						resultado2 = mysql_store_result (conn);
						row2 = mysql_fetch_row (resultado2);
						if (row2 != NULL){
							sprintf(noms, "%s%s, ", noms, row2[0]);
						}
						row = mysql_fetch_row (resultado);
					}
					if (strcmp(noms, " ")== 0)
						sprintf(respuesta, "no hay jugadores menores de edad");
					else 
						sprintf (respuesta, "Nombre de la persona: %s", noms );		
				}
			}
			
			else if (codigo == 4){ //consulta 2
				p = strtok(NULL, "/");
				strcpy(nombre, p);
				strcpy(noms, " ");
				sprintf (consulta,"SELECT Id FROM Jugadores WHERE username = '%s';", nombre); 
				err=mysql_query (conn, consulta); 
				if (err!=0) {
					sprintf (respuesta, "Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				//recogemos el resultado de la consulta 
				resultado = mysql_store_result (conn); 
				row = mysql_fetch_row (resultado);
				if (row == NULL)
					sprintf (respuesta, "No existe este usuario\n");
				else{
					sprintf (consulta,"SELECT Partida FROM Participacion WHERE Jugador = %s;", row[0]); 
					err=mysql_query (conn, consulta); 
					if (err!=0) {
						sprintf (respuesta, "Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
						exit (1);
					}
					//recogemos el resultado de la consulta 
					resultado = mysql_store_result (conn); 
					row = mysql_fetch_row (resultado);
					if (row == NULL)
						sprintf (respuesta, "No hay partidas jugadas por este usuario\n");
					else{
						while(row!=NULL){//tabla con id de las partidas 
							sprintf (consulta, "SELECT Ciudad FROM Partidas WHERE Id = %s;", row[0]);
							err = mysql_query (conn, consulta);
							if (err!=0) {
								printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
								exit (1);
							}
							resultado2 = mysql_store_result (conn);
							row2 = mysql_fetch_row (resultado2);
							if (row2 != NULL){
								sprintf(noms, "%s%s, ", noms, row2[0]);
							}
							row = mysql_fetch_row (resultado);
						}
						sprintf (respuesta, "Nombre de las ciudades: %s", noms );	
					}
					
				}
			}
			
			else if (codigo == 5){ //consulta 2
				p = strtok(NULL, "/");
				strcpy(nombre, p);
				strcpy(noms, " ");
				sprintf (consulta,"SELECT Id FROM Jugadores WHERE username = '%s';", nombre); 
				err=mysql_query (conn, consulta); 
				if (err!=0) {
					sprintf (respuesta, "Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
					exit (1);
				}
				//recogemos el resultado de la consulta 
				resultado = mysql_store_result (conn); 
				row = mysql_fetch_row (resultado);
				if (row == NULL)
					sprintf (respuesta, "No existe este usuario\n");
				else{
					id = atoi(row[0]);
					sprintf (consulta,"SELECT Id FROM Partidas WHERE winner = %s;", row[0]); 
					err=mysql_query (conn, consulta); 
					if (err!=0) {
						sprintf (respuesta, "Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
						exit (1);
					}
					//recogemos el resultado de la consulta 
					resultado = mysql_store_result (conn); 
					row = mysql_fetch_row (resultado);
					if (row == NULL)
						sprintf (respuesta, "No hay partidas ganadas por este usuario\n");
					else{
						while(row!=NULL){//tabla con id de las partidas 
							strcat(noms,"Partida");
							strcat(noms, row[0]);
							strcat(noms, ": ");
							sprintf (consulta, "SELECT Jugador FROM Participacion WHERE Partida = %s;", row[0]);
							err = mysql_query (conn, consulta);
							if (err!=0) {
								printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
								exit (1);
							}
							resultado2 = mysql_store_result (conn);
							row2 = mysql_fetch_row (resultado2);//jugadores id
							while (row2 != NULL){
								int id2 = atoi(row2[0]);
								if (id != id2){
									sprintf (consulta, "SELECT username FROM Jugadores WHERE Id = %s ;", row2[0]);
									err = mysql_query (conn, consulta);
									if (err!=0) {
										printf ("Error al consultar datos de la base %u %s\n", mysql_errno(conn), mysql_error(conn));
										exit (1);
									}
									resultado3 = mysql_store_result (conn);
									row3 = mysql_fetch_row (resultado3);
									sprintf(noms, "%s%s, ", noms, row3[0]);
								}
								row2 = mysql_fetch_row (resultado2);
							}
							row = mysql_fetch_row (resultado);
						}
						sprintf (respuesta, "Nombre de los jugadores: %s", noms );	
					}
					
				}
			}
			
			else{
				printf("no hay consulta");
			}
			
			if (codigo !=0){ // enviamos la respuesta al cliente
				printf ("Respuesta: %s\n", respuesta);
				// Enviamos respuesta
				write (sock_conn,respuesta, strlen(respuesta));
			}
		}
		mysql_close (conn);
		exit(0);
		// Se acabo el servicio para este cliente
		close(sock_conn); 
	}
}