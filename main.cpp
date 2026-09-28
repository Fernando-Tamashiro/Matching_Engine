#include <iostream>
#include <map>
#include <deque>
#include <string>
#include <functional>

enum class Side { Buy, Sell };

struct Ordem {
    std::string id;
    Side side;
    long long preco;   // em centavos: 10.10 -> 1010
    int qty;
    long long seq;     // ordem de chegada
};

class OrderBook {
private:
    std::map<long long, std::deque<Ordem>, std::greater<long long>> compras;
    std::map<long long, std::deque<Ordem>> vendas;

public:
    void inserir(const Ordem& o) {
        // sua parte
    }

    void imprimir() const {
        // sua parte
    }
};

int main() {
    OrderBook book;
    book.inserir({"1", Side::Buy, 1000, 200, 1});
    book.inserir({"2", Side::Buy, 999, 100, 2});
    book.inserir({"3", Side::Sell, 1050, 100, 3});
    book.imprimir();
    return 0;
}