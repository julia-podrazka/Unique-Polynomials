/** @file
  Implementacja klasy wielomianów rzadkich wielu zmiennych.
  Założenia w implementacji:
  - tworzone wielomiany są sortowane malejąco po wykładnikach
  - wielomiany (oprócz wielomianu zerowego) nie zawierają jednomianów
  o zerowym współczynniku
  - dopusczalne jest tworzenie wielomianów stałych jako jednomianów o stałym
  współczynniku i zerowym wykładniku
  @author Julia Podrażka
*/

#include <stdio.h>
#include <stdlib.h>

#include "poly.h"

/** Pierwszy indeks w tablicy. */
#define FIRST_IDX 0
/** Wykładnik zerowy. */
#define EXP_ZERO 0
/** Wykładnik o wartości jeden. */
#define EXP_ONE 1
/** Wartość skalara, przez który mnożymy wielomian przy jego klonowaniu. */
#define CLONE 1
/** Wartość skalara, przez który mnożymy wielomian przy jego negowaniu. */
#define NEG -1
/** Wartość zwracana przy porównywaniu wykładników,
 * jeśli pierwszy jest mniejszy od drugiego. */
#define LESSER -1
/** Wartość zwracana przy porównywaniu wykładników,
 * jeśli pierwszy jest większy od drugiego. */
#define GREATER 1
/** Wartość zwracana przy porównywaniu wykładników,
 * jeśli pierwszy jest równy drugiemu. */
#define EQUAL 0
/** Wartość zwracana, jeśli wielomian jest zerowy. */
#define POLY_ZERO -1
/** Wartość zwracana, jeśli wielomian jest stały. */
#define POLY_COEFF 0
/** Jeden element w tablicy. */
#define ONE_ELEMENT 1
/** Dwa elementy. */
#define TWO 2
/** Wartość zwracana przez funkcję modulo, jeśli liczba jest parzysta. */
#define PARITY 0
/** Początkowy rozmiar tablicy. */
#define ARR_SIZE 2

void PolyDestroy(Poly *p) {

    assert(p != NULL);

    if (!PolyIsCoeff(p)) {
        for (size_t i = FIRST_IDX; i < p -> size; i++) {
            MonoDestroy(&(p -> arr[i]));
        }
        free(p -> arr);
    }

}

/**
 * Zwraca wielomian, którego tablica jednomianów to @p mono_arr z
 * usuniętymi wielomianami zerowymi
 * @param[in] current_idx : rozmiar tablicy @p mono_arr
 * @param[in] mono_arr : tablica jednomianów
 * @return wielomian z tablicą jednomianów @p mono_arr bez zerowych wielomianów
 */
static Poly DeletePolyZero(size_t current_idx, Mono **mono_arr) {

    Mono *final_arr = (Mono *) malloc(current_idx * sizeof(Mono));
    CHECK_PTR(final_arr);
    int new_size = FIRST_IDX - ONE_ELEMENT;
    for (size_t i = FIRST_IDX; i < current_idx; i++) {
        if (!PolyIsZero(&(*mono_arr)[i].p)) {
            new_size++;
            final_arr[new_size] = (*mono_arr)[i];
        } else MonoDestroy(&(*mono_arr)[i]);
    }
    free(*mono_arr);
    int first_size = FIRST_IDX - ONE_ELEMENT;
    if (new_size == first_size) {
        free(final_arr);
        return PolyZero();
    }
    new_size += ONE_ELEMENT;
    final_arr = realloc(final_arr, new_size * sizeof(Mono));
    CHECK_PTR(final_arr);
    return (Poly) {.size = new_size, .arr = final_arr};

}

/**
 * Mnoży wielomian @p p przez skalar.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] scalar : skalar
 * @return @f$p * scalar@f$
 */
