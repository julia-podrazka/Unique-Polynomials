/** @file
  Implementacja klasy parsującej komendy.
  @author Julia Podrażka
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#include "poly.h"
#include "make_command.h"

/** Kod ascii znaku nowej linii. */
#define NEWLINE 10
/** Pierwszy indeks w tablicy. */
#define FIRST_IDX 0
/** Jeden element w tablicy. */
#define ONE_ELEMENT 1
/** Dwa elementy w tablicy. */
#define TWO_ELEMENTS 2
/** Wartość zwracana przez funkcję porównującą dwa napisy, jeśli są one takie same. */
#define SAME 0
/** Baza konwertowania liczb. */
#define BASE 10
/** Początkowa wartość errno. */
#define ERRNO 0
/** Kod ascii znaku nowej linii. */
#define NEWLINE 10
/** Kod ascii znaku plus. */
#define PLUS 43
/** Kod ascii znaku minusa. */
#define MINUS 45
/** Kod ascii znaku spacji. */
#define SPACE 32
/** Kod ascii znaku zerowego. */
#define NULL_CHAR 0

/**
 * Wypisuje na standardowe wyjście błędów błąd parsowania komendy.
 * @param[in] line_number : numer aktualnego wiersza
 */
static void PrintStackUnderflow(size_t line_number) {

    fprintf(stderr, "ERROR %zu STACK UNDERFLOW\n", line_number);

}

/**
 * Wykonuje komendy IS_ZERO i ZERO.
 * @param[in] s : stos wielomianów
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] op : funkcja będąca albo AllExpZero albo PolyIsZero
 */
static void ParseZero(stack *s, size_t line_number, bool (*op)(const Poly *)) {

    if (IsEmpty(s)) PrintStackUnderflow(line_number);
    else {
        Poly p = Top(s);
        if (op(&p)) printf("1\n");
        else printf("0\n");
    }

}

/**
 * Wykonuje komendę CLONE.
 * @param[in] s : stos wielomianów
 * @param[in] line_number : numer aktualnego wiersza
 */
static void ParseClone(stack *s, size_t line_number) {

    if (IsEmpty(s)) PrintStackUnderflow(line_number);
    else {
        Poly p = Top(s);
        Push(s, PolyClone(&p));
    }

}

/**
 * Wykonuje komendy ADD, MUL i SUB.
 * @param[in] s : stos wielomianów
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] op : funkcja dwuargumentowa będąca albo PolyAdd albo PolySub albo PolyMul
 */
static void ParseTwoArgumentFunctions(stack *s, size_t line_number,
                                      Poly (*op)(const Poly *, const Poly *)) {

    if (!AreTwoElements(s)) PrintStackUnderflow(line_number);
    else {
        Poly p1 = Pop(s);
        Poly p2 = Pop(s);
        Push(s, op(&p1, &p2));
        PolyDestroy(&p1);
        PolyDestroy(&p2);
    }

}

/**
 * Wykonuje komendę NEG.
 * @param[in] s : stos wielomianów
 * @param[in] line_number : numer aktualnego wiersza
 */
static void ParseNeg(stack *s, size_t line_number) {

    if (IsEmpty(s)) PrintStackUnderflow(line_number);
    else {
        Poly p = Pop(s);
        Push(s, PolyNeg(&p));
        PolyDestroy(&p);
    }

}

/**
 * Wykonuje komendę IS_EQ.
 * @param[in] s : stos wielomianów
 * @param[in] line_number : numer aktualnego wiersza
 */
static void ParseIsEq(stack *s, size_t line_number) {

    if (!AreTwoElements(s)) PrintStackUnderflow(line_number);
    else {
        Poly p1 = Top(s);
        Poly p2 = SecondTop(s);
        if (PolyIsEq(&p1, &p2)) printf("1\n");
        else printf("0\n");
    }

}

/**
 * Wykonuje komendę DEG.
 * @param[in] s : stos wielomianów
 * @param[in] line_number : numer aktualnego wiersza
 */
static void ParseDeg(stack *s, size_t line_number) {

    if (IsEmpty(s)) PrintStackUnderflow(line_number);
    else {
        Poly p = Top(s);
        printf("%d\n", PolyDeg(&p));
    }

}

/**
 * Wypisuje na standardowe wyjście wielomian.
 * @param[in] p : wielomian
 */
static void Print(Poly p) {

    if (PolyIsCoeff(&p) || AllExpZero(&p)) printf("%ld", GetCoeff(&p));
    else {
        for (long long int i = p.size - ONE_ELEMENT; i >= FIRST_IDX; i--) {
            Mono current_mono = p.arr[i];
            printf("(");
            Print(current_mono.p);
            printf(",%d)", MonoGetExp(&current_mono));
            if (i != FIRST_IDX) printf("+");
        }
    }

}

/**
 * Wykonuje komendę PRINT.
 * @param[in] s : stos wielomianów
 * @param[in] line_number : numer aktualnego wiersza
 */
