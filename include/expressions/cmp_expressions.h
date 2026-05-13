#pragma once

#include <algorithm>

#include "../scheme/batch.h"
#include "scheme/column.h"

struct BoolExpression {
    virtual ~BoolExpression() = default;
    virtual std::vector<bool> Evaluate(const Batch& batch) = 0;
};

struct ContainsOp {
    inline bool operator()(const std::string& text, const std::string& pattern) const {
        return text.contains(pattern);
    }
};

struct NotContainsOp {
    inline bool operator()(const std::string& text, const std::string& pattern) const {
        return !text.contains(pattern);
    }
};

template <typename Operation, typename T>
class CompareExpression : public BoolExpression {
public:
    CompareExpression(const std::string& name, const T& value) : name_(name), value_(value) {};
    std::vector<bool> Evaluate(const Batch& batch) override {
        Column column = batch.GetColumn(name_);
        if (!std::holds_alternative<std::vector<T>>(column.Value())) {
            throw std::runtime_error("Type mismatch in CompareExpression for column " + name_);
        }

        size_t n = column.GetSize();
        std::vector<bool> res(n);
        const auto& vec = std::get<std::vector<T>>(column.Value());
        for (size_t i = 0; i < n; ++i) {
            res[i] = Operation()(vec[i], value_);
        }

        return res;
    }

private:
    std::string name_;
    T value_;
};

template <typename T>
class InExpression : public BoolExpression {
public:
    InExpression(const std::string& name, std::vector<T>&& values)
        : name_(name), values_(std::move(values)) {
    }

    std::vector<bool> Evaluate(const Batch& batch) override {
        Column column = batch.GetColumn(name_);
        if (!std::holds_alternative<std::vector<T>>(column.Value())) {
            throw std::runtime_error("Type mismatch in InExpression for column " + name_);
        }

        size_t n = column.GetSize();
        std::vector<bool> res(n);
        const auto& vec = std::get<std::vector<T>>(column.Value());
        for (size_t i = 0; i < n; ++i) {
            res[i] = std::find(values_.begin(), values_.end(), vec[i]) != values_.end();
        }

        return res;
    }

private:
    std::string name_;
    std::vector<T> values_;
};