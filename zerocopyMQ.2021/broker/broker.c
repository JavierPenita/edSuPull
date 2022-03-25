#include <stdio.h>
#include "comun.h"
#include "diccionario.h"
#include "cola.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/select.h>

//en el main vamos a utilizar el ejemplo de eco_serv_tcp.c no usamos la parte "leido" ya que trateremos el tamaño luego
int main(int argc, char *argv[]){



    int s, s_conec;
	unsigned int tam_dir;
	struct sockaddr_in dir, dir_cliente;
	int opcion=1;
	
	if(argc!=2) {
        fprintf(stderr, "Uso: %s puerto\n", argv[0]);
        return 1;
    }
	if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("error creando socket");
		return 1;
	}
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion))<0){
                perror("error en setsockopt");
                return 1;
    }
	dir.sin_addr.s_addr=INADDR_ANY;
	dir.sin_port=htons(atoi(argv[1]));
	dir.sin_family=PF_INET;

	if (bind(s, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
		perror("error en bind");
		close(s);
		return 1;
	}
	if (listen(s, 5) < 0) {
		perror("error en listen");
		close(s);
		return 1;
	}
	diccionario = dic_create();
    diccionarioBloc= dic_create();
	while (1) 
	{
		tam_dir =sizeof(dir_cliente); 
		if ((s_conect=accept(s, (struct sockaddr *)&dir_cliente, &tam_dir))<0) {
			perror("error en accept");
			close(s);
			return 1;
         }
		 servicio((long)s_conec);
	}
	close(s);

    return 0;
}
