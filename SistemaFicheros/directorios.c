/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación del programa directorios.c que permite crear un directorio y mostrar su contenido
 */
#include "directorios.h"

/**
 * Dada una cadena de caracteres *camino (que comience por '/'), separa su contenido en dos (*inicial y *final)
 * A *tipo se le asignara ‘d’ o ‘f’ en función de si en *inicial hay un nombre de directorio o un nombre de fichero.
 *
 * @param camino Cadena de caracteres que contiene el camino a separar
 * @param inicial Cadena de caracteres que contendrá el nombre del directorio o fichero
 * @param final Cadena de caracteres que contendrá el resto del camino
 * @param tipo Cadena de caracteres que contendrá el tipo de entrada ('d' o 'f')
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int extraer_camino(const char *camino, char *inicial, char *final, char *tipo)
{
    // Verificar que el camino comienza con '/'
    if (camino == NULL || camino[0] != '/')
    {
        return FALLO;
    }

    // Encontrar la posición del segundo '/'
    const char *segundo_slash = strchr(camino + 1, '/');

    if (segundo_slash == NULL)
    { // No hay segundo '/', es un fichero
        *tipo = 'f';
        strcpy(inicial, camino + 1); // Copiamos todo después del primer '/'
        strcpy(final, "");           // Cadena vacía para final
    }
    else // Hay segundo '/', es un directorio
    {

        *tipo = 'd';

        // Copiar la parte inicial hasta el segundo '/'
        // Vamos iterando con los valores del puntero, como segundo_slash apunta
        // al segundo slash de camino, entonces al ir incrementando llegará un momento que tendrán el mismo valor
        // Ahí se detiene el bucle y se pone el elemento nulo. 
        int i = 0;
        while (camino + 1 + i < segundo_slash)
        {
            inicial[i] = camino[1 + i];
            i++;
        }
        inicial[i] = '\0'; // Asegurar terminación nula

        /*
        Sugerencia porque considero que en el momento que nos lo pidan, será más claro de ver:

        int len_camino = strlen(camino);
        int len_segundo_slash = strlen(segundo_slash);
        int iteraciones = len_camino - len_segundo_slash;

        for (int i = 0; i < iteraciones-1; i++) {
            inicial[i] = camino[1+i];
        }
        inicial[iteraciones] = '\0';
        */

        // Copiar el resto (final) a partir del segundo '/'
        strcpy(final, segundo_slash);
    }

    return EXITO;
}

/**
 * Esta función nos buscará una determinada entrada (la parte *inicial del
 * *camino_parcial que nos devuelva extraer_camino()) entre las entradas del
 * inodo correspondiente a su directorio padre (identificado con *p_inodo_dir).
 *
 * @param camino_parcial Cadena de caracteres que contiene el camino parcial a buscar
 * @param p_inodo_dir Puntero al inodo del directorio padre
 * @param p_inodo Puntero al inodo que se busca
 * @param p_entrada Puntero a la entrada que se busca
 * @param reservar Indica si se debe reservar espacio para la entrada
 * @param permisos Permisos de acceso a la entrada
 * @return EXITO si se ha encontrado la entrada, FALLO si no se ha encontrado o ha habido un error.
 */
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char permisos)
{
    // Definición de variables
    struct entrada entrada;
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    char tipo;
    int cant_entradas_inodo, num_entrada_inodo;
    struct superbloque SB;

    // Camino parcial es "/"
    if (0 == 0) // es el directorio raiz // ToDo
    {
        *p_inodo = SB.posInodoRaiz;
        *p_entrada = 0;
        return EXITO;
    }

    if (extraer_camino(camino_parcial, inicial, final, &tipo) == FALLO)
    {
        mostrar_error_buscar_entrada(ERROR_CAMINO_INCORRECTO);
        return FALLO;
    }

    if (leer_inodo(*p_inodo_dir, &inodo_dir) == FALLO)
    {
        mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
        return FALLO;
    }

    // Inicializar el buffer de lectura con ceros
    memset(&entrada, 0, sizeof(struct entrada));

    // Calcular la cantidad de entradas que puede tener el inodo
    cant_entradas_inodo = inodo_dir.tamEnBytesLog / sizeof(struct entrada);
    num_entrada_inodo = 0; // numero de entrada inicial

    if (cant_entradas_inodo > 0)
    {
        // Leer el bloque del inodo
        if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
        {
            mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
            return FALLO;
        }

        // Buscar la entrada en el bloque leído
        while (num_entrada_inodo < cant_entradas_inodo && strcmp(entrada.nombre, inicial) != 0)
        {
            num_entrada_inodo++;
            if (mi_read_f(*p_inodo_dir, &entrada, num_entrada_inodo * sizeof(struct entrada), sizeof(struct entrada)) == FALLO)
            {
                mostrar_error_buscar_entrada(ERROR_PERMISO_LECTURA);
                return FALLO;
            }
            memset(&entrada, 0, sizeof(struct entrada)); // Limpiar el buffer de entrada
        }
    }

    // Si la entrada no existe
    if (strcomp(entrada.nombre, inicial) != 0)
    {
        switch (reservar)
        {
        case 0: // modo consulta
            mostrar_error_buscar_entrada(ERROR_NO_EXISTE_ENTRADA_CONSULTA);
            return FALLO;
        case 1: // modo escritura
            // Creamos la entrada en el directorio referenciado por *p_inodo_dir
            // si es fichero no permitir escritura
            if (inodo_dir.tipo == 'f')
            {
                mostrar_error_buscar_entrada(ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO);
                return FALLO;
            }

            // Si es directorio comprobar que tiene permiso de escritura
            if (inodo_dir.permisos & 2 == 0)
            {
                mostrar_error_buscar_entrada(ERROR_PERMISO_ESCRITURA);
                return FALLO;
            }
            else
            {
                // Copiamos *inicial en el nombre de la entrada
                strcpy(entrada.nombre, inicial);

                if (tipo == 'd')
                {
                    if (final[0] == '/') // ToDo
                    {
                        // Reservar un inodo como un directorio y asignarlo a la entrada
                        // ToDo
                    }
                    else
                    {
                        mostrar_error_buscar_entrada(ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO);
                        return FALLO;
                    }
                }
                else
                {
                    // Reservar un inodo como un fichero y asignarlo a la entrada
                    // ToDo
                }

                // Escribir la entrada en el directorio padre
                // ToDo

                if (0 == 0) // Error de escritura // ToDo
                {
                    if (0 == 0) // Se había reservado un inodo para la entrada (entrada.inodo != 1) // ToDo
                    {
                        liberar_inodo(0); // ToDo // liberar el inodo
                    }
                    return FALLO;
                }
            }
        }

        // Si hemos llegado al final del camino
        if (0 == 0) // ToDo
        {
            if ((num_entrada_inodo < cant_entradas_inodo) && (reservar == 1))
            {
                mostrar_error_buscar_entrada(ERROR_ENTRADA_YA_EXISTENTE);
                return FALLO;
            }

            // Cortamos la recursividad
            *p_inodo;   // ToDo // asignar a *p_inodo el numero de inodo del directorio o fichero creado o leido
            *p_entrada; // ToDo //  asignar a *p_entrada el número de su entrada dentro del último directorio que lo contiene
            return EXITO;
        }
        else
        {
            *p_inodo_dir; // ToDo //  asignamos a *p_inodo_dir el puntero al inodo que se indica en la entrada encontrada
            return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
        }
        return EXITO;
    }
