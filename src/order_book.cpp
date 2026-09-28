#include "order_book.hpp"
#include "preco.hpp"
#include <iostream>

void OrderBook::inserir(const Ordem& o) {
    if (o.side == Side::Buy) {
        compras[o.preco].push_back(o);
    } else {
        vendas[o.preco].push_back(o);
    }
}

void OrderBook::imprimir() const {
    for (const auto& [preco, fila] : compras) {
        for (const auto& o : fila) {
            std::cout << o.qty << " @ " << formatarPreco(preco) << "\n";
        }
    }
}