static Poly PolyCloneAndMultiplyByScalar(const Poly *p, poly_coeff_t scalar) {

    if (!PolyIsCoeff(p)) {
        Mono *new_monos = (Mono *) malloc(p -> size * sizeof(Mono));
        CHECK_PTR(new_monos);
        for (size_t i = FIRST_IDX; i < p -> size; i++) {
            Mono mono = p -> arr[i];
            Poly new_poly = PolyCloneAndMultiplyByScalar(&mono.p, scalar);
            if (!PolyIsZero(&new_poly)) {
                new_monos[i] = (Mono) {.p = new_poly, .exp = mono.exp};
            } else new_monos[i] = (Mono) {.p = new_poly, .exp = EXP_ZERO};
        }
        return DeletePolyZero(p -> size, &new_monos);
    } else return PolyFromCoeff(scalar * (p -> coeff));

}

Poly PolyClone(const Poly *p) {

    return PolyCloneAndMultiplyByScalar(p, CLONE);

}

/**
 * Dodaje wielomian stały do wielomianu niestałego
 * @param[in] p : wielomian stały @f$p@f$
 * @param[in] q : wielomian niestały @f$q@f$
 * @return @f$p + q@f$
 */
static Poly AddPolyAndCoeff(const Poly *p, const Poly *q) {

    Mono new_mono = MonoFromPoly(p, EXP_ZERO);
    Poly new_poly = (Poly) {.size = ONE_ELEMENT, .arr = &new_mono};
    return PolyAdd(q, &new_poly);

}

/**
 * Klonuje jednomiany z wielomianu @p p do tablicy @p mono_arr
 * @param[in] counter : indeks, od którego klonujemy jednomiany
 * @param[in] p : wielomian @f$p@f$
 * @param[in] mono_arr : tablica jednomianów
 * @param[in] idx : aktualny indeks tablicy @p mono_arr
 */
static void CopyRemainingMonos(size_t counter, const Poly *p,
                               Mono **mono_arr, size_t *idx) {

    for (size_t i = counter; i < p -> size; i++) {
        Mono p_mono = p -> arr[i];
        (*mono_arr)[(*idx)] = MonoClone(&p_mono);
        (*idx)++;
    }

}

Poly PolyAdd(const Poly *p, const Poly *q) {

    if (PolyIsCoeff(p) && PolyIsCoeff(q))
        return PolyFromCoeff(p->coeff + q->coeff);
    else if (PolyIsCoeff(p)) return AddPolyAndCoeff(p, q);
    else if (PolyIsCoeff(q)) return AddPolyAndCoeff(q, p);
    else {
        size_t p_counter = FIRST_IDX;
        size_t q_counter = FIRST_IDX;
        size_t current_mono_arr_size = p -> size + q -> size;
        Mono *mono_arr =
                (Mono *) malloc(current_mono_arr_size * sizeof(Mono));
        CHECK_PTR(mono_arr);
        size_t current_idx = FIRST_IDX;
        while(p_counter < p -> size && q_counter < q -> size) {
            Mono p_mono = p -> arr[p_counter];
            Mono q_mono = q -> arr[q_counter];
            if (MonoGetExp(&p_mono) == MonoGetExp(&q_mono)) {
                Poly new_poly = PolyAdd(&p_mono.p, &q_mono.p);
                poly_exp_t exp;
                // Jeśli dodany wielomian jest zerowy, to zamieniamy sumę
                // wykładników na wykładnik zerowy przy tworzeniu jednomianu.
                if (PolyIsZero(&new_poly)) exp = EXP_ZERO;
                else exp = MonoGetExp(&p_mono);
                mono_arr[current_idx] = MonoFromPoly(&new_poly, exp);
                p_counter++;
                q_counter++;
            } else if (MonoGetExp(&p_mono) > MonoGetExp(&q_mono)) {
                mono_arr[current_idx] = MonoClone(&p_mono);
                p_counter++;
            } else {
                mono_arr[current_idx] = MonoClone(&q_mono);
                q_counter++;
            }
            current_idx++;
        }
        CopyRemainingMonos(p_counter, p, &mono_arr, &current_idx);
        CopyRemainingMonos(q_counter, q, &mono_arr, &current_idx);
        return DeletePolyZero(current_idx, &mono_arr);
    }

}

