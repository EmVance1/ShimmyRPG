#include "pch.h"
#include "action.h"


bool operator==(const ContextAction& lhs, const ContextAction& rhs) {
    return lhs.index() == rhs.index();
}

