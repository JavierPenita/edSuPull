int begin_clnt(void){
    int s,escrito, aux;
    int Long = strlen(generate_UUID(uuid)); // + caracter nulo final
    int Datos[] = generate_UUID(uuid);
   /* socket(AF_INET vale para los dos casos, misma maquina o distintas
    ,  si el socket es orientado a conexi�n (SOCK_STREAM)
    ,  protocolo a emplear, en nuestro caso IPPROTO_TCP))

    devuelve un entero que es el descriptor de fichero o �1 si ha habido alg�n error*/
    if ((s=socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("error creando socket");
		return -1;
	}
    /*getaddrinfo(Puntero al nombre del host (una direccion como 127.0.....)
                , Numero de puerto pasado como cadena
                , NULL (Estructura para UDP o TCP)
                , puntero que se apuntar� a un nuevo ?addrinfo)

    devuelve 0 si �xito y negativo si se produce un error en el camino*/
    struct addrinfo *res; 
        if (getaddrinfo(BROKER_HOST, BROKER_PORT, NULL, &res) != 0) { 
                perror("error en getaddrinfo");
                close(s);
                return -1;
        }
        /* connect(Descriptor en nuestro caso s
        , res->ai_addr
        , res->ai_addrlen)
        devuelve 0 si �xito y negativo si se produce un error en el camino*/
         if (connect(s, res->ai_addr, res->ai_addrlen) < 0) {
                perror("error en connect");
                close(s);
                return -1;
        }
        freeaddrinfo(res);
        /*write() admite como par�metros
        -Descriptor del fichero / socket del que se quiere leer
        -Puntero a char donde se almacenar�n los datos le�dos
        -N�mero de caracteres que se quieren leer.
        */
   while (escrito < Long){

     if ((aux=writev(s, (char)Datos + escrito, Long - escrito )) < 0) {
        	perror("error en writev");
        	close(s);
        	return -1;
     } 
     //Si no da error
     escrito = escrito + Aux; 
   } 
	printf("escrito %d\n", escrito);
    return 0;
}