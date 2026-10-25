// Diagnostico somente de leitura. Execute na raiz do guia:
// root -l -b -q 'macros/inspecionar.C("dados/primeiro.root")'
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TBranch.h>
#include <TObjArray.h>
#include <iostream>
#include <stdexcept>
void inspecionar(const char* arquivo) {
  TFile f(arquivo,"READ");
  if(f.IsZombie()) throw std::runtime_error("Arquivo ROOT inacessivel");
  f.ls(); // ;1 e ;2 sao ciclos de chaves, nao arvores fisicas adicionais.
  for(const char* nome : {"Header","Shower","LeadingInteractions"}) {
    auto t=f.Get<TTree>(nome); // Sem ciclo: busca a chave mais recente.
    if(!t) {
      if(std::string(nome)=="LeadingInteractions") {
        std::cout<<"LeadingInteractions ausente: normal quando -K 0\n";continue;
      }
      throw std::runtime_error(std::string("Arvore ausente: ")+nome);
    }
    std::cout << nome << ": " << t->GetEntries() << " entradas\n";
    t->Print(); // Nome, leaflist, tipo e contador de dimensao variavel.
    if(t->GetEntries()) t->Show(0);
  }
}
