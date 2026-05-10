#pragma once

#include <memory>

#include "../expressions/cmp_expressions.h"
#include "../scheme/batch.h"
#include "../scheme/column.h"
#include "operators/base_operator.h"

class FilterOperator : public IOperator {
public:
    FilterOperator(std::unique_ptr<IOperator> child, std::unique_ptr<BoolExpression> expr);

    std::optional<Batch> Next();

private:
    std::unique_ptr<IOperator> child_;
    std::unique_ptr<BoolExpression> expr_;
};

Column ApplyMaskToColumn(const Column& column, const std::vector<bool>& mask);