#include <gplayerdeamon.h>
#include <pystring.h>
#include <direct.h>
#include <gplatformutil.h>
#include <gutil.h>
#include <sstream>
#include <gmd5.h>

#if 0

static FILE * g_fopen ( const char * filename, const char * mode )
{
    printf("fopen: %s\n", filename);
    return (FILE*)1;
}

static int g_fclose ( FILE * stream )
{
    return 0;
}

static size_t g_fwrite ( const void * ptr, size_t size, size_t count, FILE * stream )
{
    return count;
}

#define fopen g_fopen
#define fclose g_fclose
#define fwrite g_fwrite

#endif



#define GET 0
#define POST 1

struct ConnectionInfoStruct
{
    int connectiontype;
    struct MHD_PostProcessor *postprocessor;
    FILE *fp;
    const char *rootdirectory;
    const char *filepath;
    const char *answerstring;
    int answercode;
};

static void requestCompleted(void *cls,
                             struct MHD_Connection *connection,
                             void **con_cls,
                             enum MHD_RequestTerminationCode toe)
{
    ConnectionInfoStruct *con_info = (ConnectionInfoStruct*)(*con_cls);

    if (con_info == NULL)
       return;

    if (con_info->connectiontype == POST)
    {
        if (con_info->postprocessor != NULL)
            MHD_destroy_post_processor (con_info->postprocessor);

        if (con_info->fp != NULL)
        {
            fclose(con_info->fp);
            // if the fp is not NULL, this is an incomplete file, delete it
            remove(con_info->filepath);
        }

        free((void*)con_info->rootdirectory);
        free((void*)con_info->filepath);
    }

    free(con_info);
    *con_cls = NULL;
}

static int iteratePost(void *cls,
                       enum MHD_ValueKind kind,
                       const char *key,
                       const char *filename,
                       const char *content_type,
                       const char *transfer_encoding,
                       const char *data,
                       uint64_t off,
                       size_t size)
{
    ConnectionInfoStruct *con_info = (ConnectionInfoStruct*)cls;

    con_info->answerstring = "error 1";
    con_info->answercode = MHD_HTTP_INTERNAL_SERVER_ERROR;

    if (strcmp(key, "file"))
        return MHD_NO;

    if (filename == NULL)
        return MHD_NO;

    if (con_info->fp == NULL)
    {
        std::string tmpdir = pystring::os::path::join(con_info->rootdirectory, ".tmp");
        _mkdir(tmpdir.c_str());
        std::string filepath = pystring::os::path::join(tmpdir, filename);
        con_info->filepath = strdup(filepath.c_str());
        con_info->fp = fopen(con_info->filepath, "wb");
        if (con_info->fp == NULL)
            return MHD_NO;
    }

    if (size > 0)
    {
        if (!fwrite(data, size, sizeof(char), con_info->fp))
            return MHD_NO;
    }

    con_info->answerstring = "ok";
    con_info->answercode = MHD_HTTP_OK;

    return MHD_YES;
}



GPlayerDaemon::GPlayerDaemon()
{
    d_ = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                          15000,
                          NULL,
                          NULL,
                          handler_s,
                          this,
                          MHD_OPTION_NOTIFY_COMPLETED,
                          requestCompleted,
                          NULL,
                          MHD_OPTION_END);
}

GPlayerDaemon::~GPlayerDaemon()
{
    MHD_stop_daemon(d_);
}

void GPlayerDaemon::setRootDirectory(const char *dir)
{
    rootDirectory_ = dir;
}


int GPlayerDaemon::handler_s(void *cls,
                             struct MHD_Connection *connection,
                             const char *url,
                             const char *method,
                             const char *version,
                             const char *upload_data,
                             size_t *upload_data_size,
                             void **con_cls)
{
    return static_cast<GPlayerDaemon*>(cls)->handler(connection, url, method, version, upload_data, upload_data_size, con_cls);
}

static int sendPage(MHD_Connection *connection, const char* page, int status_code = MHD_HTTP_OK)
{
    MHD_Response *response = MHD_create_response_from_buffer(strlen (page), (void*)page, MHD_RESPMEM_MUST_COPY);

    if (!response)
        return MHD_NO;

    MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_TYPE, "text/html");
    int ret = MHD_queue_response(connection, status_code, response);
    MHD_destroy_response(response);

    return ret;
}

int GPlayerDaemon::handler(struct MHD_Connection *connection,
                           const char *url,
                           const char *method,
                           const char *version,
                           const char *upload_data,
                           size_t *upload_data_size,
                           void **con_cls)
{
    if (*con_cls == NULL)
    {
        ConnectionInfoStruct *con_info = (ConnectionInfoStruct*)malloc(sizeof(ConnectionInfoStruct));
        if (con_info == NULL)
          return MHD_NO;

        con_info->fp = NULL;

        if (!strcmp(method, "POST"))
        {
            con_info->postprocessor = MHD_create_post_processor(connection, 16 * 1024, iteratePost, (void*)con_info);
            if (con_info->postprocessor == NULL)
            {
                free(con_info);
                return MHD_NO;
            }
            con_info->connectiontype = POST;
            con_info->answercode = MHD_HTTP_OK;
            con_info->answerstring = "ok";
            con_info->rootdirectory = strdup(rootDirectory_.c_str());
            con_info->filepath = NULL;
        }
        else
            con_info->connectiontype = GET;

        *con_cls = (void*)con_info;

        return MHD_YES;
    }

    if (!strcmp(method, "GET"))
    {
        return handleGet(connection, url);
    }
    else if (!strcmp(method, "POST"))
    {
        ConnectionInfoStruct *con_info = (ConnectionInfoStruct *)*con_cls;

        if (*upload_data_size > 0)
        {
            MHD_post_process(con_info->postprocessor, upload_data, *upload_data_size);
            *upload_data_size = 0;
            return MHD_YES;
        }

        if (con_info->fp != NULL)
        {
          fclose (con_info->fp);
          con_info->fp = NULL;
        }

        if (con_info->answercode == MHD_HTTP_OK)
            return handlePost(connection, url, con_info->filepath);

        return sendPage(connection, con_info->answerstring, con_info->answercode);
    }

    return sendPage(connection, "error 2", MHD_HTTP_BAD_REQUEST);
}

