#ifndef GPLATFORMUTIL_H
#define GPLATFORMUTIL_H

#include <vector>
#include <string>
#include <time.h>

void gGetDirectoryListing(const char *dir, std::vector<std::string> *files, std::vector<std::string> *directories);
time_t gFileAge(const char *file);
time_t gFileLastModifiedTime(const char *file);

#endif
