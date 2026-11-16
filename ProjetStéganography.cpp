#include "Bmp.h"
#include <iostream>

int main(int argc, char* argv[])
{
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
    return 0;
}
