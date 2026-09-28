#pragma once
#include <map>
#include <deque>
#include <vector>
#include <functional>
#include "order.hpp"
#include "trade.hpp"

class OrderBook {
private:
    std::map<long long, std::deque<Ordem>, std::greater<long long>> compras;
    std::map<long long, std::deque<Ordem>> vendas;

public:
    void inserir(const Ordem& o);
    void imprimir() const;
    std::vector<Trade> executar(Side side, int& qty, bool temLimite, long long precoLimite);
}