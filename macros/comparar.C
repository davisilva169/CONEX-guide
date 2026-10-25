// Histogramas e resumo: sem ajuste e sem hipotese de normalidade.
#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderValue.h>
#include <TH1D.h>
#include <TCanvas.h>
#include <TLegend.h>
#include <TStyle.h>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
void preencher(const char* arquivo, TH1D& h, std::ofstream& csv,
               const char* rotulo) {
  TFile f(arquivo,"READ");
  auto t=f.Get<TTree>("Shower");
  if(f.IsZombie() || !t || !t->GetBranch("XmxdEdX"))
    throw std::runtime_error("Arquivo sem Shower/XmxdEdX");
  TTreeReader r(t);
  TTreeReaderValue<Float_t> x(r,"XmxdEdX");
  Long64_t n=0,invalidos=0; double media=0,m2=0;
  while(r.Next()) {
    if(!std::isfinite(*x) || *x<=0) {++invalidos;continue;}
    h.Fill(*x); ++n;
    double d=*x-media; media+=d/n; m2+=d*(*x-media);
  }
  if(r.GetEntryStatus()!=TTreeReader::kEntryBeyondEnd)
    throw std::runtime_error("Leitura interrompida por erro");
  if(n<2) throw std::runtime_error("Exige pelo menos 2 eventos validos");
  double s=std::sqrt(m2/(n-1)); // Desvio amostral, denominador n-1.
  csv<<rotulo<<","<<n<<","<<invalidos<<","<<media<<","<<s<<","<<s/std::sqrt(n)
     <<","<<h.GetBinContent(0)<<","<<h.GetBinContent(h.GetNbinsX()+1)<<"\n";
  std::cout<<rotulo<<": n="<<n<<" media="<<media<<" s="<<s
           <<" erro_media="<<s/std::sqrt(n)<<" g/cm2\n";
}
void comparar(const char* arquivoA, const char* arquivoB,
              const char* prefixo="comparacao") {
  if(std::ifstream(std::string(prefixo)+".csv") || std::ifstream(std::string(prefixo)+".pdf"))
    throw std::runtime_error("Saida ja existe");
  // As classes e as bordas sao iguais nos dois conjuntos.
  TH1D a("xA",";X_{max}^{dE/dX} [g/cm^{2}];Eventos / 40 g cm^{-2}",30,0,1200);
  TH1D b("xB","",30,0,1200);
  a.SetDirectory(nullptr); b.SetDirectory(nullptr);
  std::ofstream csv(std::string(prefixo)+".csv");
  if(!csv) throw std::runtime_error("Falha ao criar resumo");
  csv<<std::setprecision(17);
  csv<<"grupo,n,invalidos,media_gcm2,desvio_amostral_gcm2,erro_media_gcm2,underflow,overflow\n";
  preencher(arquivoA,a,csv,"proton"); preencher(arquivoB,b,csv,"ferro");
  gStyle->SetOptStat(0);
  TCanvas c("cComparar","Amostra didatica",850,530);
  c.SetLeftMargin(.13);
  a.SetLineColor(kOrange+7); b.SetLineColor(kBlue+1);
  a.SetLineWidth(3); b.SetLineWidth(3);
  a.SetMaximum(1.3*std::max(a.GetMaximum(),b.GetMaximum()));
  a.Draw("HIST"); b.Draw("HIST SAME");
  TLegend leg(.67,.72,.90,.89); leg.SetBorderSize(0);
  leg.AddEntry(&a,"Proton","l");leg.AddEntry(&b,"Ferro","l");leg.Draw();
  c.Print((std::string(prefixo)+".pdf").c_str());
}
