#pragma once

#include <cstdint>

namespace donjon::entities {

using EntityId = std::uint32_t;

struct Position {
    int x = 0;
    int y = 0;
};

bool IsValidEntityId(EntityId id);

}
