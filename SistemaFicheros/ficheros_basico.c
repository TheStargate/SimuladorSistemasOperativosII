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

    int contInodos = SB.posPrimerInodoLibre + 1; // Inicializado posPrimerinodoLibre = 0.

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
 * Escribe en el bloque indicado de MB un bit para indicar si está ocupado (1) o libre (0)
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
    int posbyteAjustado = posbyte % BLOCKSIZE;

    unsigned char mascara = 128;          // 10000000
    mascara >>= posbit;                   // desplazamiento de bits a la derecha, los que indique posbit
    mascara &= bufferMB[posbyteAjustado]; // operador AND para bits
    mascara >>= (7 - posbit);             // desplazamiento de bits a la derecha
    // para dejar el 0 o 1 en el extremo derecho y leerlo en decimal

#if DEBUGN3
    printf(GRAY "\n[leer_bit(%u)→ posbyte:%d, posbyte (ajustado): %d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]" RESET, nbloque, posbyte, posbyteAjustado, posbit, nbloqueMB, nbloqueabs);
#endif

    return mascara;
}

/**
 * Encuentra el primer bloque libre, consultando el MB (primer bit a 0), lo ocupa (poniendo el correspondiente bit a 1 con la ayuda de la función escribir_bit()) y devuelve su posición.
 *
 * @return Posición del bloque reservado
 */
