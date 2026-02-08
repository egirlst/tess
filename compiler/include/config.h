#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>

typedef struct {
    char *key;
    char *value;
} ConfigEntry;

typedef struct {
    ConfigEntry *entries;
    size_t count;
    size_t capacity;
} ConfigMap;

int config_load_file(const char *filename, ConfigMap *config);
const char* config_get(ConfigMap *config, const char *key);
void config_destroy(ConfigMap *config);

#endif