static char byteMap[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
static void bytesToHexString(const uint8_t bytes[16], char str[33])
{
    int i;

    for (i = 0; i < 16; ++i)
    {
        str[i * 2] = byteMap[bytes[i] >> 4];
        str[i * 2 + 1] = byteMap[bytes[i] & 0x0f];
    }

    str[32] = '\0';
}

int GPlayerDaemon::handleGet(MHD_Connection *connection, const char *url)
{
    std::vector<std::string> paths;
    pystring::split(url + 1, paths, "/");

    if (paths.size() < 1)
        return sendPage(connection, "error");

    if (paths[0] == "play")
    {
        if (paths.size() < 2)
            return sendPage(connection, "error");
        commandQueue_.push_back(std::make_pair(ePlay, paths[1]));
        return sendPage(connection, "ok");
    }
    else if (paths[0] == "stop")
    {
        commandQueue_.push_back(std::make_pair(eStop, std::string()));
        return sendPage(connection, "ok");
    }
    else if (paths[0] == "list")
    {
        if (paths.size() < 2)
            return sendPage(connection, "error");

        std::stringstream stream;

        std::string path = rootDirectory_;
        path = pystring::os::path::join(path, paths[1]);
        std::string md5filename = pystring::os::path::join(path, "md5.bin");
        path = pystring::os::path::join(path, "Resources");

        std::vector<std::string> files;
        gGetDirectoryListingRecursive(path.c_str(), &files, NULL);

        GMD5 md5;
        md5.load(md5filename.c_str());

        stream << "[\n";

        bool updated = false;
        for (std::size_t i = 0; i < files.size(); ++i)
        {
            std::string fullpath = pystring::os::path::join(path, files[i]);

            if (md5.update(files[i].c_str(), fullpath.c_str()))
                updated = true;

            GMD5::iterator iter = md5.find(files[i]);

            if (i > 0)
                stream << ",\n";

            char md5str[33];
            bytesToHexString(&iter->second.second[0], md5str);

            stream << "\t{\"file\": \"" << pystring::replace(files[i], "\\", "/") << "\", \"md5\": \"" << md5str << "\", \"age\": " << time(NULL) - iter->second.first << "}";
        }

        stream << "\n]\n";

        if (updated)
            md5.save(md5filename.c_str());

        return sendPage(connection, stream.str().c_str());
    }
    else if (paths[0] == "delete")
    {
        if (paths.size() < 3)
            return sendPage(connection, "error");

        std::string path = rootDirectory_;
        path = pystring::os::path::join(path, paths[1]);
        std::string md5filename = pystring::os::path::join(path, "md5.bin");
        path = pystring::os::path::join(path, "Resources");
        for (size_t i = 2; i < paths.size(); ++i)
            path = pystring::os::path::join(path, paths[i]);

        std::string key;
        for (size_t i = 2; i < paths.size(); ++i)
            key = pystring::os::path::join(key, paths[i]);

        /* TODO: if remove is succesful retrun ok, else return error */
        remove(path.c_str());

        GMD5 md5;
        md5.load(md5filename.c_str());
        md5.erase(key);
        md5.save(md5filename.c_str());

        return sendPage(connection, "ok");
    }

    return sendPage(connection, "error");
}

int GPlayerDaemon::handlePost(MHD_Connection *connection, const char *url, const char *filepath)
{
    std::vector<std::string> paths;
    pystring::split(url + 1, paths, "/");

    if (paths.size() < 2)
        return sendPage(connection, "error");

    if (paths[0] != "upload")
        return sendPage(connection, "error");

    std::string basename = pystring::os::path::basename(filepath);

    std::string path = rootDirectory_;
    path = pystring::os::path::join(path, paths[1]);
    _mkdir(path.c_str());
    std::string md5filename = pystring::os::path::join(path, "md5.bin");
    path = pystring::os::path::join(path, "Resources");
    _mkdir(path.c_str());
    for (size_t i = 2; i < paths.size(); ++i)
    {
        path = pystring::os::path::join(path, paths[i]);
        _mkdir(path.c_str());
    }
    path = pystring::os::path::join(path, basename);

    rename(filepath, path.c_str());

    std::string key = "";
    for (size_t i = 2; i < paths.size(); ++i)
        key = pystring::os::path::join(key, paths[i]);
    key = pystring::os::path::join(key, basename);

    GMD5 md5;
    md5.load(md5filename.c_str());
    if (md5.update(key.c_str(), path.c_str()))
        md5.save(md5filename.c_str());

    return sendPage(connection, "ok");
}

