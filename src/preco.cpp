#include "preco.hpp"
#include <cctype>

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

// Converte "10.5" em 1050. Retorna -1 se o texto nao for um preco valido.
long long parsePreco(const std::string& texto) {
    size_t ponto = texto.find('.');
    std::string inteira = texto.substr(0, ponto);
    std::string decimal = (ponto == std::string::npos) ? "" : texto.substr(ponto + 1);

    if (inteira.empty() || decimal.size() > 2) return -1;
    for (char c : inteira + decimal) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return -1;
    }
    while (decimal.size() < 2) decimal += "0";

    return std::stoll(inteira) * 100 + std::stoll(decimal);
}