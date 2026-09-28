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

std::vector<Trade> OrderBook::executarMarket(Side side, int qty) {
    std::vector<Trade> trades;

    auto consumir = [&](auto& ladoOposto) {
        // enquanto a market ainda precisa E existe alguem do outro lado
        while (qty > 0 && !ladoOposto.empty()) {

            // pega o primeiro do melhor preco
            auto nivel = ladoOposto.begin();
            long long preco = nivel->first;
            std::deque<Ordem>& fila = nivel->second;
            Ordem& primeira = fila.front();

            // negocia a menor quantidade entre os dois, e diminui dos dois
            int negociado = std::min(qty, primeira.qty);
            qty -= negociado;
            primeira.qty -= negociado;

            // agrupa por preco: mesmo preco do ultimo trade, soma; senao, linha nova
            if (!trades.empty() && trades.back().preco == preco) {
                trades.back().qty += negociado;
            } else {
                trades.push_back({preco, negociado});
            }

            // quem zerou sai da fila; fila vazia, o preco sai do livro
            if (primeira.qty == 0) fila.pop_front();
            if (fila.empty()) ladoOposto.erase(nivel);
        }
        // se saiu do laco com qty > 0, o livro acabou: o resto e descartado
    };

    // compra consome as vendas; venda consome as compras
    if (side == Side::Buy) {
        consumir(vendas);
    } else {
        consumir(compras);
    }

    return trades;
}