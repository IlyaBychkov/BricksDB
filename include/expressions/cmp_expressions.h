#pragma once

#include <algorithm>
#include <memory>

#include "../schema/batch.h"
#include "schema/column.h"

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

template <typename Operation>
class LogicalExpression : public BoolExpression {
public:
    LogicalExpression(std::unique_ptr<BoolExpression> left, std::unique_ptr<BoolExpression> right)
        : left_(std::move(left)), right_(std::move(right)) {};

    std::vector<bool> Evaluate(const Batch& batch) override {
        std::vector<bool> lvec = left_->Evaluate(batch);
        std::vector<bool> rvec = right_->Evaluate(batch);

        size_t n = lvec.size();
        std::vector<bool> res(n);
        for (size_t i = 0; i < n; ++i) {
            res[i] = Operation()(lvec[i], rvec[i]);
        }

        return res;
    }

private:
    std::unique_ptr<BoolExpression> left_;
    std::unique_ptr<BoolExpression> right_;
};

struct LogicalAndOp {
    inline bool operator()(bool a, bool b) const {
        return a && b;
    }
};
