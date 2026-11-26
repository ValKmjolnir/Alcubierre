#pragma once

#include <string>

// Break a single-line paragraph into lines that fit max_width with the raylib
// default font. Existing '\n' characters are kept as hard breaks.
std::string wrap_text(const std::string& text, int fontsize, int max_width);
