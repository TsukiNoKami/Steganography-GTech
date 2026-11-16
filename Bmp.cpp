#include "Bmp.h"
#include <cstdio>
#include <utility>

bool Bmp::load(const std::string& path)
{
    FILE* file = std::fopen(path.c_str(), "rb");

    if (file == nullptr)
    {
        return false;
    }

    std::vector<std::uint8_t> data;
    std::uint8_t buffer[4096];

    std::size_t bytesRead;

    do
    {
        bytesRead = std::fread(buffer, 1, sizeof(buffer), file);
        data.insert(data.end(), buffer, buffer + bytesRead);
    } while (bytesRead == sizeof(buffer));

    const bool readOk = std::ferror(file) == 0;
    std::fclose(file);

    if (!readOk || data.size() < 54 ||
        data[0] != 'B' || data[1] != 'M')
    {
        return false;
    }

    bytes = std::move(data);
    return true;
}
