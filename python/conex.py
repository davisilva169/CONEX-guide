"""Read small CONEX samples with PyROOT.

Run from the repository root: python python/conex.py --help
Requires ROOT/PyROOT and the Python standard library. No pandas,
NumPy, matplotlib or uproot are required. Input files are opened in READ mode.
"""
from array import array
from collections import Counter
from contextlib import contextmanager
from pathlib import Path
import argparse
import csv
import math
import statistics
import ROOT

ROOT.gROOT.SetBatch(True)  # Export PDF files without requiring a graphical window.


@contextmanager
def open_file(name):
    """Keep the TFile open while its tree objects are in use."""
    file = ROOT.TFile.Open(str(name), "READ")
    if not file or file.IsZombie():
        raise RuntimeError(f"Cannot open ROOT file: {name}")
    try:
        yield file
    finally:
        file.Close()


def require_tree(file, name):
    t = file.Get(name)  # Without ;cycle, ROOT selects the latest key.
    if not t or not t.InheritsFrom("TTree"):
        raise RuntimeError(f"Missing tree: {name}")
    return t


def leaf(t, name, dtype=None):
    """Use the branch name; its leaf can have different spelling (mult -> Mult)."""
    b = t.GetBranch(name)
    if not b or b.GetListOfLeaves().GetEntries() != 1:
        raise RuntimeError(f"Missing or composite branch: {name}")
    l = b.GetListOfLeaves().At(0)
    if dtype and l.GetTypeName() != dtype:
        raise RuntimeError(f"Unexpected type in {name}: {l.GetTypeName()}")
    return l


def values(t, name, dtype=None):
    """Copy values from the current entry, respecting its length counter."""
    l = leaf(t, name, dtype)
    convert = int if l.GetTypeName() == "Int_t" else float
    return [convert(l.GetValue(i)) for i in range(l.GetLen())]


def scalar(t, name, dtype=None):
    v = values(t, name, dtype)
    if len(v) != 1:
        raise RuntimeError(f"Expected a scalar: {name}")
    return v[0]


def load_entry(t, index):
    if not 0 <= index < t.GetEntries() or t.GetEntry(index) <= 0:
        raise RuntimeError(f"Missing or unreadable entry: {index}")


def output_paths(prefix, extensions):
    """Refuse to overwrite; the destination directory must exist."""
    names = [Path(str(prefix) + ext) for ext in extensions]
    for name in names:
        if name.exists():
            raise RuntimeError(f"Output already exists: {name}")
        if not name.parent.is_dir():
            raise RuntimeError(f"Create the output directory: {name.parent}")
    return names


def inspect(name):
    with open_file(name) as f:
        f.ls()
        for tree_name in ["Header", "Shower", "LeadingInteractions"]:
            t = f.Get(tree_name)
            if not t:
                if tree_name == "LeadingInteractions":
                    print("LeadingInteractions is absent: expected with -K 0.")
                    continue
                raise RuntimeError(f"Missing required tree: {tree_name}")
            print(tree_name, "entries:", t.GetEntries())
            t.Print()
            if t.GetEntries():
                t.Show(0)


def read_profile(name, event=0):
    with open_file(name) as f:
        t = require_tree(f, "Shower")
        load_entry(t, event)
        n = scalar(t, "nX", "Int_t")
        x, ncar, dep = [values(t, k, "Float_t") for k in ["X", "N", "dEdX"]]
        if n < 2 or any(len(v) != n for v in [x, ncar, dep]):
            raise RuntimeError("Dimensions do not match nX")
        if not all(math.isfinite(v) for v in x + ncar + dep):
            raise RuntimeError("Profile contains a non-finite value")
        if any(x[i + 1] <= x[i] for i in range(n - 1)):
            raise RuntimeError("Depths are not strictly increasing")
    mid = [(x[i] + x[i + 1]) / 2 for i in range(n - 1)]
    integral = sum(dep[i] * (x[i + 1] - x[i]) for i in range(n - 1))
    return x, ncar, mid, dep[:n - 1], integral


