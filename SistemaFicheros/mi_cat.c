#include "directorios.h"
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
    
    struct inodo inodo;          // Estructura para almacenar metadatos del fichero
    int tam_buf = 1500*BLOCKSIZE; // Tamaño del buffer (1500 bloques)
    char buffer[tam_buf];        // Buffer para almacenar datos leídos
    int offset;                  // Desplazamiento para lectura
    char string[128];            // Buffer para mensajes
    
    if (argc != 3){
        fprintf(stderr, "Error. Sintaxis correcta: mi_cat <disco> </ruta_fichero>\n");
        return FALLO;
    }
    
    
    char *disco = argv[1];    // Primer argumento: dispositivo virtual
    char *camino = argv[2];   // Segundo argumento: ruta al fichero

    if (camino[strlen(camino)-1] != '/'){ //No es un fichero.
        return FALLO;
    }
    bmount(disco);  // Monta el dispositivo virtual especificado
    
    offset = 0;
    int leidos, total_leidos=0;

    memset(buffer, 0, tam_buf);  // Limpia el buffer
    
    // Obtenemos los metadatos del fichero
    int res;
    unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
    
    // Busca la entrada del fichero para obtener su inodo
    if ((res = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7)) < 0){
        mostrar_error_buscar_entrada(res);
        return res;
    }

    // Lee el fichero por primera vez
    leidos = mi_read_f(p_inodo, buffer, offset, tam_buf);

    // Bucle de lectura mientras haya contenido
    while (leidos > 0){
        write(1, buffer, leidos);       // Escribe en salida estándar (1)
        total_leidos += leidos;         // Acumula bytes leídos
        offset += tam_buf;              // Avanza el offset
        memset(buffer, 0, tam_buf);     // Limpia el buffer
        leidos = mi_read_f(p_inodo, buffer, offset, tam_buf); // Lee siguiente porción
    }
    
    // Muestra el total de bytes leídos por salida de error (2)
    sprintf(string, "\ntotal_leidos %d\n", total_leidos);
    write(2, string, strlen(string));

    
    leer_inodo(p_inodo, &inodo);  // Lee los metadatos del inodo

    // Muestra el tamaño lógico del fichero
    sprintf(string, "tamEnBytesLog %d\n", inodo.tamEnBytesLog);
    write(2, string, strlen(string));
    
    bumount();  // Desmonta el dispositivo
    
    return EXITO;
}