/**
 * Porównuje wykładniki jednomianów
 * @param[in] arr1 : pierwsza tablica jednomianów do porównania
 * @param[in] arr2 : druga tablica jednomianów do porównania
 * @return 1 jeśli pierwszy wykładnik jest mniejszy, 1 jeśli drugi i 0 jeśli są równe
 */
static int CompareMonos(const void *arr1, const void *arr2) {

    Mono new_arr1 = *(Mono *) arr1;
    Mono new_arr2 = *(Mono *) arr2;
    if (MonoGetExp(&new_arr1) < MonoGetExp(&new_arr2)) return GREATER;
    if (MonoGetExp(&new_arr1) > MonoGetExp(&new_arr2)) return LESSER;
    return EQUAL;

}

/**
 * Tworzy jednomian o współczynniku @p new_poly i zapisuje go do tablicy @p
 * final_arr, jeśli jest niezerowy.
 * @param[in] new_poly : wielomian @f$p@f$
 * @param[in] final_arr : tablica jednomianów
 * @param[in] final_arr_counter : rozmiar tablicy jednomianów
 * @param[in] i : indeks tablicy @p mono_arr
 * @param[in] mono_arr : tablica jednomianów
 */
static void AddPolyToArray(Poly new_poly, Mono **final_arr, int *final_arr_counter,
                           size_t i, Mono *mono_arr) {

    if (!PolyIsZero(&new_poly)) {
        (*final_arr)[*final_arr_counter] = MonoFromPoly(&new_poly, mono_arr[i].exp);
    } else {
        PolyDestroy(&new_poly);
        (*final_arr_counter)--;
    }
    MonoDestroy(&mono_arr[i]);

}

/**
 * Zapisuje jednomian z tablicy @p monos do tablicy @p final_arr, jeśli
 * współczynnik jednomianu jest niezerowy.
 * @param[in] monos : tablica jednomianów
 * @param[in] i : indeks tablicy @p monos
 * @param[in] final_arr : tablica jednomianów
 * @param[in] final_arr_counter : rozmiar tablicy jednomianów
 */
static void AddMonoToArray(Mono *monos, size_t i, Mono **final_arr,
                           int *final_arr_counter) {

    if (PolyIsZero(&monos[i].p)) {
        MonoDestroy(&monos[i]);
        (*final_arr_counter)--;
    } else (*final_arr)[*final_arr_counter] = monos[i];

}

/**
 * Sumuje listę jednomianów i tworzy z nich wielomian. Przejmuje na własność
 * pamięć wskazywaną przez @p monos i jej zawartość.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @return wielomian będący sumą jednomianów
 */
static Poly AddMonos(size_t count, Mono *monos) {

    Mono *final_arr = (Mono *) malloc(count * sizeof(Mono));
    CHECK_PTR(final_arr);

    qsort(monos, count, sizeof(Mono), CompareMonos);

    size_t i = FIRST_IDX;
    int final_arr_counter = FIRST_IDX;
    while (i < count) {
        // Porównujemy wykładniki sąsiednich jednomianów, jeśli takie istnieją
        // i tablica final_arr jest pusta.
        if (i != count - ONE_ELEMENT && final_arr_counter == FIRST_IDX &&
            monos[i].exp == monos[i + ONE_ELEMENT].exp) {
            Poly new_poly =
                    PolyAdd(&monos[i].p, &monos[i + ONE_ELEMENT].p);
            AddPolyToArray(new_poly, &final_arr, &final_arr_counter, i, monos);
            MonoDestroy(&monos[i + ONE_ELEMENT]);
            i++;
        } else if (final_arr_counter == FIRST_IDX)
            AddMonoToArray(monos, i, &final_arr, &final_arr_counter);
        else if (monos[i].exp == final_arr[final_arr_counter - ONE_ELEMENT].exp) {
            final_arr_counter--;
            Poly new_poly =
                    PolyAdd(&monos[i].p, &final_arr[final_arr_counter].p);
            MonoDestroy(&final_arr[final_arr_counter]);
            AddPolyToArray(new_poly, &final_arr, &final_arr_counter, i, monos);
        } else AddMonoToArray(monos, i, &final_arr, &final_arr_counter);
        i++;
        final_arr_counter++;
    }

    free(monos);
    if (final_arr_counter == FIRST_IDX) {
        free(final_arr);
        return PolyZero();
    }
    final_arr = realloc(final_arr, final_arr_counter * sizeof(Mono));
    CHECK_PTR(final_arr);
    return (Poly) {.size = final_arr_counter, .arr = final_arr};

}

