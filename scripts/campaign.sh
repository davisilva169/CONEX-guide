#!/usr/bin/env bash
# Usage: bash scripts/campaign.sh CONEX NEW_DESTINATION
# Requires an active ROOT environment and an already compiled CONEX installation.
set -euo pipefail
if [ "$#" -ne 2 ]; then echo "Usage: $0 CONEX NEW_DESTINATION" >&2; exit 2; fi
CX="$(realpath "$1")"
test -x "$CX/bin/conex2r"
test -r "$CX/cfg/conex_sibyll.param"
mkdir "$2" # Fail if it exists: preserve earlier campaigns.
DEST="$(realpath "$2")"
export CONEX_ROOT="$CX/cfg"
export ROOT_OUT="$DEST"
cd "$CX"
{ date -u; uname -a; root-config --version; env | sort | grep -E '^(CONEX|ROOT|CORSIKA|URQMD)'; } > "$DEST/environment.txt"
cp cfg/conex_sibyll.param "$DEST/used-config.param"
printf 'group,primary,lgE_eV,zenith_deg,model,events,seed\n' > "$DEST/campaign.csv"
for p in 100 5600; do
  seed=271828; name=proton
  if [ "$p" = 5600 ]; then seed=271829; name=iron; fi
  printf '%s,%s,17,0,5,12,%s\n' "$name" "$p" "$seed" >> "$DEST/campaign.csv"
  bin/conex2r -m 5 -p "$p" -e 17 -E 17 -z 0 -Z 0 \
    -n 12 -s "$seed" -x "$name" > "$DEST/$name.log" 2>&1
done
sha256sum "$DEST"/*.root "$DEST/used-config.param" > "$DEST/SHA256SUMS"
