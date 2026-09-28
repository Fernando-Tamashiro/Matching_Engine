#pragma once
#include <map>
#include <deque>
#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include "order.hpp"
#include "trade.hpp"

class OrderBook {
private:
    std::map<long long, std::deque<Ordem>, std::greater<long long>> compras;
    std::map<long long, std::deque<Ordem>> vendas;

    // indice: para cada id, onde a ordem esta no livro
    struct Local {
        Side side;
        long long preco;
    };
    std::unordered_map<std::string, Local> indice;

public:
    void inserir(const Ordem& o);
    void imprimir() const;
    std::vector<Trade> executar(Side side, int& qty, bool temLimite, long long precoLimite);
    bool cancelar(const std::string& id);
    bool buscar(const std::string& id, Ordem& saida) const;
    bool reduzirQuantidade(const std::string& id, int novaQty);
};