static void PolyPrint(stack *s, size_t line_number) {

    if (IsEmpty(s)) PrintStackUnderflow(line_number);
    else {
        Poly p = Top(s);
        Print(p);
        printf("\n");
    }

}

/**
 * Wykonuje komendę POP.
 * @param[in] s : stos wielomianów
 * @param[in] line_number : numer aktualnego wiersza
 */
static void ParsePop(stack *s, size_t line_number) {

    if (IsEmpty(s)) PrintStackUnderflow(line_number);
    else {
        Poly p = Pop(s);
        PolyDestroy(&p);
    }

}

/**
 * Wypisuje błąd dotyczący komendy AT na standardowe wyjście błędów.
 * @param[in] line_number : numer aktualnego wiersza
 */
static void PrintAtError(size_t line_number) {

    fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line_number);

}

/**
 * Wypisuje błąd dotyczący komendy DEG_BY na standardowe wyjście błędów.
 * @param[in] line_number : numer aktualnego wiersza
 */
static void PrintDegByError(size_t line_number) {

    fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line_number);

}

/**
 * Wypisuje błąd dotyczący komendy COMPOSE na standardowe wyjście błędów.
 * @param[in] line_number : numer aktualnego wiersza
 */
static void PrintComposeError(size_t line_number) {

    fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line_number);

}

/**
 * Wypisuje błąd dotyczący komendy COMPOSE lub DEG_BY na standardowe wyjście
 * błędów w zależności od wartości @p is_deg_by.
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] is_deg_by : zmienna określająca, czy wypisać błąd DEG_BY czy
 * COMPOSE
 */
static void PrintDegByOrComposeError(size_t line_number, bool is_deg_by) {

    if (is_deg_by) PrintDegByError(line_number);
    else PrintComposeError(line_number);

}

/**
 * Wykonuje komendę AT.
 * @param[in] char_arr : tablica znaków aktualnego wiersza
 * @param[in] char_number : liczba znaków w aktualnym wierszu
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] s : stos wielomianów
 */
static void ParseAt(char *char_arr, size_t char_number, size_t line_number,
                    stack *s) {

    size_t i = 2;
    if ((i + TWO_ELEMENTS) > char_number || (int) char_arr[i] != SPACE ||
        (!isdigit(char_arr[i + ONE_ELEMENT]) &&
        (int) char_arr[i + ONE_ELEMENT] != MINUS)) {
        if (!isspace(char_arr[i])) {
            fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line_number);
        } else PrintAtError(line_number);
    } else {
        char space[] = " ";
        char *divided_char = strtok(char_arr, space);
        size_t divided_idx = FIRST_IDX;
        while (divided_char != NULL) {
            if (divided_idx == FIRST_IDX && strcmp(divided_char, "AT") != SAME) {
                PrintAtError(line_number);
                break;
            } else if (divided_idx == ONE_ELEMENT) {
                if ((int) divided_char[FIRST_IDX] == PLUS) {
                    PrintAtError(line_number);
                    break;
                }
                char *last_char;
                errno = ERRNO;
                long long int value = strtoll(divided_char, &last_char, BASE);
                if (errno == ERRNO && (last_char == NULL ||
                    (int) last_char[FIRST_IDX] == NEWLINE ||
                    (int) last_char[FIRST_IDX] == NULL_CHAR)) {
                    divided_char = strtok(NULL, space);
                    if (divided_char != NULL) {
                        PrintAtError(line_number);
                        break;
                    }
                    if (IsEmpty(s)) PrintStackUnderflow(line_number);
                    else {
                        Poly p = Pop(s);
                        Push(s, PolyAt(&p, value));
                        PolyDestroy(&p);
                    }
                } else {
                    PrintAtError(line_number);
                    break;
                }
            } else if (divided_idx > ONE_ELEMENT) {
                PrintAtError(line_number);
                break;
            }
            divided_char = strtok(NULL, space);
            divided_idx++;
        }
    }

}

/**
 * Wykonuje komendę COMPOSE zakładając, że mamy wystarczająco dużo wielomianów
 * na stosie @p s.
 * @param[in] s : stos wielomianów
 * @param[in] value : argument komendy COMPOSE
 */
static void MakeCompose(stack *s, unsigned long long int value) {

    Poly p = Pop(s);
    Poly *q;
    if (value != FIRST_IDX) {
        q = (Poly *) malloc(value *
                            sizeof(Poly));
        for (size_t k = ONE_ELEMENT;
             k <= value; k++) q[value - k] = Pop(s);
    } else q = (Poly *) malloc(ONE_ELEMENT *
                               sizeof(Poly));
    CHECK_PTR(q);
    Push(s, PolyCompose(&p, value, q));
    PolyDestroy(&p);
    if (value != FIRST_IDX) {
        for (size_t k = ONE_ELEMENT;
             k <= value; k++) PolyDestroy(&q[value - k]);
    }
    free(q);

}

/**
 * Wykonuje komendę DEG_BY lub COMPOSE a zależności od @p is_deg_by.
 * @param[in] char_arr : tablica znaków aktualnego wiersza
 * @param[in] char_number : liczba znaków w aktualnym wierszu
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] s : stos wielomianów
 * @param[in] is_deg_by : określa, czy wykonać komendę DEG_BY czy COMPOSE
 */
