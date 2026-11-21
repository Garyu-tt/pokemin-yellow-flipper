#ifndef GLITCHDEX_H
#define GLITCHDEX_H
#include <stdint.h>
#include <stdbool.h>
#define GLITCHDEX_COUNT 105u
uint8_t glitchdex_id(uint8_t index);
bool glitchdex_is_missingno(uint8_t id);
const char* glitchdex_label(uint8_t id);
#endif
