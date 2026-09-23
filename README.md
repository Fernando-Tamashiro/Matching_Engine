# Matching Engine

Matching engine de ativo único, em memória, com ordens limit, market e pegged.

## Escopo

- Um único ativo
- Armazenamento em memória, sem persistência
- Sem preocupação com escalabilidade, nuvem ou elasticidade

## Requisitos

### Base

- [ ] Inserção de ordens com tipo (limit/market), side (buy/sell), price (limit) e qty
- [ ] Ordem limit: passiva, a preço fixo
- [ ] Ordem market: preenchida no melhor preço disponível, imediatamente
- [ ] Saída de trade no formato `Trade, price: <preço>, qty: <qty>`
- [ ] Definir e justificar o comportamento de limit orders cujo preço geraria trade (ignorar ou preencher)

### Adicionais

- [ ] **Visualização do livro.** Função ou método que exibe o estado do book
- [ ] **Ordem de chegada.** Prioridade FIFO dentro do mesmo nível de preço
- [ ] **Cancelamento.** A ordem cancelada é retirada da engine
- [ ] **Alteração.** Preço, quantidade ou ambos. Alteração de preço recoloca a ordem na faixa adequada, perdendo prioridade na fila
- [ ] **Ordens pegged.** Preço atrelado ao melhor bid ou ao melhor offer, atualizado pela engine

## Comandos

```
limit <buy|sell> <preço> <qty>
market <buy|sell> <qty>
peg <bid|offer> <buy|sell> <qty>
cancel order <id>
print book
```

## Exemplo

```
>>> limit buy 10 100
>>> limit sell 20 100
>>> limit sell 20 200
>>> market buy 150
Trade, price: 20, qty: 150
>>> market buy 200
Trade, price: 20, qty: 150
>>> market sell 200
Trade, price: 10, qty: 100
```

## Como executar

```bash
# a definir
```
