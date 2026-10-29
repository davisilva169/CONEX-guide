#!/usr/bin/env bash
# Usage: bash scripts/reproduce.sh CONEX NEW_DESTINATION [python|cpp]
# CONEX and its tables must be obtained through the authors' authorized channels.
set -euo pipefail
if [ "$#" -lt 2 ] || [ "$#" -gt 3 ]; then
  echo "Usage: $0 CONEX NEW_DESTINATION [python|cpp]" >&2; exit 2
fi
ROUTE="${3:-python}"
if [ "$ROUTE" != python ] && [ "$ROUTE" != cpp ]; then echo 'Choose python or cpp' >&2; exit 2; fi
GUIDE="$(cd "$(dirname "$0")/.." && pwd)"
CX="$(realpath "$1")"
test -x "$CX/bin/conex2r"
test -r "$CX/cfg/conex_sibyll.param"
command -v root >/dev/null
mkdir "$2"
DEST="$(realpath "$2")"
mkdir "$DEST/first" "$DEST/analysis" "$DEST/leading"
export CONEX_ROOT="$CX/cfg"
export ROOT_OUT="$DEST/first"
cd "$CX"
bin/conex2r -m 5 -p 100 -e 17 -E 17 -z 0 -Z 0 -n 1 \
  -s 314159 -x first > "$DEST/first/run.log" 2>&1
ARQ="$DEST/first/first_sibyll23e_000314159_100.root"
cd "$GUIDE"
if [ "$ROUTE" = python ]; then
  python python/conex.py inspect "$ARQ" > "$DEST/analysis/schema.txt" 2>&1
  python python/conex.py profile "$ARQ" --output "$DEST/analysis/profile" > "$DEST/analysis/profile.log" 2>&1
else
  root -l -b -q "macros/inspect.C(\"$ARQ\")" > "$DEST/analysis/schema.txt" 2>&1
  root -l -b -q "macros/profile.C(\"$ARQ\",0,\"$DEST/analysis/profile\")" > "$DEST/analysis/profile.log" 2>&1
fi
bash scripts/campaign.sh "$CX" "$DEST/campaign"
A="$DEST/campaign/proton_sibyll23e_000271828_100.root"
B="$DEST/campaign/iron_sibyll23e_000271829_5600.root"
if [ "$ROUTE" = python ]; then
  python python/conex.py histogram "$A" --output "$DEST/analysis/histogram" > "$DEST/analysis/histogram.log" 2>&1
  python python/conex.py compare "$A" "$B" --output "$DEST/analysis/comparison" > "$DEST/analysis/comparison.log" 2>&1
else
  root -l -b -q "macros/histogram.C(\"$A\",\"$DEST/analysis/histogram\")" > "$DEST/analysis/histogram.log" 2>&1
  root -l -b -q "macros/compare.C(\"$A\",\"$B\",\"$DEST/analysis/comparison\")" > "$DEST/analysis/comparison.log" 2>&1
fi
export ROOT_OUT="$DEST/leading"
cd "$CX"
bin/conex2r -m 5 -p 100 -e 17 -E 17 -z 0 -Z 0 -n 1 -s 424242 \
  -K 2 -x leading > "$DEST/leading/run.log" 2>&1
LIDER="$DEST/leading/leading_sibyll23e_000424242_100.root"
cd "$GUIDE"
if [ "$ROUTE" = python ]; then
  python python/conex.py interactions "$LIDER" --output "$DEST/analysis/interactions" > "$DEST/analysis/interactions.log" 2>&1
else
  root -l -b -q "macros/interactions.C(\"$LIDER\",0,\"$DEST/analysis/interactions\")" > "$DEST/analysis/interactions.log" 2>&1
fi
echo "Local results: $DEST"
echo 'Check schema, logs, entries and summaries. Verify conditions before sharing events.'
