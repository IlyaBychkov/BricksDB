#include "expressions.h"

ColumnExpression::ColumnExpression(const std::string& name) : name_(name) {
}
Column ColumnExpression::Evaluate(const Batch& batch) {
    return batch.GetColumn(name_);
}

RegexpReplaceExpression::RegexpReplaceExpression(std::unique_ptr<IExpression> operand,
                                                 const std::string& pattern,
                                                 const std::string& replacement)
    : operand_(std::move(operand)), regex_(pattern), replacement_(replacement) {
}

Column RegexpReplaceExpression::Evaluate(const Batch& batch) {
    Column col = operand_->Evaluate(batch);
    size_t n = col.GetSize();

    if (!std::holds_alternative<std::vector<std::string>>(col.Value())) {
        throw std::runtime_error("RegexpReplaceExpression only supports String columns");
    }

    const auto& vec = std::get<std::vector<std::string>>(col.Value());
    std::vector<std::string> res(n);
    for (size_t i = 0; i < n; ++i) {
        res[i] = std::regex_replace(vec[i], regex_, replacement_);
    }

    return Column(Type::string, std::move(res));
}

CaseExpression::CaseExpression(std::unique_ptr<BoolExpression> condition,
                               std::unique_ptr<IExpression> then_expr,
                               std::unique_ptr<IExpression> else_expr, Type res_type)
    : condition_(std::move(condition)),
      then_expr_(std::move(then_expr)),
      else_expr_(std::move(else_expr)),
      res_type_(res_type) {
}

Column CaseExpression::Evaluate(const Batch& batch) {
    std::vector<bool> condvec = condition_->Evaluate(batch);
    Column tcol = then_expr_->Evaluate(batch);
    Column ecol = else_expr_->Evaluate(batch);

    size_t n = condvec.size();
    if (n != tcol.GetSize() || n != ecol.GetSize()) {
        throw std::runtime_error("Size mismatch in CaseExpression");
    }

    return std::visit(
        [&]<typename T, typename U>(const std::vector<T>& tvec,
                                    const std::vector<U>& evec) -> Column {
            if constexpr (std::is_same_v<T, U>) {
                std::vector<T> res(n);
                for (size_t i = 0; i < n; ++i) {
                    if (condvec[i]) {
                        res[i] = tvec[i];
                    } else {
                        res[i] = evec[i];
                    }
                }
                return Column(res_type_, std::move(res));
            } else {
                throw std::runtime_error(
                    "Invalid types in CaseExpression: " + std::string(typeid(T).name()) + ", " +
                    std::string(typeid(U).name()));
            }
        },
        tcol.Value(), ecol.Value());
}