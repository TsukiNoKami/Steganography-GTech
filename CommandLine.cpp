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
    std::cout << "\nExemple :\n";
    std::cout << "  programme embed source.bmp resultat.bmp " "\"Salut a toi\" --key \"ABC\"\n";
}

bool readKey(int argc, char* argv[], int requiredArgs, std::string& key)
{
    if (argc == requiredArgs)
    {
        return true;
    }

    if (argc != requiredArgs + 2 ||
        std::string(argv[requiredArgs]) != "--key")
    {
        std::cout << "Erreur : option invalide.\n";
        return false;
    }

    key = argv[requiredArgs + 1];

    if (key.empty())
    {
        std::cout << "Erreur : la cle ne peut pas etre vide.\n";
        return false;
    }

    return true;
}

int executeEmbed(Bmp& image, char* argv[], const std::string& key)
{
    if (!embedMessage(image, argv[4], key))
    {
        std::cout << "Erreur : insertion impossible (capacite ou donnees).\n";
        return 1;
    }

    if (!image.save(argv[3]))
    {
        std::cout << "Erreur : destination existante ou sauvegarde impossible.\n";
        return 1;
    }

    std::cout << "Message insere avec succes !\n";
    return 0;
}

int executeExtract(const Bmp& image, const std::string& key)
{
    std::string message;

    if (!extractMessage(image, message, key))
    {
        std::cout << "Erreur : message absent, invalide ou cle requise.\n";
        return 1;
    }

    std::cout << "Message extrait : " << message << "\n";
    return 0;
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
    if (!readKey(argc, argv, embedding ? 5 : 3, key))
    {
        return 1;
    }

    Bmp image;
    if (!image.load(argv[2]))
    {
        std::cout << "Erreur : fichier BMP absent, inaccessible ou invalide.\n";
        return 1;
    }

    if (embedding)
    {
        return executeEmbed(image, argv, key);
    }

    return executeExtract(image, key);
}