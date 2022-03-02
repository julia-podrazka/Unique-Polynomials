/** @file
  Interfejs klasy parsującej wielomian.
  @author Julia Podrażka
 */
#ifndef MAKE_POLY_H
#define MAKE_POLY_H

/**
 * Parsuje wiersz zawierający wielomian i wstawia go na stos, jeśli nie
 * znaleziono żadnego błędu.
 * @param[in] s : stos
 * @param[in] char_arr : tablica znaków aktualnego wiersza
 * @param[in] line_number : numer aktualnego wiersza
 */
void MakePoly(stack *s, char *char_arr, size_t line_number);

#endif
