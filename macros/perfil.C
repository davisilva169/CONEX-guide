// Um evento; arrays dinamicos com TTreeReaderArray evitam buffers fixos.
#include <TFile.h>
#include <TTree.h>
#include <TTreeReader.h>
#include <TTreeReaderArray.h>
#include <TTreeReaderValue.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TStyle.h>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <stdexcept>
void perfil(const char* arquivo, Long64_t evento=0,
            const char* prefixo="perfil") {
  if(std::ifstream(std::string(prefixo)+".csv") || std::ifstream(std::string(prefixo)+".pdf"))
    throw std::runtime_error("Saida ja existe");
  TFile f(arquivo,"READ");
  auto t=f.Get<TTree>("Shower");
  if(f.IsZombie() || !t || evento<0 || evento>=t->GetEntries())
    throw std::runtime_error("Arquivo, arvore ou indice invalido");
  for(auto nome : {"nX","X","N","dEdX"})
    if(!t->GetBranch(nome)) throw std::runtime_error("Branch ausente");
  TTreeReader r(t);
  TTreeReaderValue<Int_t> n(r,"nX");
  TTreeReaderArray<Float_t> x(r,"X"), ncar(r,"N"), dep(r,"dEdX");
  if(r.SetEntry(evento)!=TTreeReader::kEntryValid ||
     n.GetSetupStatus()<0 || x.GetSetupStatus()<0 ||
     ncar.GetSetupStatus()<0 || dep.GetSetupStatus()<0)
    throw std::runtime_error("Tipos ou entrada incompativeis");
  if(*n<2 || x.GetSize()!=size_t(*n) || ncar.GetSize()!=x.GetSize() ||
     dep.GetSize()!=x.GetSize()) throw std::runtime_error("Dimensoes invalidas");
  std::vector<double> xx, yy, meio, dd;
  std::ofstream csv(std::string(prefixo)+".csv");
  if(!csv) throw std::runtime_error("Falha ao criar CSV");
  csv << std::setprecision(17);
  csv << "X_gcm2,N_cargas,Xmeio_gcm2,dEdX_GeV_por_gcm2\n";
  double integral=0;
  for(int i=0;i<*n;++i) {
    if(!std::isfinite(x[i]) || !std::isfinite(ncar[i]))
      throw std::runtime_error("Perfil nao finito");
    xx.push_back(x[i]); yy.push_back(ncar[i]);
    csv<<x[i]<<","<<ncar[i]<<",";
    // Ha nX planos e apenas nX-1 intervalos entre planos.
    if(i+1<*n) {
      const double xi=x[i], proximo=x[i+1];
      double dx=proximo-xi;
      if(dx<=0 || !std::isfinite(dep[i]))
        throw std::runtime_error("Profundidades ou deposito invalidos");
      meio.push_back((xi+proximo)/2.); dd.push_back(dep[i]);
      integral+=dx*dep[i];
      csv<<meio.back()<<","<<dep[i];
    } else csv<<",";
    csv<<"\n";
  }
  gStyle->SetOptStat(0);
  TCanvas c("cPerfil","Perfil de um evento",1000,470);
  c.Divide(2,1);
  TGraph gn(xx.size(),xx.data(),yy.data());
  TGraph gd(meio.size(),meio.data(),dd.data());
  gn.SetTitle("Particulas carregadas;X [g/cm^{2}];N(X)");
  gd.SetTitle("Deposito nos intervalos;X [g/cm^{2}];dE/dX [GeV/(g/cm^{2})]");
  gn.SetLineColor(kOrange+7); gn.SetLineWidth(2);
  gd.SetLineColor(kBlue+1); gd.SetLineWidth(2);
  c.cd(1); gPad->SetLeftMargin(.15); gn.Draw("AL");
  c.cd(2); gPad->SetLeftMargin(.15); gd.Draw("AL");
  c.Print((std::string(prefixo)+".pdf").c_str());
  std::cout<<"Integral nos intervalos armazenados [GeV]: "<<integral<<"\n";
}
