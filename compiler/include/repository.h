#ifndef REPOSITORY_H
#define REPOSITORY_H

int repository_download(const char *package_name, const char *version);
int repository_search(const char *query);

#endif
