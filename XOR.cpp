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

std::string XOR::xorDecrypt(const std::string& encryptedMessage, const std::string& key) {
    if (key.empty()) {
        return "";
    }
    std::string result = encryptedMessage;
    for (size_t i = 0; i < encryptedMessage.size(); i++) {
        result[i] = encryptedMessage[i] ^ key[i % key.size()];
    }
    return result;
}