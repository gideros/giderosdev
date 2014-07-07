#include <gutil.h>
#include <gplatformutil.h>
#include <stack>
#include <string.h>
#include <pystring.h>

void gGetDirectoryListingRecursive(const char *directory, std::vector<std::string> *filesout, std::vector<std::string> *directoriesout)
{
    std::stack<std::string> stack;

    std::string dir = directory;

    stack.push("");

    while (!stack.empty())
    {
        std::string subdir = stack.top();
        stack.pop();

        std::vector<std::string> files, directories;
        gGetDirectoryListing(pystring::os::path::join(dir, subdir).c_str(), &files, &directories);

        for (std::size_t i = 0; i < files.size(); ++i)
        {
            std::string str = pystring::os::path::join(subdir, files[i]);
            if (filesout)
                filesout->push_back(str);
        }

        for (std::size_t i = 0; i < directories.size(); ++i)
        {
            std::string str = pystring::os::path::join(subdir, directories[i]);
            if (directoriesout)
                directoriesout->push_back(str);
            stack.push(str);
        }
    }
}
