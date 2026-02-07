#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#include <io.h>
#define mkdir(path, mode) _mkdir(path)
#define access _access
#define F_OK 0
#else
#include <unistd.h>
#endif
#include "tess.h"
#include "config.h"
#include "repository.h"

#define TESS_PACKAGE_DIR ".tess_packages"
#define TESS_GLOBAL_DIR "tess_packages"

static int ensure_package_dir(void) {
    if (access(TESS_PACKAGE_DIR, F_OK) != 0) {
        if (mkdir(TESS_PACKAGE_DIR, 0755) != 0) {
            perror("Failed to create package directory");
            return 0;
        }
    }
    return 1;
}

int tess_install(const char *package) {
    if (strcmp(package, ".") == 0) {
        /* Install dependencies from .tess.noah (like npm install) */
        ConfigMap noah = {0};
        if (config_load_file(".tess.noah", &noah)) {
            const char *name = config_get(&noah, "name");
            const char *version = config_get(&noah, "version");
            
            if (name) {
                printf("Project: %s", name);
                if (version) {
                    printf(" v%s", version);
                }
                printf("\n");
            }
            
            /* Install dependencies */
            int deps_installed = 0;
            for (size_t i = 0; i < noah.count; i++) {
                if (strcmp(noah.entries[i].key, "dependencies") == 0 ||
                    strncmp(noah.entries[i].key, "dep", 3) == 0) {
                    /* Parse dependencies (comma-separated or space-separated) */
                    char *deps = strdup(noah.entries[i].value);
                    char *dep = strtok(deps, ", ");
                    while (dep) {
                        printf("Installing dependency: %s\n", dep);
                        /* Install to local .tess_packages directory */
                        if (repository_download(dep, NULL) == 0) {
                            deps_installed++;
                        } else {
                            printf("Warning: Could not install '%s'\n", dep);
                        }
                        dep = strtok(NULL, ", ");
                    }
                    free(deps);
                }
            }
            
            if (deps_installed > 0) {
                printf("Installed %d dependencies\n", deps_installed);
            } else {
                printf("No dependencies found in .tess.noah\n");
            }
            
            config_destroy(&noah);
            return 0;
        } else {
            printf("Error: No .tess.noah file found in current directory\n");
            return 1;
        }
    }
    
    /* Install specific package to local project */
    if (!ensure_package_dir()) {
        return 1;
    }
    
    printf("Installing package '%s' to project...\n", package);
    
    /* Try to download from repository to local .tess_packages */
    if (repository_download(package, NULL) == 0) {
        printf("Package '%s' installed to project!\n", package);
        return 0;
    }
    
    printf("Package not found in repository.\n");
    return 1;
}

int tess_uninstall(const char *package) {
    printf("Removing package '%s'...\n", package);
    printf("(Package removal not yet implemented)\n");
    return 0;
}

int tess_update(const char *package) {
    printf("Updating package '%s'...\n", package);
    printf("(Package update not yet implemented)\n");
    return 0;
}

