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
            // sua parte
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