#pragma once

#include <cstdlib>

class OrderHandle {
public:
    explicit OrderHandle(std::size_t order_id) : order_id_(order_id) {}

    std::size_t GetId() const {
        return order_id_;
    }

private:
    std::size_t order_id_;
};