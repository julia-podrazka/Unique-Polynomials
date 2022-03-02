/** @file
  Interfejs klasy obsługującej stos wielomianów rzadkich wielu zmiennych.
  @author Julia Podrażka
 */
#ifndef STACK_H
#define STACK_H

#include "poly.h"

/**
 * To jest struktura przechowująca stos wielomianów opisanych w zadaniu.
 */
typedef struct stack {
    Poly *poly_stack; ///< tablica wielomianów
    size_t top; ///< indeks, na który wkładamy następny wielomian
    size_t size; ///< rozmiar tablicy poly_stack
} stack;

/**
 * Inicjalizuje nowy stos.
 * @return stos
 */
stack InitStack();

/**
 * Sprawdza, czy stos jest pusty.
 * @param[in] s : stos
 * @return Czy stos pusty?
 */
bool IsEmpty(stack *s);

/**
 * Sprawdza, czy stos zawiera co najmniej dwa wielomiany.
 * @param[in] s : stos
 * @return Czy stos ma co najmniej dwa wielomiany?
 */
bool AreTwoElements(stack *s);

/**
 * Zwraca liczbę elementów na stosie @p s.
 * @param[in] s : stos
 * @return liczba elementów na stosie
 */
size_t NumberOfElements(stack *s);

/**
 * Wstawia wielomian na stos.
 * @param[in] s : stos
 * @param[in] p : wielomian
 */
void Push(stack *s, Poly p);

/**
 * Usuwa i zwraca wielomian z wierzchołka stosu
 * @param[in] s : stos
 * @return wielomian z wierzchołka stosu
 */
Poly Pop(stack *s);

/**
 * Zwraca wielomian z wierzchołka stosu.
 * @param[in] s : stos
 * @return wielomian z wierzchołka stosu
 */
Poly Top(stack *s);

/**
 * Zwraca drugi od góry wielomian ze stosu.
 * @param[in] s : stos
 * @return drugi od góry wielomian ze stosu
 */
Poly SecondTop(stack *s);

/**
 * Usuwa stos i wielomiany ze stosu z pamięci.
 * @param[in] s : stos
 */
void RemoveStack(stack *s);

#endif