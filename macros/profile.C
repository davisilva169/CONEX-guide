// Single-event profile. Dynamic TTreeReader arrays avoid fixed-size buffers.
// There are nX planes but only nX-1 intervals for energy-deposit integration.
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
void profile(const char* file, Long64_t event=0,
            const char* prefix="profile") {
  if(std::ifstream(std::string(prefix)+".csv") || std::ifstream(std::string(prefix)+".pdf"))
    throw std::runtime_error("Output already exists");
  TFile f(file,"READ");
  auto t=f.Get<TTree>("Shower");
  if(f.IsZombie() || !t || event<0 || event>=t->GetEntries())
    throw std::runtime_error("Invalid file, tree or entry index");
  for(auto name : {"nX","X","N","dEdX"})
    if(!t->GetBranch(name)) throw std::runtime_error("Missing branch");
  TTreeReader r(t);
  TTreeReaderValue<Int_t> n(r,"nX");
  TTreeReaderArray<Float_t> x(r,"X"), ncar(r,"N"), dep(r,"dEdX");
  if(r.SetEntry(event)!=TTreeReader::kEntryValid ||
     n.GetSetupStatus()<0 || x.GetSetupStatus()<0 ||
     ncar.GetSetupStatus()<0 || dep.GetSetupStatus()<0)
    throw std::runtime_error("Incompatible types or entry");
  if(*n<2 || x.GetSize()!=size_t(*n) || ncar.GetSize()!=x.GetSize() ||
     dep.GetSize()!=x.GetSize()) throw std::runtime_error("Invalid dimensions");
  std::vector<double> xx, yy, mid, dd;
  std::ofstream csv(std::string(prefix)+".csv");
  if(!csv) throw std::runtime_error("Cannot create CSV");
  csv << std::setprecision(17);
  csv << "X_gcm2,N_charged,Xmid_gcm2,dEdX_GeV_per_gcm2\n";
  double integral=0;
  for(int i=0;i<*n;++i) {
    if(!std::isfinite(x[i]) || !std::isfinite(ncar[i]))
      throw std::runtime_error("Non-finite profile");
    xx.push_back(x[i]); yy.push_back(ncar[i]);
    csv<<x[i]<<","<<ncar[i]<<",";
    if(i+1<*n) {
      const double xi=x[i], next_x=x[i+1];
      double dx=next_x-xi;
      if(dx<=0 || !std::isfinite(dep[i]))
        throw std::runtime_error("Invalid depths or energy deposit");
      mid.push_back((xi+next_x)/2.); dd.push_back(dep[i]);
      integral+=dx*dep[i];
      csv<<mid.back()<<","<<dep[i];
    } else csv<<",";
    csv<<"\n";
  }
  gStyle->SetOptStat(0);
  TCanvas c("cProfile","Single-event profile",1000,470);
  c.Divide(2,1);
  TGraph gn(xx.size(),xx.data(),yy.data());
  TGraph gd(mid.size(),mid.data(),dd.data());
  gn.SetTitle("Charged particles;X [g/cm^{2}];N(X)");
  gd.SetTitle("Deposit in the intervals;X [g/cm^{2}];dE/dX [GeV/(g/cm^{2})]");
  gn.SetLineColor(kOrange+7); gn.SetLineWidth(2);
  gd.SetLineColor(kBlue+1); gd.SetLineWidth(2);
  c.cd(1); gPad->SetLeftMargin(.15); gn.Draw("AL");
  c.cd(2); gPad->SetLeftMargin(.15); gd.Draw("AL");
  c.Print((std::string(prefix)+".pdf").c_str());
  std::cout<<"Integral over stored intervals [GeV]: "<<integral<<"\n";
}
