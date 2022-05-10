#include <stdio.h>
#include <unistd.h>
#include "edsu.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include<arpa/inet.h>
#include "comun.h"

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
int client_socket;
UUID_t uuid;

char *to_str(int num) {
    char* buffer = malloc( 2 );
    snprintf( buffer, 2, "%d", num );
    return buffer;
}
// operaciones que implementan la funcionalidad del proyecto
int begin_clnt(void){
    int return_val;
    generate_UUID(uuid);
    //declare sockets variables
    printf("The uid is %s\n", uuid);
	
	struct sockaddr_in server_address;

	//creates client Socket
	if((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("\n Error when Creating Socket\n");
		return -1;
	} else {
        printf("created socket\n");
    }
    
	int broker_port = atoi(getenv("BROKER_PORT"));
    char *broker_host = getenv("BROKER_HOST");
	//Sets server's address Variables
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(broker_port);
	server_address.sin_addr.s_addr = inet_addr(broker_host);

    printf("before connecting\n");
	//Connects TCP client socket to a server
	if (connect (client_socket, (struct sockaddr*)&server_address,
        sizeof(server_address))<0){
        printf("\n Error when Connecting Socket to server\n");
        return -1;
    } else {
        printf("connected to broker\n");
    }
    printf("after connecting\n");
    char tipo_op[] = "01";
    char send_buffer[OP_LEN + UUID_SIZE];
    strncpy(send_buffer, tipo_op, OP_LEN);
    strcpy(send_buffer + OP_LEN, uuid);
    printf("sending %s and uuid %s\n", send_buffer, uuid);
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE, 0) < 0) {
        printf("error when sending\n");
        return_val =-1;
    } else {
        printf("Message has been sent succesfully");
        char rcv_buffer[MAX_INT_SIZE];
        if (recv(client_socket, rcv_buffer, MAX_INT_SIZE, 0) < 0) return_val = -1;
        else return_val = atoi(rcv_buffer);
    }
    return return_val;
    
}

int end_clnt(void){
    int return_val;
    char tipo_op[] = "10";
    char send_buffer[OP_LEN + UUID_SIZE];
    strncpy(send_buffer, tipo_op, OP_LEN);
    strcpy(send_buffer+OP_LEN, uuid);
    printf("sending %s and uuid %s\n", send_buffer, uuid);
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE, 0) < 0) {
        printf("error when sending\n");
        return_val = -1;
    } else {
        printf("End message has been sent succesfully");
        char rcv_buffer[MAX_INT_SIZE];
        if (recv(client_socket, rcv_buffer, MAX_INT_SIZE, 0) < 0) return_val = -1;
        else return_val = atoi(rcv_buffer);
    }
    printf("ending client\n");
    return return_val;
}
int subscribe(const char *tema){
    int return_val = -1;
    //prepare message to send for supscription
    char tipo_op[] = "02";
    uint32_t len_tema = strlen(tema);
    char* tam_tema =(char*) malloc( MAX_INT_SIZE );
    snprintf( tam_tema, MAX_INT_SIZE, "%d", len_tema );
    char send_buffer[OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema)];
    //add the op to the string
    strncpy(send_buffer, tipo_op, OP_LEN);
    //copiar el uuid
    strncpy(send_buffer + OP_LEN, uuid, UUID_SIZE );
    //copiar el tamaño del tema
    strncpy(send_buffer + OP_LEN + UUID_SIZE, tam_tema, MAX_INT_SIZE);
    //copiar el tema
    strncpy(send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE, tema, strlen(tema)+1);
    printf("The tema is : %s and send buffer tema is %s\n", tema, send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE);
    //enviar el buffer al broker
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema), 0) < 0) {
        printf("error when sending\n");
        return_val = -1;
    } else {
        printf("Message subscribe has been sent succesfully\n");
        char rcv_buffer[MAX_INT_SIZE];
        if (recv(client_socket, rcv_buffer, MAX_INT_SIZE, 0) < 0) {return_val = -1;}
        else {return_val = atoi(rcv_buffer);}
        printf("the ack is %s\n", rcv_buffer);
    }
    return return_val;
}
int unsubscribe(const char *tema){
    int return_val;
    //prepare message to send for supscription
    char tipo_op[] = "03";
    uint32_t len_tema = strlen(tema);
    char* tam_tema =(char*) malloc( MAX_INT_SIZE );
    snprintf( tam_tema, MAX_INT_SIZE, "%d", len_tema );
    char send_buffer[OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema)];
    //add the op to the string
    strncpy(send_buffer, tipo_op, OP_LEN);
    //copiar el uuid
    strncpy(send_buffer + OP_LEN, uuid, UUID_SIZE );
    //copiar el tamaño del tema
    strncpy(send_buffer + OP_LEN + UUID_SIZE, tam_tema, MAX_INT_SIZE);
    //copiar el tema
    strncpy(send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE, tema, strlen(tema)+1);
    printf("The tema is : %s and send buffer tema is %s\n", tema, send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE);
    //enviar el buffer al broker
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema), 0) < 0) {
        printf("error when sending\n");
        return_val = -1;
    } else {
        printf("Message unsubscribe has been sent succesfully\n");
        char rcv_buffer[MAX_INT_SIZE];
        if (recv(client_socket, rcv_buffer, MAX_INT_SIZE, 0) < 0) return_val = -1;
        else return_val = atoi(rcv_buffer);
    }
    return return_val;
}

