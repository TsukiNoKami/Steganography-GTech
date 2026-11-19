#include "XOR.h"

std::string XOR::xorCrypt(const std::string& message, const std::string& key)
{
    if (key.empty())
    {
        return "";
    }

    std::string result = message;

    for (size_t i = 0; i < message.size(); i++)
    {
        result[i] = message[i] ^ key[i % key.size()];
    }

    return result;
}

std::string XOR::xorDecrypt(const std::string& encryptedMessage, const std::string& key)
{
    return xorCrypt(encryptedMessage, key);
}