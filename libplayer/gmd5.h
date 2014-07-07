#ifndef GMD5_H
#define GMD5_H

#include <map>
#include <string>
#include <vector>
#include <stdint.h>

class GMD5 : public std::map<std::string, std::pair<time_t, std::vector<uint8_t> > >
{
public:
    void load(const char *filename);
    void save(const char *filename) const;
    bool update(const char *key, const char *filename);
};

#endif
