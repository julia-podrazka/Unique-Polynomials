/** @file
  Interfejs klasy parsującej komendy.
  @author Julia Podrażka
 */
#ifndef MAKE_COMMAND_H
#define MAKE_COMMAND_H

#include "stack.h"

/**
 * Parsuje wiersz zawierający komendę i ją wykonuje jeśli nie
 * znaleziono żadnego błędu.
 * @param[in] s : stos
 * @param[in] char_arr : tablica znaków aktualnego wiersza
 * @param[in] char_number : liczba znaków w aktualnym wierszu
 * @param[in] line_number : numer aktualnego wiersza
 */
void MakeCommand(stack *s, char *char_arr, size_t char_number, size_t line_number);

#endif