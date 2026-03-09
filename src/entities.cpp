#include "entities.hpp"

namespace donjon::entities {

bool IsValidEntityId(EntityId id) {
    return id != 0;
}

}