int reservar_bloque()
{
    struct superbloque SB;
    unsigned char bufferAux[BLOCKSIZE];
    unsigned char bufferMB[BLOCKSIZE];
    int nbloqueMB = 0;
    int res = 0;

    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    if (SB.cantBloquesLibres == 0)
    { // Comprobamos la variable de superbloque para saber si quedan bloques libres.
        return FALLO;
    }

    memset(bufferAux, 255, BLOCKSIZE); // Llenamos el buffer auxilar con bits a 1.

    while (nbloqueMB < SB.posUltimoBloqueMB && res == 0)
    { // Vamos iterando por todos los bloques.
        if (bread(nbloqueMB + SB.posPrimerBloqueMB, bufferMB) == FALLO)
        {
            return FALLO;
        }
        res = memcmp(bufferAux, bufferMB, BLOCKSIZE); // Si después de la comparación dentro de res queda un valor distinto de 0.
                                                      // significa que ha quedado algun bit libre, es decir con 0 y salimos del bucle teniendo dicho bloque en el bufferMB.
        if (res == 0)
        {
            nbloqueMB++;
        }
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

    int nbloque = (nbloqueMB * BLOCKSIZE * BYTE) + (posbyte * BYTE) + posbit;
    escribir_bit(nbloque, 1);

    SB.cantBloquesLibres--; // Decrementamos cantidad de bloques libres.

    unsigned char bufferCero[BLOCKSIZE];
    memset(bufferCero, 0, BLOCKSIZE); // Llenamos de ceros
    if (bwrite(nbloque + SB.posPrimerBloqueDatos, bufferCero) == FALLO)
    {
        return FALLO; // Error al limpiar el bloque de datos
    }

    // Escribir el superbloque actualizado
    if (bwrite(posSB, &SB) == FALLO)
        return FALLO;

    return nbloque;
}

/**
 * Libera un bloque determinado (con la ayuda de la función escribir_bit()).
 *
 * @param nbloque Número de bloque que queremos liberar
 * @return EXITO si todo ha ido bien, FALLO si ha habido algún error.
 */
int liberar_bloque(unsigned int nbloque)
{
    struct superbloque SB;

    // Leer el superbloque
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Poner a 0 el bit correspondiente al bloque
    if (escribir_bit(nbloque, 0) == FALLO)
        return FALLO;

    // Incrementar la cantidad de bloques libres
    SB.cantBloquesLibres++;

    // Escribir el superbloque actualizado
    if (bwrite(posSB, &SB) == FALLO)
        return FALLO;

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
    struct superbloque SB;

    // Leemos el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Obtenemos el nº de bloque del array de inodos que tiene el inodo solicitado
    int nbloqueAI = ninodo / (BLOCKSIZE / INODOSIZE);

    // Calculamos la posición absoluta del bloque en el dispositivo
    int nbloqueabs = SB.posPrimerBloqueAI + nbloqueAI;

    // Declaramos un buffer para leer el bloque
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Leemos el bloque del disco
    if (bread(nbloqueabs, inodos) == FALLO)
        return FALLO;

    // Calculamos la posición relativa del inodo dentro del bloque
    int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);

    // Copiamos el inodo en la posición correspondiente
    inodos[posinodo] = *inodo;

    // Escribimos el bloque modificado de vuelta al disco
    if (bwrite(nbloqueabs, inodos) == FALLO)
        return FALLO;

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
    struct superbloque SB;

    // Leemos el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    // Obtenemos el nº de bloque del array de inodos que tiene el inodo solicitado
    int nbloqueAI = ninodo / (BLOCKSIZE / INODOSIZE);

    // Calculamos la posición absoluta del bloque en el dispositivo
    int nbloqueabs = SB.posPrimerBloqueAI + nbloqueAI;

    // Declaramos un buffer para leer el bloque
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    // Leemos el bloque del disco
    if (bread(nbloqueabs, inodos) == FALLO)
        return FALLO;

    // Calculamos la posición relativa del inodo dentro del bloque
    int posinodo = ninodo % (BLOCKSIZE / INODOSIZE);

    // Volcamos el inodo correspondiente
    *inodo = inodos[posinodo];

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
    struct superbloque SB;

    // Leemos el superbloque para obtener la localización del array de inodos
    if (bread(posSB, &SB) == FALLO)
        return FALLO;

    if (SB.cantInodosLibres == 0)
    { // Comprobamos si hay inodos libres, si no los hay devolvemos error.
        printf(RED "No hay inodos libres");
        printf(RESET);
        return FALLO;
    }
    int posInodoReservado;
    posInodoReservado = SB.posPrimerInodoLibre; // Guardamos en variable auxiliar la posición del primer inodo libre, es decir el que reservaremos.
    struct inodo inodoNuevo;
    if (leer_inodo(posInodoReservado, &inodoNuevo) == FALLO) // Ahora volcamos el inodo reservado dentro de nuestra variable.
        return FALLO;

    SB.posPrimerInodoLibre = inodoNuevo.punterosDirectos[0]; // El SB apunta al siguiente inodo, ya que cada inodo contiene la dirección del siguiente inodo en la lista enlazada.
    inodoNuevo.tipo = tipo;
    inodoNuevo.permisos = permisos;
    inodoNuevo.nlinks = 1;
    inodoNuevo.tamEnBytesLog = 0;
    inodoNuevo.atime = time(NULL);
    inodoNuevo.ctime = time(NULL);
    inodoNuevo.mtime = time(NULL);
    inodoNuevo.btime = time(NULL);
    inodoNuevo.numBloquesOcupados = 0;

    // Inicializamos todo con ceros.
    memset(inodoNuevo.punterosDirectos, 0, sizeof(inodoNuevo.punterosDirectos));
    memset(inodoNuevo.punterosIndirectos, 0, sizeof(inodoNuevo.punterosIndirectos));

    escribir_inodo(posInodoReservado, &inodoNuevo);
    SB.cantInodosLibres--;

    // Escribir el superbloque actualizado
    if (bwrite(posSB, &SB) == FALLO)
        return FALLO;

    return posInodoReservado;
}

