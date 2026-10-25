# Operações no CONEX — APOEMA

Guia de Davi S-S para começar no CONEX e analisar chuveiros atmosféricos extensos. O manual anterior de Marcos Olegário contribuiu com o percurso de instalação e a apresentação das árvores. CONEX, ROOT e modelos de interação pertencem a seus respectivos autores; consulte as referências do guia.

## Começar

1. Abra ou baixe o [guia em PDF](guia-conex.pdf).
2. Use **Code > Download ZIP** e extraia o pacote. Preserve as pastas.
3. Siga a instalação do CONEX e ROOT no guia. Este repositório não instala o simulador nem fornece suas tabelas.
4. Escolha uma rota: **Python/PyROOT** (seção 7) ou **C++/ROOT** (seção 8). Basta uma.
5. Gere os arquivos `.root` localmente pelas [receitas de resultado](resultado/README.md).

O PDF final e os códigos são publicados juntos. Os fontes LaTeX e os registros de manutenção não fazem parte deste repositório.

## Organização

| Pasta/arquivo | Conteúdo |
|---|---|
| `guia-conex.pdf` | Guia completo |
| [`script_python/`](script_python/) | Um script PyROOT, cinco operações |
| [`macros/`](macros/) | Cinco macros C++ equivalentes |
| [`scripts/`](scripts/) | Campanha e reprodução |
| [`resultado/`](resultado/) | Receitas, gráficos, resumo e procedência; sem eventos `.root` |

Abra o terminal Linux na raiz do pacote extraído. Ative o ambiente preparado no guia. O caminho `CX` deve apontar à cópia compilada do CONEX.

```bash
conda activate conex-apoema
export CX="$HOME/simulacoes/conex-7.801"
python script_python/conex.py --help
```

## Python com PyROOT

Requer o pacote ROOT com PyROOT e a biblioteca padrão do Python. Não exige pandas, NumPy, matplotlib ou uproot para executar este script.

```bash
python script_python/conex.py inspecionar "$ARQ"
python script_python/conex.py perfil "$ARQ" --saida perfil-python
python script_python/conex.py histograma "$A" --saida histograma-python
python script_python/conex.py comparar "$A" "$B" --saida comparacao-python
python script_python/conex.py interacoes "$LIDER" --saida interacoes-python
```

Defina `ARQ`, `A`, `B` e `LIDER` pelos caminhos descritos nas receitas. `--evento 0` é o padrão de perfil e interacoes. Os dados são abertos somente para leitura; as análises recusam sobrescrever resultados.

## C++ com ROOT

Baixe a pasta macros inteira: `histograma.C` depende de `comparar.C`. Com as mesmas variáveis:

```bash
root -l -b -q "macros/inspecionar.C(\"$ARQ\")"
root -l -b -q "macros/perfil.C(\"$ARQ\",0,\"perfil-cpp\")"
root -l -b -q "macros/histograma.C(\"$A\",\"histograma-cpp\")"
root -l -b -q "macros/comparar.C(\"$A\",\"$B\",\"comparacao-cpp\")"
root -l -b -q "macros/interacoes.C(\"$LIDER\",0,\"interacoes-cpp\")"
```

As duas interfaces cobrem as mesmas operações. Não é preciso rodar as duas.

## Reprodução de uma vez

Depois de instalar e compilar o CONEX, execute na raiz deste repositório:

```bash
bash scripts/reproduzir.sh "$CX" "$PWD/resultado/reproducao" python
```

O destino precisa ser novo. O terceiro argumento é opcional: `python` é o padrão; `cpp` escolhe as macros. Serão gerados 26 eventos: um primeiro chuveiro, 12 prótons, 12 ferros e um evento com `-K 2`. Os resultados, logs e análises ficam em `resultado/reproducao/` e são ignorados pelo Git. Os scripts não baixam nem compilam o CONEX.

## Dados e distribuição

Os eventos `.root` ficam fora deste repositório, conforme a condição de consentimento para repasse de eventos presente em `src/urqmd13/Copyright` da distribuição estudada. Reproduza-os localmente. [Procedência dos gráficos e resumos](resultado/PROCEDENCIA.md).

CONEX de referência: pasta conex2r7.801, README 7.80, versão interna 7801 e formato de saída 2.51. Esses identificadores não são intercambiáveis. Exemplos validados em Linux pelo WSL 2 com ROOT 6.36.04; isso não comprova suporte nativo ao Windows. As amostras são didáticas e não sustentam conclusões com estatística de pesquisa.
