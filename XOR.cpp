#include "XOR.h"
#include <iostream>

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

void XOR::testXOR() {
	std::string message;
	std::cout << "Enter the message to encrypt: ";
	std::cin >> message;
	std::string key;
	std::cout << "Enter the key for encryption/decryption: ";
	std::cin >> key;
    std::string crypted = xorCrypt(message, key);
	std::cout << "Encrypted message: " << crypted << std::endl;
	std::cout << "Decrypted message: " << xorDecrypt(crypted, key) << std::endl;
}