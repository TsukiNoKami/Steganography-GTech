#pragma once

#include "Bmp.h"
#include <string>

bool embedMessage(Bmp& image, const std::string& message);

bool extractMessage(const Bmp& image, std::string& message);