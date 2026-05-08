#include "expressions.h"

ColumnExpression::ColumnExpression(const std::string& name) : name_(name) {
}
Column ColumnExpression::Evaluate(const Batch& batch) {
    return batch.GetColumn(name_);
}