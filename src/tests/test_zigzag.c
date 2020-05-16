#include <stdint.h>
#include <stdio.h>

#include "zigzag.h"

int main(void) {
    // Matrice 8x8 de nombres aléatoires entre -1024 et 1023 (output DCT)
    int16_t matrice[8][8];

    int16_t nb = 0;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrice[i][j] = nb;
            nb++;
            printf("%u ", matrice[i][j]);
        }
        printf("\n");
    }

    int16_t tableau[64] = {0};
    zigzag(matrice, tableau);

    printf("Affichage zigzag de la matrice\n");
    int inc = 8;
    for (int i = 0; i < 64; i += inc) {
        for (int j = 0; j < 8; j++) {
            printf("%d ", tableau[i + j]);
        }
        printf("\n");
    }

    zigzag_inplace(matrice);

    printf("Affichage zigzag_inplace de la matrice\n");
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            printf("%u ", matrice[i][j]);
        }
        printf("\n");
    }
}