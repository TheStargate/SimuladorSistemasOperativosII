#include "directorios.h"
#include <string.h>
#include <stdlib.h>

/**
 * Programa principal para escribir texto en un fichero dentro de un sistema de ficheros personalizado.
 * 
 * @param argc Número de argumentos de línea de comandos
 * @param argv Array de argumentos de línea de comandos
 * @return 0 si éxito, -1 si error
 */
int main(int argc, char **argv) {
    int nbytes;     // Para almacenar la longitud del texto a escribir
    int escritos;   // Para almacenar el número de bytes escritos realmente

    // Verificamos que se hayan proporcionado exactamente 4 argumentos (más el nombre del programa)
    if (argc != 5) {
        fprintf(stderr, "Error. Sintaxis correcta: mi_escribir <disco> </ruta_fichero> <texto> <offset>");
        return -1;
    }
    
    // Recuperamos los argumentos de la línea de comandos
    char *disco = argv[1];   // Nombre del archivo de disco/dispositivo
    char *camino = argv[2];  // Ruta del fichero donde escribir
    char *texto = argv[3];   // Texto a escribir en el fichero
    int offset = atoi(argv[4]); // Posición desde donde comenzar a escribir (convertido de string a int)

    // Calculamos la longitud del texto a escribir
    nbytes = strlen(texto);
    printf("longitud texto: %d\n\n", nbytes);

    // Creamos un buffer con el texto (podría usarse directamente 'texto')
    char buffer[nbytes];
    strcpy(buffer, texto);
    
    // Montamos el sistema de ficheros
    bmount(disco);
        
    // Intentamos escribir el texto en el fichero especificado
    if ((escritos = mi_write(camino, buffer, offset, nbytes)) < 0) {
        fprintf(stderr, "Error de escritura en el fichero\n");
        return -1;
    }

    // Mostramos cuántos bytes se escribieron realmente
    printf("Bytes escritos: %d\n\n", escritos);
    
    // Desmontamos el sistema de ficheros
    bumount();
    
    return EXITO;
}