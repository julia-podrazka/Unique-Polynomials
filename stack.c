/** @file
  Implementacja klasy obsługującej stos wielomianów rzadkich wielu zmiennych.
  @author Julia Podrażka
 */
#include <stdlib.h>
#include <stdbool.h>

#include "stack.h"

/** Początkowy rozmiar tablicy. */
#define ARR_SIZE 2
/** Pierwszy indeks w tablicy. */
#define FIRST_IDX 0
/** Jeden element w tablicy. */
#define ONE_ELEMENT 1
/** Dwa elementy w tablicy. */
#define TWO_ELEMENTS 2

stack InitStack() {

    Poly *poly_stack = (Poly *) malloc(ARR_SIZE * sizeof(Poly));
    CHECK_PTR(poly_stack);
    return (stack) {.poly_stack = poly_stack, .top = FIRST_IDX, .size = ARR_SIZE};

}

bool IsEmpty(stack *s) {

    return ((*s).top == FIRST_IDX);

}

bool AreTwoElements(stack *s) {

    return ((*s).top >= TWO_ELEMENTS);

}

size_t NumberOfElements(stack *s) {

    return (*s).top;

}

/**
 * Realokuje pamięć przeznaczoną na stos.
 * @param[in] s : stos
 */
static void ReallocPolyStack(stack *s) {

    if ((*s).top == (*s).size) {
        (*s).size = (*s).size * ARR_SIZE;
        (*s).poly_stack = realloc((*s).poly_stack, (*s).size * sizeof(Poly));
        CHECK_PTR((*s).poly_stack);
    }

}

void Push(stack *s, Poly p) {

    (*s).poly_stack[(*s).top] = p;
    (*s).top = (*s).top + 1;
    ReallocPolyStack(s);

}

Poly Pop(stack *s) {

    ((*s).top)--;
    return (*s).poly_stack[(*s).top];

}

Poly Top(stack *s) {

    return (*s).poly_stack[(*s).top - ONE_ELEMENT];

}

Poly SecondTop(stack *s) {

    return (*s).poly_stack[(*s).top - TWO_ELEMENTS];

}

void RemoveStack(stack *s) {

    for (size_t i = FIRST_IDX; i < (*s).top; i++) PolyDestroy(&(*s).poly_stack[i]);
    free((*s).poly_stack);

}