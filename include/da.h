/* Dynamic Array Implementation */

#ifndef DA_H
#define DA_H

#include "pch.h"

/*
	 E verificado se a quantidade (COUNT) atingiu a capacidade (CAPACITY) do array dinâmico (DA).
	 Se estiver realoca utilizando realloc para dobrar (*2) e valida utilizando ASSERT se a realocação foi bem-sucedida.
	 Dado gravado no ELEMENT e incrementa a quantidade (COUNT) do array dinâmico (DA).
*/


#define APPEND(da, element)                                                     \
    do {                                                                        \
        if ((da)->count >= (da)->capacity) {                                    \
            size_t new_cap = ((da)->capacity == 0) ? 2 : (da)->capacity * 2;    \
            void *new_ptr = realloc((da)->data, new_cap * sizeof(*(da)->data)); \
            assert(new_ptr != NULL);                                            \
            (da)->data = new_ptr;                                               \
            (da)->capacity = new_cap;                                           \
        }                                                                       \
        (da)->data[(da)->count] = (element);                                    \
        (da)->count++;                                                          \
    } while (0)

#endif // DA_H