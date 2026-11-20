#include "CommandLine.h"
#include "Bmp.h"
#include "Steganography.h"
#include <iostream>
#include <string>

void printHelp()
{
    std::cout << "Utilisation :\n";
    std::cout << "  programme --help\n";
    std::cout << "  programme embed source.bmp destination.bmp \"message\" [--key \"cle\"]\n";
    std::cout << "  programme extract image.bmp [--key \"cle\"]\n";
}

int runApplication(int argc, char* argv[])
{
    if (argc == 2 && std::string(argv[1]) == "--help")
    {
        printHelp();
        return 0;
    }

    if (argc < 3)
    {
        printHelp();
        return 1;
    }

    const std::string mode = argv[1];

    const bool embedding = mode == "embed" && (argc == 5 || argc == 7);
    const bool extracting = mode == "extract" && (argc == 3 || argc == 5);

    if (!embedding && !extracting)
    {
        std::cout << "Erreur : commande ou arguments invalides.\n";
        return 1;
    }

    std::string key;
    
    if (embedding && argc == 7)
    {
        if (std::string(argv[5]) != "--key")
        {
            std::cout << "Erreur : option invalide.\n";
            return 1;
        }

        key = argv[6];

        if (key.empty())
        {
            std::cout << "Erreur : la cle ne peut pas etre vide.\n";
            return 1;
        }
    }

    else if (extracting && argc == 5)
    {
        if (std::string(argv[3]) != "--key")
        {
            std::cout << "Erreur : option invalide.\n";
            return 1;
        }

        key = argv[4];

        if (key.empty())
        {
            std::cout << "Erreur : la cle ne peut pas etre vide.\n";
            return 1;
        }
    }
    
    Bmp image;

    if (!image.load(argv[2]))
    {
        std::cout << "Erreur : chargement BMP impossible.\n";
        return 1;
    }

    if (embedding)
    {
        if (!embedMessage(image, argv[4], key))
        {
            std::cout << "Erreur : insertion impossible.\n";
            return 1;
        }

        if (!image.save(argv[3]))
        {
            std::cout << "Erreur : sauvegarde impossible.\n";
            return 1;
        }

        std::cout << "Message insere avec succes !\n";
    }
    else
    {
        std::string message;

        if (!extractMessage(image, message, key))
        {
            std::cout << "Erreur : extraction impossible.\n";
            return 1;
        }

        std::cout << "Message extrait : " << message << "\n";
    }

    return 0;
}