Poly PolyOwnMonos(size_t count, Mono *monos) {

    if (count == FIRST_IDX) {
        if (monos != NULL) free(monos);
        return PolyZero();
    }
    return AddMonos(count, monos);

}

/**
 * W zależności od wartości @p clone, albo klonuje jednomiany z tablicy, albo
 * je przepisuje. Sumuje listę jednomianów i tworzy z nich wielomian.
 * @param[in] count : liczba jednomianów
 * @param[in] monos : tablica jednomianów
 * @param[in] clone : określa, czy należy klonować jednomiany
 * @return wielomian będący sumą jednomianów
 */
static Poly CopyAddMonos(size_t count, const Mono monos[], bool clone) {

    if (count == FIRST_IDX || monos == NULL) return PolyZero();

    Mono *mono_arr = (Mono *) malloc(count * sizeof(Mono));
    CHECK_PTR(mono_arr);

    for (size_t i = FIRST_IDX; i < count; i++) {
        if (clone) mono_arr[i] = MonoClone(&monos[i]);
        else mono_arr[i] = monos[i];
    }

    return AddMonos(count, mono_arr);

}

Poly PolyCloneMonos(size_t count, const Mono monos[]) {

    return CopyAddMonos(count, monos, true);

}

Poly PolyAddMonos(size_t count, const Mono monos[]) {

    return CopyAddMonos(count, monos, false);

}

Poly PolyMul(const Poly *p, const Poly *q) {

    if (PolyIsZero(p) || PolyIsZero(q)) return PolyZero();

    if (PolyIsCoeff(p)) return PolyCloneAndMultiplyByScalar(q, p->coeff);
    else if (PolyIsCoeff(q)) return PolyCloneAndMultiplyByScalar(p, q->coeff);
    else {
        size_t mono_arr_size = p -> size * q -> size;
        size_t current_idx = FIRST_IDX;
        Mono *mono_arr = (Mono *) malloc(mono_arr_size * sizeof(Mono));
        CHECK_PTR(mono_arr);
        for (size_t i = FIRST_IDX; i < p -> size; i++) {
            for (size_t k = FIRST_IDX; k < q -> size; k++) {
                Mono mono_p = p -> arr[i];
                Mono mono_q = q -> arr[k];
                Poly poly_mul = PolyMul(&mono_p.p, &mono_q.p);
                // Jeśli pomnożony wielomian jest zerowy, to zamieniamy sumę
                // wykładników na wykładnik zerowy przy tworzeniu jednomianu.
                if (!PolyIsZero(&poly_mul)) {
                    mono_arr[current_idx] =
                            MonoFromPoly(&poly_mul, mono_p.exp + mono_q.exp);
                } else mono_arr[current_idx] = MonoFromPoly(&poly_mul, EXP_ZERO);
                current_idx++;
            }
        }
        Poly new_poly = PolyOwnMonos(mono_arr_size, mono_arr);
        return new_poly;
    }

}

