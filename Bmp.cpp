#include "Bmp.h"
#include <cstdio>
#include <utility>
#include <limits>

std::uint32_t readLittleEndian(
    const std::vector<std::uint8_t>& data,
    std::size_t offset,
    unsigned count)
{
    std::uint32_t result = 0;

    for (unsigned i = 0; i < count; ++i)
    {
        result |= static_cast<std::uint32_t>(data[offset + i]) << (8 * i);
    }

    return result;
}

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

    const std::uint32_t offset = readLittleEndian(data, 10, 4);
    const std::uint32_t headerSize = readLittleEndian(data, 14, 4);
    const std::uint32_t imageWidth = readLittleEndian(data, 18, 4);
    const std::uint32_t imageHeight = readLittleEndian(data, 22, 4);
    const std::uint32_t planes = readLittleEndian(data, 26, 2);
    const std::uint32_t bitsPerPixel = readLittleEndian(data, 28, 2);
    const std::uint32_t compression = readLittleEndian(data, 30, 4);
    const std::uint32_t fileSize = readLittleEndian(data, 2, 4);

    if (headerSize != 40 || planes != 1 ||
        bitsPerPixel != 24 || compression != 0 ||
        static_cast<std::uint64_t>(fileSize) != data.size())
    {
        return false;
    }

    if (imageWidth == 0 || imageHeight == 0 ||
        imageWidth > std::numeric_limits<std::int32_t>::max() ||
        imageHeight > std::numeric_limits<std::int32_t>::max())
    {
        return false;
    }

    if (offset < 54 || offset > data.size())
    {
        return false;
    }

    const std::uint64_t rowBytes =
        static_cast<std::uint64_t>(imageWidth) * 3;

    const std::uint64_t stride =
        ((rowBytes + 3) / 4) * 4;

    const std::uint64_t pixelDataSize =
        stride * imageHeight;

    if (pixelDataSize > data.size() - offset)
    {
        return false;
    }

    width = static_cast<std::int32_t>(imageWidth);
    height = static_cast<std::int32_t>(imageHeight);
    pixelOffset = offset;
    rowStride = static_cast<std::size_t>(stride);

    bytes = std::move(data);
    return true;
}

bool Bmp::save(const std::string& path) const
{
    if (bytes.empty())
    {
        return false;
    }

    FILE* file = std::fopen(path.c_str(), "wbx");

    if (file == nullptr)
    {
        return false;
    }

    const std::size_t written =
        std::fwrite(bytes.data(), 1, bytes.size(), file);

    const int closeResult = std::fclose(file);

    if (written != bytes.size() || closeResult != 0)
    {
        std::remove(path.c_str());
        return false;
    }

    return true;
}

std::size_t Bmp::componentOffset(std::size_t componentIndex) const
{
    const std::size_t colorBytesPerRow =
        static_cast<std::size_t>(width) * 3;

    const std::size_t row =
        componentIndex / colorBytesPerRow;

    const std::size_t positionInRow =
        componentIndex % colorBytesPerRow;

    return static_cast<std::size_t>(pixelOffset)
        + row * rowStride
        + positionInRow;
}

std::size_t Bmp::capacityBits() const
{
    if (bytes.empty())
    {
        return 0;
    }

    return static_cast<std::size_t>(width)
        * static_cast<std::size_t>(height)
        * 3;
}

bool Bmp::writeBit(std::size_t index, std::uint8_t bit)
{
    if (index >= capacityBits() || bit > 1)
    {
        return false;
    }

    const std::size_t offset = componentOffset(index); // permet de sauter les octets de padding

    bytes[offset] = static_cast<std::uint8_t>(
        (bytes[offset] & 0xFEu) | bit
        );

    return true;
}

bool Bmp::readBit(std::size_t index, std::uint8_t& bit) const
{
    if (index >= capacityBits()) // empeche de lire ou modifier une composante hors de l'image
    {
        return false;
    }

    const std::size_t offset = componentOffset(index);

    bit = bytes[offset] & 1u; // Conserve uniquement le dernier bit de l'octet.

    return true;
}
