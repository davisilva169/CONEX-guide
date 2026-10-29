// Direct sample summaries and identical histogram bins; no Gaussian fit.
// Sample standard deviation uses n-1. Rejected values are counted explicitly.
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
void fill_summary(const char* file, TH1D& h, std::ofstream& csv,
               const char* label) {
  TFile f(file,"READ");
  auto t=f.Get<TTree>("Shower");
  if(f.IsZombie() || !t || !t->GetBranch("XmxdEdX"))
    throw std::runtime_error("File does not contain Shower/XmxdEdX");
  TTreeReader r(t);
  TTreeReaderValue<Float_t> x(r,"XmxdEdX");
  Long64_t n=0,rejected=0; double mean=0,m2=0;
  while(r.Next()) {
    if(!std::isfinite(*x) || *x<=0) {++rejected;continue;}
    h.Fill(*x); ++n;
    double d=*x-mean; mean+=d/n; m2+=d*(*x-mean);
  }
  if(r.GetEntryStatus()!=TTreeReader::kEntryBeyondEnd)
    throw std::runtime_error("Reading stopped because of an error");
  if(n<2) throw std::runtime_error("At least 2 valid events are required");
  double s=std::sqrt(m2/(n-1));
  csv<<label<<","<<n<<","<<rejected<<","<<mean<<","<<s<<","<<s/std::sqrt(n)
     <<","<<h.GetBinContent(0)<<","<<h.GetBinContent(h.GetNbinsX()+1)<<"\n";
  std::cout<<label<<": n="<<n<<" mean="<<mean<<" s="<<s
           <<" sem="<<s/std::sqrt(n)<<" g/cm2\n";
}
void compare(const char* fileA, const char* fileB,
              const char* prefix="comparison") {
  if(std::ifstream(std::string(prefix)+".csv") || std::ifstream(std::string(prefix)+".pdf"))
    throw std::runtime_error("Output already exists");
  TH1D a("xA",";X_{max}^{dE/dX} [g/cm^{2}];Events / 40 g cm^{-2}",30,0,1200);
  TH1D b("xB","",30,0,1200);
  a.SetDirectory(nullptr); b.SetDirectory(nullptr);
  std::ofstream csv(std::string(prefix)+".csv");
  if(!csv) throw std::runtime_error("Cannot create summary");
  csv<<std::setprecision(17);
  csv<<"group,n,rejected,mean_gcm2,sample_std_gcm2,sem_gcm2,underflow,overflow\n";
  fill_summary(fileA,a,csv,"proton"); fill_summary(fileB,b,csv,"iron");
  gStyle->SetOptStat(0);
  TCanvas c("cComparar","Teaching sample",850,530);
  c.SetLeftMargin(.13);
  a.SetLineColor(kOrange+7); b.SetLineColor(kBlue+1);
  a.SetLineWidth(3); b.SetLineWidth(3);
  a.SetMaximum(1.3*std::max(a.GetMaximum(),b.GetMaximum()));
  a.Draw("HIST"); b.Draw("HIST SAME");
  TLegend leg(.67,.72,.90,.89); leg.SetBorderSize(0);
  leg.AddEntry(&a,"Proton","l");leg.AddEntry(&b,"Iron","l");leg.Draw();
  c.Print((std::string(prefix)+".pdf").c_str());
}
