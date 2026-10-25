// Usa a mesma selecao, os mesmos bins e o resumo da macro comparar.C.
#include "comparar.C"
void histograma(const char* arquivo, const char* prefixo="histograma") {
  const std::string base(prefixo);
  if(std::ifstream(base+".csv") || std::ifstream(base+".pdf"))
    throw std::runtime_error("Saida ja existe");
  TH1D h("hUnico",";X_{max}^{dE/dX} [g/cm^{2}];Eventos / 40 g cm^{-2}",30,0,1200);
  h.SetDirectory(nullptr);
  std::ofstream csv(base+".csv");
  if(!csv) throw std::runtime_error("Falha ao criar resumo");
  csv<<std::setprecision(17);
  csv<<"grupo,n,invalidos,media_gcm2,desvio_amostral_gcm2,erro_media_gcm2,underflow,overflow\n";
  preencher(arquivo,h,csv,"amostra"); // Exige n>=2 para calcular desvio amostral.
  gStyle->SetOptStat(0);
  TCanvas c("cUnico","Maximos",850,530);
  c.SetLeftMargin(.13);
  h.SetLineColor(kBlue+1);h.SetLineWidth(3);h.Draw("HIST");
  c.Print((base+".pdf").c_str());
}
