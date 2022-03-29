#include <stdio.h>
#include "util/map.h"
#include "util/queue.h"
#include "util/set.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <pthread.h>

//extern unsigned long GenerateUUID(uuid_t *uuid);

typedef struct tema {
    const char *nombre; // clave de acceso
    set *subscritos;      // clientes subscritos al tema
    // ....
} tema;

typedef struct cliente {
    const char *identificador = uuid; // clave de acceso UUID
    set *tema_subscritos;        // temas a los que pertenece
    queue *eventos;    // eventos encolados a ese cliente
    // ....
} cliente;

typedef struct evento {
    char *texto;
    clock_t fecha;
    int contador;
    // ....
} evento;

// crea un cliente y lo a�ade al mapa
cliente * crea_cliente(map *mc, const char *identificador) {
    cliente *c = malloc(sizeof(cliente));
    c->identificador = identificador;
    c->tema_subscritos = set_create(0); //almacena el conjunto de descriptores de los temas subscritos
    // inicialmente, no tiene mensajes
    c->eventos = queue_create(0); //creamos cola de eventos
    // se inserta en el mapa de clientes para poder ser accedido por id
    map_put(mc, c->identificador, c); // el mapa de cliente mc con clave id y valor cliente
    return p;
}

// crea un tema y lo a�ade al mapa
tema * crea_tema(map *mt, const char *nombre) {
    tema *t = malloc(sizeof(tema));
    t->nombre = nombre;
    t->subscritos = set_create(0); //almacena el conjunto de descriptores de clientes subscritos al tema
    // se inserta en el mapa de temas para poder ser accedido por nombre
    map_put(mt, t->nombre, t);
    return t;
}
//crear evento
evento * crea_evento(char *texto) {
    evento *e = malloc(sizeof(evento + 1)); //caracter nulo final???
    e->texto=texto;
    e->fecha=times(NULL);
    e->contador=0;
    return e;
}

// subscribe cliente al tema
int subscribe_cliente_al_tema(tema *t, cliente *c) {
    int ret = -1;
    if (set_add(c->tema_subscritos, t)<0)
        fprintf(stderr, "error: el cliente ya estaba subscrito al tema\n");
    else if (set_add( t->nombre, c)<0)
        fprintf(stderr, "error: el tema ya tiene a este cliente subscrito\n");
    else ret = 0;
    return ret;
}
// desubscribe cliente del tema
int baja_persona_en_grupo(map *mt, const char *nt, map *mc, const char *nc) {
    int error;
    tema * t = map_get(mt, nt, &error);
    if (error==-1) return -1;
    cliente * c = map_get(mc, nc, &error);
    if (error==-1) return -1;
    set_remove(t->subscritos, c, NULL);
    set_remove(c->tema_subscritos, t, NULL);
    return 0;
}

// encolar evento al descriptor de cliente
int encola_evento(map *mt, char *nombre_tema, evento *e){
    int error;
    tema * t = map_get(mt, nombre_tema, &error);
    if (error==-1) return -1;

    set_iter *i = set_iter_init(t->subscritos);
    for ( ; set_iter_has_next(i); set_iter_next(i)) {
	++e->contador;
        cliente *c = ((cliente *)(set_iter_value(i)));
        queue_push_back(c->eventos, e);
    }
    set_iter_exit(i);
    return 0;
}
//consumir evento
int consume_evento(map *mc, char *id_cliente){
    // imprime mensajes recibidos por esa persona
    int error;
    cliente *c = map_get(mc, id_cliente, &error);
    if (error==-1) return -1;
    evento *e;
    printf("-----------------------------------\n");
    printf("Persona %s: mensajes encolados %d\n", id_cliente, queue_length(c->eventos));
    for ( ; queue_length(c->eventos); ){
        e=queue_pop_front(c->eventos, NULL);
        printf("%s ha recibido el mensaje (%s) en el momento %ld\n",
           id_cliente, e->texto, e->fecha);
	if (--e->contador==0)
	    free(e);
    }
    return 0;
}

