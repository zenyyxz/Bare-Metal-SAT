#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

int is_literal_satisfied(int literal, int assignment);
int is_literal_falsified(int literal, int assignment);
int simd_find_literal(int* lits, int size, int* assigns);

#ifdef __cplusplus
}
#endif

#endif // LOW_LEVEL_H
