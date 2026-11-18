#include "Bmp.h"
#include "Steganography.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage :\n";
        std::cout << "  programme embed source.bmp destination.bmp message\n";
        std::cout << "  programme extract image.bmp\n";
        return 1;
    }

    const std::string mode = argv[1];

    const bool embedding = mode == "embed" && argc == 5;
    const bool extracting = mode == "extract" && argc == 3;

    if (!embedding && !extracting)
    {
        std::cout << "Erreur : commande ou arguments invalides.\n";
        return 1;
    }

    Bmp image;

    if (!image.load(argv[2]))
    {
        std::cout << "Erreur : chargement BMP impossible.\n";
        return 1;
    }

    if (embedding)
    {
        if (!embedMessage(image, argv[4]))
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

        if (!extractMessage(image, message))
        {
            std::cout << "Erreur : extraction impossible.\n";
            return 1;
        }

        std::cout << "Message extrait : " << message << "\n";
    }

    return 0;
}
