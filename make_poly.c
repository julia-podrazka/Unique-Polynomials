/** @file
  Implementacja klasy parsującej wielomian.
  @author Julia Podrażka
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "poly.h"
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
/** Początkowa wartość errno. */
#define ERRNO 0

/**
 * Wypisuje na standardowe wyjście błędów błąd parsowania wielomianu.
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] is_error : określa, czy znaleziono już błąd przy parsowaniu
 */
static void PrintError(size_t line_number, bool *is_error) {

    fprintf(stderr, "ERROR %zu WRONG POLY\n", line_number);
    (*is_error) = true;

}

/**
 * Wypisuje na standardowe wyjście błędów błąd parsowania wielomianu i zwraca
 * wielomian zerowy.
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] is_error : określa, czy znaleziono już błąd przy parsowaniu
 * @return wielomian zerowy
 */
static Poly PrintPolyError(size_t line_number, bool *is_error) {

    if (!(*is_error)) PrintError(line_number, is_error);
    return PolyZero();

}

/**
 * Wypisuje na standardowe wyjście błędów błąd parsowania wielomianu i zwraca
 * jednomian o zerowym wielomianie.
 * @param[in] poly : wielomian do usunięcia
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] is_error : określa, czy znaleziono już błąd przy parsowaniu
 * @return jednomian o zerowym wielomianie
 */
static Mono PrintMonoError(Poly poly, size_t line_number, bool *is_error) {

    PolyDestroy(&poly);
    if (!(*is_error)) PrintError(line_number, is_error);
    Poly p = PolyZero();
    return MonoFromPoly(&p, FIRST_IDX);

}

/**
 * Zlicza znaki będące plusami należące do aktualnie przetwarzanego wielomianu
 * w wierszu i sprawdza, czy nie występują błędne znaki nie należące do wielomianu.
 * @param[in] char_arr : tablica znaków w wierszu
 * @param[in] plus_number : liczba plusów
 * @param[in] is_error_char : określa, czy znaleziono błędny znak
 */
static void CheckChars(char **char_arr, size_t *plus_number,
                       bool *is_error_char) {

    size_t open_brackets = FIRST_IDX;
    size_t closed_brackets = FIRST_IDX;
    size_t i = FIRST_IDX;
    while ((int) (*char_arr)[i] != NULL_CHAR && (int) (*char_arr)[i] != NEWLINE &&
           ((int) (*char_arr)[i] != COMMA || open_brackets != closed_brackets)) {
        if ((int) (*char_arr)[i] == PLUS && open_brackets == closed_brackets)
            (*plus_number)++;
        else if ((int) (*char_arr)[i] == OPEN_BRACKET) open_brackets++;
        else if ((int) (*char_arr)[i] == CLOSE_BRACKET) closed_brackets++;
        else if (!isdigit((*char_arr)[i]) && (int) (*char_arr)[i] != MINUS &&
                 (int) (*char_arr)[i] != COMMA && (int) (*char_arr)[i] != PLUS) {
            *is_error_char = true;
            break;
        }
        i++;
    }
    if (open_brackets != closed_brackets) *is_error_char = true;

}

/**
 * Parsuje tablice znaków sprawdzając, czy na jej początku wystepuje liczba.
 * Jeśli tak, to parsuje ją do odpowiedniego typu, w przeciwnym przypadku
 * wyspiuje błąd. Zwraca wielomian stały równy tej liczbie.
 * @param[in] char_arr : tablica znaków w wierszu
 * @param[in] line_number : numer aktualnego wiersza
 * @param[in] is_error : określa, czy znaleziono już błąd przy parsowaniu
 * @return wielomian stały
 */
static Poly ParseCoeff(char **char_arr, size_t line_number, bool *is_error) {

    if ((int) (*char_arr)[FIRST_IDX] == PLUS)
        return PrintPolyError(line_number, is_error);
    char *last_char;
    errno = ERRNO;
    long long int number = strtoll(*char_arr, &last_char, BASE);
    if (errno == ERRNO && (last_char == NULL ||
        (int) last_char[FIRST_IDX] == COMMA ||
        (int) last_char[FIRST_IDX] == NEWLINE ||
        (int) last_char[FIRST_IDX] == NULL_CHAR)) {
        *char_arr = last_char;
        return PolyFromCoeff(number);
    } else return PrintPolyError(line_number, is_error);

}

