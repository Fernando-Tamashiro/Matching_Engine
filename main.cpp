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

std::string formatarPreco(long long centavos) {
    std::string s = std::to_string(centavos / 100);
    long long resto = centavos % 100;
    if (resto != 0) {
        s += ".";
        if (resto < 10) s += "0";
        s += std::to_string(resto);
        if (s.back() == '0') s.pop_back();
    }
    return s;
}

class OrderBook {
private:
    std::map<long long, std::deque<Ordem>, std::greater<long long>> compras;
    std::map<long long, std::deque<Ordem>> vendas;

public:
    void inserir(const Ordem& o) {
        if (o.side == Side::Buy) {
            compras[o.preco].push_back(o);
        } else {
            vendas[o.preco].push_back(o);
        }
    }

    void imprimir() const {
        for (const auto& [preco, fila] : compras) {
            for (const auto& o : fila) {
                std::cout << o.qty << " @ " << formatarPreco(preco) << "\n";
            }
        }
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