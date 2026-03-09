#pragma once

#include <string>
#include <vector>

namespace donjon::util {

std::vector<std::string> WrapTextStrict(const std::string& text, int maxWidth, int fontSize);

}
