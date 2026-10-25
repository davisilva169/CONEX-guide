// Leitura de LeadingInteractions. Branch mult possui folha Mult nesta copia.
// A lista global usa nPart; os vetores por interacao usam nInt.
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TBranch.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
void interacoes(const char* arquivo, int evento=0, const char* prefixo="interacoes") {
  TFile f(arquivo,"READ");
  auto t=f.Get<TTree>("LeadingInteractions");
  if(f.IsZombie() || !t || evento<0 || evento>=t->GetEntries())
    throw std::runtime_error("Arvore ausente ou entrada invalida: ver -K");
  if(t->GetEntry(evento)<=0) throw std::runtime_error("Falha de leitura");
  auto folha=[&](const char* nome, const char* tipo)->TLeaf* {
    auto b=t->GetBranch(nome);
    if(!b || b->GetListOfLeaves()->GetEntries()!=1)
      throw std::runtime_error(std::string("Branch ausente/composta: ")+nome);
    auto l=static_cast<TLeaf*>(b->GetListOfLeaves()->At(0));
    if(std::string(l->GetTypeName())!=tipo)
      throw std::runtime_error(std::string("Tipo inesperado: ")+nome);
    return l;
  };
  int ni=folha("nInt","Int_t")->GetValue(), np=folha("nPart","Int_t")->GetValue();
  auto mult=folha("mult","Int_t"), id=folha("idInt","Int_t");
  auto pid=folha("pId","Int_t"), mat=folha("matg","Int_t");
  auto pe=folha("pEnergy","Double_t"), ki=folha("kinel","Double_t");
  auto de=folha("depth","Double_t"), he=folha("height","Double_t");
  for(auto l:{mult,pid,mat,pe,ki,de,he})
    if(l->GetLen()!=ni) throw std::runtime_error("Dimensao diferente de nInt");
  for(const char* nome:{"Energy","Type","idInt"}) {
    auto b=t->GetBranch(nome);
    if(!b || static_cast<TLeaf*>(b->GetListOfLeaves()->At(0))->GetLen()!=np)
      throw std::runtime_error("Dimensao diferente de nPart");
  }
  std::map<int,int> contagem;
  for(int j=0;j<np;++j) {
    int k=id->GetValue(j);
    if(k<1 || k>ni) throw std::runtime_error("idInt fora de 1..nInt");
    ++contagem[k];
  }
  const std::string saida=std::string(prefixo)+".csv";
  if(std::ifstream(saida)) throw std::runtime_error("Saida ja existe");
  std::ofstream csv(saida);
  if(!csv) throw std::runtime_error("Falha ao criar CSV");
  csv<<std::setprecision(17);
  csv<<"idInt,pId,pEnergy_GeV,mult,particulas_na_lista,kinel,matg,depth_gcm2,height_m\n";
  for(int i=0;i<ni;++i) {
    csv<<i+1<<","<<pid->GetValue(i)<<","<<pe->GetValue(i)<<","<<mult->GetValue(i)
       <<","<<contagem[i+1]<<","<<ki->GetValue(i)<<","<<mat->GetValue(i)
       <<","<<de->GetValue(i)<<","<<he->GetValue(i)<<"\n";
    std::cout<<"interacao "<<i+1<<" mult="<<mult->GetValue(i)
             <<" particulas_na_lista="<<contagem[i+1]<<"\n";
  }
  std::cout<<"nInt="<<ni<<" nPart="<<np<<"\n";
}
