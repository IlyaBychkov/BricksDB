#pragma once

#include <memory>

#include "operators/base_operator.h"

class LimitOperator : public IOperator {
public:
    LimitOperator(std::unique_ptr<IOperator> child, size_t limit, size_t offset = 0);

    std::optional<Batch> Next() override;

private:
    std::unique_ptr<IOperator> child_;
    size_t limit_;
    size_t offset_;
};