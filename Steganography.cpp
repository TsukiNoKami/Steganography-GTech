#include "Steganography.h"
#include "XOR.h"
#include <cstdint>
#include <limits>
#include <utility>

namespace
{
    constexpr std::size_t HEADER_BYTES = 9;
    constexpr std::uint8_t SIGNATURE[4] = { 'S', 'T', 'E', 'G' };

    bool writeByte(Bmp& image, std::size_t& position,
        std::uint8_t value)
    {
        for (unsigned bit = 0; bit < 8; ++bit)
        {
            const std::uint8_t bitValue =
                (value >> bit) & 1u;

            if (!image.writeBit(position, bitValue))
            {
                return false;
            }

            ++position;
        }

        return true;
    }

    bool readByte(const Bmp& image, std::size_t& position,
        std::uint8_t& value)
    {
        value = 0;

        for (unsigned bit = 0; bit < 8; ++bit)
        {
            std::uint8_t bitValue = 0;

            if (!image.readBit(position, bitValue))
            {
                return false;
            }

            value |= static_cast<std::uint8_t>(bitValue << bit);
            ++position;
        }

        return true;
    }
}

bool canEmbed(const Bmp& image, std::size_t messageSize)
{
    const std::size_t availableBytes = image.capacityBits() / 8;

    if (availableBytes < HEADER_BYTES ||
        messageSize > availableBytes - HEADER_BYTES ||
        messageSize > std::numeric_limits<std::uint32_t>::max())
    {
        return false;
    }
    return true;
}

bool writeSignature(Bmp& image, std::size_t& position)
{
    for (std::uint8_t byte : SIGNATURE)
    {
        if (!writeByte(image, position, byte))
        {
            return false;
        }
    }
    return true;
}

bool writeLength(Bmp& image, std::size_t& position,
    std::uint32_t length)
{
    for (unsigned i = 0; i < 4; ++i)
    {
        const std::uint8_t byte =
            static_cast<std::uint8_t>((length >> (8 * i)) & 0xFFu);

        if (!writeByte(image, position, byte))
        {
            return false;
        }
    }
    return true;
}

bool writePayload(Bmp& image, std::size_t& position,
    const std::string& payload)
{
    for (unsigned char character : payload)
    {
        if (!writeByte(image, position, character))
        {
            return false;
        }
    }
    return true;
}

bool checkSignature(const Bmp& image, std::size_t& position)
{
    for (std::uint8_t expected : SIGNATURE)
    {
        std::uint8_t actual = 0;

        if (!readByte(image, position, actual) || actual != expected)
        {
            return false;
        }
    }
    return true;
}

bool readLength(const Bmp& image, std::size_t& position,
    std::uint32_t& length)
{
    length = 0;

    for (unsigned i = 0; i < 4; ++i)
    {
        std::uint8_t byte = 0;

        if (!readByte(image, position, byte))
        {
            return false;
        }

        length |= static_cast<std::uint32_t>(byte) << (8 * i);
    }
    return true;
}

bool readPayload(const Bmp& image, std::size_t& position,
    std::uint32_t length, std::string& extracted)
{
    extracted.reserve(length);

    for (std::uint32_t i = 0; i < length; ++i)
    {
        std::uint8_t byte = 0;

        if (!readByte(image, position, byte))
        {
            return false;
        }

        extracted.push_back(static_cast<char>(byte));
    }
    return true;
}

bool embedMessage(Bmp& image, const std::string& message, const std::string& key)
{
    if (!canEmbed(image, message.size()))
    {
        return false;
    }

    XOR xor1;
    std::string payload = message;
    std::uint8_t xorFlag = 0;
    std::size_t position = 0;

    if (!key.empty())
    {
        payload = xor1.xorCrypt(message, key);
        xorFlag = 1;
    }

    const std::uint32_t length = static_cast<std::uint32_t>(message.size());

    return writeSignature(image, position) &&
        writeLength(image, position, length) &&
        writeByte(image, position, xorFlag) &&
        writePayload(image, position, payload);
}

bool extractMessage(const Bmp& image, std::string& message, const std::string& key)
{
    std::size_t position = 0;
    std::uint32_t length = 0;
    std::uint8_t xorFlag = 0;
    if (!canEmbed(image, 0) || !checkSignature(image, position) ||
        !readLength(image, position, length) || !readByte(image, position, xorFlag) ||
        xorFlag > 1 || !canEmbed(image, length) || (xorFlag == 1 && key.empty()))
    {
        return false;
    }
    std::string extracted;
    if (!readPayload(image, position, length, extracted))
    {
        return false;
    }
    if (xorFlag == 1)
    {
        XOR xor1;
        extracted = xor1.xorDecrypt(extracted, key);
    }
    message = std::move(extracted);
    return true;
}