//numero de temas topic()
int numero_temas(tema *t) {
    ret = set_size(t->nombre);
    printf("Temas: %s\n", set_size(t->nombre));
    return ret;
}
//numero de clients()
int numero_clientes(cliente *c) {
    ret = set_size(c->identificador);
    printf("Clientes: %s\n", set_size(c->identificador));
    return ret;
}
//numero de subscriptores por tema
int numero_clientes_subscritos_tema(tema *t) {
    ret = set_size(t->subscritos);
    return ret;
}

//Conexion del broker con UUID
void revierte(char *b, int t){
    char aux;
    for (int i=0; i<t/2; i++) {
        aux=b[i];
        b[i]=b[t-i-1];
        b[t-i-1]=aux;
    }
}

void *servicio(void *arg){
        int s_srv, tam;
        s_srv=(long) arg;
        printf("nuevo cliente\n");
        while (recv(s_srv, &tam, sizeof(tam), MSG_WAITALL)>0) {
            printf("recibida petici�n cliente\n");
            int tamn=ntohl(tam);
            char *dato = malloc(tamn);
            recv(s_srv, dato, tamn, MSG_WAITALL);
            // sleep(5); // para probar que servicio no es concurrente
            revierte(dato, tamn);
            send(s_srv, dato, tamn, 0);
        }
        close(s_srv);
	return NULL;
}
// Cambiar para que coja operaciones trader
int main(int argc, char *argv[]){
    int s, s_conec;
    unsigned int tam_dir;
    struct sockaddr_in dir, dir_cliente;
    struct stat st;
    int opcion=1;

    if(argc!=3) {
        fprintf(stderr, "Uso: %s puerto fichero_temas\n", argv[0]);
        return 1;
    }
    if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("error creando socket");
        return 1;
    }
    /* Para reutilizar puerto inmediatamente */
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opcion, sizeof(opcion))<0){
                perror("error en setsockopt");
                return 1;
        }
    // Campos del socket
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
    pthread_t thid;
    pthread_attr_t atrib_th;
    pthread_attr_init(&atrib_th); // evita pthread_join
    pthread_attr_setdetachstate(&atrib_th, PTHREAD_CREATE_DETACHED);

    //leer fichero_temas\n open(argumento temas, solo lectura)

    if ((f = open(argv[0], O_RDONLY)) < 0) {
        perror("error abriendo fichero");
        continue;
    }
    // fstat(fichero,buffer) examina el fichero y llena el buffer
    fstat(f, &st);
    //La funci�n htonl convierte el entero de 32 bits dado por hostlong desde el orden
    //de bytes del hosts al orden de bytes de la red
    int tam=st.st_size;
	int tamn=htonl(tam);
    // lee (fichero, buffer, bytes del tema + caracter nulo?)
	void *p = read(f,&st,tamn + 1);
        close(f);

    while(1) {
        tam_dir=sizeof(dir_cliente);
        if ((s_conec=accept(s, (struct sockaddr *)&dir_cliente, &tam_dir))<0){
            perror("error en accept");
            close(s);
            return 1;
        }
        /* 
        	struct cabecera cab;
		recv(s_conec, &cab, sizeof(cab), MSG_WAITALL);
		int tam1=ntohl(cab.long1);
		int tam2=ntohl(cab.long2);
		char *dato1 = malloc(tam1+1);
		char *dato2 = malloc(tam2+1);
		recv(s_conec, dato1, tam1, MSG_WAITALL);
		recv(s_conec, dato2, tam2, MSG_WAITALL);
		dato1[tam1]='\0';
		dato2[tam2]='\0';
		close(s_conec);
		printf("dato1 %s dato2 %s\n", dato1, dato2);
        */
	pthread_create(&thid, &atrib_th, servicio, (void *)(long)s_conec);
    }
   
    close(s);
    return 0;
}
