/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que muestra recursivamente el contenido en forma de árbol
 */

#include "directorios.h"
#include <string.h>
#include <stdio.h>

#define MAX_ENTRADAS 1024

void imprimir_indentacion(int nivel)
{
    for (int i = 0; i < nivel; i++)
    {
        printf("│   ");
    }
}

// Función recursiva que recorre un árbol de directorios e imprime su estructura
int recorrer_arbol(const char *camino, int nivel)
{

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;

    struct inodo inodo;
    struct entrada entradas[MAX_ENTRADAS];

    if (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7) == FALLO)
    {
        return FALLO;
    }

    if (leer_inodo(p_inodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    if (inodo.tipo != 'd')
        return FALLO; // Solo nos interesan directorios

    // Calculamos el número de entradas (ficheros o subdirectorios) dentro del directorio
    unsigned int n_entradas = inodo.tamEnBytesLog / sizeof(struct entrada);
    if (n_entradas == 0)
        return FALLO;

    // Leemos todas las entradas del directorio
    int bytes_leidos = mi_read(camino, entradas, 0, sizeof(struct entrada) * n_entradas);
    if (bytes_leidos < 0)
    {
        return FALLO;
    }

    // Recorremos todas las entradas
    for (int i = 0; i < n_entradas; i++)
    {
        struct entrada ent = entradas[i];
        struct inodo inodo_ent;

        if (leer_inodo(ent.ninodo, &inodo_ent) == FALLO)
            continue;

        // Imprimimos la entrada con su indentación
        imprimir_indentacion(nivel);
        printf("├── %s\n", ent.nombre);

        // Si la entrada es un directorio, la recorremos recursivamente
        if (inodo_ent.tipo == 'd')
        {
            // Construimos el nuevo camino para la llamada recursiva
            char nuevo_camino[1024];
            if (strcmp(camino, "/") == 0)
                snprintf(nuevo_camino, sizeof(nuevo_camino), "/%s/", ent.nombre);
            else
                snprintf(nuevo_camino, sizeof(nuevo_camino), "%s%s/", camino, ent.nombre);

            recorrer_arbol(nuevo_camino, nivel + 1);
        }
    }
    return EXITO;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, RED "Sintaxis: %s <disco> </ruta>\n" RESET, argv[0]);
        return FALLO;
    }

    if (bmount(argv[1]) == FALLO)
    {
        bumount();
        return FALLO;
    }

    printf("%s\n", argv[2]);
    recorrer_arbol(argv[2], 0);

    bumount();
}