#pragma once

#include <memory>

#include "../scheme/batch.h"
#include "operators/base_operator.h"

class ProjectionOperator : public IOperator {
public:
    ProjectionOperator(std::unique_ptr<IOperator> child, std::vector<std::string>&& projections);

    std::optional<Batch> Next();

private:
    std::unique_ptr<IOperator> child_;
    std::vector<std::string> projections_;
};
