#pragma once
#include <string>

inline std::string xorCrypt(const std::string& message, const std::string& key);
inline std::string xorDecrypt(const std::string& encryptedMessage, const std::string& key);