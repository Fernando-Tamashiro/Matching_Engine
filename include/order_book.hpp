#pragma once
#include <map>
#include <deque>
#include <functional>
#include "order.hpp"

class OrderBook {
private:
    std::map<long long, std::deque<Ordem>, std::greater<long long>> compras;
    std::map<long long, std::deque<Ordem>> vendas;

public:
    void inserir(const Ordem& o);
    void imprimir() const;
};