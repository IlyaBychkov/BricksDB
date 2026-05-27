#pragma once

#include <optional>

#include "schema/batch.h"

class IOperator {
public:
    virtual ~IOperator() = default;
    virtual std::optional<Batch> Next() = 0;
};
