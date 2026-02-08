#ifndef TESS_H
#define TESS_H

int tess_run(const char *filename);
int tess_build(const char *filename);
int tess_install(const char *package);
int tess_uninstall(const char *package);
int tess_update(const char *package);
int tess_new_project(const char *name);
int tess_info(void);
int tess_repl(void);
int tess_exec(const char *code);
int tess_version(void);
int tess_fmt(const char *file);
int tess_lint(const char *file);
int tess_check(const char *file);
int tess_venv(const char *dir);
int tess_test(void);

#endif
