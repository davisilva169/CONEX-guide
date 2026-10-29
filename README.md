# CONEX guide

A practical guide to standalone CONEX for the APOEMA group, from installation and a first air-shower simulation to reading ROOT trees and performing basic analyses.

**Read the guide:** [English PDF](CONEX-guide-en.pdf) · [Português brasileiro PDF](CONEX-guide-pt-BR.pdf)

Both editions use the same English file paths and executable commands. Choose either **Python/PyROOT** or **C++/ROOT** for analysis; you do not need both routes. The editable LaTeX projects are supplied separately and are not published here.

## Get started

1. Download a guide PDF and use **Code > Download ZIP** to obtain the companion files. Extract the archive and preserve its directories.
2. Follow the Linux or WSL 2 installation route in the guide. Obtain CONEX and model tables through the authors' authorized channels; this repository does not supply or install them.
3. Run commands from the repository root, with the ROOT environment active and `CX` pointing to your compiled CONEX copy.
4. Generate event files locally using the [results recipes](results/README.md). Event `.root` files are not available for download here.
5. Follow section 7 for Python/PyROOT or section 8 for C++/ROOT.

```bash
conda activate conex-apoema
CX="$HOME/simulations/conex-7.801"
python python/conex.py --help
```

Once assigned, `"$CX"` expands automatically to the stored path. Adjust the assignment to your installation; do not replace every later `$CX` by hand. Reactivate the environment and redefine paths in each new terminal session.

## Repository layout

| Path | Contents |
|---|---|
| `CONEX-guide-en.pdf` | Complete English guide |
| `CONEX-guide-pt-BR.pdf` | Complete Brazilian Portuguese guide |
| [`python/`](python/) | One PyROOT script with five analysis operations |
| [`macros/`](macros/) | Five equivalent C++/ROOT macros |
| [`scripts/`](scripts/) | Campaign generation and full example reproduction |
| [`results/`](results/) | Recipes, reference plots, summary and provenance; no event files |

## Python with PyROOT

Requires ROOT with PyROOT and the Python standard library. This script does not require pandas, NumPy, matplotlib or uproot. Define `ARQ`, `A`, `B` and `LIDER` using the [data recipes](results/README.md).

```bash
python python/conex.py inspect "$ARQ"
python python/conex.py profile "$ARQ" --output profile-python
python python/conex.py histogram "$A" --output histogram-python
python python/conex.py compare "$A" "$B" --output comparison-python
python python/conex.py interactions "$LIDER" --output interactions-python
```

`--event 0` is the default for profile and interactions. Input files are opened read-only. Existing output files are refused; choose a new prefix when repeating an analysis. The guide uses `profile-study` for the later detailed example so it does not collide with the first profile.

## C++ with ROOT

Download the whole macros directory: `histogram.C` includes `compare.C`. Use the same data variables:

```bash
root -l -b -q "macros/inspect.C(\"$ARQ\")"
root -l -b -q "macros/profile.C(\"$ARQ\",0,\"profile-cpp\")"
root -l -b -q "macros/histogram.C(\"$A\",\"histogram-cpp\")"
root -l -b -q "macros/compare.C(\"$A\",\"$B\",\"comparison-cpp\")"
root -l -b -q "macros/interactions.C(\"$LIDER\",0,\"interactions-cpp\")"
```

Both interfaces implement the same profile conventions, selection and summaries. The illustrative TTreeReader excerpt in the guide is not a standalone program; execute the complete macro instead.

## Reproduce the examples

After installing ROOT and compiling CONEX, run at the repository root:

```bash
bash scripts/reproduce.sh "$CX" "$PWD/results/reproduction" python
```

The destination must be new. The optional third argument is `python` by default or `cpp` for the macro route. The script produces **26 events**: one first shower, 12 protons, 12 iron nuclei and one event with `-K 2`. Logs, configurations and analyses are stored under the destination. Generated `results/reproduction/` is ignored by Git. The scripts do not download or compile CONEX.

## Scope, versions and data sharing

The reference distribution is stored as `conex2r7.801`, identified by its README as 7.80, with internal version 7801 and output format 2.51. These identifiers have different meanings and are not interchangeable. The guide does not claim this is the newest worldwide release.

The examples were validated on Linux through WSL 2 using ROOT 6.36.04 and Python 3.13.17. This does not establish native Windows support. Samples are educational and too small for precision research conclusions.

Event `.root` files are excluded because the studied distribution's `src/urqmd13/Copyright` requires the authors' consent to pass UrQMD-generated events to third parties. Reproduce events locally. Reference plots and aggregate summaries are identified in [provenance](results/PROVENANCE.md).

## Credits

Guide author: **Davi S-S**, APOEMA. The earlier manual by **Marcos Olegário** contributed the installation route and introduction to output trees. The supplied APOEMA template and logo, credited to **DAVI**, retain their visual identity. CONEX, ROOT and interaction models are the work of their respective authors; follow the guide's scientific references and the distribution's citation instructions.

The former `guia-conex-apoema` repository was renamed to `CONEX-guide`. English resource names replace the earlier Portuguese paths; use the commands in the current PDFs.
