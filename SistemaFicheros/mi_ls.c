/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Programa que lista el contenido de un directorio
 */

#include "directorios.h"
#define TAMFILA 100
#define TAMBUFFER (TAMFILA * 1000)

int main(int argc, char const *argv[])
{
    const char *camino;
    char buffer[TAMBUFFER];
    char flag, tipo;
    unsigned int total_entradas;
    int num_token = 1;

    if (argc == 3)
    {
        camino = argv[2]; // Simple
        flag = 's';
        if (bmount(argv[1]) == FALLO)
    {
        bumount();
        return FALLO;
    }
    }
    else if (argc == 4 && strcmp(argv[1], "-l") == 0)
    {
        camino = argv[3]; // Long
        flag = 'l';
        if (bmount(argv[2]) == FALLO)
    {
        bumount();
        return FALLO;
    }
    }
    else
    {
        fprintf(stderr, RED "Número de argumentos incorrecto. Sintaxis: ./mi_ls <disco> </ruta> o ./mi_ls -l <disco> </ruta>\n" RESET);
        return FALLO;
    }

    if (camino[strlen(camino) - 1] != '/')
    { // Fichero
        tipo = 'f';
        total_entradas = mi_dir(camino, buffer, tipo, flag);
        if (total_entradas == -1) {
            bumount();
            return FALLO;
        }
    }
    else
    { // Directorio
        tipo = 'd';
        total_entradas = mi_dir(camino, buffer, tipo, flag);
        if (total_entradas == -1) {
            bumount();
            return FALLO;
        }
    }

    char *token;
    char separador[] = "|";
    token = strtok(buffer, separador);
    if (total_entradas > 0)
    {
        if (flag == 'l') //Formato Long
        {
            if (tipo == 'd') // Solo se imprime si es un directorio
            {
                printf("Total: %d\n", total_entradas);
            }
            printf("Tipo\tPermisos\tmTime\t\tTamaño\tNombre\n");
            printf("--------------------------------------------------------------------------------------------\n");
            
            
            
            while (token != NULL)
            {
                printf("%c\t", token[0]); // Primer campo: tipo
                token = strtok(NULL, separador);

                printf("%s\t", token); // Segundo campo: permisos
                token = strtok(NULL, separador);

                printf("%s\t", token); // Tercer campo: mtime
                token = strtok(NULL, separador);

                printf("%s\t", token); // Cuarto campo: tamaño
                token = strtok(NULL, separador);

                printf("%s\n", token);           // Quinto campo: nombre
                token = strtok(NULL, separador); // Avanzar a la siguiente entrada
            }
        } else { // Formato Simple
            if (tipo == 'd')
            {
                printf("Total: %d\n", total_entradas);
            }
            while (token!=NULL) {
                if (num_token % 5 == 0) { //son aquellas posiciones que contienen el nombre, ya que consideremaos que cada 5 posiciones está el nombre.
                    printf("%s\t", token);
                }
                token = strtok(NULL,separador);
                num_token++;
            }


        }
    }

    return 0;
}