bool AllExpZero(const Poly *p) {

    if (!PolyIsCoeff(p)) {
        if (p -> size == ONE_ELEMENT) {
            Mono current_mono = p -> arr[FIRST_IDX];
            if (MonoGetExp(&current_mono) != EXP_ZERO) return false;
            return AllExpZero(&current_mono.p);
        } else return false;
    } else return true;

}

poly_coeff_t GetCoeff(const Poly *p) {

    if(!PolyIsCoeff(p)) {
        return GetCoeff(&(p -> arr[FIRST_IDX]).p);
    } else return p -> coeff;

}

/**
 * Podnosi @p x do potęgi @p exp w czasie logarytmicznym.
 * @param[in] x : baza potęgowania
 * @param[in] exp : potęga potęgowania
 * @return @f$x^exp@f$
 */
static poly_coeff_t FastPow(poly_coeff_t x, poly_exp_t exp) {

    if (exp == FIRST_IDX) return ONE_ELEMENT;
    else if (exp % TWO == PARITY) {
        poly_coeff_t y = FastPow(x, exp / TWO);
        return  (y * y);
    } else return (x * FastPow(x, exp - ONE_ELEMENT));

}

/**
 * Wykonuje szybkie potęgowanie wielomianu @p p.
 * @param[in] p : wielomian
 * @param[in] exp : potęga
 * @return wielomian @p p podniesiony do potęgi @p exp
 */
static Poly FastPolyPow(Poly p, poly_exp_t exp) {

    if (exp == FIRST_IDX) return PolyFromCoeff(ONE_ELEMENT);
    // Jeśli nasz wielomian jest tożsamościowo równy wielomianowi stałemu,
    // to zwracamy wielomian stały ze stałą podniesioną do potęgi exp.
    else if (AllExpZero(&p)) return PolyFromCoeff(FastPow(GetCoeff(&p), exp));
    else if (exp % TWO == PARITY) {
        Poly y = FastPolyPow(p, exp / TWO);
        Poly mul = PolyMul(&y, &y);
        PolyDestroy(&y);
        return mul;
    } else {
        Poly y = FastPolyPow(p, exp - ONE_ELEMENT);
        Poly mul = PolyMul(&p, &y);
        PolyDestroy(&y);
        return mul;
    }

}

/**
 * Funkcja pomocnicza do funkcji PolyCompose.
 * @param[in] p : wielomian @f$p@f$
 * @param[in] k : rozmiar tablicy wielomianów
 * @param[in] q : tablica wielomianów
 * @param[in] idx : aktualny indeks stojący przy @f$x_i@f$
 * @return wielomian będący złożeniem wielomianów
 */
static Poly PolyComposeHelper(const Poly *p, size_t k, const Poly q[],
                              size_t idx) {

    if (PolyIsCoeff(p)) return PolyFromCoeff(p -> coeff);
    if (AllExpZero(p)) return PolyFromCoeff(GetCoeff(p));
    Poly final = PolyZero();
    for (size_t i = FIRST_IDX; i < p -> size; i++) {
        Mono current_mono = (p -> arr)[i];
        if (idx < k) {
            Poly pow_poly;
            // Jeśli podnosimy wielomian do potęgi pierwszej, to nie musimy
            // klonować jednomianu, zamiast tego przepisujemy go z tablicy q.
            if (MonoGetExp(&current_mono) == EXP_ONE) pow_poly = q[idx];
            else pow_poly = FastPolyPow(q[idx], MonoGetExp(&current_mono));
            Poly new_compose = PolyComposeHelper(&current_mono.p, k, q,
                                            (idx + ONE_ELEMENT));
            Poly mul = PolyMul(&pow_poly, &new_compose);
            // Jeśli podnosiliśmy wielomian do potęgi pierwszej, to nie
            // usuwamy tego wielomianu, bo został on przepisany.
            if (MonoGetExp(&current_mono) != EXP_ONE) PolyDestroy(&pow_poly);
            PolyDestroy(&new_compose);
            Poly add = PolyAdd(&final, &mul);
            PolyDestroy(&final);
            final = add;
            PolyDestroy(&mul);
        // Przypadek 0^0, wtedy mnożymy następne zagłebione wielomiany przez 1.
        } else if (current_mono.exp == EXP_ZERO) {
            Poly new_compose = PolyComposeHelper(&current_mono.p, k, q,
                                                 (idx + ONE_ELEMENT));
            Poly add = PolyAdd(&final, &new_compose);
            PolyDestroy(&new_compose);
            PolyDestroy(&final);
            final = add;
        }
    }
    if (!PolyIsCoeff(&final)) return DeletePolyZero(final.size, &final.arr);
    return final;

}

