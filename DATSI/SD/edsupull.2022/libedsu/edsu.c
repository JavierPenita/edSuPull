#include <stdio.h>
#include <unistd.h>
#include "edsu.h"
#include "comun.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "util/map.h"


int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
int uuidCont = 1;
uuid = generate_UUID(uuidCont);

// se ejecuta antes que el main de la aplicación
__attribute__((constructor)) void inicio(void){
    if (begin_clnt()<0) {
        fprintf(stderr, "Error al iniciarse aplicación\n");
        // terminamos con error la aplicación antes de que se inicie
	// en el resto de la biblioteca solo usaremos return
        _exit(1);
    }
}

// se ejecuta después del exit de la aplicación
__attribute__((destructor)) void fin(void){
    if (end_clnt()<0) {
        fprintf(stderr, "Error al terminar la aplicación\n");
        // terminamos con error la aplicación
	// en el resto de la biblioteca solo usaremos return
        _exit(1);
    }
}

struct cabecera {
	int evento;
    int id;
    int uuid;
    int tema;
};

// operaciones que implementan la funcionalidad del proyecto

int Trader(const void *evento, uint32_t tam_evento, char *id, int uuid,const char *tema){
    int escrito;
    char rec[16];
    if(id == "1"){
        uuidCont = uuidCont +1;
    }
    if(id == "2"){
        close(s)
    }
    if( id > "2" ) {
        //Escritura en broker
        struct cabecera cab;
        cab.evento=htonl(strlen(evento));
        cab.id=htonl(strlen(id));
        cab.uuid=htonl(strlen(uuid));
        cab.tema=htonl(strlen(tema));
      
        struct iovec iov[4];
            iov[0].iov_base=&cab;
	        iov[0].iov_len=sizeof(cab);

            iov[1].iov_base=&evento;
	        iov[1].iov_len=strlen(evento);

	        iov[2].iov_base=id;
	        iov[2].iov_len=strlen(id);

            iov[3].iov_base=uuid;
	        iov[3].iov_len=strlen(uuid);

            iov[4].iov_base=&tema;
	        iov[4].iov_len=strlen(tema);

        if ((escrito=writev(s, iov,5)) < 0) {
        	    perror("error en writev");
        	    close(s);
        	    return -1;
        printf("escrito %d\n", escrito);

        //Recepcion en broker
	    while((leido=recv(s, rec, TAM,0))>0){
		    printf("rec: %s\n",rec);
		    if(strcmp(rec,"OK")==0) {
			    close(s);
			    return 0;
		    }
		    else if(strcmp(rec,"FAIL")==0){
			    printf("Recepcion datos no válida");
			    close(s);
			    return -1;
		    }
		    if (leido<0) {
			    printf("error en read");
			    close(s);
			    return -1;
		    }
        }
	}
    return 0;
}

int begin_clnt(void){
    
    if ((s < 0) {
		perror("error creando socket");
		return -1;
	}
    struct addrinfo *res; 
        if (getaddrinfo(getenv("BROKER_HOST"), getenv("BROKER_PORT"), NULL, &res) != 0) { 
                perror("error en getaddrinfo");
                close(s);
                return -1;
        }
        if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
                perror("error en connect");
                close(s);
                return -1;
        }
        freeaddrinfo(res);

        Trader("\0", 0,"1", uuid,NULL)
        printf("Conexion exito por parte del cliente %d\n", uuid);
   }
   return 0;
}

int end_clnt(void){
    int trade =Trader("\0", 0,"2", uuid,NULL)
        printf("Conexion de %d finalizada con exito \n", uuid);
    return trade;
}

int subscribe(const char *tema){
    int trade = Trader("/0", 0,"3",uuid,tema);
    return trade; 
}
int unsubscribe(const char *tema){
    int trade = Trader("/0", 0,"4",uuid,tema);
    return trade; 
}
int publish(const char *tema, const void *evento, uint32_t tam_evento){
    int trade = Trader(evento, tam_evento,"5",uuid,tema);
    return trade; 
}
// reservar memoria
int get(char **tema, void **evento, uint32_t *tam_evento){
    int trade = Trader(evento, tam_evento,"6",uuid,tema);
    return trade;
}



// operaciones que facilitan la depuración y la evaluación
int topics(){ // cuántos temas existen en el sistema
    int trade =  Trader("/0", 0,"7",uuid,NULL);
    return trade;    
}
int clients(){ // cuántos clientes existen en el sistema
    int trade = Trader("/0", 0,"8",uuid,NULL);
    return trade;    
}
int subscribers(const char *tema){ // cuántos subscriptores tiene este tema
    int trade = Trader("/0", 0,"9",uuid,tema);
    return trade;  
}
int events() { // nº eventos pendientes de recoger por este cliente
    int trade = Trader("/0", 0,"10",uuid,NULL);
    return trade;  
}
