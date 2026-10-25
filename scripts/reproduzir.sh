#!/usr/bin/env bash
# ROOT precisa estar ativo. CONEX e suas tabelas devem ter origem autorizada.
# Exemplo: bash scripts/reproduzir.sh /home/ana/conex /home/ana/run-novo python
# Terceiro argumento opcional: python (padrao) ou cpp. Nao precisa executar ambos.
set -euo pipefail
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  echo "Uso: $0 CONEX DESTINO_NOVO [python|cpp]" >&2; exit 2
fi
ROTA="${3:-python}"
if [ "$ROTA" != python ] && [ "$ROTA" != cpp ]; then
  echo 'Escolha python ou cpp' >&2; exit 2
fi
GUIA="$(cd "$(dirname "$0")/.." && pwd)"
CX="$(realpath "$1")"
test -x "$CX/bin/conex2r"
test -r "$CX/cfg/conex_sibyll.param"
command -v root >/dev/null
mkdir "$2"
DEST="$(realpath "$2")"
mkdir "$DEST/primeiro" "$DEST/analise" "$DEST/lideres"
export CONEX_ROOT="$CX/cfg"
export ROOT_OUT="$DEST/primeiro"
cd "$CX"
bin/conex2r -m 5 -p 100 -e 17 -E 17 -z 0 -Z 0 -n 1 \
  -s 314159 -x primeiro > "$DEST/primeiro/run.log" 2>&1
ARQ="$DEST/primeiro/primeiro_sibyll23e_000314159_100.root"
cd "$GUIA"
if [ "$ROTA" = python ]; then
  python script_python/conex.py inspecionar "$ARQ" > "$DEST/analise/esquema.txt" 2>&1
  python script_python/conex.py perfil "$ARQ" --saida "$DEST/analise/perfil" > "$DEST/analise/perfil.log" 2>&1
else
  root -l -b -q "macros/inspecionar.C(\"$ARQ\")" > "$DEST/analise/esquema.txt" 2>&1
  root -l -b -q "macros/perfil.C(\"$ARQ\",0,\"$DEST/analise/perfil\")" > "$DEST/analise/perfil.log" 2>&1
fi
bash scripts/campanha.sh "$CX" "$DEST/campanha"
A="$DEST/campanha/proton_sibyll23e_000271828_100.root"
B="$DEST/campanha/ferro_sibyll23e_000271829_5600.root"
if [ "$ROTA" = python ]; then
  python script_python/conex.py histograma "$A" --saida "$DEST/analise/histograma" > "$DEST/analise/histograma.log" 2>&1
  python script_python/conex.py comparar "$A" "$B" --saida "$DEST/analise/comparacao" > "$DEST/analise/comparacao.log" 2>&1
else
  root -l -b -q "macros/histograma.C(\"$A\",\"$DEST/analise/histograma\")" > "$DEST/analise/histograma.log" 2>&1
  root -l -b -q "macros/comparar.C(\"$A\",\"$B\",\"$DEST/analise/comparacao\")" > "$DEST/analise/comparacao.log" 2>&1
fi
export ROOT_OUT="$DEST/lideres"
cd "$CX"
bin/conex2r -m 5 -p 100 -e 17 -E 17 -z 0 -Z 0 -n 1 -s 424242 \
  -K 2 -x lideres > "$DEST/lideres/run.log" 2>&1
LIDER="$DEST/lideres/lideres_sibyll23e_000424242_100.root"
cd "$GUIA"
if [ "$ROTA" = python ]; then
  python script_python/conex.py interacoes "$LIDER" --saida "$DEST/analise/interacoes" > "$DEST/analise/interacoes.log" 2>&1
else
  root -l -b -q "macros/interacoes.C(\"$LIDER\",0,\"$DEST/analise/interacoes\")" > "$DEST/analise/interacoes.log" 2>&1
fi
echo "Resultados locais: $DEST"
echo 'Confira esquema, logs, entradas e resumo; nao redistribua eventos sem verificar condicoes.'
