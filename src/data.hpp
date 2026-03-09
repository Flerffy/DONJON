#pragma once

#include <string>

namespace donjon::data {

struct KeyValue {
    std::string key;
    std::string value;
};

bool IsValid(const KeyValue& entry);

}
