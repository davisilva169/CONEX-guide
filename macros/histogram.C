// Reuse the selection, bins and summary implementation from compare.C.
#include "compare.C"
void histogram(const char* file, const char* prefix="histogram") {
  const std::string base(prefix);
  if(std::ifstream(base+".csv") || std::ifstream(base+".pdf"))
    throw std::runtime_error("Output already exists");
  TH1D h("hUnico",";X_{max}^{dE/dX} [g/cm^{2}];Events / 40 g cm^{-2}",30,0,1200);
  h.SetDirectory(nullptr);
  std::ofstream csv(base+".csv");
  if(!csv) throw std::runtime_error("Cannot create summary");
  csv<<std::setprecision(17);
  csv<<"group,n,rejected,mean_gcm2,sample_std_gcm2,sem_gcm2,underflow,overflow\n";
  fill_summary(file,h,csv,"sample");
  gStyle->SetOptStat(0);
  TCanvas c("cUnico","Maxima",850,530);
  c.SetLeftMargin(.13);
  h.SetLineColor(kBlue+1);h.SetLineWidth(3);h.Draw("HIST");
  c.Print((base+".pdf").c_str());
}
