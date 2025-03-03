/*
 * Autores: [Aaron Satyar Daghigh-Nia Tudor, Jaume Juan Huguet, Miguel Sansó Febrer]
 * Descripción: Implementación de funciones para gestionar un sistema de ficheros
 */

#include "ficheros_basico.h"

/**
 * Función para calcular el tamaño en bloques necesario para el mapa de bits.
 *
 * @param nbloques Número de bloques del dispositivo virtual.
 * @return Tamaño en bloques necesario para el mapa de bits.
 */
int tamMB(unsigned int nbloques)
{

    if (nbloques % BYTE == 0 && nbloques / BYTE % BLOCKSIZE == 0)
    {
        return nbloques / BYTE / BLOCKSIZE;
    }
    else
    {
        return nbloques / BYTE / BLOCKSIZE + 1;
    }
}

/**
 * Función para calcular el tamaño en bloques del array de inodos.
 *
 * @param ninodos Número de inodos del dispositivo virtual.
 * @return Tamaño en bloques del array de inodos.
 */
int tamAI(unsigned int ninodos)
{

    if (ninodos % (BLOCKSIZE / INODOSIZE) == 0)
    {
        return ninodos / (BLOCKSIZE / INODOSIZE);
    }
    else
    {
        return ninodos / (BLOCKSIZE / INODOSIZE) + 1;
    }
}

/**
 * Función para inicializar los datos del superbloque.
 *
 * Llama a: tamMB(), tamAI() y bwrite()
 *
 * @param nbloques Número de bloques del dispositivo virtual.
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;

    SB.posPrimerBloqueMB = posSB + tamSB; // posSB = 0, tamSB = 1
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;

    if (bwrite(posSB, &SB) == FALLO)
        return FALLO;
    return EXITO;
}

/**
 * Función para inicializar el mapa de bits poniendo a 1 los bits que representan los metadatos.
 *
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initMB()
{
    struct superbloque SB;
    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Número de bloques que ocupan los metadatos
    int metadatos = tamSB + tamMB(SB.totBloques) + tamAI(SB.totInodos);

    // Inicializamos bufferMB
    unsigned char bufferMB[BLOCKSIZE];
    memset(bufferMB, 255, BLOCKSIZE);

    // Miramos la cantidad de bloques que ocupan los bits de los metadatos
    int bloquesMeta = tamMB(metadatos);

    // Si hay más de un bloque, rellenamos los bits de todos los bloques a 1 menos el último
    for (int i = 0; i < bloquesMeta - 1; i++)
    {
        if (bwrite(SB.posPrimerBloqueMB + i, bufferMB) == FALLO)
            return FALLO;
    }

    memset(bufferMB, 0, BLOCKSIZE);

    // Ponemos los bits necesarios del último bloque a 1
    int numBits1 = metadatos / BYTE - (bloquesMeta - 1) * BLOCKSIZE;
    for (int i = 0; i < numBits1; i++)
    {
        bufferMB[i] = 255;
    }

    // Si la división no es exacta, añadimos los bits que falten
    char resto = 0;
    for (int i = metadatos % BYTE; i > 0; i--)
    {
        resto += (1 << (BYTE - i));
    }

    bufferMB[numBits1] = resto;

    if (bwrite(SB.posPrimerBloqueMB + bloquesMeta - 1, bufferMB) == FALLO)
        return FALLO;

    // Reducimos la cantidad de bloques libres
    SB.cantBloquesLibres -= metadatos;

    if (bwrite(posSB, &SB) == FALLO)
        return FALLO;

    return EXITO;
}

/**
 * Función para inicializar la lista de inodos libres.
 *
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int initAI()
{
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    struct superbloque SB;

    // Leer el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    int contInodos = SB.posPrimerInodoLibre + 1; // Inicializado posPrimerInodoLibre = 0.

    // Iterar sobre los bloques del array de inodos
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    {
        // Leer el bloque de inodos desde el dispositio virtual
        if (bread(i, inodos) == FALLO)
            return FALLO;
        // Iterar sobre los inodos dentro del bloque
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
        {
            inodos[j].tipo = 'l'; // Libre
            if (contInodos < SB.totInodos)
            {
                inodos[j].punterosDirectos[0] = contInodos; // Enlazar con el siguiente
                contInodos++;
            }
            else
            { // hemos llegado al último nodo.
                inodos[j].punterosDirectos[0] = UINT_MAX;
                break; // Úlitmo bloque no tiene porque estar completo.
            }
        } // Escribir el bloque de inodos actualizado en el dispositivo
        if (bwrite(i, inodos) == FALLO)
            return FALLO;
    }

    if (bwrite(posSB, &SB) == FALLO)
        return FALLO;

    return EXITO;
}

/**
 * Escribe en el bloque indicado por MB seleccionado un bit para indicar si está ocupado (1) o libre (0)
 *
 * @param nbloque Posición de MB que queremos modificar
 * @param bit Valor que se le asignará al bit correspondiente (1 o 0)
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    struct superbloque SB;

    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Calculamos donde se ecuentra el bit correspondiente a nbloque en MB
    int posbyte = nbloque / BYTE;
    int posbit = nbloque % BYTE;

    // Miramos donde se encuentra el bit a nivel absoluto
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // Inicializamos bufferMB
    unsigned char bufferMB[BLOCKSIZE];

    // Leemos el bloque físico que contiene el bit que queremos modificar
    if (bread(nbloqueabs, bufferMB) == FALLO)
        return FALLO;

    // Obtenemos el byte que contiene el bit a modificar
    posbyte = posbyte % BLOCKSIZE;

    // Modificamos el bit
    unsigned char mascara = 128; // 10000000
    mascara >>= posbit;          // desplazamiento de bits a la derecha
    if (bit == 1)
    {
        bufferMB[posbyte] |= mascara; // OR
    }
    else
    {
        bufferMB[posbyte] &= ~mascara; // AND y NOT
    }

    // Escribimos el bloque físico con el bit modificado
    if (bwrite(nbloqueabs, bufferMB) == FALLO)
        return FALLO;

    return EXITO;
};

/**
 * Lee un determinado bit del MB y devuelve el valor del bit leído.
 *
 * @param nbloque Posición de MB que queremos leer
 * @return Valor del bit correspondiente (1 o 0)
 */
