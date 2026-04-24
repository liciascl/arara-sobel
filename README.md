
### 1.  Passe para GPU

A partir do código fornecido:

* implemente um **kernel CUDA**
* cada thread deve processar **1 pixel**
* mantenha a mesma lógica da versão em CPU

Essa será sua versão **GPU ingênua**


### 2. Otimização

A partir da versão ingênua, aplique:

* **tiling**
* uso de **shared memory**

Objetivo: reduzir acessos à memória global



### 3. Testes

Meça o tempo de execução e complete a tabela:


| Versão         | Block Size | Tempo (ms) |
|---------------|-----------|-----------|
| CPU           | -         |           |
| GPU ingênua   | -       |           |
| GPU otimizada | 8×8       |           |
| GPU otimizada | 16×16     |           |
| GPU otimizada | 32×32     |           |


### 4. Análise

Responda:

* Qual configuração apresentou melhor desempenho?
* A otimização trouxe ganho significativo?
* O código é limitado por computação ou memória?

