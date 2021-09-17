#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <float.h>

/*
 * This program uses arbitrary precision floating point libraries to
 * compute the exact number of expected collisions given a number of
 * hashes and a hash width. It is used to generate the tables in
 * main.cpp that are used to evaluate the EstimateNbCollisions()
 * implementation.
 *
 * This can be compiled via either
 *   gcc -O3 -march=native -o exactcoll exactcoll.c -lmpfi -lmpfr -lgmp -lm -DUSE_MPFI
 *       OR
 *   gcc -O3 -march=native -o exactcoll exactcoll.c        -lmpfr -lgmp -lm -DUSE_MPFR
 *
 * If MPFI is used, then the program can detect that an insufficient
 * number of bits were used, and that the number should be increased
 * to get a correct result. MPFI is less widely available than MPFR,
 * however, and the APIs are naturally quite similar, so that is also
 * provided as an option.
 */

/* Number of bits of precision to use */
#define PRECISION 768
/* Number of digits to emit beyond that specified by DBL_DECIMAL_DIG */
#define EXTRA_DIGITS 0

#ifndef USE_MPFI
#ifndef USE_MPFR
#error "Exactly one of USE_MPFI and USE_MPFR must be defined"
#endif
#endif

#ifdef USE_MPFI
#ifdef USE_MPFR
#error "Exactly one of USE_MPFI and USE_MPFR must be defined"
#endif
#endif

#ifdef USE_MPFI
#include <mpfi.h>
#include <mpfi_io.h>
typedef mpfi_t mp_t;
#else
#include <mpfr.h>
typedef mpfr_t mp_t;
#endif

char buf[3*PRECISION];
FILE * membuf;

#ifdef USE_MPFI
#define MP(x,...) mpfi_##x(__VA_ARGS__)
#else
#define MP(x,...) mpfr_##x(__VA_ARGS__, MPFR_RNDN)
#endif

void printcoll(uint64_t balls, uint64_t log2bins) {
    mp_t m, n, p, e, f, c;

#ifdef USE_MPFI
    mpfi_init(m);
    mpfi_init(n);
    mpfi_init(p);
    mpfi_init(e);
    mpfi_init(f);
    mpfi_init(c);
#else
    mpfr_inits(m, n, p, e, f, c, NULL);
#endif

    /* m = balls, n = bins */
    MP(set_ui, m, balls);
    MP(set_ui, n, log2bins);
    MP(exp2, n, n);

    /* Probability that a given bin is unoccupied after 1 ball */
    /* p = (1-(1/n)) */
    MP(set_ui, p, 1);
    MP(div_2ui, p, p, log2bins);
    MP(ui_sub, p, 1, p);

    /* Probability that a given bin is unoccupied after m balls */
#ifdef USE_MPFI /* There is no mpfi_pow() :-(  */
    /* p = log(1-(1/n)) */
    MP(log, p, p);
    /* p = m*log(1-(1/n)) */
    MP(mul, p, p, m);
    /* p = exp(m*log(1-(1/n))) = (1-(1/n))**m */
    MP(exp, p, p);
#else
    /* p = (1-(1/n))**m */
    MP(pow, p, p, m);
#endif

    /* Expected number of empty bins after m balls */
    /* e = n*p */
    MP(mul, e, p, n);

    /* Expected number of full bins after m balls */
    /* f = n-e = n-n*p */
    MP(sub, f, n, e);

    /* Expected number of collisions */
    /* c = m-f = m-(n-n*p) */
    MP(sub, c, m, f);

#if 0
    printf("Results for %ld, %ld:\n", balls, log2bins);
    printf("p(empty)\n");
    MP(out_str, stdout, 10, 0, p);
    printf("\nE(empty)\n");
    MP(out_str, stdout, 10, 0, e);
    printf("\nE(full)\n");
    MP(out_str, stdout, 10, 0, f);
    printf("\nm - E(full)\n");
    MP(out_str, stdout, 10, 0, c);
#endif

    /* Print the entire result to a buffer */
    memset(buf, 0, sizeof(buf));
    MP(out_str, membuf, 10, 0, c);
    fflush(membuf);
#ifdef USE_MPFI
    /*
     * If we have interval bounds, ensure that the digits that fit in
     * a double match (that the bounds are tighter than a double can
     * represent).
     */
    double lb = strtod(&buf[1], NULL);
    double ub = strtod(strchr(buf, ',')+1, NULL);
    if (lb != ub) {
        printf("BOUNDS DO NOT MATCH TO DOUBLE PRECISION!\n");
        printf("Increase PRECISION and recompile.\n");
        exit(1);
    }
#else
    double lb = strtod(&buf[0], NULL);
#endif
    /*
     * Print the result to a buffer, read it back, and ensure the
     * value is unchanged. This lets us verify that enough digits are
     * printed so that the result is accurate.
     */
    snprintf(buf, sizeof(buf), "%.*e", DBL_DECIMAL_DIG + (EXTRA_DIGITS), lb);
    double lbr = strtod(&buf[0], NULL);
    if (lb != lbr) {
        printf("Did not read back value with correct precision.\n");
        printf("Increase EXTRA_DIGITS slightly and recompile\n");
        exit(1);
    }
    printf("%s", buf);

#ifdef USE_MPFI
    mpfi_clear(m);
    mpfi_clear(n);
    mpfi_clear(p);
    mpfi_clear(e);
    mpfi_clear(f);
    mpfi_clear(c);
#else
    mpfr_clears(m, n, p, e, f, c, NULL);
#endif
}

