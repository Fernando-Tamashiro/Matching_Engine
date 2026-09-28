#include <iostream>
#include <sstream>
#include <string>
#include "order_book.hpp"
#include "preco.hpp"

int main() {
    OrderBook book;
    long long proximoSeq = 1;
    std::string linha;

    while (std::getline(std::cin, linha)) {
        std::istringstream iss(linha);
        std::string comando;
        iss >> comando;

        if (comando == "limit") {
            std::string sideTexto, precoTexto, sobra;
            int qty;

            // 1. forma correta: tem side, preco e quantidade
            if (!(iss >> sideTexto >> precoTexto >> qty)) {
                std::cout << "Erro: uso correto e 'limit <buy|sell> <preco> <qty>'\n";
                continue;
            }
            if (iss >> sobra) {
                std::cout << "Erro: texto a mais no fim da linha\n";
                continue;
            }

            // 2. side e buy ou sell
            Side side;
            if (sideTexto == "buy") {
                side = Side::Buy;
            } else if (sideTexto == "sell") {
                side = Side::Sell;
            } else {
                std::cout << "Erro: side deve ser buy ou sell\n";
                continue;
            }

            // 3. numeros formatados e nao negativos
            long long preco = parsePreco(precoTexto);
            if (preco < 0) {
                std::cout << "Erro: preco invalido\n";
                continue;
            }
            if (qty < 0) {
                std::cout << "Erro: quantidade invalida\n";
                continue;
            }

            // 4. tudo certo: cria a ordem e coloca no livro
            Ordem o{std::to_string(proximoSeq), side, preco, qty, proximoSeq};
            proximoSeq++;
            book.inserir(o);
            std::cout << "Order created: " << sideTexto << " " << qty << " @ "
                      << formatarPreco(preco) << " " << o.id << "\n";
        } else if (comando == "print") {
            book.imprimir();
        } else if (comando.empty()) {
            // linha em branco, ignora
        } else {
            std::cout << "Comando desconhecido: " << comando << "\n";
        }
    }
    return 0;
}