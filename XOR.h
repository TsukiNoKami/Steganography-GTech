#pragma once
#include <string>

struct XOR {
    std::string xorCrypt(const std::string& message, const std::string& key);
    std::string xorDecrypt(const std::string& encryptedMessage, const std::string& key);
};