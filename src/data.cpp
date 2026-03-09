#include "data.hpp"

namespace donjon::data {

bool IsValid(const KeyValue& entry) {
    return !entry.key.empty();
}

}
