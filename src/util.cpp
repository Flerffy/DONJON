#include "util.hpp"

#include "raylib.h"

namespace donjon::util {

std::vector<std::string> WrapTextStrict(const std::string& text, int maxWidth, int fontSize) {
    std::vector<std::string> lines;
    std::string currentLine;

    for (char c : text) {
        if (c == '\n') {
            lines.push_back(currentLine);
            currentLine.clear();
            continue;
        }

        std::string test = currentLine + c;
        if (!currentLine.empty() && MeasureText(test.c_str(), fontSize) > maxWidth) {
            lines.push_back(currentLine);
            currentLine.clear();
        }
        currentLine += c;
    }

    if (!currentLine.empty()) lines.push_back(currentLine);
    return lines;
}

}
