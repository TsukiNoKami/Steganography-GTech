#include "Steganography.h"
#include "XOR.h"
#include <cstdint>
#include <limits>
#include <utility>

namespace
{
    constexpr std::size_t HEADER_BYTES = 9;

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

bool embedMessage(Bmp& image, const std::string& message, const std::string& key)
{
    const std::size_t availableBytes =
        image.capacityBits() / 8;

    if (availableBytes < HEADER_BYTES ||
        message.size() > availableBytes - HEADER_BYTES ||
        message.size() >
        std::numeric_limits<std::uint32_t>::max())
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

    // Signature du message
    const std::uint8_t signature[4] = {
        'S', 'T', 'E', 'G'
    };

    for (std::uint8_t byte : signature)
    {
        if (!writeByte(image, position, byte))
        {
            return false;
        }
    }

    // Longueur du message sur 4 octets (little-endian)
    const std::uint32_t length =
        static_cast<std::uint32_t>(message.size());

    for (unsigned i = 0; i < 4; ++i)
    {
        const std::uint8_t byte =
            static_cast<std::uint8_t>(
                (length >> (8 * i)) & 0xFFu
                );

        if (!writeByte(image, position, byte))
        {
            return false;
        }
    }

    // Indicateur XOR : 0 = message non chiffre
    if (!writeByte(image, position, xorFlag))
    {
        return false;
    }

    // Ecriture du message
    for (unsigned char character : payload)
    {
        if (!writeByte(image, position, character))
        {
            return false;
        }
    }

    return true;
}

bool extractMessage(const Bmp& image, std::string& message, const std::string& key)
{
    const std::size_t availableBytes =
        image.capacityBits() / 8;

    if (availableBytes < HEADER_BYTES)
    {
        return false;
    }

    std::size_t position = 0;

    // Verification de la signature
    const std::uint8_t signature[4] = {
        'S', 'T', 'E', 'G'
    };

    for (std::uint8_t expected : signature)
    {
        std::uint8_t actual = 0;

        if (!readByte(image, position, actual) ||
            actual != expected)
        {
            return false;
        }
    }

    // Lecture de la longueur sur 4 octets
    std::uint32_t length = 0;

    for (unsigned i = 0; i < 4; ++i)
    {
        std::uint8_t byte = 0;

        if (!readByte(image, position, byte))
        {
            return false;
        }

        length |= static_cast<std::uint32_t>(byte) << (8 * i);
    }

    // Lecture de l'indicateur XOR
    std::uint8_t xorFlag = 0;

    if (!readByte(image, position, xorFlag) || xorFlag > 1)
    {
        return false;
    }
    
    if (key.empty() && xorFlag == 1)
    {
        return false;
    }

    // Verification de la longueur annoncee
    if (length > availableBytes - HEADER_BYTES)
    {
        return false;
    }

    std::string extracted;
    extracted.reserve(length);

    // Reconstruction du message
    for (std::uint32_t i = 0; i < length; ++i)
    {
        std::uint8_t byte = 0;

        if (!readByte(image, position, byte))
        {
            return false;
        }

        extracted.push_back(static_cast<char>(byte));
    }

    if (xorFlag == 1)
    {
        XOR xor1;
        extracted = xor1.xorDecrypt(extracted, key);
    }

    message = std::move(extracted);

    return true;
}
