# Results and reproduction recipes

This directory provides reference plots and an aggregate summary. Event `.root` files are **not distributed**. Generate them locally using CONEX and model tables obtained through authorized channels. Links from ROOT filenames in either PDF lead to these recipes, not to event downloads.

Use Bash on Linux or WSL, with the guide's environment active. Set `CX` to your compiled copy. Check actual output names before analysis and use a new directory for each run: CONEX can overwrite existing output.

```bash
conda activate conex-apoema
CX="$HOME/simulations/conex-7.801"
```

## First

Generate `first_sibyll23e_000314159_100.root`: one vertical proton at 10^17 eV, model 5 (SIBYLL 2.3e), seed 314159, without LeadingInteractions (`-K 0`, the runtime default).

```bash
cd "$CX"
export CONEX_ROOT="$CX/cfg"
mkdir first-run
export ROOT_OUT="$CX/first-run"
bin/conex2r -m 5 -p 100 -e 17 -E 17 -z 0 -Z 0 \
  -n 1 -s 314159 -x first > first-run/run.log 2>&1
status=$?
printf 'Exit status: %s\n' "$status"
tail -n 25 first-run/run.log
export ARQ="$CX/first-run/first_sibyll23e_000314159_100.root"
```

If a command fails, read the log before continuing. Return to the repository root to run `python/conex.py` or macros. Check Header and Shower, each with one entry; missing LeadingInteractions is expected.

## Campaign

At the repository root, generate 12 protons and 12 iron nuclei at 10^17 eV, vertically incident, model 5; seeds 271828 and 271829. The destination must be new.

```bash
bash scripts/campaign.sh "$CX" "$PWD/my-campaign"
export A="$PWD/my-campaign/proton_sibyll23e_000271828_100.root"
export B="$PWD/my-campaign/iron_sibyll23e_000271829_5600.root"
```

Use `A` and `B` for histogram and compare. Check the 12 Shower entries in each file and the proton.log/iron.log logs. This sample teaches the workflow; it does not precisely determine physical distributions.

## Leading

Generate `leading_sibyll23e_000424242_100.root` with `-K 2`: up to two successive interactions along the leading chain, not the complete shower genealogy. One vertical proton at 10^17 eV, model 5, seed 424242.

```bash
cd "$CX"
mkdir leading-run
export CONEX_ROOT="$CX/cfg"
export ROOT_OUT="$CX/leading-run"
bin/conex2r -m 5 -p 100 -e 17 -E 17 -z 0 -Z 0 \
  -n 1 -s 424242 -K 2 -x leading > leading-run/run.log 2>&1
export LIDER="$CX/leading-run/leading_sibyll23e_000424242_100.root"
tail -n 25 leading-run/run.log
```

Return to the repository root. Inspect LeadingInteractions, nInt, nPart and mult; idInt starts at 1. Both guides explain types, units, CORSIKA numbering and the qualification on kinel in later interactions. This tree does not contain every shower generation.

## All examples

At the repository root with `CX` defined:

```bash
bash scripts/reproduce.sh "$CX" "$PWD/results/reproduction" python
```

Choose `cpp` in the final argument for that route. The script creates `first/`, `campaign/`, `leading/` and `analysis/` inside the new destination. Adapt `ARQ`, `A`, `B` and `LIDER` to those directories if using this route rather than the individual recipes. Generated output is ignored by Git.

## Reference products to download

- [First-shower profile](first-profile.pdf).
- [Proton and iron comparison](primary-comparison.pdf).
- [Aggregate comparison CSV](comparison-summary.csv).
- [Provenance, settings and limitations](PROVENANCE.md).

These products come from identified real simulations. Repetitions across versions, compilers or environments are not guaranteed to be byte-identical. Check units and settings before comparing.