/**
 * Esta función se encarga de obtener el nº de bloque físico correspondiente a un bloque lógico determinado del inodo indicado.
 * Enmascara la gestión de los diferentes rangos de punteros directos e indirectos del inodo, de manera que funciones externas
 * no tienen que preocuparse de cómo acceder a los bloques físicos apuntados desde el inodo.
 *
 * @param ninodo Número de inodo que queremos leer
 * @param nblogico Número de bloque lógico que queremos traducir
 * @param reservar Indica si se debe reservar el bloque en caso de que no exista
 * @return Número de bloque físico correspondiente al bloque lógico indicado
 */
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar)
{
    unsigned int ptr = 0, ptr_ant = 0, salvar_inodo = 0, indice = 0;
    int nRangoBL, nivel_punteros;
    unsigned int buffer[NPUNTEROS];
    struct inodo inodo;

    if (leer_inodo(ninodo, &inodo) == FALLO)
        return FALLO;
    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    nivel_punteros = nRangoBL;                           // El nivel_punteros más alto es el que cuelga directamente del inodo

    while (nivel_punteros > 0) // Iterar para cada nivel de punteros indirectos
    {
        if (ptr == 0) // No cuelgan bloques de punteros
        {
            if (reservar == 0) // Bloque inexistente
                return FALLO;
            // reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
            ptr = reservar_bloque(); // de punteros
            inodo.numBloquesOcupados++;
            inodo.ctime = time(NULL); // Fecha actual
            salvar_inodo = 1;
            if (nivel_punteros == nRangoBL)
            { // El bloque cuelga directamente del inodo
                inodo.punterosIndirectos[nRangoBL - 1] = ptr;
#if DEBUGN5
                printf(GRAY "\n[traducir_bloque_inodo()→ inodo.punterosIndirectos[%u] = %u (reservado BF %u para punteros_nivel%u)]" RESET, nRangoBL - 1, ptr, ptr, nivel_punteros);
#endif
            }
            else
            { // El bloque cuelga de otro bloque de punteros
                buffer[indice] = ptr;
                if (bwrite(ptr_ant, buffer) == FALLO)
                { // Salvamos en el dispositivo el buffer de punteros modificado
                    return FALLO;
                }
#if DEBUGN5
                printf(GRAY "\n[traducir_bloque_inodo()→ punteros_nivel%u [%u] = %u (reservado BF %u para punteros_nivel%u)]" RESET, nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);
#endif
            }
            memset(buffer, 0, BLOCKSIZE); // Ponemos a 0 todos los punteros del buffer
        }
        else
        {
            bread(ptr, buffer); // Leemos del dispositivo el bloque de punteros ya existente
        }
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        // Guardamos el puntero actual
        ptr = buffer[indice]; // Lo desplazamos al siguiente nivel
        nivel_punteros--;

    } // Al salir de este bucle ya estamos al nivel de datos
    if (ptr == 0)
    {                      // No existe bloque de datos
        if (reservar == 0) // Error lectura ∄ bloque
        {
            return FALLO;
        }
        ptr = reservar_bloque(); // De datos
        inodo.numBloquesOcupados++;
        inodo.ctime = time(NULL);
        salvar_inodo = 1;
        if (nRangoBL == 0)
        {                                           // Si era puntero directo
            inodo.punterosDirectos[nblogico] = ptr; // Asignamos la dirección del  bloque de datos en el inodo
#if DEBUGN5
            printf(GRAY "\n[traducir_bloque_inodo()→ inodo.punterosDirectos[%u] = %u (reservado BF %u para BL %u)]" RESET, nblogico, ptr, ptr, nblogico);
#endif
        }
        else
        {
            buffer[indice] = ptr;    // Asignamos la dirección del bloque de datos en el buffer
            bwrite(ptr_ant, buffer); // Salvamos en el dispositivo el buffer de punteros modificado
#if DEBUGN5
            printf(GRAY "\n[traducir_bloque_inodo()→ punteros_nivel%u [%u] = %u (reservado BF %u para BL %u)]" RESET, nivel_punteros + 1, indice, ptr, ptr, nblogico);
#endif
        }
    }

    if (salvar_inodo)
    {
        escribir_inodo(ninodo, &inodo);
    }

    return ptr;
}

/**
 * Generaliza la obtención de los índices de los bloques de punteros.
 *
 * @param nblogico Número de bloque lógico que queremos traducir
 * @param nivel_punteros Nivel de punteros en el que nos encontramos
 * @return Índice correspondiente al bloque de punteros
 */
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    // Calculamos el ndice a partir del nivel de punteros en el que nos encontramos
    if (nblogico < DIRECTOS) // <12
    {
        return nblogico;
    }
    else if (nblogico < INDIRECTOS0) // <268
    {
        return nblogico - DIRECTOS;
    }
    else if (nblogico < INDIRECTOS1) // <65.804
    {
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
        else
        {
            return FALLO;
        }
    }
    else if (nblogico < INDIRECTOS2) // <16.843.020
    {
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }

        else if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
        else
        {
            return FALLO;
        }
    }
    printf(RED "Bloque lógico fuera de rango");
    printf(RESET);
    return FALLO;
}

/**
 * Asocia un rango para cada conjunto de bloques lógicos.
 *
 * @param inodo Número de inodo que queremos leer
 * @param nblogico Número de bloque lógico que queremos traducir
 * @param ptr Puntero a la dirección de memoria donde se almacenará el bloque físico correspondiente
 * @return Número de rango asociado al bloque lógico indicado
 */
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS) // <12
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0) // <268
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1) // <65.804
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2) // <16.843.020
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }

    *ptr = 0;
    printf(RED "Bloque lógico fuera de rango");
    printf(RESET);
    return FALLO;
}

