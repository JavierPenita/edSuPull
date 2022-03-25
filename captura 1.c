int end_clnt(void){

  if( != "mas operaciones" ) //TODO
       close(c);
    return 0;
}

int Trader(const void *msgIda, void **msgVuelta){
int conn = begin_clnt();
end_clnt();
}

int subscribe(const char *tema){
    int trade = Trader(tema, NULL);
    return trade; 
}
int unsubscribe(const char *tema){
    int trade = Trader(tema, NULL);
    return trade; 
}
int clients(){ // cuántos clientes existen en el sistema
    int trade = Trader("\0", NULL);
    return trade;    
}