def profile(name, event, prefix):
    pdf, csv_path = output_paths(prefix, [".pdf", ".csv"])
    x, ncar, mid, dep, integral = read_profile(name, event)
    # array('d') provides TGraph-compatible numerical buffers without NumPy.
    gn = ROOT.TGraph(len(x), array("d", x), array("d", ncar))
    gd = ROOT.TGraph(len(mid), array("d", mid), array("d", dep))
    gn.SetTitle("Charged particles;X [g/cm^{2}];N(X)")
    gd.SetTitle("Deposit in the intervals;X [g/cm^{2}];dE/dX [GeV/(g/cm^{2})]")
    gn.SetLineColor(ROOT.kOrange + 7)
    gd.SetLineColor(ROOT.kBlue + 1)
    gn.SetLineWidth(2)
    gd.SetLineWidth(2)
    c = ROOT.TCanvas("cProfilePython", "Profile", 1000, 470)
    c.Divide(2, 1)
    c.cd(1).SetLeftMargin(.15)
    gn.Draw("AL")
    c.cd(2).SetLeftMargin(.15)
    gd.Draw("AL")
    c.Print(str(pdf))
    with csv_path.open("w", newline="", encoding="utf-8") as out:
        w = csv.writer(out)
        w.writerow(["X_gcm2", "N_charged", "Xmid_gcm2", "dEdX_GeV_per_gcm2"])
        for i in range(len(x)):
            w.writerow([x[i], ncar[i], mid[i] if i < len(mid) else "",
                        dep[i] if i < len(dep) else ""])
    print("Integral over stored intervals [GeV]:", integral)
    c.Close()


def maxima(name):
    data, rejected = [], 0
    with open_file(name) as f:
        t = require_tree(f, "Shower")
        leaf(t, "XmxdEdX", "Float_t")
        for i in range(t.GetEntries()):
            load_entry(t, i)
            x = scalar(t, "XmxdEdX", "Float_t")
            if not math.isfinite(x) or x <= 0:
                rejected += 1
            else:
                data.append(x)
    if not data:
        raise RuntimeError("No valid maxima")
    return data, rejected


def summary(data, rejected=0):
    n = len(data)
    s = statistics.stdev(data) if n > 1 else None
    return dict(n=n, rejected=rejected, mean=statistics.fmean(data),
                std_dev=s, sem=s / math.sqrt(n) if s is not None else None)


def hist(data, name, color):
    h = ROOT.TH1D(name, ";X_{max}^{dE/dX} [g/cm^{2}];Events / 40 g cm^{-2}",
                  30, 0, 1200)
    h.SetDirectory(0)  # Prevent TFile closure from deleting the histogram.
    h.SetLineColor(color)
    h.SetLineWidth(3)
    for x in data:
        h.Fill(x)
    return h


def write_summaries(name, groups):
    with name.open("w", newline="", encoding="utf-8") as out:
        w = csv.writer(out)
        w.writerow(["group", "n", "rejected", "mean_gcm2", "sample_std_gcm2",
                    "sem_gcm2", "underflow", "overflow"])
        for label, r, h in groups:
            w.writerow([label, r["n"], r["rejected"], r["mean"], r["std_dev"],
                        r["sem"], h.GetBinContent(0), h.GetBinContent(31)])
            print(label, r, "underflow=", h.GetBinContent(0),
                  "overflow=", h.GetBinContent(31))


def histogram(name, prefix):
    pdf, csv_path = output_paths(prefix, [".pdf", ".csv"])
    data, rejected = maxima(name)
    if len(data) < 2:
        raise RuntimeError("Histogram summaries require at least two valid events")
    h = hist(data, "hPython", ROOT.kBlue + 1)
    ROOT.gStyle.SetOptStat(0)
    c = ROOT.TCanvas("cHistPython", "Maxima", 850, 530)
    c.SetLeftMargin(.13)
    h.Draw("HIST")
    c.Print(str(pdf))
    write_summaries(csv_path, [("sample", summary(data, rejected), h)])
    c.Close()