int publish(const char *tema, const void *evento, uint32_t tam_evento){
    int return_val;
    //prepare message to send for supscription
    char tipo_op[] = "04";
    int len_tema = strlen(tema);
    char* tam_tema =(char*) malloc( MAX_INT_SIZE );
    snprintf( tam_tema, MAX_INT_SIZE, "%d", len_tema );
    char send_buffer[OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema) + MAX_INT_SIZE + tam_evento];
    //add the op to the string
    strncpy(send_buffer, tipo_op, OP_LEN);
    //copiar el uuid
    strncpy(send_buffer + OP_LEN, uuid, UUID_SIZE );
    //copiar el tamaño del tema
    strncpy(send_buffer + OP_LEN + UUID_SIZE, tam_tema, MAX_INT_SIZE);
    //copiar el tema
    strncpy(send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE, tema, strlen(tema) + 1);
    printf("The tema is : %s and send buffer tema is %s\n", tema, send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE);
    //copiar el tamaño del buffer
    char* tam_event =(char*) malloc( MAX_INT_SIZE );
    snprintf( tam_event, MAX_INT_SIZE, "%d", tam_evento );
    strncpy(send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema), tam_event, MAX_INT_SIZE);
    printf("The tema is : %s and send buffer tema is %s and len of tema in str is %s and in int is %d\n", tema, send_buffer + OP_LEN + UUID_SIZE + MAX_TEMA_LEN, tam_event, tam_evento); 
    //copiar el evento
    memcpy(send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema) + MAX_INT_SIZE, evento, tam_evento);
    //enviar el buffer al broker
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema) + MAX_INT_SIZE + tam_evento, 0) < 0) {
        printf("error when sending\n");
        return_val = -1;
    } else {
        printf("Message publish has been sent succesfully\n");
        char rcv_buffer[MAX_INT_SIZE];
        if (recv(client_socket, rcv_buffer, MAX_INT_SIZE, 0) < 0) return_val = -1;
        else return_val = atoi(rcv_buffer);
    }
    return return_val;
}
int get(char **tema, void **evento, uint32_t *tam_evento){
    int return_val;
    char send_buffer[OP_LEN + UUID_SIZE];
    char tipo_op[] = "05";
    //add the op to the string
    strncpy(send_buffer, tipo_op, OP_LEN);
    //copiar el uuid
    strncpy(send_buffer + OP_LEN, uuid, UUID_SIZE );
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE, 0) < 0) {
        printf("error when sending evento\n");
        return_val = -1;
    } else {
        printf("GEt message has been succefully sent\n");
        char tam_tema[MAX_INT_SIZE];
        char tam_event[MAX_INT_SIZE];
        //recibir los tamaños
        recv(client_socket, tam_tema, MAX_INT_SIZE, 0);
        if (atoi(tam_tema) >= 0) {
            recv(client_socket, tam_event, MAX_INT_SIZE, 0);
            char *eptr;
            unsigned long resultado_num_tema;
            unsigned long resultado_num_event;
            /* Convert the provided value to a decimal unsigned long */
            resultado_num_tema = strtoul(tam_tema, &eptr, 10);
            resultado_num_event = strtoul(tam_event, &eptr, 10);

            //asignar tam de evento
            *tam_evento = resultado_num_event;
            //recibir el tema
            *tema = (char*)malloc(sizeof(char) * (resultado_num_tema + 1));
            recv(client_socket, *tema, resultado_num_tema, 0);
            (*tema)[resultado_num_tema] = '\0';
            printf("In get the length of tema is %ld\n", resultado_num_tema);
            //recibir el evento
            *evento = (void*)malloc(sizeof(void*) * resultado_num_event);
            recv(client_socket, *evento, resultado_num_event, 0);
            printf("The event gotten is %s\n",(char*) *evento);
            char rcv_buffer[MAX_INT_SIZE];
            if (recv(client_socket, rcv_buffer, MAX_INT_SIZE, 0) < 0) return_val = -1;
            else return_val = atoi(rcv_buffer);
        }
        else {
            return_val = -1;
        }
    }
    return return_val;
}

