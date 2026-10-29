// Read-only inspection. Branch names and types are printed before analysis.
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TBranch.h>
#include <TObjArray.h>
#include <iostream>
#include <stdexcept>
void inspect(const char* file) {
  TFile f(file,"READ");
  if(f.IsZombie()) throw std::runtime_error("Cannot open ROOT file");
  f.ls();
  for(const char* name : {"Header","Shower","LeadingInteractions"}) {
    auto t=f.Get<TTree>(name);
    if(!t) {
      if(std::string(name)=="LeadingInteractions") {
        std::cout<<"LeadingInteractions is absent: expected with -K 0\n";continue;
      }
      throw std::runtime_error(std::string("Missing tree: ")+name);
    }
    std::cout << name << ": " << t->GetEntries() << " entries\n";
    t->Print();
    if(t->GetEntries()) t->Show(0);
  }
}
