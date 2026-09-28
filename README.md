# Matching Engine

Matching engine de ativo único, em memória, escrita em C++17. Suporta ordens limit, market e pegged, com cancelamento, alteração e prioridade preço-tempo.

## Sumário

- [Como compilar e executar](#como-compilar-e-executar)
- [Comandos](#comandos)
- [Arquitetura](#arquitetura)
- [Estruturas de dados](#estruturas-de-dados)
- [Regras de matching](#regras-de-matching)
- [Decisões de projeto](#decisões-de-projeto)
- [Complexidade](#complexidade)
- [Testes](#testes)
- [Limitações conhecidas](#limitações-conhecidas)

---

## Como compilar e executar

Requer um compilador com suporte a C++17 (testado com g++ 16 via MSYS2 no Windows).

```bash
g++ -std=c++17 -Iinclude src/main.cpp src/order_book.cpp src/preco.cpp -o engine
```

Modo interativo (um comando por linha; encerra com Ctrl+C):

```bash
./engine
```

Executando um arquivo de cenário:

```bash
./engine < testes/enunciado.txt              # bash
Get-Content testes\enunciado.txt | .\engine  # PowerShell
```

## Comandos

| Comando | Sintaxe |
|---|---|
| Limit | `limit <buy\|sell> <preço> <qty>` |
| Market | `market <buy\|sell> <qty>` |
| Pegged | `peg bid buy <qty>` ou `peg offer sell <qty>` |
| Cancelar | `cancel order <id>` |
| Alterar | `modify order <id> <preço> <qty>` |
| Visualizar livro | `print book` |

### Saídas

```
Order created: <side> <qty> @ <preço> <id>
Order modified: <side> <qty> @ <preço> <id>
Order cancelled
Trade, price: <preço>, qty: <qty>
```

Entradas inválidas geram uma linha `Erro: <motivo>` e não alteram o livro.

### Exemplo (enunciado)

```
limit buy 10 100
Order created: buy 100 @ 10 1
limit sell 20 100
Order created: sell 100 @ 20 2
limit sell 20 200
Order created: sell 200 @ 20 3
market buy 150
Trade, price: 20, qty: 150
market buy 200
Trade, price: 20, qty: 150
market sell 200
Trade, price: 10, qty: 100
```

---

## Arquitetura

```
include/
  order.hpp        Side e Ordem
  trade.hpp        Trade (preço e quantidade de um negócio)
  preco.hpp        conversão entre texto e centavos
  order_book.hpp   o livro de ofertas
src/
  main.cpp         leitura de comandos, validação e impressão
  preco.cpp
  order_book.cpp
testes/            um arquivo de comandos por cenário
```

**Divisão de responsabilidades.** O `OrderBook` guarda as ordens e aplica as regras de matching, cancelamento, alteração e repreçamento. O `main` lê os comandos, valida a entrada, atribui id e sequência, e imprime as respostas.

**O matching não imprime.** `OrderBook::executar` devolve um `std::vector<Trade>`, e quem imprime é o `main`. Assim, a regra de negócio não depende do formato da saída.

**Encapsulamento.** O livro e o índice por id são privados do `OrderBook`. Isso garante que os dois nunca fiquem inconsistentes: toda entrada e saída de ordem passa por métodos da classe, que atualizam ambos.

---

## Estruturas de dados

| Estrutura | Uso | Motivo |
|---|---|---|
| `std::map<preço, std::deque<Ordem>>` | cada lado do livro | mantém os preços ordenados; o melhor preço é sempre o primeiro elemento |
| `std::greater` no lado de compra | ordenação decrescente | o melhor comprador é o que paga mais |
| `std::deque<Ordem>` | fila de cada nível de preço | entrada no fim e saída no começo em tempo constante (FIFO) |
| `std::unordered_map<id, (lado, preço)>` | índice por id | cancelamento e alteração vão direto ao nível certo, sem varrer o livro |

Alternativas descartadas para os níveis de preço:

- `unordered_map`: não mantém ordem; achar o melhor preço exigiria olhar todos.
- `vector` ordenado: inserir um preço novo no meio desloca todos os seguintes.
- `priority_queue`: dá acesso rápido ao melhor preço, mas não permite percorrer o livro em ordem (necessário para imprimir) nem remover um nível do meio.

---

## Regras de matching

**Prioridade preço-tempo.** Primeiro o melhor preço; dentro do mesmo preço, a ordem de chegada (campo `seq`).

**Preço do trade.** É sempre o preço da ordem que já estava no livro (passiva). A ordem que chega aceita os termos publicados. Uma compra limite a 22 que encontra uma venda a 20 negocia a 20.

**Execução parcial.** Uma ordem pode ser preenchida por várias contrapartes. A que zera sai do livro; a que sobra permanece com a quantidade reduzida e mantém sua posição na fila.

---

## Decisões de projeto

### 1. Preço em centavos, convertido a partir do texto

Preços são guardados como `long long` em centavos (`10.5` vira `1050`). Ponto flutuante não representa valores como `10.1` exatamente, e duas ordens "ao mesmo preço" poderiam cair em níveis diferentes do livro.

A conversão é feita direto no texto digitado (`parsePreco`), separando parte inteira e decimal. Ler como `double` e multiplicar por 100 reintroduziria o erro na leitura (`0.29 * 100` em `double` não resulta exatamente em `29`).

**Premissa:** no máximo duas casas decimais. Preços com mais casas são rejeitados.

### 2. Limit que cruza o spread é executada

O enunciado permite ignorar ou executar. A escolha foi **executar**: a ordem casa com o outro lado até onde o seu preço permite, e o que sobrar vai para o livro como ordem passiva.

Motivos: é o comportamento de bolsas reais, e reaproveita o mesmo código do matching da market order. A única diferença entre as duas é um limite de preço.

### 3. Market order descarta o saldo

A market não descansa no livro. Se o lado oposto acabar antes de completar, o restante é descartado (é o que o exemplo do enunciado mostra no `market buy 200`, que executa apenas 150). Uma market que chega com o livro vazio não produz saída.

### 4. Saída de trade agregada por preço

Uma ordem que consome várias contrapartes ao mesmo preço gera **uma** linha de `Trade`, com a soma das quantidades. Se atravessar preços diferentes, gera uma linha por preço.

O exemplo do enunciado exige a agregação: `market buy 150` consome duas vendas a 20 e produz uma única linha. Agregar tudo numa linha só, mesmo com preços diferentes, obrigaria a exibir um preço médio em que nenhum negócio aconteceu de fato.

### 5. `Order created` antes dos trades

A confirmação da ordem é impressa antes dos trades que ela gerar, mesmo que seja preenchida por completo. A ordem foi aceita e recebeu um id independentemente do que aconteceu depois.

### 6. Validação de entrada

São rejeitados: side diferente de `buy` ou `sell`; preço não numérico, negativo, zero ou com mais de duas casas; quantidade não inteira, negativa ou zero; texto sobrando no fim da linha.

A checagem de texto sobrando cobre um caso sutil: em `limit buy 10 10.5`, a leitura de inteiro para no ponto e deixa `.5` para trás. Sem essa checagem, a ordem entraria com quantidade 10.

Zero é rejeitado porque não tem significado útil e, na alteração, deixaria uma ordem de quantidade zero parada no livro.

### 7. Cancelamento por índice

Um `unordered_map` guarda, para cada id, o lado e o preço da ordem. O cancelamento consulta o índice e procura apenas dentro da fila daquele preço.

O índice é atualizado em todos os pontos em que uma ordem sai do livro: cancelamento, alteração e **execução completa no matching**. Por isso uma ordem já preenchida responde como "não encontrada".

Cancelar uma ordem parcialmente preenchida remove apenas o saldo restante. O que foi executado é irreversível.

### 8. Alteração

Sintaxe: `modify order <id> <preço> <qty>`, sempre com os dois valores.

| Caso | Comportamento |
|---|---|
| Mesmo preço, quantidade menor ou igual | altera no lugar, **mantém** a prioridade |
| Preço diferente, ou quantidade maior | cancela e recria com o mesmo id e nova sequência: **perde** a prioridade |

Diminuir a quantidade não prejudica ninguém na fila. Aumentar seria uma forma de furar a fila (entrar cedo com pouco e crescer depois). Mudança de preço perde prioridade por exigência do enunciado.

Se o novo preço cruzar o spread, a ordem recriada negocia como qualquer limit (decisão 2). O livro nunca fica em estado cruzado.

**Id e sequência são contadores separados.** O id é a identidade da ordem e nunca muda. A sequência (`seq`) é a posição na fila e muda quando a ordem perde prioridade.

### 9. Ordens pegged

**Combinações suportadas:** `peg bid buy` (compra que segue o melhor bid) e `peg offer sell` (venda que segue o melhor offer). As combinações cruzadas (`peg offer buy`, `peg bid sell`) nasceriam cruzando o livro e implicariam execução contínua contra o lado oposto, comportamento que o enunciado não descreve. O texto do enunciado exemplifica `peg bid buy` e diz que "o mesmo funciona para uma ordem peg to offer", o que indica o espelho.

**Cálculo da referência.** O preço de referência é o melhor preço do lado **entre as ordens não pegged**. Ignorar todas as pegged evita que uma pegged que se torna o melhor preço passe a seguir a si mesma, e que duas pegged no mesmo lado se sigam mutuamente.

**Direção.** A pegged segue a referência para cima e para baixo.

**Prioridade no repreçamento.**

| Movimento | Exemplo | Sequência |
|---|---|---|
| Melhorou para a contraparte | compra subindo, venda descendo | mantém a original |
| Piorou para a contraparte | compra descendo, venda subindo | recebe uma nova |

A primeira linha vem do exemplo do enunciado: a pegged repreçada de 10 para 10.1 fica **à frente** da limit de 300 que chegou depois dela. A justificativa é que o movimento foi feito pela engine, não pelo trader.

A segunda segue a mesma lógica da alteração: piorar os termos custa a posição na fila. Sem essa regra, uma pegged que passou a sessão num preço melhor poderia descer e passar à frente de ordens que já esperavam no preço de baixo.

**Inserção na fila por sequência.** Como uma pegged pode chegar a um nível mantendo uma sequência antiga, a inserção procura a posição correta pelo `seq`, de trás para frente. Para ordens novas (maior `seq`), isso para na primeira comparação e equivale a inserir no fim.

**Sem referência.** Uma pegged que chega sem referência disponível é rejeitada, pois não há de onde tirar um preço. Se a referência desaparecer depois que a pegged já está no livro, ela permanece congelada no último preço e volta a seguir quando uma referência reaparecer.

**Alteração.** Ordens pegged não podem ser alteradas. O preço delas pertence à engine, e aplicar o `modify` as transformaria em limits comuns sem que o usuário pedisse. Para mudar uma pegged, cancela-se e cria-se outra.

**Quando o repreçamento acontece.** Ao fim de todo comando bem-sucedido.

**O repreçamento nunca gera trade.** Uma `peg bid buy` vai para o melhor bid não pegged, que é sempre menor que o melhor offer; o espelho vale para a venda. Por isso uma única passada basta: repreçar não altera nenhuma referência (as referências ignoram pegged) e não dispara matching.

---

## Complexidade

P = níveis de preço de um lado, k = ordens em um nível, N = ordens no livro.

| Operação | Custo |
|---|---|
| Melhor preço | O(1) |
| Inserir ordem nova | O(log P) |
| Consumir uma contraparte no matching | O(1), mais O(log P) quando um nível esvazia |
| Cancelar, buscar, reduzir quantidade | O(1) no índice + O(log P) + O(k) dentro do nível |
| Reposicionar pegged repreçada | O(log P) + O(k) |
| Repreçamento ao fim de cada comando | O(N) no pior caso |

---

## Testes

Cada arquivo em `testes/` é um cenário. A saída é conferida contra o resultado esperado.

| Arquivo | Cobre |
|---|---|
| `enunciado.txt` | exemplo base do enunciado, linha a linha |
| `teste.txt` | validação de entrada e mensagens de erro |
| `limit_cruzando.txt` | limit que cruza o spread, nos dois lados, com saldo indo ao livro |
| `cancelamento.txt` | cancelamento simples, duplo, de ordem parcialmente e totalmente preenchida |
| `alteracao.txt` | exemplo do enunciado; redução mantendo prioridade; aumento perdendo prioridade; alteração que cruza o spread |
| `pegged.txt` | exemplo do enunciado; repreçamento para cima e para baixo; espelho na venda; matching contra pegged; congelamento e rejeição sem referência |

---

## Limitações conhecidas

- **Ativo único e memória volátil**, conforme as premissas do enunciado.
- **Duas casas decimais** no máximo para preços.
- **Repreçamento varre o livro** a cada comando (O(N)). Uma melhoria seria manter uma lista separada das pegged de cada lado e só repreçá-las quando a referência mudar.
- **Remoção no meio da fila** (cancelamento, alteração, reposicionamento de pegged) é linear no tamanho do nível. Uma `std::list` com iteradores guardados no índice tornaria isso O(1), ao custo de mais memória por ordem e de ter que manter os iteradores válidos.
- **Testes por conferência de saída**, sem framework de asserção automatizada.
- **Quantidade como `int`**, limitada a cerca de 2,1 bilhões.