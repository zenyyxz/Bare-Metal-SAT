#include "../include/bitset.h"
#include <stdlib.h>
#include <string.h>

Bitset* bitset_create(size_t size) {
    Bitset* b = (Bitset*)malloc(sizeof(Bitset));
    b->size = size;
    size_t num_words = (size + 63) / 64;
    b->bits = (uint64_t*)calloc(num_words, sizeof(uint64_t));
    return b;
}

void bitset_destroy(Bitset* b) {
    if (b) {
        free(b->bits);
        free(b);
    }
}

void bitset_reset(Bitset* b) {
    if (b) {
        size_t num_words = (b->size + 63) / 64;
        memset(b->bits, 0, num_words * sizeof(uint64_t));
    }
}
