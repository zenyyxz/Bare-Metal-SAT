#ifndef BITSET_H
#define BITSET_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint64_t* bits;
    size_t size;
} Bitset;

Bitset* bitset_create(size_t size);
void bitset_destroy(Bitset* b);
void bitset_set(Bitset* b, size_t index);
void bitset_clear(Bitset* b, size_t index);
int bitset_get(Bitset* b, size_t index);
void bitset_reset(Bitset* b);

#ifdef __cplusplus
}
#endif

#endif // BITSET_H
