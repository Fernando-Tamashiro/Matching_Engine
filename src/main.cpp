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

            // 4. ordem aceita
            Ordem o{std::to_string(proximoSeq), side, preco, qty, proximoSeq};
            proximoSeq++;
            std::cout << "Order created: " << sideTexto << " " << qty << " @ "
                      << formatarPreco(preco) << " " << o.id << "\n";

            // 5. tenta casar com o outro lado, respeitando o limite
            int restante = qty;
            for (const Trade& t : book.executar(side, restante, true, preco)) {
                std::cout << "Trade, price: " << formatarPreco(t.preco)
                          << ", qty: " << t.qty << "\n";
            }

            // 6. o que sobrou vai para o livro
            if (restante > 0) {
                o.qty = restante;
                book.inserir(o);
            }

        } else if (comando == "market") {
            std::string sideTexto, sobra;
            int qty;

            // 1. forma correta: tem side e quantidade
            if (!(iss >> sideTexto >> qty)) {
                std::cout << "Erro: uso correto e 'market <buy|sell> <qty>'\n";
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

            // 3. quantidade nao negativa
            if (qty < 0) {
                std::cout << "Erro: quantidade invalida\n";
                continue;
            }

            // 4. executa sem limite; a sobra e descartada
            for (const Trade& t : book.executar(side, qty, false, 0)) {
                std::cout << "Trade, price: " << formatarPreco(t.preco)
                          << ", qty: " << t.qty << "\n";
            }

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