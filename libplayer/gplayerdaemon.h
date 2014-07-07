#ifndef GPLAYERDEAMON_H
#define GPLAYERDEAMON_H

#include <string>
#include <deque>
#include <map>
#include <vector>

#ifdef _WIN32
#ifdef PLAYERDAEMON_LIBRARY
#define PLAYERDAEMON_API __declspec(dllexport)
#else
#define PLAYERDAEMON_API __declspec(dllimport)
#endif
#else
#define PLAYERDAEMON_API
#endif

struct MHD_Connection;
struct MHD_Daemon;


class PLAYERDAEMON_API GPlayerDaemon
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
};

#endif
