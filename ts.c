/* ts.c - Alias for tess command */
/* This allows 'ts' to work as an alias for 'tess' */

int main(int argc, char *argv[]) {
w
    extern int main_tess(int argc, char *argv[]);
    return main_tess(argc, argv);
}