/**
 * Liberar un inodo implica por un lado, que tal inodo pasará a la cabeza de la lista de inodos libres
 * (actualizando el campo SB.posPrimerInodoLibre) y tendremos un inodo más libre en el sistema,
 * SB.cantInodosLibres, y por otro lado, habrá que recorrer la estructura de enlaces del inodo para
 * liberar todos aquellos bloques de datos que estaba ocupando, más todos aquellos bloques índice que
 * se hubieran creado para apuntar a esos bloques.
 *
 * @param ninodo Número de inodo que queremos liberar
 * @return Número de inodo liberado
 */
int liberar_inodo(unsigned int ninodo)
{
    struct inodo inodo;
    struct superbloque SB;

    // Leemos el inodo
    if (leer_inodo(ninodo, &inodo) == FALLO)
        return FALLO;

    // Liberamos todos los bloques del inodo (numBloquesOcupados debe quedar igual a 0)
    int liberados = liberar_bloques_inodo(0, &inodo);
    inodo.numBloquesOcupados = inodo.numBloquesOcupados - liberados;
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    // Leemos el superbloque
    if (bread(posSB, &SB) == FALLO)
    {
        return FALLO;
    }

    // Incluimos el inodo en la lista de inodos libres actualizando punteros
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    SB.cantInodosLibres++;
    if (bwrite(posSB, &SB) == FALLO)
    {
        return FALLO;
    }

    inodo.ctime = time(NULL);

    if (escribir_inodo(ninodo, &inodo) == FALLO)
    {
        return FALLO;
    }

    return ninodo;
}

/**
 * Libera todos los bloques ocupados (con la ayuda de la función liberar_bloque())
 * a partir del primer bloque lógico indicado por el argumento primerBL.
 * Explora las ramificaciones de punteros desde las hiojas hacia la raíz en el inodo
 *
 * @param primerBL Primer bloque lógico a liberar
 * @param inodo Puntero al inodo que contiene los bloques a liberar
 * @return Número de bloques liberados
 */
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    unsigned int nivel_punteros = 0, indice = 0, ptr = 0, nBL, ultimoBL;
    int nRangoBL, contador_bwrites = 0, contador_breads = 0;
    unsigned int bloques_punteros[3][NPUNTEROS]; // array de bloques de punteros por niveles
    unsigned int bufAux_punteros[NPUNTEROS];     // para llenar de 0s y comparar
    memset(bufAux_punteros, 0, BLOCKSIZE);
    int ptr_nivel[3];  // punteros a bloques de punteros de cada nivel
    int indices[3];    // indices de cada nivel
    int liberados = 0; // nº de bloques liberados

    if (inodo->tamEnBytesLog == 0) // el fichero está vacío
        return liberados;

    // Calculamos el último bloque lógico ocupado del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0)
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE - 1;
    else
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;

#if DEBUGN6
    fprintf(stderr, BLUE "\n[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]" RESET, primerBL, ultimoBL);
