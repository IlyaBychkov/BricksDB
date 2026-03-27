#include <optional>

#include "scheme/batch.h"

class IOperator {
public:
    virtual ~IOperator() = default;
    virtual std::optional<Batch> Next() = 0;
};