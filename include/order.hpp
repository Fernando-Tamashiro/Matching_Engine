#pragma once
#include <string>

enum class Side { Buy, Sell };

struct Ordem {
    std::string id;
    Side side;
    long long preco;   // em centavos: 10.10 -> 1010
    int qty;
    long long seq;     // ordem de chegada
    bool pegged = false;
};