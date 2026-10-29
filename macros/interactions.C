// nInt counts interactions; nPart counts all stored secondaries.
// Obtain leaves from branches: mult contains Mult in this distribution.
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TBranch.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <stdexcept>
void interactions(const char* file, int event=0, const char* prefix="interactions") {
  TFile f(file,"READ");
  auto t=f.Get<TTree>("LeadingInteractions");
  if(f.IsZombie() || !t || event<0 || event>=t->GetEntries())
    throw std::runtime_error("Missing tree or invalid entry: check -K");
  if(t->GetEntry(event)<=0) throw std::runtime_error("Read failure");
  auto leaf=[&](const char* name, const char* dtype)->TLeaf* {
    auto b=t->GetBranch(name);
    if(!b || b->GetListOfLeaves()->GetEntries()!=1)
      throw std::runtime_error(std::string("Missing branch/composta: ")+name);
    auto l=static_cast<TLeaf*>(b->GetListOfLeaves()->At(0));
    if(std::string(l->GetTypeName())!=dtype)
      throw std::runtime_error(std::string("Unexpected type: ")+name);
    return l;
  };
  int ni=leaf("nInt","Int_t")->GetValue(), np=leaf("nPart","Int_t")->GetValue();
  auto mult=leaf("mult","Int_t"), id=leaf("idInt","Int_t");
  auto pid=leaf("pId","Int_t"), mat=leaf("matg","Int_t");
  auto pe=leaf("pEnergy","Double_t"), ki=leaf("kinel","Double_t");
  auto de=leaf("depth","Double_t"), he=leaf("height","Double_t");
  for(auto l:{mult,pid,mat,pe,ki,de,he})
    if(l->GetLen()!=ni) throw std::runtime_error("Length does not match nInt");
  for(const char* name:{"Energy","Type","idInt"}) {
    auto b=t->GetBranch(name);
    if(!b || static_cast<TLeaf*>(b->GetListOfLeaves()->At(0))->GetLen()!=np)
      throw std::runtime_error("Length does not match nPart");
  }
  std::map<int,int> counts;
  for(int j=0;j<np;++j) {
    int k=id->GetValue(j);
    if(k<1 || k>ni) throw std::runtime_error("idInt is outside 1..nInt");
    ++counts[k];
  }
  const std::string output=std::string(prefix)+".csv";
  if(std::ifstream(output)) throw std::runtime_error("Output already exists");
  std::ofstream csv(output);
  if(!csv) throw std::runtime_error("Cannot create CSV");
  csv<<std::setprecision(17);
  csv<<"idInt,pId,pEnergy_GeV,mult,stored_particles,kinel,matg,depth_gcm2,height_m\n";
  for(int i=0;i<ni;++i) {
    csv<<i+1<<","<<pid->GetValue(i)<<","<<pe->GetValue(i)<<","<<mult->GetValue(i)
       <<","<<counts[i+1]<<","<<ki->GetValue(i)<<","<<mat->GetValue(i)
       <<","<<de->GetValue(i)<<","<<he->GetValue(i)<<"\n";
    std::cout<<"interaction "<<i+1<<" mult="<<mult->GetValue(i)
             <<" stored_particles="<<counts[i+1]<<"\n";
  }
  std::cout<<"nInt="<<ni<<" nPart="<<np<<"\n";
}