#endif

    // recorrido BLs hasta EOF
    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    {

        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
        if (nRangoBL < 0)
            return FALLO;
        nivel_punteros = nRangoBL; // el nivel_punteros +alto cuelga del inodo

        while (ptr > 0 && nivel_punteros > 0) // cuelgan bloques de punteros
        {
            indice = obtener_indice(nBL, nivel_punteros);
            if (indice == 0 || nBL == primerBL)
            {
                // solo leemos del dispositivo si no está ya cargado previamente en un buffer
                bread(ptr, bloques_punteros[nivel_punteros - 1]);
                contador_breads++;
            }
            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;
            ptr = bloques_punteros[nivel_punteros - 1][indice];
            nivel_punteros--;
        }

        if (ptr > 0) // si existe bloque de datos
        {
            // Liberamos los bloques de datos ocupados
            liberar_bloque(ptr);
            liberados++;
#if DEBUGN6
            if (nivel_punteros == 0)
            {
                fprintf(stderr, GRAY "\n[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]" RESET, ptr, nBL);
            }
            else
            {
                fprintf(stderr, GRAY "\n[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]" RESET, ptr, nivel_punteros, nBL);
            }
#endif

            // Miramos si es un puntero directo
            if (nRangoBL == 0)
            {
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                while (nivel_punteros <= nRangoBL)
                {
                    indice = indices[nivel_punteros - 1];
                    bloques_punteros[nivel_punteros - 1][indice] = 0;
                    ptr = ptr_nivel[nivel_punteros - 1];

                    // Comprobamos si quedan más punteros ocupados
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0)
                    {
                        // No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                        liberar_bloque(ptr);
                        liberados++;
#if DEBUGN6
                        if (nivel_punteros == 0)
                        {
                            fprintf(stderr, GRAY "\n[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]" RESET, ptr, nBL);
                        }
                        else
                        {
                            fprintf(stderr, GRAY "\n[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d correspondiente al BL %d]" RESET, ptr, nivel_punteros, nBL);
                        }
#endif

                        // MEJORA 1 : Saltar los bloques lógicos que ya no es necesario explorar
                        // al haber eliminado un bloque de punteros
                        /*
#if DEBUGN6
                        int nBLOriginal = nBL + 1;
#endif
                        if (nivel_punteros == 1)
                        {
                            // Calculamos el módulo a partir del bloque actual pamedir el número de bloques a saltar
                            int modulo = (nBL - DIRECTOS) % (NPUNTEROS); // % 256
                            int salto = (INDIRECTOS0 - DIRECTOS) - modulo;
                            nBL += salto - 1;
                        }
                        else if (nivel_punteros == 2)
                        {
                            int modulo = (nBL - DIRECTOS) % ((NPUNTEROS * NPUNTEROS));
                            int salto = (INDIRECTOS1 - DIRECTOS) - modulo;
                            nBL += salto - 1;
                        }
                        else if (nivel_punteros == 3)
                        {
                            int modulo = (nBL - DIRECTOS) % ((NPUNTEROS * NPUNTEROS * NPUNTEROS));
                            int salto = (INDIRECTOS2 - DIRECTOS) - modulo;
                            nBL += salto - 1;
                        }
                            */

#if DEBUGN6
                        fprintf(stderr, GREEN "\n[liberar_bloques_inodo()→ Saltamos del BL %d al BL %d]" RESET, nBLOriginal, nBL);
#endif

                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                        }
                        nivel_punteros++;
                    }
                    else
                    {
                        // escribimos en el dispositivo el bloque de punteros modificado
                        bwrite(ptr, bloques_punteros[nivel_punteros - 1]);
                        contador_bwrites++;

#if DEBUGN6
                        fprintf(stderr, ORANGE "\n[liberar_bloques_inodo()→ salvado BF %d de punteros_nivel%d correspondiente al BL %d]" RESET, ptr, nivel_punteros, nBL);
#endif

                        // Salimos del bucle (no es necesario liberar los bloques de niveles superiores a los que cuelga)
                        nivel_punteros = nRangoBL + 1;
                    }
                }
            }
        }
        else if (nBL >= DIRECTOS)
        {
            /*
            // MEJORA 2:  saltar los BLs que no se necesite explorar
            // cuando se pone a 0 un puntero a bloque de punteros

#if DEBUGN6
            int nBLOriginal = nBL;
#endif
            int salto = 0;

            for (int i = indice; i < NPUNTEROS; i++)
            {
                if (bloques_punteros[nivel_punteros][i] == 0)
                {
                    salto++;
                }
                else
                {
                    break;
                }
            }

            if (nivel_punteros == 0)
            {
                nBL += salto - 1;
            }
            else if (nivel_punteros == 1)
            {
                nBL += salto * NPUNTEROS - 1;
            }
            else if (nivel_punteros == 2)
            {
                nBL += (salto * NPUNTEROS * NPUNTEROS) - 1;
            }
            */
#if DEBUGN6
            fprintf(stderr, BLUE "\n[liberar_bloques_inodo()→ Saltamos del BL %d al BL %d]" RESET, nBLOriginal, nBL);
#endif
        }
    }
#if DEBUGN6
    fprintf(stderr, BLUE "\n[liberar_bloques_inodo()→ total bloques liberados: %d, total_breads: %d, total_bwrites:%d]\n" RESET, liberados, contador_breads, contador_bwrites);
#endif
    return liberados;
}