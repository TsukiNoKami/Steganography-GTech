#pragma once

#include "XOR.h"
#include "Bmp.h"
#include <string>

bool embedMessage(Bmp& image, const std::string& message, const std::string& key = "");
bool extractMessage(const Bmp& image, std::string& message, const std::string& key = "");