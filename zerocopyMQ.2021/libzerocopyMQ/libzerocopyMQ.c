#include <stdint.h>
#include "zerocopyMQ.h"
#include "comun.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//al final comprobar el tamaño maximo del mensaje y del nombre de la cola


//para crear conexion vamos a apoyarnos en el código de guía de eco_clie_tcp.c
int crearConnection(){
    int s;
    struct sockaddr_in dir;
    struct hostent *host_info;

    if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("error creando socket");
		return 1;
	}

    host_info=gethostbyname(getenv("BROKER_HOST");
    dir.sin_addr=*(struct in_addr *)host_info->h_addr;
	dir.sin_port=htons(atoi(getenv("BROKER_PORT")));
	dir.sin_family=PF_INET;

    if (connect(s, (struct sockaddr *)&dir, sizeof(dir)) < 0) {
		perror("error en connect");
		close(s);
		return 1;
	}
    return s;
}


int intercambioBroker(void **msjVuelta, uint32_t *tamMsjVuelta, const void *msjida, uint32_t tamMsjIda, const char *cola, char *type){
int c = crearConnection();



close(c);

}

int createMQ(const char *cola) {
    int res = intercambioBroker(NULL,NULL,"\0",0,cola,"1");
    return res;
}
int destroyMQ(const char *cola){
    int res = intercambioBroker(NULL,NULL,"\0",0,cola,"2");
    return res;
}
int put(const char *cola, const void *mensaje, uint32_t tam) {
    int res = intercambioBroker(NULL,NULL,mensaje,tam,cola,"3");
    return res;
}
int get(const char *cola, void **mensaje, uint32_t *tam, bool blocking) {
    int res;
    if(blocking){
    res = intercambioBroker(cola,mensaje,"\0",0,cola,"4");
	}
    else{
     res = intercambioBroker(cola,mensaje,"\0",0,cola,"5");
	}
    return res;
}
