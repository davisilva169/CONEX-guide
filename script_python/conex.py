"""Leitura de pequenas amostras CONEX com PyROOT.

Executar a partir da raiz do projeto: python script_python/conex.py --help
Depende de ROOT/PyROOT e da biblioteca padrao do Python. Nao exige pandas,
NumPy, matplotlib ou uproot. Os arquivos de entrada sao abertos em READ.
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

ROOT.gROOT.SetBatch(True)  # Exportar PDF sem depender de uma janela grafica.


@contextmanager
def abrir(nome):
    """Manter o TFile aberto enquanto os objetos da arvore sao utilizados."""
    arquivo = ROOT.TFile.Open(str(nome), "READ")
    if not arquivo or arquivo.IsZombie():
        raise RuntimeError(f"Arquivo ROOT inacessivel: {nome}")
    try:
        yield arquivo
    finally:
        arquivo.Close()


def arvore(arquivo, nome):
    t = arquivo.Get(nome)  # Sem ;ciclo: ROOT seleciona a chave mais recente.
    if not t or not t.InheritsFrom("TTree"):
        raise RuntimeError(f"Arvore ausente: {nome}")
    return t


def folha(t, nome, tipo=None):
    """Usar o nome da branch; a folha pode ter outra grafia (mult -> Mult)."""
    b = t.GetBranch(nome)
    if not b or b.GetListOfLeaves().GetEntries() != 1:
        raise RuntimeError(f"Branch ausente ou composta: {nome}")
    l = b.GetListOfLeaves().At(0)
    if tipo and l.GetTypeName() != tipo:
        raise RuntimeError(f"Tipo inesperado em {nome}: {l.GetTypeName()}")
    return l


def valores(t, nome, tipo=None):
    """Copiar os valores da entrada atual, respeitando seu contador."""
    l = folha(t, nome, tipo)
    converter = int if l.GetTypeName() == "Int_t" else float
    return [converter(l.GetValue(i)) for i in range(l.GetLen())]


def escalar(t, nome, tipo=None):
    v = valores(t, nome, tipo)
    if len(v) != 1:
        raise RuntimeError(f"Esperado escalar: {nome}")
    return v[0]


def entrada(t, indice):
    if not 0 <= indice < t.GetEntries() or t.GetEntry(indice) <= 0:
        raise RuntimeError(f"Entrada inexistente ou ilegivel: {indice}")


def saidas(prefixo, extensoes):
    """Recusar sobrescrita; a pasta de destino deve existir."""
    nomes = [Path(str(prefixo) + ext) for ext in extensoes]
    for nome in nomes:
        if nome.exists():
            raise RuntimeError(f"Saida ja existe: {nome}")
        if not nome.parent.is_dir():
            raise RuntimeError(f"Crie a pasta de saida: {nome.parent}")
    return nomes


def inspecionar(nome):
    with abrir(nome) as f:
        f.ls()
        for nome_arvore in ["Header", "Shower", "LeadingInteractions"]:
            t = f.Get(nome_arvore)
            if not t:
                if nome_arvore == "LeadingInteractions":
                    print("LeadingInteractions ausente: normal quando -K 0.")
                    continue
                raise RuntimeError(f"Arvore obrigatoria ausente: {nome_arvore}")
            print(nome_arvore, "entradas:", t.GetEntries())
            t.Print()
            if t.GetEntries():
                t.Show(0)


def ler_perfil(nome, evento=0):
    with abrir(nome) as f:
        t = arvore(f, "Shower")
        entrada(t, evento)
        n = escalar(t, "nX", "Int_t")
        x, ncar, dep = [valores(t, k, "Float_t") for k in ["X", "N", "dEdX"]]
        if n < 2 or any(len(v) != n for v in [x, ncar, dep]):
            raise RuntimeError("Dimensoes incompativeis com nX")
        if not all(math.isfinite(v) for v in x + ncar + dep):
            raise RuntimeError("Perfil contem valor nao finito")
        if any(x[i + 1] <= x[i] for i in range(n - 1)):
            raise RuntimeError("Profundidades nao crescentes")
    meio = [(x[i] + x[i + 1]) / 2 for i in range(n - 1)]
    integral = sum(dep[i] * (x[i + 1] - x[i]) for i in range(n - 1))
    return x, ncar, meio, dep[:n - 1], integral


def perfil(nome, evento, prefixo):
    pdf, csv_nome = saidas(prefixo, [".pdf", ".csv"])
    x, ncar, meio, dep, integral = ler_perfil(nome, evento)
    # array('d') fornece buffers numericos compativeis com TGraph, sem NumPy.
    gn = ROOT.TGraph(len(x), array("d", x), array("d", ncar))
    gd = ROOT.TGraph(len(meio), array("d", meio), array("d", dep))
    gn.SetTitle("Particulas carregadas;X [g/cm^{2}];N(X)")
    gd.SetTitle("Deposito nos intervalos;X [g/cm^{2}];dE/dX [GeV/(g/cm^{2})]")
    gn.SetLineColor(ROOT.kOrange + 7)
    gd.SetLineColor(ROOT.kBlue + 1)
    gn.SetLineWidth(2)
    gd.SetLineWidth(2)
    c = ROOT.TCanvas("cPerfilPython", "Perfil", 1000, 470)
    c.Divide(2, 1)
    c.cd(1).SetLeftMargin(.15)
    gn.Draw("AL")
    c.cd(2).SetLeftMargin(.15)
    gd.Draw("AL")
    c.Print(str(pdf))
    with csv_nome.open("w", newline="", encoding="utf-8") as out:
        w = csv.writer(out)
        w.writerow(["X_gcm2", "N_cargas", "Xmeio_gcm2", "dEdX_GeV_por_gcm2"])
        for i in range(len(x)):
            w.writerow([x[i], ncar[i], meio[i] if i < len(meio) else "",
                        dep[i] if i < len(dep) else ""])
    print("Integral nos intervalos armazenados [GeV]:", integral)
    c.Close()


def maximos(nome):
    dados, invalidos = [], 0
    with abrir(nome) as f:
        t = arvore(f, "Shower")
        folha(t, "XmxdEdX", "Float_t")
        for i in range(t.GetEntries()):
            entrada(t, i)
            x = escalar(t, "XmxdEdX", "Float_t")
            if not math.isfinite(x) or x <= 0:
                invalidos += 1
            else:
                dados.append(x)
    if not dados:
        raise RuntimeError("Nenhum maximo valido")
    return dados, invalidos


def resumo(dados, invalidos=0):
    n = len(dados)
    s = statistics.stdev(dados) if n > 1 else None
    return dict(n=n, invalidos=invalidos, media=statistics.fmean(dados),
                desvio=s, erro_media=s / math.sqrt(n) if s is not None else None)


def hist(dados, nome, cor):
    h = ROOT.TH1D(nome, ";X_{max}^{dE/dX} [g/cm^{2}];Eventos / 40 g cm^{-2}",
                  30, 0, 1200)
    h.SetDirectory(0)  # Nao deixar o fechamento de um TFile destruir o histograma.
    h.SetLineColor(cor)
    h.SetLineWidth(3)
    for x in dados:
        h.Fill(x)
    return h


def gravar_resumos(nome, grupos):
    with nome.open("w", newline="", encoding="utf-8") as out:
        w = csv.writer(out)
        w.writerow(["grupo", "n", "invalidos", "media_gcm2", "desvio_amostral_gcm2",
                    "erro_media_gcm2", "underflow", "overflow"])
        for rotulo, r, h in grupos:
            w.writerow([rotulo, r["n"], r["invalidos"], r["media"], r["desvio"],
                        r["erro_media"], h.GetBinContent(0), h.GetBinContent(31)])
            print(rotulo, r, "underflow=", h.GetBinContent(0),
                  "overflow=", h.GetBinContent(31))


def histograma(nome, prefixo):
    pdf, csv_nome = saidas(prefixo, [".pdf", ".csv"])
    dados, invalidos = maximos(nome)
    if len(dados) < 2:
        raise RuntimeError("Histograma com resumo exige pelo menos dois eventos validos")
    h = hist(dados, "hPython", ROOT.kBlue + 1)
    ROOT.gStyle.SetOptStat(0)
    c = ROOT.TCanvas("cHistPython", "Maximos", 850, 530)
    c.SetLeftMargin(.13)
    h.Draw("HIST")
    c.Print(str(pdf))
    gravar_resumos(csv_nome, [("amostra", resumo(dados, invalidos), h)])
    c.Close()


def comparar(nome_a, nome_b, prefixo):
    pdf, csv_nome = saidas(prefixo, [".pdf", ".csv"])
    grupos = []
    for nome, rotulo, cor in [(nome_a, "proton", ROOT.kOrange + 7),
                              (nome_b, "ferro", ROOT.kBlue + 1)]:
        dados, invalidos = maximos(nome)
        if len(dados) < 2:
            raise RuntimeError("Comparacao exige pelo menos dois eventos validos por grupo")
        grupos.append((rotulo, resumo(dados, invalidos), hist(dados, "h_" + rotulo, cor)))
    ROOT.gStyle.SetOptStat(0)
    c = ROOT.TCanvas("cCompararPython", "Comparacao", 850, 530)
    c.SetLeftMargin(.13)
    a, b = grupos[0][2], grupos[1][2]
    a.SetMaximum(1.3 * max(a.GetMaximum(), b.GetMaximum()))
    a.Draw("HIST")
    b.Draw("HIST SAME")
    leg = ROOT.TLegend(.67, .72, .90, .89)
    leg.SetBorderSize(0)
    leg.AddEntry(a, "Proton", "l")
    leg.AddEntry(b, "Ferro", "l")
    leg.Draw()
    c.Print(str(pdf))
    gravar_resumos(csv_nome, grupos)
    c.Close()


def ler_interacoes(nome, evento=0):
    """Dois contadores: nInt para interacoes, nPart para particulas."""
    with abrir(nome) as f:
        t = arvore(f, "LeadingInteractions")
        entrada(t, evento)
        ni = escalar(t, "nInt", "Int_t")
        npart = escalar(t, "nPart", "Int_t")
        ints = {k: valores(t, k, tipo) for k, tipo in [
            ("pId", "Int_t"), ("pEnergy", "Double_t"), ("mult", "Int_t"),
            ("matg", "Int_t"), ("kinel", "Double_t"), ("depth", "Double_t"),
            ("height", "Double_t")]}
        parts = {k: valores(t, k, tipo) for k, tipo in [
            ("Energy", "Double_t"), ("Type", "Int_t"), ("idInt", "Int_t")]}
        for k in ["px", "py", "pz"]:
            if t.GetBranch(k):
                parts[k] = valores(t, k, "Double_t")
        if any(len(v) != ni for v in ints.values()) or any(len(v) != npart for v in parts.values()):
            raise RuntimeError("Dimensoes invalidas em LeadingInteractions")
        if any(not 1 <= i <= ni for i in parts["idInt"]):
            raise RuntimeError("idInt fora da convencao 1..nInt desta distribuicao")
    return ints, parts


def interacoes(nome, evento, prefixo):
    (csv_nome,) = saidas(prefixo, [".csv"])
    ints, parts = ler_interacoes(nome, evento)
    contagem = Counter(parts["idInt"])
    with csv_nome.open("w", newline="", encoding="utf-8") as out:
        w = csv.writer(out)
        w.writerow(["idInt", "pId", "pEnergy_GeV", "mult", "particulas_na_lista",
                    "kinel", "matg", "depth_gcm2", "height_m"])
        for i in range(len(ints["mult"])):
            linha = [i + 1, ints["pId"][i], ints["pEnergy"][i], ints["mult"][i],
                     contagem[i + 1], ints["kinel"][i], ints["matg"][i],
                     ints["depth"][i], ints["height"][i]]
            w.writerow(linha)
            print(linha)
    print("nInt =", len(ints["mult"]), "nPart =", len(parts["Energy"]))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    sub = p.add_subparsers(dest="acao", required=True)
    sub.add_parser("inspecionar").add_argument("arquivo")
    for acao in ["perfil", "interacoes"]:
        q = sub.add_parser(acao)
        q.add_argument("arquivo")
        q.add_argument("--evento", type=int, default=0)
        q.add_argument("--saida", default=acao)
    q = sub.add_parser("histograma")
    q.add_argument("arquivo")
    q.add_argument("--saida", default="histograma")
    q = sub.add_parser("comparar")
    q.add_argument("proton")
    q.add_argument("ferro")
    q.add_argument("--saida", default="comparacao")
    args = p.parse_args()
    if args.acao == "inspecionar":
        inspecionar(args.arquivo)
    elif args.acao in ["perfil", "interacoes"]:
        globals()[args.acao](args.arquivo, args.evento, args.saida)
    elif args.acao == "histograma":
        histograma(args.arquivo, args.saida)
    else:
        comparar(args.proton, args.ferro, args.saida)


if __name__ == "__main__":
    main()
