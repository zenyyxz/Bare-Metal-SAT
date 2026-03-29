#ifndef LOW_LEVEL_H
#define LOW_LEVEL_H

#ifdef __cplusplus
extern "C" {
#endif

int is_literal_satisfied(int literal, int assignment);
int is_literal_falsified(int literal, int assignment);

#ifdef __cplusplus
}
#endif

#endif // LOW_LEVEL_H
