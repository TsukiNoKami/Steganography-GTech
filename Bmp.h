#pragma once

#include <vector>
#include <cstdint>
#include <cstddef>
#include <string>

class Bmp
{
public:
    bool load(const std::string& path);

private:
    std::vector<std::uint8_t> bytes;
    std::int32_t width = 0;
    std::int32_t height = 0;
    std::uint32_t pixelOffset = 0;
    std::size_t rowStride = 0;
};
