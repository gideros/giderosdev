#include <gplatformutil.h>
#include <pystring.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
#include <Windows.h>

// Convert a wide Unicode string to an UTF8 string
static std::string utf8_encode(const std::wstring &wstr)
{
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert an UTF8 string to a wide Unicode String
static std::wstring utf8_decode(const std::string &str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}
#endif

#ifdef _WIN32
void gGetDirectoryListing(const char *dir, std::vector<std::string> *files, std::vector<std::string> *directories)
{
    WIN32_FIND_DATAW ffd;
    HANDLE hFind;

    std::wstring dirstar = utf8_decode(pystring::os::path::join(dir, "*"));

    hFind = FindFirstFileW(dirstar.c_str(), &ffd);

    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            if (!wcscmp(ffd.cFileName, L".") || !wcscmp(ffd.cFileName, L".."))
                continue;

        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            if (directories)
                directories->push_back(utf8_encode(ffd.cFileName));
        }
        else
        {
            if (files)
                files->push_back(utf8_encode(ffd.cFileName));
        }
    } while (FindNextFileW(hFind, &ffd) != 0);

    FindClose(hFind);
}
#else
void getDirectoryListing(const char* dir, std::vector<std::string>* files, std::vector<std::string>* directories)
{
    struct dirent *entry;
    DIR *dp;

    dp = opendir(dir);
    if (dp == NULL)
        return;

    while((entry = readdir(dp)))
    {
        if (entry->d_type == DT_DIR)
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
                continue;

        if (entry->d_type == DT_DIR)
        {
            if (directories)
                directories->push_back(entry->d_name);
        }
        else if (entry->d_type == DT_REG)
        {
            if (files)
                files->push_back(entry->d_name);
        }
    }

    closedir(dp);
}
#endif

#ifdef _WIN32
time_t gFileLastModifiedTime(const char *file)
{
    struct _stat s;
    _wstat(utf8_decode(file).c_str(), &s);

    return s.st_mtime;
}
#else
time_t gFileLastModifiedTime(const char *file)
{
    struct stat s;
    stat(file, &s);

    return s.st_mtime;
}
#endif

time_t gFileAge(const char *file)
{
    return time(NULL) - gFileLastModifiedTime(file);
}
