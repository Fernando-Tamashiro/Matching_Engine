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
    // ordem entrou no livro: o indice anota onde ela esta
    indice[o.id] = {o.side, o.preco};
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

std::vector<Trade> OrderBook::executar(Side side, int& qty, bool temLimite, long long precoLimite) {
    std::vector<Trade> trades;

    auto consumir = [&](auto& ladoOposto) {
        // enquanto ainda precisa E existe alguem do outro lado
        while (qty > 0 && !ladoOposto.empty()) {

            // pega o primeiro do melhor preco
            auto nivel = ladoOposto.begin();
            long long preco = nivel->first;

            // o teto da limit: se o melhor preco ja passou do limite, para
            if (temLimite) {
                if (side == Side::Buy && preco > precoLimite) break;
                if (side == Side::Sell && preco < precoLimite) break;
            }

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

            // quem zerou sai da fila (e do indice); fila vazia, o preco sai do livro
            if (primeira.qty == 0) {
                indice.erase(primeira.id);
                fila.pop_front();
            }
            if (fila.empty()) ladoOposto.erase(nivel);
        }
    };

    // compra consome as vendas; venda consome as compras
    if (side == Side::Buy) {
        consumir(vendas);
    } else {
        consumir(compras);
    }

    return trades;
}

bool OrderBook::cancelar(const std::string& id) {
    // 1. consulta o indice: onde essa ordem esta?
    auto it = indice.find(id);
    if (it == indice.end()) return false;   // nao existe, ou ja saiu do livro
    Local local = it->second;

    // 2. vai direto no lado e no preco certos, e procura dentro da fila
    auto remover = [&](auto& lado) {
        auto nivel = lado.find(local.preco);
        std::deque<Ordem>& fila = nivel->second;
        for (auto pos = fila.begin(); pos != fila.end(); ++pos) {
            if (pos->id == id) {
                fila.erase(pos);
                break;
            }
        }
        // 3. se a fila ficou vazia, o preco sai do livro
        if (fila.empty()) lado.erase(nivel);
    };

    if (local.side == Side::Buy) {
        remover(compras);
    } else {
        remover(vendas);
    }

    // 4. a ordem saiu: o indice esquece dela
    indice.erase(it);
    return true;
}

bool OrderBook::buscar(const std::string& id, Ordem& saida) const {
    // consulta o indice e copia a ordem para quem pediu
    auto it = indice.find(id);
    if (it == indice.end()) return false;
    const Local& local = it->second;

    auto procurar = [&](const auto& lado) {
        for (const Ordem& o : lado.at(local.preco)) {
            if (o.id == id) {
                saida = o;
                return true;
            }
        }
        return false;
    };

    return local.side == Side::Buy ? procurar(compras) : procurar(vendas);
}

bool OrderBook::reduzirQuantidade(const std::string& id, int novaQty) {
    // muda a quantidade direto na fila: a ordem nao sai do lugar
    auto it = indice.find(id);
    if (it == indice.end()) return false;
    const Local& local = it->second;

    auto reduzir = [&](auto& lado) {
        for (Ordem& o : lado.at(local.preco)) {
            if (o.id == id) {
                o.qty = novaQty;
                return true;
            }
        }
        return false;
    };

    return local.side == Side::Buy ? reduzir(compras) : reduzir(vendas);
}