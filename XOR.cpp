#include "XOR.h"

std::string xorCrypt(const std::string& message, const std::string& key)
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