def compare(file_a, file_b, prefix):
    pdf, csv_path = output_paths(prefix, [".pdf", ".csv"])
    groups = []
    for name, label, color in [(file_a, "proton", ROOT.kOrange + 7),
                              (file_b, "iron", ROOT.kBlue + 1)]:
        data, rejected = maxima(name)
        if len(data) < 2:
            raise RuntimeError("Comparison requires at least two valid events per group")
        groups.append((label, summary(data, rejected), hist(data, "h_" + label, color)))
    ROOT.gStyle.SetOptStat(0)
    c = ROOT.TCanvas("cCompararPython", "Comparison", 850, 530)
    c.SetLeftMargin(.13)
    a, b = groups[0][2], groups[1][2]
    a.SetMaximum(1.3 * max(a.GetMaximum(), b.GetMaximum()))
    a.Draw("HIST")
    b.Draw("HIST SAME")
    leg = ROOT.TLegend(.67, .72, .90, .89)
    leg.SetBorderSize(0)
    leg.AddEntry(a, "Proton", "l")
    leg.AddEntry(b, "Iron", "l")
    leg.Draw()
    c.Print(str(pdf))
    write_summaries(csv_path, groups)
    c.Close()


def read_interactions(name, event=0):
    """Two counters: nInt for interactions and nPart for particles."""
    with open_file(name) as f:
        t = require_tree(f, "LeadingInteractions")
        load_entry(t, event)
        ni = scalar(t, "nInt", "Int_t")
        npart = scalar(t, "nPart", "Int_t")
        ints = {k: values(t, k, dtype) for k, dtype in [
            ("pId", "Int_t"), ("pEnergy", "Double_t"), ("mult", "Int_t"),
            ("matg", "Int_t"), ("kinel", "Double_t"), ("depth", "Double_t"),
            ("height", "Double_t")]}
        parts = {k: values(t, k, dtype) for k, dtype in [
            ("Energy", "Double_t"), ("Type", "Int_t"), ("idInt", "Int_t")]}
        for k in ["px", "py", "pz"]:
            if t.GetBranch(k):
                parts[k] = values(t, k, "Double_t")
        if any(len(v) != ni for v in ints.values()) or any(len(v) != npart for v in parts.values()):
            raise RuntimeError("Invalid dimensions in LeadingInteractions")
        if any(not 1 <= i <= ni for i in parts["idInt"]):
            raise RuntimeError("idInt is outside the 1..nInt convention of this distribution")
    return ints, parts


def interactions(name, event, prefix):
    (csv_path,) = output_paths(prefix, [".csv"])
    ints, parts = read_interactions(name, event)
    counts = Counter(parts["idInt"])
    with csv_path.open("w", newline="", encoding="utf-8") as out:
        w = csv.writer(out)
        w.writerow(["idInt", "pId", "pEnergy_GeV", "mult", "stored_particles",
                    "kinel", "matg", "depth_gcm2", "height_m"])
        for i in range(len(ints["mult"])):
            row = [i + 1, ints["pId"][i], ints["pEnergy"][i], ints["mult"][i],
                     counts[i + 1], ints["kinel"][i], ints["matg"][i],
                     ints["depth"][i], ints["height"][i]]
            w.writerow(row)
            print(row)
    print("nInt =", len(ints["mult"]), "nPart =", len(parts["Energy"]))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest="action", required=True)
    sub.add_parser("inspect").add_argument("file")
    for action in ["profile", "interactions"]:
        q = sub.add_parser(action)
        q.add_argument("file")
        q.add_argument("--event", type=int, default=0)
        q.add_argument("--output", default=action)
    q = sub.add_parser("histogram")
    q.add_argument("file")
    q.add_argument("--output", default="histogram")
    q = sub.add_parser("compare")
    q.add_argument("proton")
    q.add_argument("iron")
    q.add_argument("--output", default="comparison")
    args = p.parse_args()
    if args.action == "inspect":
        inspect(args.file)
    elif args.action in ["profile", "interactions"]:
        globals()[args.action](args.file, args.event, args.output)
    elif args.action == "histogram":
        histogram(args.file, args.output)
    else:
        compare(args.proton, args.iron, args.output)


if __name__ == "__main__":
    main()
