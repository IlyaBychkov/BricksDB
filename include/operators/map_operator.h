#pragma once

#include "../expressions/expressions.h"
#include "base_operator.h"

struct MapOperator : public IOperator {
public:
    MapOperator(std::unique_ptr<IOperator> child,
                std::vector<std::pair<std::unique_ptr<IExpression>, std::string>>&& expressions);

    std::optional<Batch> Next() override;

private:
    std::unique_ptr<IOperator> child_;
    std::vector<std::pair<std::unique_ptr<IExpression>, std::string>> expressions_;
};
