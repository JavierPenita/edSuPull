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
#include <sys/stat.h>

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

struct cabecera {
	int evento;
    int id;
    int uuid;
    int tema;
};

// crea un cliente y lo añade al mapa
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

// crea un tema y lo añade al mapa
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
int numero_temas(map *mt) {
    ret = map_size(map *mt);
    printf("Temas: %s\n", map_size(map *mt));
    return ret;
}
//numero de clients()
int numero_clientes(map *mc) {
    ret = map_size(map *mc);
    printf("Clientes: %s\n", map_size(map *mc));
    return ret;
}
//numero de subscriptores por tema
int numero_clientes_subscritos_tema(tema *t) {
    ret = set_size(t->subscritos);
    return ret;
}

// Cambiar para que coja operaciones trader
int main(int argc, char *argv[]){
    int s, s_conec;
    unsigned int tam_dir;
    struct sockaddr_in dir, dir_cliente;
    struct stat st;
    struct iovec iovm[2];
    int opcion=1;
    map *mt = map_create(key_string, 0);
    map *mc = map_create(key_string, 0);

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

    //leer fichero_temas\n open(argumento temas, solo lectura)
    FILE *f = fopen(argv[0], "r");
    if (f==NULL)
        perror ("Error al abrir el fichero de temas");
        return -1;
    struct stat sb;

    if (stat(f, &sb) == -1) {
        perror("stat");
        return -1;
    }
    char *contenido_tema = malloc(sb.st_size);

    while(fscanf(f, "%s", contenido_tema) != EOF){
        printf("%s", contenido_tema);            
    }
}   fclose(f);

    while(1) {
        tam_dir=sizeof(dir_cliente);
        //Accept(socket,puntero estructura sockaddr, tamaño estructura)
        if ((s_conec=accept(s, (struct sockaddr *)&dir_cliente, &tam_dir))<0){
            perror("error en accept");
            close(s);
            return 1;
        }
        struct cabecera cab;

		recv(s_conec, &cab, sizeof(cab), MSG_WAITALL);
		int tam_evento=ntohl(cab.evento);
        int tam_id=ntohl(cab.id);
		int tam_uuid=ntohl(cab.uuid);
        int tam_tema=ntohl(cab.tema);
		
		char *evento = malloc(tam_evento+1);
        char *id = malloc(tam_id+1);
		char *uuid = malloc(tam_uuid+1);
        char *tema = malloc(tam_tema+1);
		
		recv(s_conec, evento, tam_evento, MSG_WAITALL);
        recv(s_conec, id, tam_id, MSG_WAITALL);
		recv(s_conec, uuid, tam_uuid, MSG_WAITALL);
        recv(s_conec, tema, tam_tema, MSG_WAITALL);
		
		evento[tam_evento]='\0';
        id[tam_id]='\0';
		uuid[tam_uuid]='\0';
        tema[tam_tema]='\0';

		switch( id )
        {
            case 1:
                //begin connect
                break;
            case 2 :
                //end connect
                break;
            case 3 :
                //subscribe
                break;
            case 4:
                //unsubscribe
                break;
            case 5 :
                //publish
                break;
            case 6 :
                //get
                break;
             case 7:
                //topics
                printf(numero_temas(mt))
				iovm[0].iov_base = "OK";
                iovm[0].iov_len = strlen("OK")+1;
				//enviamos mensaje de estado al cliente						
				writev(s_conec, iovm, 1);
			}
                break;
            case 8 :
                //clients
                break;
            case 9 :
                //subscribers
                break;
            case 10 :
                //events
                break;
        }
		close(s_conec);
    }
   
    //close(s);
    return 0;
}
