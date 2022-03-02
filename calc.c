/** @file
  Implementacja kalkulatora działającego na wielomianach rzadkich wielu zmiennych,
  stosującego odwrotną notację polską.
  @author Julia Podrażka
 */
/** Pozwala korzystać z funkcji getline. */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "poly.h"
#include "make_command.h"
#include "stack.h"
#include "make_poly.h"

/** Pierwszy indeks w tablicy. */
#define FIRST_IDX 0
/** Jeden element w tablicy. */
#define ONE_ELEMENT 1
/** Początkowy rozmiar tablicy. */
#define SIZE 2
/** Kod ascii znaku zerowego. */
#define NULL_CHAR 0
/** Kod ascii znaku hash. */
#define COMMENT 35
/** Kod ascii znaku nowej linii. */
#define NEWLINE 10
/** Kod ascii znaku otwierającego nawiasu. */
#define OPEN_BRACKET 40
/** Kod ascii znaku zamykającego nawiasu. */
#define CLOSE_BRACKET 41
/** Kod ascii znaku plus. */
#define PLUS 43
/** Kod ascii znaku przecinka. */
#define COMMA 44
/** Kod ascii znaku minusa. */
#define MINUS 45
/** Baza konwertowania liczb. */
#define BASE 10
/** Wartość getline po wczytaniu końca pliku. */
#define LAST_LINE -1
/** Początkowa wartość errno. */
#define ERRNO 0

/**
 * Wyświetla błąd ze znakiem zerowym w zależności od początku wiersza.
 * @param[in] char_arr : tablica ze znakami z wiersza
 * @param[in] line_number : numer wiersza
 */
static void PrintNullError(char *char_arr, size_t line_number) {

    if ((int) char_arr[FIRST_IDX] != NULL_CHAR &&
        isalpha(char_arr[FIRST_IDX])) {
        if (strncmp(char_arr, "AT", 2) == 0 && isspace(char_arr[2]))
            fprintf(stderr, "ERROR %zu AT WRONG VALUE\n", line_number);
        else if (strncmp(char_arr, "DEG_BY", 6) == 0 &&
                 isspace(char_arr[6]))
            fprintf(stderr, "ERROR %zu DEG BY WRONG VARIABLE\n", line_number);
        else if (strncmp(char_arr, "COMPOSE", 7) == 0 &&
                 isspace(char_arr[7]))
            fprintf(stderr, "ERROR %zu COMPOSE WRONG PARAMETER\n", line_number);
        else fprintf(stderr, "ERROR %zu WRONG COMMAND\n", line_number);
    } else fprintf(stderr, "ERROR %zu WRONG POLY\n", line_number);

}

/**
 * Wczytuje kolejne wiersze ze standardowego wejścia i przekierowuje wiersze
 * odpowiednio do funkcji zajmującej się parsowaniem wielomianów i do funckji
 * zajmującej się parsowaniem komend lub omija wiersz czy wypisuje błąd wczytywania.
 */
static void Read() {

    stack s = InitStack();

    ssize_t line_number = ONE_ELEMENT;
    size_t char_arr_size = SIZE;
    char *char_arr = (char *) malloc(char_arr_size);
    CHECK_PTR(char_arr);

    errno = ERRNO;
    ssize_t line = getline(&char_arr, &char_arr_size, stdin);
    if (errno == ENOMEM) exit(1);
    CHECK_PTR(char_arr);

    while (line != (ssize_t) LAST_LINE) {
        ssize_t char_number = strlen(char_arr);
        // Sprawdza, czy wczytano znak zerowy, jeśli tak, to wypisuje
        // odpowiedni błąd.
        if (char_number != line) PrintNullError(char_arr, line_number);
        else if (char_number > FIRST_IDX && char_arr[FIRST_IDX] != COMMENT &&
                 char_arr[FIRST_IDX] != NEWLINE) {
            if (isalpha(char_arr[FIRST_IDX])) {
                // Jeśli na końcu wczytanego wiersza mamy znak przejścia do
                // następnej linii, to przekazujemy o jeden mniejszy rozmiar wiersza
                if (char_arr[char_number - ONE_ELEMENT] == NEWLINE) {
                    MakeCommand(&s, char_arr, char_number - ONE_ELEMENT,
                                line_number);
                } else MakeCommand(&s, char_arr, char_number, line_number);
            } else if (isdigit(char_arr[FIRST_IDX]) ||
                       char_arr[FIRST_IDX] == OPEN_BRACKET ||
                       char_arr[FIRST_IDX] == MINUS) {
                MakePoly(&s, char_arr, line_number);
            } else {
                fprintf(stderr, "ERROR %zu WRONG POLY\n", line_number);
            }
        }
        errno = ERRNO;
        line = getline(&char_arr, &char_arr_size, stdin);
        if (errno == ENOMEM) exit(1);
        CHECK_PTR(char_arr);
        line_number++;
    }

    free(char_arr);
    RemoveStack(&s);

}

/**
 * Funkcja wykonująca program.
 * @return kod wyjścia programu
 */
int main() {

    Read();
    return 0;

}