Poly PolyCompose(const Poly *p, size_t k, const Poly q[]) {

    return PolyComposeHelper(p, k, q, FIRST_IDX);

}

Poly PolyNeg(const Poly *p) {

    return PolyCloneAndMultiplyByScalar(p, NEG);

}

Poly PolySub(const Poly *p, const Poly *q) {

    Poly neg_poly = PolyNeg(q);
    Poly final_poly = PolyAdd(p, &neg_poly);
    PolyDestroy(&neg_poly);
    return final_poly;

}

/**
 * Funkcja pomocnicza znajdująca maksymalny stopień przy zmiennej o indeksie @p var_idx
 * @param[in] max : maksymalny stopień
 * @param[in] var_idx : indeks zmiennej
 * @param[in] current_idx : aktualny indeks zmiennej
 * @param[in] p : wielomian @f$p@f$
 * @return maksymalny stopień przy zmiennej o indeksie @p var_idx
 */
static poly_exp_t GetDegBy(poly_exp_t max, size_t var_idx, size_t current_idx, const Poly *p) {

    if (PolyIsCoeff(p)) return POLY_COEFF;
    if (var_idx == current_idx) return MonoGetExp(&p -> arr[FIRST_IDX]);

    for (size_t i = FIRST_IDX; i < p -> size; i++) {
        Mono current_mono = p -> arr[i];
        poly_exp_t current_deg =
                GetDegBy(max, var_idx, current_idx + ONE_ELEMENT, &current_mono.p);
        if (current_deg > max) max = current_deg;
    }

    return max;

}

poly_exp_t PolyDegBy(const Poly *p, size_t var_idx) {

    if (PolyIsZero(p)) return POLY_ZERO;

    poly_exp_t max = POLY_COEFF;

    return GetDegBy(max, var_idx, FIRST_IDX, p);

}

/**
 * Funkcja pomocnicza, która w parametrze @p max zapisuje maksyalmy stopień wielomianu @p p
 * @param[in] p : wielomian @f$p@f$
 * @param[in] max : maksymalny stopirń wielomianu @p p
 * @param[in] current_exp : aktualny stopień wielomianu @p p
 */
static void GetDeg(const Poly p, poly_exp_t *max, poly_exp_t current_exp) {

    if (current_exp > *max) *max = current_exp;

    if (!PolyIsCoeff(&p)) {
        for (size_t i = FIRST_IDX; i < p.size; i++) {
            Mono current_mono = p.arr[i];
            GetDeg(current_mono.p, max, current_exp + MonoGetExp(&current_mono));
        }
    }

}

poly_exp_t PolyDeg(const Poly *p) {

    if (PolyIsZero(p)) return POLY_ZERO;
    if (PolyIsCoeff(p)) return POLY_COEFF;

    poly_exp_t max = POLY_COEFF;
    GetDeg(*p, &max, POLY_COEFF);

    return max;

}

