# Resultados e receitas de reprodução

Esta pasta contém gráficos e um resumo de referência. Os arquivos de eventos `.root` não são distribuídos: gere-os localmente com CONEX e tabelas obtidos pelos canais dos autores. Os links do PDF para arquivos ROOT apontam para estas receitas, não para downloads de eventos.

Execute os comandos Bash no Linux/WSL, com o ambiente do guia ativo. Defina `CX` para a sua cópia compilada. Confira os nomes de saída reais antes de analisar. Não reutilize uma pasta existente: a saída CONEX pode ser sobrescrita.

```bash
conda activate conex-apoema
export CX="$HOME/simulacoes/conex-7.801"
```

## Primeiro

Gera `primeiro_sibyll23e_000314159_100.root`, um próton de 10^17 eV, vertical, modelo 5 (SIBYLL 2.3e), semente 314159, sem LeadingInteractions (`-K 0`, padrão).

```bash
cd "$CX"
export CONEX_ROOT="$CX/cfg"
mkdir primeiro-teste
export ROOT_OUT="$CX/primeiro-teste"
bin/conex2r -m 5 -p 100 -e 17 -E 17 -z 0 -Z 0 \
  -n 1 -s 314159 -x primeiro > primeiro-teste/run.log 2>&1
status=$?
printf 'Codigo de saida: %s\n' "$status"
tail -n 25 primeiro-teste/run.log
export ARQ="$CX/primeiro-teste/primeiro_sibyll23e_000314159_100.root"
```

Se o comando falhar, leia o log antes de seguir. Volte à raiz do repositório para rodar `script_python/conex.py` ou as macros. Confira Header e Shower e uma entrada em cada árvore; ausência de LeadingInteractions é esperada.

## Campanha

Na raiz do repositório, gere 12 prótons e 12 ferros a 10^17 eV, verticais, modelo 5; sementes 271828 e 271829. O destino precisa ser novo.

```bash
bash scripts/campanha.sh "$CX" "$PWD/minha-campanha"
export A="$PWD/minha-campanha/proton_sibyll23e_000271828_100.root"
export B="$PWD/minha-campanha/ferro_sibyll23e_000271829_5600.root"
```

`A` e `B` são usados em histograma e comparar. Confira as 12 entradas em Shower de cada saída e os logs proton.log/ferro.log. A amostra serve ao aprendizado, não à determinação precisa das distribuições físicas.

## Lideres

Gera `lideres_sibyll23e_000424242_100.root` com `-K 2`: duas interações solicitadas ao longo da cadeia líder, sem registrar toda a genealogia do chuveiro. Um evento, próton de 10^17 eV, vertical, modelo 5 e semente 424242.

```bash
cd "$CX"
mkdir lideres-teste
export CONEX_ROOT="$CX/cfg"
export ROOT_OUT="$CX/lideres-teste"
bin/conex2r -m 5 -p 100 -e 17 -E 17 -z 0 -Z 0 \
  -n 1 -s 424242 -K 2 -x lideres > lideres-teste/run.log 2>&1
export LIDER="$CX/lideres-teste/lideres_sibyll23e_000424242_100.root"
tail -n 25 lideres-teste/run.log
```

Volte à raiz do repositório. Confira a árvore LeadingInteractions, `nInt`, `nPart` e `mult`; `idInt` começa em 1. O guia explica tipos, unidades, convenção CORSIKA e o cuidado com `kinel` nas interações seguintes. Não interprete essa árvore como todas as gerações do chuveiro.

## Todos os exemplos

Na raiz do repositório, com `CX` definido:

```bash
bash scripts/reproduzir.sh "$CX" "$PWD/resultado/reproducao" python
```

Escolha `cpp` no último argumento se preferir essa rota. A reprodução cria `primeiro/`, `campanha/`, `lideres/` e `analise/` dentro do destino novo. Ajuste `ARQ`, `A`, `B` e `LIDER` se usar essas subpastas em vez das receitas individuais. O diretório gerado é ignorado pelo Git.

## Produtos de referência para baixar

- [Perfil do primeiro chuveiro](perfil-primeiro.pdf).
- [Comparação de prótons e ferro](comparacao-primarios.pdf).
- [Resumo da comparação em CSV](resumo-comparacao.csv).
- [Procedência, parâmetros e limites](PROCEDENCIA.md).

As figuras e o resumo vieram de simulações reais identificadas. Repetições em versões, compiladores ou ambientes diferentes não têm garantia de identidade byte a byte. Confira as unidades e as configurações antes de comparar.
