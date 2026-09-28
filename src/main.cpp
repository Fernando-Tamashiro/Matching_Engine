#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "order_book.hpp"
#include "preco.hpp"

// converte "buy"/"sell" em Side; devolve false se for outra coisa
bool lerSide(const std::string& texto, Side& side) {
    if (texto == "buy")  { side = Side::Buy;  return true; }
    if (texto == "sell") { side = Side::Sell; return true; }
    return false;
}

std::string textoSide(Side side) {
    return side == Side::Buy ? "buy" : "sell";
}

// imprime os trades no formato do enunciado
void imprimirTrades(const std::vector<Trade>& trades) {
    for (const Trade& t : trades) {
        std::cout << "Trade, price: " << formatarPreco(t.preco)
                  << ", qty: " << t.qty << "\n";
    }
}

// uma limit tenta casar com o outro lado; o que sobrar vai para o livro
void processarLimit(OrderBook& book, Ordem o) {
    int restante = o.qty;
    imprimirTrades(book.executar(o.side, restante, true, o.preco));
    if (restante > 0) {
        o.qty = restante;
        book.inserir(o);
    }
}

int main() {
    OrderBook book;
    long long proximoId = 1;    // identidade da ordem: nunca muda
    long long proximoSeq = 1;   // posicao na fila: muda quando a ordem perde prioridade
    std::string linha;

    while (std::getline(std::cin, linha)) {
        std::istringstream iss(linha);
        std::string comando;
        iss >> comando;

        if (comando == "limit") {
            std::string sideTexto, precoTexto, sobra;
            int qty;
            Side side;

            if (!(iss >> sideTexto >> precoTexto >> qty)) {
                std::cout << "Erro: uso correto e 'limit <buy|sell> <preco> <qty>'\n";
                continue;
            }
            if (iss >> sobra) {
                std::cout << "Erro: texto a mais no fim da linha\n";
                continue;
            }
            if (!lerSide(sideTexto, side)) {
                std::cout << "Erro: side deve ser buy ou sell\n";
                continue;
            }
            long long preco = parsePreco(precoTexto);
            if (preco <= 0) {
                std::cout << "Erro: preco invalido\n";
                continue;
            }
            if (qty <= 0) {
                std::cout << "Erro: quantidade invalida\n";
                continue;
            }

            Ordem o{std::to_string(proximoId), side, preco, qty, proximoSeq};
            proximoId++;
            proximoSeq++;
            std::cout << "Order created: " << sideTexto << " " << qty << " @ "
                      << formatarPreco(preco) << " " << o.id << "\n";
            processarLimit(book, o);

        } else if (comando == "market") {
            std::string sideTexto, sobra;
            int qty;
            Side side;

            if (!(iss >> sideTexto >> qty)) {
                std::cout << "Erro: uso correto e 'market <buy|sell> <qty>'\n";
                continue;
            }
            if (iss >> sobra) {
                std::cout << "Erro: texto a mais no fim da linha\n";
                continue;
            }
            if (!lerSide(sideTexto, side)) {
                std::cout << "Erro: side deve ser buy ou sell\n";
                continue;
            }
            if (qty <= 0) {
                std::cout << "Erro: quantidade invalida\n";
                continue;
            }

            // sem limite; a sobra e descartada
            imprimirTrades(book.executar(side, qty, false, 0));

        } else if (comando == "peg") {
            std::string refTexto, sideTexto, sobra;
            int qty;
            Side side;

            // 1. forma correta: peg <bid|offer> <buy|sell> <qty>
            if (!(iss >> refTexto >> sideTexto >> qty)) {
                std::cout << "Erro: uso correto e 'peg <bid|offer> <buy|sell> <qty>'\n";
                continue;
            }
            if (iss >> sobra) {
                std::cout << "Erro: texto a mais no fim da linha\n";
                continue;
            }

            // 2. so as duas combinacoes que nao nascem cruzando o livro
            if (refTexto == "bid" && sideTexto == "buy") {
                side = Side::Buy;
            } else if (refTexto == "offer" && sideTexto == "sell") {
                side = Side::Sell;
            } else {
                std::cout << "Erro: so sao aceitas 'peg bid buy' e 'peg offer sell'\n";
                continue;
            }
            if (qty <= 0) {
                std::cout << "Erro: quantidade invalida\n";
                continue;
            }

            // 3. precisa existir uma referencia para tirar o preco
            long long preco;
            if (!book.referenciaPegged(side, preco)) {
                std::cout << "Erro: sem preco de referencia no livro\n";
                continue;
            }

            // 4. entra no livro no preco da referencia. Nao precisa de matching:
            //    o melhor bid nunca alcanca o melhor offer, entao ela nunca cruza.
            Ordem o{std::to_string(proximoId), side, preco, qty, proximoSeq, true};
            proximoId++;
            proximoSeq++;
            std::cout << "Order created: " << sideTexto << " " << qty << " @ "
                      << formatarPreco(preco) << " " << o.id << "\n";
            book.inserir(o);

        } else if (comando == "cancel") {
            std::string palavra, id, sobra;

            if (!(iss >> palavra >> id) || palavra != "order") {
                std::cout << "Erro: uso correto e 'cancel order <id>'\n";
                continue;
            }
            if (iss >> sobra) {
                std::cout << "Erro: texto a mais no fim da linha\n";
                continue;
            }

            if (book.cancelar(id)) {
                std::cout << "Order cancelled\n";
            } else {
                std::cout << "Erro: ordem " << id << " nao encontrada\n";
            }

        } else if (comando == "modify") {
            std::string palavra, id, precoTexto, sobra;
            int novaQty;

            // 1. forma correta: modify order <id> <preco> <qty>
            if (!(iss >> palavra >> id >> precoTexto >> novaQty) || palavra != "order") {
                std::cout << "Erro: uso correto e 'modify order <id> <preco> <qty>'\n";
                continue;
            }
            if (iss >> sobra) {
                std::cout << "Erro: texto a mais no fim da linha\n";
                continue;
            }
            long long novoPreco = parsePreco(precoTexto);
            if (novoPreco <= 0) {
                std::cout << "Erro: preco invalido\n";
                continue;
            }
            if (novaQty <= 0) {
                std::cout << "Erro: quantidade invalida\n";
                continue;
            }

            // 2. a ordem existe no livro? e nao e pegged?
            Ordem atual{};
            if (!book.buscar(id, atual)) {
                std::cout << "Erro: ordem " << id << " nao encontrada\n";
                continue;
            }
            if (atual.pegged) {
                std::cout << "Erro: ordens pegged nao podem ser alteradas\n";
                continue;
            }

            std::cout << "Order modified: " << textoSide(atual.side) << " " << novaQty
                      << " @ " << formatarPreco(novoPreco) << " " << id << "\n";

            if (novoPreco == atual.preco && novaQty <= atual.qty) {
                // 3a. so diminuiu: nao prejudica ninguem na fila, mantem a posicao
                book.reduzirQuantidade(id, novaQty);
            } else {
                // 3b. mudou o preco ou aumentou: perde a prioridade.
                //     cancela e recria com o mesmo id e um seq novo.
                //     se o novo preco cruzar o spread, negocia como qualquer limit.
                book.cancelar(id);
                Ordem nova{id, atual.side, novoPreco, novaQty, proximoSeq};
                proximoSeq++;
                processarLimit(book, nova);
            }

        } else if (comando == "print") {
            book.imprimir();
        } else if (comando.empty()) {
            // linha em branco, ignora
        } else {
            std::cout << "Comando desconhecido: " << comando << "\n";
        }

        // qualquer comando pode ter mudado as referencias: as pegged se ajustam.
        // (os erros usam continue e pulam esta linha, porque nao mudaram o livro)
        book.reprecificarPegged(proximoSeq);
    }
    return 0;
}