/**
 * Tworzy wielomian z aktualnego wiersza i go zwraca. Jeśli napotka na błędny znak,
 * to wypisuje błąd i zwraca wielomian zerowy.
 * @param[in] char_arr : tablica znaków w wierszu
 * @param[in] is_error : określa, czy znaleziono już błąd przy parsowaniu
 * @param[in] line_number : numer aktualnego wiersza
 * @return wielomian
 */
static Poly MakePolyHelper(char **char_arr, bool *is_error, size_t line_number);

/**
 * Tworzy jednomian, który jest na początku tablicy znaków i go zwraca.
 * Jeśli napotka na błędny znak, to wypisuje błąd i zwraca jednomian o
 * wielomianie zerowym.
 * @param[in] char_arr : tablica znaków w wierszu
 * @param[in] is_error : określa, czy znaleziono już błąd przy parsowaniu
 * @param[in] line_number : numer aktualnego wiersza
 * @return jednomian
 */
static Mono MakeMonoHelper(char **char_arr, bool *is_error, size_t line_number) {

    if (*is_error) {
        Poly p = PolyZero();
        return MonoFromPoly(&p, FIRST_IDX);
    }
    // Pomijamy nawias otwierający
    (*char_arr)++;
    Poly poly = MakePolyHelper(char_arr, is_error, line_number);
    if ((int) (*char_arr)[FIRST_IDX] != COMMA)
        return PrintMonoError(poly, line_number, is_error);
    else {
        (*char_arr)++;
        if (!isdigit((*char_arr)[FIRST_IDX]))
            return PrintMonoError(poly, line_number, is_error);
        char *last_char;
        long int number = strtol(*char_arr, &last_char, BASE);
        if (number > INT_MAX) return PrintMonoError(poly, line_number, is_error);
        *char_arr = last_char;
        if ((int) (*char_arr)[FIRST_IDX] != CLOSE_BRACKET)
            return PrintMonoError(poly, line_number, is_error);
        else {
            // Pomijamy nawias zamykający
            (*char_arr)++;
            return MonoFromPoly(&poly, number);
        }
    }

}

static Poly MakePolyHelper(char **char_arr, bool *is_error, size_t line_number) {

    if (*is_error) return PolyZero();

    if ((int) (*char_arr)[FIRST_IDX] == OPEN_BRACKET) {
        size_t plus_number = ONE_ELEMENT;
        bool is_error_char = false;

        CheckChars(char_arr, &plus_number, &is_error_char);
        if (is_error_char == true) return PrintPolyError(line_number, is_error);

        Mono *mono_arr = (Mono *) malloc(plus_number * sizeof(Mono));
        CHECK_PTR(mono_arr);
        size_t k = FIRST_IDX;
        while ((int) (*char_arr)[FIRST_IDX] != COMMA &&
               (int) (*char_arr)[FIRST_IDX] != NULL_CHAR &&
               (int) (*char_arr)[FIRST_IDX] != NEWLINE) {
            mono_arr[k] = MakeMonoHelper(char_arr, is_error, line_number);
            k++;
            if (*is_error) {
                for (size_t l = FIRST_IDX; l < k; l++) MonoDestroy(&mono_arr[l]);
                free(mono_arr);
                return PolyZero();
            }
            if ((int) (*char_arr)[FIRST_IDX] == PLUS) (*char_arr)++;
            else if ((int) (*char_arr)[FIRST_IDX] != COMMA &&
                     (int) (*char_arr)[FIRST_IDX] != NULL_CHAR &&
                     (int) (*char_arr)[FIRST_IDX] != NEWLINE) {
                for (size_t l = FIRST_IDX; l < k; l++) MonoDestroy(&mono_arr[l]);
                free(mono_arr);
                return PrintPolyError(line_number, is_error);
            }
        }
        if (k != plus_number) {
            for (size_t l = FIRST_IDX; l < k; l++) MonoDestroy(&mono_arr[l]);
            free(mono_arr);
            return PrintPolyError(line_number, is_error);
        }
        Poly p = PolyAddMonos(plus_number, mono_arr);
        free(mono_arr);
        return p;
    } else return ParseCoeff(char_arr, line_number, is_error);

}

void MakePoly(stack *s, char *char_arr, size_t line_number) {

    bool is_error = false;
    Poly p = MakePolyHelper(&char_arr, &is_error, line_number);
    if (!is_error && (int) char_arr[FIRST_IDX] != NEWLINE &&
        (int) char_arr[FIRST_IDX] != NULL_CHAR) {
        PrintError(line_number, &is_error);
        PolyDestroy(&p);
    }
    if (!is_error) Push(s, p);

}