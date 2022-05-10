#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include<arpa/inet.h>
#include "comun.h"
#include "set.h"
#include "map.h"
#include "queue.h"



map *mapa_cliente;
map *mapa_tema;
FILE *f_temas;
client_socket_info_t cl[10];
int count = 0;
int main(int argc, char *argv[]){
    if(argc!=3) {
        fprintf(stderr, "Uso: %s puerto fichero_temas\n", argv[0]);
        return 1;
    }
    
    mapa_cliente = map_create(key_string, 1);
    mapa_tema = map_create(key_string, 1);
    int broker_socket;
    int opt = 1;
    int broker_port = atoi(argv[1]);
    char *archivo_temas = argv[2];
    struct sockaddr_in broker_address;
    //lee temas desde un archivo y añadirlos al descriptor de temas
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    f_temas = fopen(archivo_temas, "r");
    if (f_temas == NULL)
        exit(EXIT_FAILURE);
    //int err;
    while ((read = getline(&line, &len, f_temas)) != -1) {
        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
        //añadir el tema al descriptor de tams
        int line_len = strlen(line) - 1;
        char *aux_line = (char *) malloc(sizeof(char) * line_len);
        strncpy(aux_line, line, line_len);
        aux_line[line_len] = '\0';
        //if (map_get(mapa_tema, aux_line, &err) == NULL) {
            tema_t *tema =(tema_t*) malloc(sizeof(tema_t));
            tema->nombre_tema = aux_line;
            tema->clientes = set_create(0);
            printf("line datos string: %s, size: %ld length: %ld\n", tema->nombre_tema, strlen(line), sizeof(line));
            // se inserta en el mapa de personas para poder ser accedido por nombre
            map_put(mapa_tema, tema->nombre_tema, tema);
            //free(tema); ---------------------????????????????????????  
        //}
    }

    fclose(f_temas);
    if (line)
        free(line);
    if ((broker_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("\n Error when creating  server socket\n");
            return -1;
    }

    //sets server variables
    broker_address.sin_family = AF_INET;
    broker_address.sin_port = htons(broker_port);
    broker_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Forcefully attaching socket to a port
    if (setsockopt(broker_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
    &opt, sizeof(opt)))
    {
        printf("setsockopt");
        return -1;
    }

    //binds server to port
    if(bind(broker_socket, (struct sockaddr*)&broker_address,
        sizeof(broker_address))<0){
        printf("\n Error when binding server socket\n");
            return -1;
    }

    //listening to socket
    if(listen(broker_socket, 1)< 0){
        printf("\n Error when listening to Socket\n");
            return -1;
    }
    
    //server has been created successfully, now we wait for connections
    printf("Server started on port %d. Accepting connections\n", broker_port);

    
    //printf("Accepted conection\n");
    while (1)
    {
        //accepting client connections
        client_socket_info_t c_info;
        c_info.sockID = accept(broker_socket, (struct sockaddr*)&c_info.client_addr, &(c_info.len));
        int num_clients = map_size(mapa_cliente) +  1;
        printf("Accepted conection from client, %d are now connected\n", num_clients);
        char data[OP_LEN + UUID_SIZE];
        UUID_t uuid;
        char tipo_op[OP_LEN + 1];
        int valread = recv(c_info.sockID, data, OP_LEN + UUID_SIZE,0 );
        printf("In broker, read %d bytes\n", valread);

        //gettimgg the op type
        strncpy(tipo_op, data, OP_LEN);
        tipo_op[OP_LEN] = '\0';
        
        //getting the uuid
        strncpy(uuid, data + OP_LEN, UUID_SIZE);
        uuid[UUID_SIZE] = '\0';

        //do begin client for the new client
        broker_begin_clnt(uuid, &c_info);
        
        pthread_create(&c_info.tid, NULL, handle_clients, (void *) &c_info); //Check using client info
        count++;
    }
    
}
//message type 3 byte op_type | 37 byte UUID | 32 byte tema | 10 byte tam_even 
//second recieve event
#define TIPO_OP_SIZE 3
#define MAX_INT_SIZE 24
int test = 1; 
void *handle_clients(void *Cliente){
    client_socket_info_t *cliente = (client_socket_info_t*)Cliente;
    int client_socket = cliente->sockID;
    printf("In client thread with tid %ld\n", cliente->tid);
    while (1) {
        
        //esparar al mensaje del cliente
        char data[OP_LEN + UUID_SIZE];
        UUID_t uuid;
        char *tipo_op = (char *) malloc(sizeof(char) * (OP_LEN + 1));
        int valread = recv(client_socket, data, OP_LEN + UUID_SIZE,0 );
        if (valread <= 0) {
            continue;;
        }
        //printf("In broker thread, read %d bytes\n", valread);

        //gettimgg the op type
        strncpy(tipo_op, data, OP_LEN);
        tipo_op[OP_LEN] = '\0';

        //getting the uuid
        strncpy(uuid, data + OP_LEN, UUID_SIZE);
        uuid[UUID_SIZE] = '\0';
        
        //getting int value of operation;
        int OP = atoi(tipo_op);
        //printf("In broker thread, recieved client: operation: %s with uid %s\n", tipo_op, uuid);
        //pointer para pasar de string a int
        char *eptr;
        //tamaño de tema en uint
        unsigned long resultado_num_tema;
        //tamaño del tema en char
        char tam_tema[MAX_INT_SIZE];
        //buffer de tema
        char *tema_s;
        switch (OP)
        {

        case SUBSCRIBE:;
            // sacar el tamaño del buffer
            recv(client_socket, tam_tema, MAX_INT_SIZE, 0);
            resultado_num_tema = strtoul(tam_tema, &eptr, 10);
            tema_s = (char*)malloc(sizeof(char) * resultado_num_tema);
            //sacar el tema 
            recv(client_socket, tema_s, resultado_num_tema, 0 );
            printf("El tema es %s y tamaño %ld\n", tema_s, strlen(tema_s));
            broker_subscribe(tema_s, uuid, cliente);
            //comprobar que el cliente esta dado de alta
            break;

        case UNSUBSCRIBE:
            // sacar el tamaño del buffer
            recv(client_socket, tam_tema, MAX_INT_SIZE, 0);
            resultado_num_tema = strtoul(tam_tema, &eptr, 10);
            tema_s = (char*)malloc(sizeof(char) * resultado_num_tema);
            //sacar el tema 
            recv(client_socket, tema_s, resultado_num_tema, 0 );
            printf("El tema es %s\n", tema_s);
            broker_unsubscribe(tema_s, uuid, cliente);
            //comprobar que el cliente esta dado de alta
            break;
        
        case PUBLISH:;
            // sacar el tamaño del buffer
            recv(client_socket, tam_tema, MAX_INT_SIZE, 0);
            resultado_num_tema = strtoul(tam_tema, &eptr, 10);
            tema_s = (char*)malloc(sizeof(char) * resultado_num_tema);
            //sacar el tema 
            recv(client_socket, tema_s, resultado_num_tema, 0 );
            printf("El tema es %s\n", tema_s);
            //string para sacar el tam_even_uint de evento
            char tam_evento[MAX_INT_SIZE];
            recv(client_socket, tam_evento, MAX_INT_SIZE, 0);
            printf("El tam_even_uint del evento es: %s\n", tam_evento);
            //get the uint32 value   
            unsigned long resultado_int;
            /* Convert the provided value to a decimal unsigned long */
            resultado_int = strtoul(tam_evento, &eptr, 10);
            printf("El int del resultado es %ld\n", resultado_int);
            broker_publish(tema_s, resultado_int, uuid, cliente);
            break;
        
        case GET:
            broker_get(uuid, cliente);
            break;

        case TOPICS:
            broker_topics(cliente);
            break;

        case CLIENTS:
            broker_clients(cliente);
            break;

        case SUBSCRIBERS:
            // sacar el tamaño del buffer
            recv(client_socket, tam_tema, MAX_INT_SIZE, 0);
            resultado_num_tema = strtoul(tam_tema, &eptr, 10);
            tema_s = (char*)malloc(sizeof(char) * resultado_num_tema);
            //sacar el tema 
            recv(client_socket, tema_s, resultado_num_tema, 0 );
            printf("El tema es %s\n", tema_s);
            broker_subscribers(tema_s, cliente);
            //comprobar que el cliente esta dado de alta
            break;
        
        case EVENTS:
            broker_events(uuid, cliente);
            break;

        case END_CLIENT:;
            broker_end_clnt(cliente, uuid);
            break;

        default:
            break;
        }
    }
}

void broker_begin_clnt(UUID_t uid_t, client_socket_info_t *cliente){ 
    //adds client uuid to the client map upon creation
    cliente_t *client = (cliente_t*)malloc(sizeof(cliente_t));
    strncpy(client->uid, uid_t, UUID_SIZE + 1);
    client->c_socket_info = cliente;
    client->temas = set_create(0);
    // inicialmente, no tiene mensajes
    client->mensajes = queue_create(0);
    // se inserta en el mapa de personas para poder ser accedido por nombre
    printf("A client with %s uid has connected has been inserted\n", client->uid);
    map_put(mapa_cliente, client->uid, client);
    //send ack to client
    char* ack =(char*) malloc( MAX_INT_SIZE );
    snprintf( ack, MAX_INT_SIZE, "%d", 1 );
    send(cliente->sockID, ack, MAX_INT_SIZE, 0);  
}
int c =0;
void broker_end_clnt(client_socket_info_t *cliente, UUID_t uuid_t){
    c++;
    printf("Counting %d\n", c);
    int err = 0;
    cliente_t *existe_cliente = map_get(mapa_cliente, uuid_t, &err);
    if (err < 0) {
        //no existe
        printf("The client %s does not exist\n", uuid_t);
    } else {
        //crea iterator
        set_iter *client_iter = set_iter_init(existe_cliente->temas);
        for ( ; set_iter_has_next(client_iter); set_iter_next(client_iter) ) {
            //tema_t *curr_tema = (tema_t *) malloc(sizeof(tema_t));
            tema_t *curr_tema = ((tema_t*)(set_iter_value(client_iter)));
            if (curr_tema == NULL) printf("It is indeed null\n");
            printf("trying to print the dumb tema here %s\n", curr_tema->nombre_tema);
            //deleting all the temas
            if (set_remove(curr_tema->clientes, existe_cliente, NULL) < 0) {
                err = -1;
                printf("El cliente con uuid %s no ha suscrito al tema 1\n ", uuid_t);
            }
            else if (set_remove(existe_cliente->temas, curr_tema, NULL) < 0) {
                err = -1;
                printf("El cliente con uuid %s no haa suscrito al tema\n ", uuid_t);
            }
            if (err > -1) printf("El cliente con uuid %s se ha dado la baja al tema  con exito\n", uuid_t);
            //unsubscribe(curr_tema->nombre_tema, existe_cliente->uid, cliente);
        }
        set_iter_exit(client_iter);
        map_remove_entry(mapa_cliente, uuid_t, NULL);
        free(existe_cliente->mensajes);
        free(existe_cliente->temas);
        pthread_join(existe_cliente->c_socket_info->tid, NULL);
    }
    //send ack to client
    char* ack =(char*) malloc( MAX_INT_SIZE );
    snprintf( ack, MAX_INT_SIZE, "%d", err );
    send(cliente->sockID, ack, MAX_INT_SIZE, 0);
}

void broker_subscribe(const char *tema, UUID_t uuid, client_socket_info_t *cliente){
    //check if the client is in the map
    int err;
    cliente_t *existe_cliente = (cliente_t*) map_get(mapa_cliente, uuid, &err);
    if (err < 0) {
        //no existe
        printf("The client %s does not exist\n", uuid);
    }
    printf("line datos string: %s, size: %ld length: %ld\n", tema, strlen(tema), sizeof(tema));
    tema_t *existe_tema = (tema_t*)map_get(mapa_tema, tema, &err);
    if (err < 0) {
        //tema no existe
        printf("el tema %s no existe\n", tema);
    }
    else {
        //inserta tema en el set de clientes
        if (set_add(existe_cliente->temas, existe_tema) < 0) {
            err = -1;
            printf("El cliente con uuid %s ya ha suscrito al tema %s, 2\n ", uuid, tema);
        }
        //inserta cliente en el set de temas
        else if (set_add(existe_tema->clientes, existe_cliente) < 0) {
            err = -1;
            printf("El cliente con uuid %s ya ha suscrito al tema %s, 1\n ", uuid, existe_tema->nombre_tema);
        }
        if (err >-1 ) printf("El cliente con uuid %s a suscrito al tema %s con exito with len %ld\n", uuid, existe_tema->nombre_tema, strlen(tema));
    }
    //send ack to client
    char ack[MAX_INT_SIZE];
    snprintf( ack, MAX_INT_SIZE, "%d", err );
    send(cliente->sockID, ack, MAX_INT_SIZE, 0);
    printf("sending ack of %s\n", ack);
}

void broker_unsubscribe(const char *tema, UUID_t uuid_t, client_socket_info_t *cliente){
     //check if the client is in the map
    int err;
    cliente_t *existe_cliente = (cliente_t*) map_get(mapa_cliente, uuid_t, &err);
    if (err < 0) {
        //no existe
        printf("The client %s does not exist\n", uuid_t);
    }
    printf("line datos string: %s, size: %ld length: %ld\n", tema, strlen(tema), sizeof(tema));
    tema_t *existe_tema = (tema_t*)map_get(mapa_tema, tema, &err);
    if (err < 0) {
        //tema no existe
        printf("el tema %s no existe\n", tema);
    } 
    else {
        //check si el clienta ya ha suscrito al tema
        //inserta cliente en el set de temas
        if (set_remove(existe_tema->clientes, existe_cliente, NULL) < 0) {
            err = -1;
            printf("El cliente con uuid %s no ha suscrito al tema %s\n ", uuid_t, tema);
        }
        //inserta tema en el set de clientes
        else if (set_remove(existe_cliente->temas, existe_tema, NULL) < 0) {
            err = -1;
            printf("El cliente con uuid %s no haa suscrito al tema %s\n ", uuid_t, tema);
        }
        if (err > -1) printf("El cliente con uuid %s se ha dado la baja al tema %s con exito\n", uuid_t, tema);
    }
    //send ack to client
    char* ack =(char*) malloc( MAX_INT_SIZE );
    snprintf( ack, MAX_INT_SIZE, "%d", err );
    send(cliente->sockID, ack, MAX_INT_SIZE, 0);
}
void broker_publish(const char *tema, uint32_t tam_evento, UUID_t uuid_t, client_socket_info_t *cliente){
    int err;
    tema_t *existe_tema = (tema_t*)map_get(mapa_tema, tema, &err);
    if (err < 0) {
        //tema no existe
        printf("el tema %s no existe\n", tema);
    }
    else {
        //create buffer for the event
        char *event_buffer = (char *)malloc( sizeof(char)*tam_evento);
        //wait to recieve the event
        int valread = recv(cliente->sockID, event_buffer, tam_evento,0 );
        if(valread < 0) {
            printf("recieve failed\n");
            err = valread;
        }
        else {
            printf("the evnet in recieve is : %s\n", event_buffer);
            mensaje_t *m_evento = (mensaje_t*) malloc(sizeof(mensaje_t));
            m_evento->evento = (char *)malloc( sizeof(char)*tam_evento);
            memcpy(m_evento->evento, event_buffer, tam_evento );
            m_evento->contador = 0;
            m_evento->len = tam_evento;
            m_evento->tema = (char*)tema;
            printf("the event in char is %s\n",(char*) m_evento->evento);
            //crea iterator
            set_iter *client_iter = set_iter_init(existe_tema->clientes);
            for ( ; set_iter_has_next(client_iter); set_iter_next(client_iter) ) {
                cliente_t *curr_cliente = (cliente_t *)set_iter_value(client_iter);
                printf("trying to print the dumb uid here %s\n", curr_cliente->uid);
                queue_push_back(curr_cliente->mensajes, m_evento);
                m_evento->contador++;
            }
            set_iter_exit(client_iter);
        }
    }
    //send ack to client
    char* ack =(char*) malloc( MAX_INT_SIZE );
    snprintf( ack, MAX_INT_SIZE, "%d", err );
    send(cliente->sockID, ack, MAX_INT_SIZE, 0);

}
void broker_get(UUID_t uuid_t, client_socket_info_t *cliente){
    int err;
    cliente_t *existe_cliente = (cliente_t*) map_get(mapa_cliente, uuid_t, &err);
    mensaje_t *m_evento = (mensaje_t *)queue_pop_front(existe_cliente->mensajes, &err);
    if (err < 0) {
        printf("No event to get\n");
    }
    else {
        m_evento->contador--;
        //enviar los tamaños
        char send_buffer[MAX_INT_SIZE * 2 + strlen(m_evento->tema) + m_evento->len ];
        //copiar el tamaño del tema
        char* tam_tema =(char*) malloc( MAX_INT_SIZE );
        snprintf( tam_tema, MAX_INT_SIZE, "%ld", strlen(m_evento->tema) );
        //copiar el tamaño del evento
        char* tam_event =(char*) malloc( MAX_INT_SIZE );
        snprintf( tam_event, MAX_INT_SIZE, "%d", m_evento->len );
        strncpy(send_buffer, tam_tema, MAX_INT_SIZE);
        printf("El tamaño del tema es %s\n", tam_tema);
        strncpy(send_buffer + MAX_INT_SIZE, tam_event, MAX_INT_SIZE);
        printf("El tamaño del evento es %s\n", tam_event);
        //copiar el tema
        strncpy(send_buffer + MAX_INT_SIZE * 2, m_evento->tema, strlen(m_evento->tema));
        printf("El evento es %s \n", (char*)m_evento->evento);
        printf("El tema es %s\n", m_evento->tema);
        //copia el evento
        memcpy(send_buffer + MAX_INT_SIZE * 2 + strlen(m_evento->tema), m_evento->evento, m_evento->len);
        send(cliente->sockID, send_buffer, MAX_INT_SIZE * 2 + strlen(m_evento->tema) + m_evento->len, 0);
        if (m_evento->contador == 0) {
            free(m_evento->evento);
        }
    }
    //send ack to client
    char* ack =(char*) malloc( MAX_INT_SIZE );
    snprintf( ack, MAX_INT_SIZE, "%d", err );
    send(cliente->sockID, ack, MAX_INT_SIZE, 0);

}

// operaciones que facilitan la depuración y la evaluación
void broker_topics(client_socket_info_t *c_socket_info){ // cuántos temas existen en el sistema
    printf("About to send num of temas\n");
    int num_temas = map_size(mapa_tema);
    char *buffer = (char *) malloc(sizeof(char) * MAX_INT_SIZE);
    snprintf( buffer, MAX_INT_SIZE, "%d", num_temas );
    printf("The number of temas to send are %s\n ", buffer);
    send(c_socket_info->sockID, buffer, MAX_INT_SIZE, 0);
}

void broker_clients(client_socket_info_t *c_socket_info){ // cuántos clientes existen en el sistema
    printf("About to send num of clients\n");
    int num_clients = map_size(mapa_cliente);
    char *buffer = (char *) malloc(sizeof(char) * MAX_INT_SIZE);
    snprintf( buffer, MAX_INT_SIZE, "%d", num_clients );
    printf("The number of clients to send are %s\n ", buffer);
    send(c_socket_info->sockID, buffer, MAX_INT_SIZE, 0);
    
}

void broker_subscribers(const char *tema, client_socket_info_t *c_socket_info){ // cuántos subscriptores tiene este tema
    printf("About to send num of clients\n");
    int err;
    tema_t *existe_tema = (tema_t*)map_get(mapa_tema, tema, &err);
    if (err < 0) {
        //tema no existe
        printf("el tema %s no existe\n", tema);
        //send ack to client
        char* ack =(char*) malloc( MAX_INT_SIZE );
        snprintf( ack, MAX_INT_SIZE, "%d", err );
        send(c_socket_info->sockID, ack, MAX_INT_SIZE, 0);

    }
    else {
        int num_clients = set_size(existe_tema->clientes);
        char *buffer = (char *) malloc(sizeof(char) * MAX_INT_SIZE);
        snprintf( buffer, MAX_INT_SIZE, "%d", num_clients );
        printf("The number of clients to send are %s\n ", buffer);
        send(c_socket_info->sockID, buffer, MAX_INT_SIZE, 0);
    }
    
}

void broker_events(UUID_t uuid,  client_socket_info_t *c_socket_info) { // nº eventos pendientes de recoger por este cliente
    printf("About to send num of events clients\n");
    int err;
    cliente_t *cliente = (cliente_t*)map_get(mapa_cliente, uuid, &err);
    int num_mensajes = queue_length(cliente->mensajes);
    char *buffer = (char *) malloc(sizeof(char) * MAX_INT_SIZE);
    snprintf( buffer, MAX_INT_SIZE, "%d", num_mensajes );
    printf("The number of clients to send are %s\n ", buffer);
    send(c_socket_info->sockID, buffer, MAX_INT_SIZE, 0);
    
}