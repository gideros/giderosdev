#ifndef GPLAYERBRIDGE_H
#define GPLAYERBRIDGE_H

#include <string>
#include <microhttpd.h>
#include <deque>
#include <map>
#include <vector>

class GPlayerDaemon
{
public:
    enum GCommandType
    {
        eNone,
        ePlay,
        eStop,
    };

    GPlayerDaemon();
    ~GPlayerDaemon();

    void setRootDirectory(const char *dir);

    void print(const char *str);
    std::pair<GCommandType, std::string> dequeueCommand();

private:
    static int handler_s(void *cls,
                         struct MHD_Connection *connection,
                         const char *url,
                         const char *method,
                         const char *version,
                         const char *upload_data,
                         size_t *upload_data_size,
                         void **con_cls);

    int handler(struct MHD_Connection *connection,
                const char *url,
                const char *method,
                const char *version,
                const char *upload_data,
                size_t *upload_data_size,
                void **con_cls);

    int handleGet(MHD_Connection *connection, const char *url);
    int handlePost(MHD_Connection *connection, const char *url, const char *filepath);

    MHD_Daemon *d_;
    std::string rootDirectory_;

private:
    std::deque<std::pair<GCommandType, std::string> > commandQueue_;

#if 0
private:
    typedef std::map<std::string, std::pair<time_t, std::vector<unsigned char> > > MD5;
    static void loadMD5(MD5 &md5, const char *filename);
    static void saveMD5(const MD5 &md5, const char *filename);
    static bool updateMD5(MD5 &md5, const char *key, const char *filename);
#endif
};

#endif