// operaciones que facilitan la depuración y la evaluación
int topics(){ // cuántos temas existen en el sistema
    int return_val;
    char *tipo_op = "06";
    char send_buffer[OP_LEN + UUID_SIZE];
    char recieve_buffer[MAX_INT_SIZE];
    sprintf(send_buffer, "%s%s", tipo_op, uuid);
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE, 0) < 0) return_val = -1;
    if (recv(client_socket, recieve_buffer, MAX_INT_SIZE, 0) < 0) return_val = -1;
    else return_val = atoi(recieve_buffer);
    printf("There are %s toics \n",recieve_buffer);
    return return_val;
}
int clients(){ // cuántos clientes existen en el sistema
    int return_val;
    char *tipo_op = "07";
    char send_buffer[OP_LEN + UUID_SIZE];
    char recieve_buffer[MAX_INT_SIZE];
    sprintf(send_buffer, "%s%s", tipo_op, uuid);
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE, 0) < 0) return_val = -1;
    if (recv(client_socket, recieve_buffer, MAX_INT_SIZE, 0) < 0) return_val = -1;
    else return_val = atoi(recieve_buffer);
    printf("There are %s clients \n",recieve_buffer);
    return return_val;
}
int subscribers(const char *tema){ // cuántos subscriptores tiene este tema
    char *tipo_op = "09";
    int len_tema = strlen(tema);
    char* tam_tema =(char*) malloc( MAX_INT_SIZE );
    snprintf( tam_tema, MAX_INT_SIZE, "%d", len_tema );
    char send_buffer[OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema)];
    char recieve_buffer[MAX_INT_SIZE];
    //add the op to the string
    strncpy(send_buffer, tipo_op, OP_LEN);
    //copiar el uuid
    strncpy(send_buffer + OP_LEN, uuid, UUID_SIZE );
    //copiar el tamaño del tema
    strncpy(send_buffer + OP_LEN + UUID_SIZE, tam_tema, MAX_INT_SIZE);
    //copiar el tema
    strncpy(send_buffer + OP_LEN + UUID_SIZE + MAX_INT_SIZE, tema, strlen(tema) + 1);
    send(client_socket, send_buffer, OP_LEN + UUID_SIZE + MAX_INT_SIZE + strlen(tema), 0);
    recv(client_socket, recieve_buffer, MAX_INT_SIZE, 0);
    printf("There are %s topics \n",recieve_buffer);
    return atoi(recieve_buffer);
}
int events() { // nº eventos pendientes de recoger por este cliente
    int return_val;
    char *tipo_op = "08";
    char send_buffer[OP_LEN + UUID_SIZE];
    char recieve_buffer[MAX_INT_SIZE];
    sprintf(send_buffer, "%s%s", tipo_op, uuid);
    if (send(client_socket, send_buffer, OP_LEN + UUID_SIZE, 0) < 0) return_val = -1;
    if (recv(client_socket, recieve_buffer, MAX_INT_SIZE, 0) < 0) return_val = -1;
    else return_val = atoi(recieve_buffer);
    printf("There are %s events \n",recieve_buffer);
    return return_val;
}