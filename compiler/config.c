#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "config.h"

int config_load_file(const char *filename, ConfigMap *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return 0;
    }

    char line[1024];
    config->count = 0;
    config->capacity = 16;
    config->entries = malloc(sizeof(ConfigEntry) * config->capacity);

    while (fgets(line, sizeof(line), file)) {
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
            len--;
        }

        if (len == 0 || line[0] == '#' || line[0] == '$') {
            continue;
        }

        char *equals = strchr(line, '=');
        if (equals) {
            *equals = '\0';
            char *key = line;
            char *value = equals + 1;

            while (isspace(*key)) key++;
            while (isspace(*value)) value++;
            
            char *key_end = key + strlen(key) - 1;
            while (key_end > key && isspace(*key_end)) {
                *key_end = '\0';
                key_end--;
            }
            
            char *value_end = value + strlen(value) - 1;
            while (value_end > value && isspace(*value_end)) {
                *value_end = '\0';
                value_end--;
            }

            if (value[0] == '"' && value[strlen(value) - 1] == '"') {
                value[strlen(value) - 1] = '\0';
                value++;
            }

            if (config->count >= config->capacity) {
                config->capacity *= 2;
                config->entries = realloc(config->entries, sizeof(ConfigEntry) * config->capacity);
            }

            config->entries[config->count].key = strdup(key);
            config->entries[config->count].value = strdup(value);
            config->count++;
        }
    }

    fclose(file);
    return 1;
}

const char* config_get(ConfigMap *config, const char *key) {
    for (size_t i = 0; i < config->count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            return config->entries[i].value;
        }
    }
    return NULL;
}

void config_destroy(ConfigMap *config) {
    if (config && config->entries) {
        for (size_t i = 0; i < config->count; i++) {
            free(config->entries[i].key);
            free(config->entries[i].value);
        }
        free(config->entries);
        config->count = 0;
        config->capacity = 0;
    }
}
