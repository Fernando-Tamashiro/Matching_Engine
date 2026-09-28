#include "order_book.hpp"
#include "preco.hpp"
#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

void OrderBook::inserir(const Ordem& o) {
    if (o.side == Side::Buy) {
        compras[o.preco].push_back(o);
    } else {
        vendas[o.preco].push_back(o);
    }
}

void OrderBook::imprimir() const {
    std::vector<std::string> linhasCompra;
    for (const auto& [preco, fila] : compras) {
        for (const auto& o : fila) {
            linhasCompra.push_back(std::to_string(o.qty) + " @ " + formatarPreco(preco));
        }
    }

    std::vector<std::string> linhasVenda;
    for (const auto& [preco, fila] : vendas) {
        for (const auto& o : fila) {
            linhasVenda.push_back(std::to_string(o.qty) + " @ " + formatarPreco(preco));
        }
    }

    std::cout << "Ordens de Compra    | Ordens de Venda\n";
    std::cout << "--------------------|-----------------\n";

    size_t total = std::max(linhasCompra.size(), linhasVenda.size());
    for (size_t i = 0; i < total; i++) {
        std::string esquerda = i < linhasCompra.size() ? linhasCompra[i] : "";
        std::string direita  = i < linhasVenda.size()  ? linhasVenda[i]  : "";
        std::cout << std::left << std::setw(20) << esquerda << "| " << direita << "\n";
    }
}