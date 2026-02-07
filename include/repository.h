#ifndef REPOSITORY_H
#define REPOSITORY_H

int repository_search(const char *query);
int repository_download(const char *package_name, const char *version);

#endif /* REPOSITORY_H */

