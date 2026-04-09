#pragma once

#include <memory>
#include <set>

#include "operators/base_operator.h"
#include "scheme/type.h"

enum class AggregationType { COUNT, SUM, AVG, MIN, MAX, COUNT_DISTINCT };

struct AggregationState {
    virtual ~AggregationState() = default;
    virtual void Update(const Column& column) = 0;
    virtual Column GetResult() = 0;
};

class CountState : public AggregationState {
public:
    CountState();
    void Update(const Column& column) override;
    Column GetResult() override;

private:
    int64_t count_ = 0;
};

class SumState : public AggregationState {
public:
    SumState();
    void Update(const Column& column) override;
    Column GetResult() override;

private:
    __int128_t sum_ = 0;
};

class AvgState : public AggregationState {
public:
    AvgState();
    void Update(const Column& column) override;
    Column GetResult() override;

private:
    __int128_t sum_ = 0;
    int64_t count_ = 0;
};

template <typename T>
class MinState : public AggregationState {
public:
    MinState(Type type) : type_(type), val_(T{}), is_initialized_(false) {};

    void Update(const Column& column) override {
        std::visit(
            [&]<typename U>(const std::vector<U>& vec) {
                if constexpr (std::is_same_v<T, U>) {
                    if (!is_initialized_) {
                        val_ = vec[0];
                        is_initialized_ = true;
                    }
                    for (auto& x : vec) {
                        if (x < val_) {
                            val_ = x;
                        }
                    }
                } else {
                    throw std::runtime_error("Type mismatch in MinState");
                }
            },
            column.Value());
    }

    Column GetResult() override {
        return Column(type_, std::vector<T>{val_});
    }

private:
    Type type_;
    T val_;
    bool is_initialized_ = false;
};

template <typename T>
class MaxState : public AggregationState {
public:
    MaxState(Type type) : type_(type), val_(T{}), is_initialized_(false) {};

    void Update(const Column& column) override {
        std::visit(
            [&]<typename U>(const std::vector<U>& vec) {
                if constexpr (std::is_same_v<T, U>) {
                    if (!is_initialized_) {
                        val_ = vec[0];
                        is_initialized_ = true;
                    }
                    for (auto& x : vec) {
                        if (x > val_) {
                            val_ = x;
                        }
                    }
                } else {
                    throw std::runtime_error("Type mismatch in MaxState");
                }
            },
            column.Value());
    }

    Column GetResult() override {
        return Column(type_, std::vector<T>{val_});
    }

private:
    Type type_;
    T val_;
    bool is_initialized_ = false;
};

template <typename T>
class CountDistinctState : public AggregationState {
public:
    CountDistinctState(Type type) : type_(type) {};

    void Update(const Column& column) override {
        std::visit(
            [&]<typename U>(const std::vector<U>& vec) {
                if constexpr (std::is_same_v<T, U>) {
                    values_.insert(vec.begin(), vec.end());
                } else {
                    throw std::runtime_error("Type mismatch in CountDistinctState");
                }
            },
            column.Value());
    }

    Column GetResult() override {
        return Column(Type::int64, std::vector<int64_t>{static_cast<int64_t>(values_.size())});
    }

private:
    Type type_;
    std::set<T> values_;
};

class AggregationOperator : public IOperator {
public:
    AggregationOperator(
        std::unique_ptr<IOperator> child,
        std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>>&& states);

    std::optional<Batch> Next() override;

private:
    std::unique_ptr<IOperator> child_;
    std::vector<std::pair<std::unique_ptr<AggregationState>, std::string>> states_;
};