bool PolyIsEq(const Poly *p, const Poly *q) {

    if (!PolyIsCoeff(p) && !PolyIsCoeff(q)) {
        if (p -> size != q -> size) return false;
        bool is_eq = true;
        for (size_t i = FIRST_IDX; i < p -> size; i++) {
            Mono mono_p = p -> arr[i];
            Mono mono_q = q -> arr[i];
            if (MonoGetExp(&mono_p) != MonoGetExp(&mono_q)) return false;
            is_eq = is_eq && PolyIsEq(&mono_p.p, &mono_q.p);
        }
        return is_eq;
    } else if (!PolyIsCoeff(p) || !PolyIsCoeff(q)) {
        if (PolyIsCoeff(q) && (AllExpZero(p) || PolyIsZero(p)))
            return (GetCoeff(p) == q->coeff);
        else if (PolyIsCoeff(p) && (AllExpZero(q) || PolyIsZero(q)))
            return (GetCoeff(q) == p->coeff);
        else return false;
    } else return (p -> coeff == q -> coeff);

}

/**
 * Realokuje tablicę @p mono_arr, jeśli jest taka potrzeba
 * @param[in] mono_arr : tablica jednomianów
 * @param[in] current_size : aktualny rozmiar tablicy @p mono_arr
 * @param[in] current_idx : aktualny indeks tablicy @p mono_arr
 */
static void ArrayRealloc(Mono **mono_arr, size_t *current_size, size_t current_idx) {

    if (current_idx >= *current_size) {
        *current_size = (*current_size) * ARR_SIZE;
        *mono_arr = realloc(*mono_arr,(*current_size) * sizeof(Mono));
        CHECK_PTR(mono_arr);
    }

}

/**
 * Dodaje jednomian @p mono do tablicy @p mono_arr pod indeksem @p mono_arr_idx
 * @param[in] mono_arr : tablica jednomianów
 * @param[in] mono_arr_size : rozmiar tablicy @p mono_arr
 * @param[in] mono_arr_idx : indeks tablicy @p mono_arr
 * @param[in] mono : jednomian
 */
static void AddMonoToDynamicArray(Mono **mono_arr, size_t *mono_arr_size,
                           size_t *mono_arr_idx, Mono mono) {

    ArrayRealloc(mono_arr, mono_arr_size, *mono_arr_idx);
    (*mono_arr)[*mono_arr_idx] = mono;
    (*mono_arr_idx)++;

}

Poly PolyAt(const Poly *p, poly_coeff_t x) {

    if (PolyIsCoeff(p)) return PolyFromCoeff(p -> coeff);

    Mono *mono_arr = (Mono *) malloc(ARR_SIZE * sizeof(Mono));
    CHECK_PTR(mono_arr);
    size_t mono_arr_size = ARR_SIZE;
    size_t mono_arr_idx = FIRST_IDX;

    for (size_t i = FIRST_IDX; i < p -> size; i++) {
        Mono current_mono = p -> arr[i];
        poly_coeff_t new_coeff = FastPow(x, MonoGetExp(&current_mono));
        // Jeśli współczynnik aktualnie przetwarzanego jednomianu jest wielomianem
        // stałym, to tworzy jednomian z tym współczynnikiem i wykładnikiem zerowym.
        if (PolyIsCoeff(&current_mono.p)) {
            Poly new_poly = PolyFromCoeff(new_coeff * current_mono.p.coeff);
            AddMonoToDynamicArray(&mono_arr, &mono_arr_size, &mono_arr_idx,
                            MonoFromPoly(&new_poly, EXP_ZERO));
            free(new_poly.arr);
        } else {
            Poly new_poly = PolyCloneAndMultiplyByScalar(&current_mono.p, new_coeff);
            for (size_t k = FIRST_IDX; k < new_poly.size; k++) {
                AddMonoToDynamicArray(&mono_arr, &mono_arr_size, &mono_arr_idx,
                                      new_poly.arr[k]);
            }
            free(new_poly.arr);
        }
    }

    Poly final_poly = PolyAddMonos(mono_arr_idx, mono_arr);
    free(mono_arr);
    if (AllExpZero(&final_poly)) {
        Poly new_poly = PolyFromCoeff(GetCoeff(&final_poly));
        PolyDestroy(&final_poly);
        return new_poly;
    }
    return final_poly;

}