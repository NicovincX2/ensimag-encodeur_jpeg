#include "encode_ACDC.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>  // abs()

#include "bitstream.h"
#include "huffman.h"
#include "verbose.h"

/* type: int16_t
 * rtype: uint8_t
 * Calcule la classe de magnitude correpondant
 * à la valeur freq et la renvoie
 */
static uint8_t get_magnitude(int16_t freq) {
    /* On passe en positif*/
    freq = (int16_t)(freq > 0 ? freq : -freq);
    uint8_t classe = 0;
    while (freq > 0) {
        freq >>= 1;
        classe++;
    }
    return classe;
}

/* type: bitstream*, huff_table*, int16_t
 * rtype: void
 * Calcule l'indice de la valeur passé en paramètre
 * et sa classe de magnitude, les encode et écrit dans
 * le bitstream
 */
static void encode_DC_freq(bitstream *stream, huff_table *dc_table, int16_t difference_DC) {
    /* On code la différence entre valeurs DC de deux blocs consécutifs */
    uint8_t classe_magnitude = get_magnitude(difference_DC);
    uint16_t indice = (uint16_t)abs(difference_DC);
    if (difference_DC < 0) {
        indice = (uint16_t)((1 << classe_magnitude) - indice - 1);
    }
    if (verbose) printf("Magnitude: %u, index: %u\n", classe_magnitude, indice);

    uint8_t nb_bits_magnitude = 0;
    uint32_t code_magnitude = huffman_table_get_path(dc_table, classe_magnitude, &nb_bits_magnitude);

    bitstream_write_bits(stream, code_magnitude, nb_bits_magnitude, false);
    bitstream_write_bits(stream, indice, classe_magnitude, false);
}

/* type: bitstream*, huff_table*, int16_t, uint8_t
 * rtype: void
 * Calcule l'indice de la valeur passé en paramètre
 * et sa classe de magnitude ainsi que le nombre de
 * ZRL et de 0 à encoder, les encode et écrit dans
 * le bitstream
 */
static void encoder_AC_freq(bitstream *stream, huff_table *ac_table, int16_t freq_AC, uint8_t zeros_count) {
    uint8_t classe_magnitude = get_magnitude(freq_AC);
    uint16_t indice = (uint16_t)abs(freq_AC);
    if (freq_AC < 0) {
        indice = (uint16_t)((1 << classe_magnitude) - indice - 1);
    }
    if (verbose) printf("Magnitude: %u, index: %u\n", classe_magnitude, indice);
    /*
        On doit coder le nombre de coefficients nuls puis la classe de magnitude
        On écrit un octet.
        Bitshifts the input 4 bits to the left, then masks by the lower 4 bits.
        Le deuxième masquage est sécuritaire.
    */
    if (verbose) printf("Nombre de zéros %u ", zeros_count);
    uint8_t value = (uint8_t)(((zeros_count << 4) & 0xf0) | (classe_magnitude & 0x0f));
    if (verbose) printf("RLE %u \n", value);
    uint8_t nb_bits_zeros_magnitude = 0;
    uint32_t code_RLE = huffman_table_get_path(ac_table, value, &nb_bits_zeros_magnitude);
    bitstream_write_bits(stream, code_RLE, nb_bits_zeros_magnitude, false);
    bitstream_write_bits(stream, indice, classe_magnitude, false);
}

void ecrire_coeffs(bitstream *stream, const int16_t data_unit[8][8], huff_table *dc_table, huff_table *ac_table, int16_t difference_DC) {
    uint8_t zeros_count = 0;
    difference_DC = (int16_t)(data_unit[0][0] - difference_DC);

    if (verbose) printf("Recherche du dernier coefficient non nul\n");
    uint8_t last_non_zero_line = 0;
    uint8_t last_non_zero_col = 0;
    for (int8_t i = 7; i >= 0; i--) {
        for (int8_t j = 7; j >= 0; j--) {
            if (data_unit[i][j] != 0) {
                last_non_zero_line = (uint8_t)i;
                last_non_zero_col = (uint8_t)j;
                goto got_last_non_zero;
            }
        }
    }
// We use goto to break of 2 loops
got_last_non_zero:

    if (verbose) printf("Last non zero: (%d, %d)\n", last_non_zero_line, last_non_zero_col);

    if (verbose) printf("Encodage\n");
    for (size_t i = 0; i < 8; i++) {
        for (size_t j = 0; j < 8; j++) {
            if (i == 0 && j == 0) {
                if (verbose) printf("Encodage DC\n");
                if (verbose) printf("Valeur %d ", difference_DC);
                encode_DC_freq(stream, dc_table, difference_DC);
                if (verbose) printf("Encodage AC\n");
                continue;
            }
            if (data_unit[i][j] == 0) {
                zeros_count++;
                if (zeros_count == 16) {
                    // Write ZRL
                    uint8_t nb_bits_ZRL = 0;
                    uint32_t code_ZRL = huffman_table_get_path(ac_table, 0xf0, &nb_bits_ZRL);
                    // ou 240 en décimal
                    if (verbose) printf("Code ZRL: %u\n", code_ZRL);
                    bitstream_write_bits(stream, code_ZRL, nb_bits_ZRL, false);
                    zeros_count = 0;
                }
            } else {
                if (verbose) printf("Valeur %d ", data_unit[i][j]);
                encoder_AC_freq(stream, ac_table, data_unit[i][j], zeros_count);
                zeros_count = 0;
            }
            if (i == last_non_zero_line && j == last_non_zero_col) {
                goto writeEOB;
            }
        }
    }
// Write EOB only if we are not on the last coefficient
writeEOB:;
    if (!(last_non_zero_col == 7 && last_non_zero_line == 7)) {
        uint8_t nb_bits_EOB = 0;
        uint32_t code_EOB = huffman_table_get_path(ac_table, 0x00, &nb_bits_EOB);
        bitstream_write_bits(stream, code_EOB, nb_bits_EOB, false);
    }
}
