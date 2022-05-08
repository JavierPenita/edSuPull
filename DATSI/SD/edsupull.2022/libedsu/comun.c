/*
 * Incluya en este fichero todas las implementaciones que pueden
 * necesitar compartir el broker y la biblioteca, si es que las hubiera.
 */
#include "comun.h"

#include <unistd.h>
#include <stdio.h>

 // debe usarse para obtener un UUID para el cliente
int generate_UUID(UUID_t uuid) {
    FILE* d = popen("uuidgen", "r");
    if (d) {
        fread(uuid, sizeof(UUID_t), 1, d);
        uuid[sizeof(UUID_t)] = '\0';
    }
    pclose(d);
    return (d ? 0 : -1);
}