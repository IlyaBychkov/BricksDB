#pragma once

#include <cstdint>
#include <memory>
#include <regex>

#include "../scheme/batch.h"
#include "../scheme/column.h"

struct IExpression {
    virtual ~IExpression() = default;
    virtual Column Evaluate(const Batch& batch) = 0;
};

struct ColumnExpression : public IExpression {
public:
    ColumnExpression(const std::string& name);
    Column Evaluate(const Batch& batch) override;

private:
    std::string name_;
};

template <typename T>
struct ConstantExpression : public IExpression {
public:
    ConstantExpression(Type type, const T& value) : type_(type), value_(value) {
    }
    Column Evaluate(const Batch& batch) override {
        return Column(type_, std::vector<T>(batch.RowsCnt(), value_));
    }

private:
    Type type_;
    T value_;
};

template <typename Operation>
struct BinaryExpression : public IExpression {
public:
    BinaryExpression(std::unique_ptr<IExpression> left, std::unique_ptr<IExpression> right,
                     Type res_type)
        : left_(std::move(left)), right_(std::move(right)), res_type_(res_type) {
    }

    Column Evaluate(const Batch& batch) override {
        Column lcol = left_->Evaluate(batch);
        Column rcol = right_->Evaluate(batch);

        size_t n = lcol.GetSize();
        if (n != rcol.GetSize()) {
            throw std::runtime_error("Size mismatch in BinaryExpression");
        }

        return std::visit(
            [&]<typename T, typename U>(const std::vector<T>& lvec,
                                        const std::vector<U>& rvec) -> Column {
                if constexpr (requires { Operation()(std::declval<T>(), std::declval<U>()); }) {
                    using ResultT = decltype(Operation()(std::declval<T>(), std::declval<U>()));

                    std::vector<ResultT> result;
                    result.reserve(n);
                    for (size_t i = 0; i < n; ++i) {
                        result.push_back(Operation()(lvec[i], rvec[i]));
                    }

                    return Column(res_type_, std::move(result));
                } else {
                    throw std::runtime_error(
                        "Unsupported types in BinaryExpression: " + TypeToString(lcol.GetType()) +
                        " and " + TypeToString(rcol.GetType()));
                }
            },
            lcol.Value(), rcol.Value());
    }

private:
    std::unique_ptr<IExpression> left_;
    std::unique_ptr<IExpression> right_;
    Type res_type_;
};

struct MinusOp {
    template <typename T, typename U>
    auto operator()(const T& a, const U& b) const
        requires requires { a - b; }
    {
        return a - b;
    }
};

struct PlusOp {
    template <typename T, typename U>
    auto operator()(const T& a, const U& b) const
        requires requires { a + b; }
    {
        return a + b;
    }
};

template <typename Operation>
struct UnaryExpression : public IExpression {
public:
    UnaryExpression(std::unique_ptr<IExpression> operand, Type res_type)
        : operand_(std::move(operand)), res_type_(res_type) {
    }

    Column Evaluate(const Batch& batch) override {
        Column col = operand_->Evaluate(batch);
        size_t n = col.GetSize();

        return std::visit(
            [&]<typename T>(const std::vector<T>& vec) -> Column {
                if constexpr (requires { Operation()(std::declval<T>()); }) {
                    using ResultT = decltype(Operation()(std::declval<T>()));

                    std::vector<ResultT> result;
                    result.reserve(n);
                    for (size_t i = 0; i < n; ++i) {
                        result.push_back(Operation()(vec[i]));
                    }

                    return Column(res_type_, std::move(result));
                } else {
                    throw std::runtime_error("Unsupported type in UnaryExpression: " +
                                             TypeToString(col.GetType()));
                }
            },
            col.Value());
    }

private:
    std::unique_ptr<IExpression> operand_;
    Type res_type_;
};

struct ExtractMinuteOp {
    int16_t operator()(const int64_t timestamp) const {
        return timestamp % 3600 / 60;
    }
};

struct TruncMinuteOp {
    int64_t operator()(const int64_t timestamp) const {
        return timestamp - timestamp % 60;
    }
};

struct StrLenOp {
    int32_t operator()(const std::string& str) const {
        return static_cast<int32_t>(str.size());
    }
};

class RegexpReplaceExpression : public IExpression {
public:
    RegexpReplaceExpression(std::unique_ptr<IExpression> operand, const std::string& pattern,
                            const std::string& replacement)
        : operand_(std::move(operand)), regex_(pattern), replacement_(replacement) {
    }

    Column Evaluate(const Batch& batch) override {
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

private:
    std::unique_ptr<IExpression> operand_;
    std::regex regex_;
    std::string replacement_;
};