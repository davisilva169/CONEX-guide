#!/usr/bin/env bash
# Uso: bash scripts/campanha.sh /caminho/sem/espacos/conex /destino/novo
# Requer ROOT e compiladores ja ativados. Nao compila nem altera CONEX.
set -euo pipefail
if [ "$#" -ne 2 ]; then echo "Uso: $0 CONEX DESTINO_NOVO" >&2; exit 2; fi
CX="$(realpath "$1")"
test -x "$CX/bin/conex2r"
test -r "$CX/cfg/conex_sibyll.param"
mkdir "$2" # Falha se ja existir: previne sobrescrita de campanha.
DEST="$(realpath "$2")"
export CONEX_ROOT="$CX/cfg"
export ROOT_OUT="$DEST"
# Executar a partir da raiz e explicitar cfg evita depender do PWD anterior.
cd "$CX"
{ date -u; uname -a; root-config --version; env | sort | grep -E '^(CONEX|ROOT|CORSIKA|URQMD)'; } > "$DEST/ambiente.txt"
cp cfg/conex_sibyll.param "$DEST/config-usada.param"
printf 'grupo,primario,lgE_eV,zenith_graus,modelo,eventos,seed\n' > "$DEST/campanha.csv"
for p in 100 5600; do
  seed=271828; nome=proton
  if [ "$p" = 5600 ]; then seed=271829; nome=ferro; fi
  printf '%s,%s,17,0,5,12,%s\n' "$nome" "$p" "$seed" >> "$DEST/campanha.csv"
  bin/conex2r -m 5 -p "$p" -e 17 -E 17 -z 0 -Z 0 \
    -n 12 -s "$seed" -x "$nome" > "$DEST/$nome.log" 2>&1
done
sha256sum "$DEST"/*.root "$DEST/config-usada.param" > "$DEST/SHA256SUMS"