int main(void) {
    mpfr_set_default_prec(PRECISION);
    membuf = fmemopen(buf, sizeof(buf), "w");

    const uint64_t keys[] = {
        149633745, 86536545, 75498113, 56050289, 49925029,
        44251425, 43691201, 33558529, 33554432, 26977161,
        22370049, 18877441, 18616785, 17676661, 16777216,
        16777214, 15082603, 14986273, 14776336, 14196869,
        12204240, 11017633, 9437505, 8390657, 8388608,
        8303633, 6445069, 5471025, 5461601, 5000000,
        4720129, 4598479, 4514873, 4216423, 4194304,
        4000000, 3981553, 3469497, 2796417, 2396744,
        2098177, 2097152, 1271626, 1180417, 1048576,
        1000000, 819841, 652545, 524801, 401857,
        264097, 204800, 200000, 102774, 100000,
        77163, 50643, 6
    };
    const uint64_t bits[] = {256, 224, 160, 128, 64, 55, 45, 42, 39, 36, 32, 29, 27, 24, 22, 19, 12, 8};
    const uint64_t keycnt = sizeof(keys)/sizeof(keys[0]);
    const uint64_t bitcnt = sizeof(bits)/sizeof(bits[0]);

    printf("double realcoll[%d][%d] = {\n", keycnt, bitcnt);

    for (int i = 0; i < keycnt; i++) {
        const uint64_t key = keys[i];
        printf("    /* %d */\n    { ", key);
        for (int j = 0; j < bitcnt; j++) {
            const uint64_t bit = bits[j];
            printcoll(key, bit);
            if (j == bitcnt - 1) {
                printf(" },\n");
            } else if ((j%3)==2) {
                printf(",\n      ");
            } else {
                printf(", ");
            }

        }
    }

    printf("};\n\n    const int keys[] = {\n      ");
    for (int i = 0; i < keycnt; i++) {
        printf("%d", (int)keys[i]);
        if (i == keycnt - 1) {
            printf("\n    };\n");
        } else if ((i % 6) == 5) {
            printf(",\n      ");
        } else {
            printf(", ");
        }
    }

    printf("    const int bits[] = { ");
    for (int i = 0; i < bitcnt; i++) {
        printf("%d", (int)bits[i]);
        if (i == bitcnt - 1) {
            printf(" };\n");
        } else {
            printf(", ");
        }
    }

    fclose(membuf);

    return 0;
}
