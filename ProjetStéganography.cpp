#include "Bmp.h"
#include "XOR.h"
#include <iostream>

int main(int argc, char* argv[])
{
    XOR xor1;
    if (argc != 3)
    {
        std::cout << "Usage : programme source.bmp copie.bmp\n";
        return 1;
    }

    Bmp image;

    if (!image.load(argv[1]))
    {
        std::cout << "Erreur : chargement BMP impossible.\n";
        return 1;
    }

    if (!image.save(argv[2]))
    {
        std::cout << "Erreur : sauvegarde impossible.\n";
        return 1;
    }

    std::cout << "Image BMP sauvegardee avec succes !\n";

    std::string message;
    std::cout << "Enter the message to encrypt: ";
    std::cin >> message;
    std::string key;
    std::cout << "Enter the key for encryption/decryption: ";
    std::cin >> key;
    std::cout << "Encrypted message: " << xor1.xorCrypt(message, key) << std::endl;
    std::cout << "Decrypted message: " << xor1.xorDecrypt(xor1.xorCrypt(message, key), key) << std::endl;

    return 0;
}
