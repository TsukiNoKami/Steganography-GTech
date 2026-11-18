#pragma once
#include "XOR.h"
#include "XOR.cpp"
#include <iostream>

void test() {
	std::string message;
	std::cout << "Enter the message to encrypt: ";
	std::cin >> message;
	std::string key;
	std::cout << "Enter the key for encryption/decryption: ";
	std::cin >> key;
	std::cout << "Encrypted message: " << xorCrypt(message, key) << std::endl;
	std::cout << "Decrypted message: " << xorDecrypt(xorCrypt(message, key), key) << std::endl;
}
