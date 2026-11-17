#include "Xor.h"
#include <iostream>
#include <string>

bool testXor()
{
    const std::string message = "Bonjour tout le monde";
    const std::string key = "ABC";

    const std::string encrypted = xorCrypt(message, key);

    const std::string decrypted = xorCrypt(encrypted, key);

    return decrypted == message;
}

int main()
{
    if (testXor())
    {
        std::cout << "Test XOR reussi !" << std::endl;
        return 0;
    }

    std::cout << "Test XOR echoue !" << std::endl;
    return 1;
}