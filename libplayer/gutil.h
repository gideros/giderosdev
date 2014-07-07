#ifndef GUTIL_H
#define GUTIL_H

#include <vector>
#include <string>

void gGetDirectoryListingRecursive(const char *dir, std::vector<std::string> *filesout, std::vector<std::string> *directoriesout);



#endif
