#include <gmd5.h>

extern "C" {
#include <md5.h>
}
#include <gplatformutil.h>
#include <stdint.h>


static bool md5_fromfile(const char* filename, uint8_t md5sum[16])
{
    FILE* f = fopen(filename, "rb");

    if (f == NULL)
        return false;

    md5_context ctx;
    uint8_t buf[1000];

    md5_starts( &ctx );

    int i;
    while( ( i = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
    {
        md5_update( &ctx, buf, i );
    }

    md5_finish( &ctx, md5sum );

    fclose(f);

    return true;
}

void GMD5::load(const char *md5filename_)
{
    FILE *fis = fopen(md5filename_, "rb");

    if (fis == NULL)
        return;

    this->clear();

    int nfiles;
    fread(&nfiles, sizeof(int), 1, fis);

    for (int i = 0; i < nfiles; ++i)
    {
        int strlen;
        fread(&strlen, sizeof(int), 1, fis);

        std::vector<char> buffer(strlen);
        fread(&buffer[0], 1, strlen, fis);
        std::string filename(&buffer[0], strlen);

        time_t time;
        fread(&time, sizeof(time_t), 1, fis);

        std::vector<uint8_t> md5(16);
        fread(&md5[0], 1, 16, fis);

        (*this)[filename] = std::make_pair(time, md5);
    }

    fclose(fis);
}

void GMD5::save(const char *md5filename_) const
{
    FILE* fos = fopen(md5filename_, "wb");
    if (fos == NULL)
        return;

    int nfiles = this->size();
    fwrite(&nfiles, sizeof(int), 1, fos);

    const_iterator iter, end = this->end();
    for (iter = this->begin(); iter != end; ++iter)
    {
        int strlen = iter->first.size();
        fwrite(&strlen, sizeof(int), 1, fos);
        fwrite(iter->first.c_str(), 1, strlen, fos);
        fwrite(&iter->second.first, sizeof(time_t), 1, fos);
        fwrite(&iter->second.second[0], 1, 16, fos);
    }

    fclose(fos);
}

bool GMD5::update(const char *key, const char *filename)
{
    iterator iter = this->find(key);
    time_t time = gFileLastModifiedTime(filename);

    if (iter == this->end() || iter->second.first != time)
    {
        std::vector<uint8_t> md5v(16);
        if (md5_fromfile(filename, &md5v[0]))
        {
            (*this)[key] = std::make_pair(time, md5v);
            return true;
        }
    }

    return false;
}

