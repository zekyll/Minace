#pragma once

namespace cm {

enum class NodeType : unsigned
{
    NONE = 0,
    EXACT = 1,
    LOWER_BOUND = 2,
    UPPER_BOUND = 3,
};

}