static void ParseDegByOrCompose(char *char_arr, size_t char_number, size_t line_number,
                                stack *s, bool is_deg_by) {

    size_t i;
    if (is_deg_by) i = 6;
    else i = 7;
    if ((i + TWO_ELEMENTS) > char_number || (int) char_arr[i] != SPACE ||
        !isdigit(char_arr[i + ONE_ELEMENT])) {
        if (!isspace(char_arr[i])) {
            fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line_number);
        } else if (is_deg_by) PrintDegByError(line_number);
        else PrintComposeError(line_number);
    } else {
        char space[] = " ";
        char *divided_char = strtok(char_arr, space);
        size_t divided_idx = FIRST_IDX;
        while (divided_char != NULL) {
            if (divided_idx == FIRST_IDX && strcmp(divided_char, "DEG_BY") != SAME
                && strcmp(divided_char, "COMPOSE") != SAME) {
                PrintDegByOrComposeError(line_number, is_deg_by);
                break;
            } else if (divided_idx == ONE_ELEMENT) {
                if ((int) divided_char[FIRST_IDX] == PLUS ||
                    (int) divided_char[FIRST_IDX] == MINUS) {
                    PrintDegByOrComposeError(line_number, is_deg_by);
                    break;
                }
                char *last_char;
                errno = ERRNO;
                unsigned long long int value = strtoull(divided_char, &last_char, BASE);
                if (errno == ERRNO && (last_char == NULL ||
                                       (int) last_char[FIRST_IDX] == NEWLINE ||
                                       (int) last_char[FIRST_IDX] == NULL_CHAR)) {
                    divided_char = strtok(NULL, space);
                    if (divided_char != NULL) {
                        PrintDegByOrComposeError(line_number, is_deg_by);
                        break;
                    }
                    if (IsEmpty(s)) PrintStackUnderflow(line_number);
                    else {
                        if (is_deg_by) {
                            Poly p = Top(s);
                            printf("%d\n", PolyDegBy(&p, value));
                        } else {
                            if (NumberOfElements(s) - ONE_ELEMENT < value) {
                                PrintStackUnderflow(line_number);
                                break;
                            }
                            MakeCompose(s, value);
                        }
                    }
                } else {
                    PrintDegByOrComposeError(line_number, is_deg_by);
                    break;
                }
            } else if (divided_idx > ONE_ELEMENT) {
                PrintDegByOrComposeError(line_number, is_deg_by);
                break;
            }
            divided_char = strtok(NULL, space);
            divided_idx++;
        }
    }

}

void MakeCommand(stack *s, char *char_arr, size_t char_number, size_t line_number) {

    assert ((int) char_arr[char_number - ONE_ELEMENT] != NEWLINE);

    if (char_number == 4 && strncmp(char_arr, "ZERO", char_number) == SAME)
        Push(s, PolyZero());
    else if (char_number == 8 && strncmp(char_arr, "IS_COEFF", char_number) == SAME)
        ParseZero(s, line_number, AllExpZero);
    else if (char_number == 7 && strncmp(char_arr, "IS_ZERO", char_number) == SAME)
        ParseZero(s, line_number, PolyIsZero);
    else if (char_number == 5 && strncmp(char_arr, "CLONE", char_number) == SAME)
        ParseClone(s, line_number);
    else if (char_number == 3 && strncmp(char_arr, "ADD", char_number) == SAME)
        ParseTwoArgumentFunctions(s, line_number, PolyAdd);
    else if (char_number == 3 && strncmp(char_arr, "MUL", char_number) == SAME)
        ParseTwoArgumentFunctions(s, line_number, PolyMul);
    else if (char_number == 3 && strncmp(char_arr, "NEG", char_number) == SAME)
        ParseNeg(s, line_number);
    else if (char_number == 3 && strncmp(char_arr, "SUB", char_number) == SAME)
        ParseTwoArgumentFunctions(s, line_number, PolySub);
    else if (char_number == 5 && strncmp(char_arr, "IS_EQ", char_number) == SAME)
        ParseIsEq(s, line_number);
    else if (char_number == 3 && strncmp(char_arr, "DEG", char_number) == SAME)
        ParseDeg(s, line_number);
    else if (char_number == 5 && strncmp(char_arr, "PRINT", char_number) == SAME)
        PolyPrint(s, line_number);
    else if (char_number == 3 && strncmp(char_arr, "POP", char_number) == SAME)
        ParsePop(s, line_number);
    else if (strncmp(char_arr, "AT", 2) == SAME)
        ParseAt(char_arr, char_number, line_number, s);
    else if (strncmp(char_arr, "DEG_BY", 6) == SAME)
        ParseDegByOrCompose(char_arr, char_number, line_number, s, true);
    else if (strncmp(char_arr, "COMPOSE", 7) == SAME)
        ParseDegByOrCompose(char_arr, char_number, line_number, s, false);
    else fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line_number);

}