char leer_bit(unsigned int nbloque)
{
    struct superbloque SB;

    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Calculamos donde se ecuentra el bit correspondiente a nbloque en MB
    int posbyte = nbloque / BYTE;
    int posbit = nbloque % BYTE;

    // Miramos donde se encuentra el bit a nivel absoluto
    int nbloqueMB = posbyte / BLOCKSIZE;
    int nbloqueabs = SB.posPrimerBloqueMB + nbloqueMB;

    // Inicializamos bufferMB
    unsigned char bufferMB[BLOCKSIZE];

    // Leemos el bloque físico que contiene el bit que queremos modificar
    if (bread(nbloqueabs, bufferMB) == FALLO)
        return FALLO;

    // Obtenemos el byte que contiene el bit a leer
    posbyte = posbyte % BLOCKSIZE;

    unsigned char mascara = 128;  // 10000000
    mascara >>= posbit;           // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyte]; // operador AND para bits
    mascara >>= (7 - posbit);     // desplazamiento de bits a la derecha
                                  // para dejar el 0 o 1 en el extremo derecho y leerlo en decimal
    return mascara;
}

/**
 * Encuentra el primer bloque libre, consultando el MB (primer bit a 0), lo ocupa (poniendo el correspondiente bit a 1 con la ayuda de la función escribir_bit()) y devuelve su posición.
 *
 * @param nbloque Posición de MB que queremos leer
 * @return Valor del bit correspondiente (1 o 0)
 */
int reservar_bloque()
{
    struct superbloque SB;
    unsigned char bufferAux[BLOCKSIZE];
    unsigned char bufferMB[BLOCKSIZE];
    int nbloqueMB = 0;
    int res = 0;
    if (SB.cantBloquesLibres == 0)
    { // Comprobamos la variable de superbloque para saber si quedan bloques libres.
        return FALLO;
    }

    memset(bufferAux, 255, BLOCKSIZE); // Llenamos el buffer auxilar con bits a 1.

    while (nbloqueMB < SB.posUltimoBloqueMB && res == 0)
    { // Vamos iterando por todos los bloques.
        if (bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == -1) {
            return FALLO;
        }
        res = memcmp(bufferAux, bufferMB, BLOCKSIZE); // Si después de la comparación dentro de res queda un valor distinto de 0.
        nbloqueMB++;                                  // significa que ha quedado algun bit libre, es decir con 0 y salimos del bucle teniendo dicho bloque en el bufferMB.
    }

    int posbyte = 0; // Vamos recorriendo todos los bytes del bloque y comparamos si el byte, es igual a 255 que significa que es todo 1.
    while (bufferMB[posbyte] == 255 && posbyte < BLOCKSIZE)
    { // Seguirá hasta que llegue al final del bloque.
        posbyte++;
    }

    unsigned char mascara = 128; // 10000000
    int posbit = 0;
    while (bufferMB[posbyte] & mascara)
    { // Operacion de AND entre máscara y valor de byte.
        bufferMB[posbyte] <<= 1;
        posbit++;
    }
    int nbloque = (nbloqueMB * BLOCKSIZE * 8) + (posbyte * 8) + posbit;
    escribir_bit(nbloque, 1);
    SB.cantBloquesLibres--; // Decrementamos cantidad de bloques libres.
    if (bwrite(0, &SB) == -1) {
        return FALLO; // Error al salvar el superbloque
    }
    unsigned char bufferCero[BLOCKSIZE];
    memset(bufferCero, 0, BLOCKSIZE); // Llenamos de ceros
    if (bwrite(nbloque + SB.posPrimerBloqueDatos, bufferCero) == -1)
    {
        return FALLO; // Error al limpiar el bloque de datos
    }
    return nbloque;
}

/**
 * Libera un bloque determinado (con la ayuda de la función escribir_bit()).
 *
 * @param ninodo Número de inodo que queremos escribir
 * @param inodo Puntero a la estructura inodo que queremos escribir
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int liberar_bloque(unsigned int nbloque)
{
    return EXITO;
}

/**
 * Escribe el contenido de una variable de tipo struct inodo, pasada por referencia, en un determinado inodo del array de inodos, inodos.
 *
 * @param ninodo Número de inodo que queremos leer
 * @param inodo Puntero a la estructura inodo donde queremos volcar el inodo leído
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int escribir_inodo(unsigned int ninodo, struct inodo *inodo)
{
    return EXITO;
}

/**
 * Lee un determinado inodo del array de inodos para volcarlo en una variable de tipo struct inodo pasada por referencia.
 *
 * @param ninodo Número de inodo que queremos leer
 * @param inodo Puntero a la estructura inodo donde queremos volcar el inodo leído
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    return EXITO;
}

/**
 * Encuentra el primer inodo libre (dato almacenado en el superbloque), lo reserva (con la ayuda de la función escribir_inodo()), devuelve su número y actualiza la lista enlazada de inodos libres.
 *
 * @param tipo Tipo de inodo que queremos reservar
 * @param permisos Permisos del inodo que queremos reservar
 * @return Número de inodo reservado
 */
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    return 7;
}