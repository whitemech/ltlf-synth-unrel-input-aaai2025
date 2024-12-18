#include "CoRDFA_syn.h"

using std::make_unique;
using std::move;
using std::set;
using std::shared_ptr;
using std::string;
using std::unique_ptr;
using std::vector;
/**
 * This method implements the construction described in Sect 4. 
 * The synchronous product is implemented in the DFA class.
 * The complementation and projection of unreliable input is done in the SSNFA class.
 */
CoRDFA_syn::CoRDFA_syn(shared_ptr<Cudd> m, string filename, string filenonbackup, string partfile)
{
    #ifdef BUILD_DEBUG
    std::cout << "Starting cordfa syn" << std::endl;
    #endif // BUILD_DEBUG
    mgr = m;
    // Read the DFA from MONA for main formula
    bddMain = make_unique<DFA>(m);
    bddMain->initialize(filenonbackup, partfile, false);

    // Read the DFA from MONA for backup formula
    // Do the complementation and project the unobservables out
    bddBackups = make_unique<SSNFA>(m);
    bddBackups->initialize_mona(filename, partfile);
    bddBackups->project_unobservables();
    bddBackups->complement();
    // Reverse the transitions
    for (int i = 0; i < bddBackups->nstates; ++i) {
      bddBackups->res.push_back(mgr->bddZero());
      for (int j = 0; j < bddBackups->nstates; ++j) {
        BDD trans = bddBackups->bddvars[j] & bddBackups->labels[i][j];
        bddBackups->res[i] |= trans;
      }
    }
    bddBackups->dump_automaton("ssnfa");
    bdd = make_unique<DFA>(m);

    // Adjust the initial state (we need this, since otherwise the two automata are out of sync)
    // Assumes w.l.o.g. that the DFA from MONA has initial state 0 
    
    bddMain->init = 1;
    bddMain->initbv[bddMain->initbv.size()-1] = 1;
    bddMain->dump_automaton("dfa");
    bddBackups->dump_automaton("ssnfa");
    bdd->init_from_cross_product(bddMain.release(), bddBackups.release());
    bdd->dump_automaton("cross");
   // Initialize standard synthesis
    syn::initializer();
}

CoRDFA_syn::~CoRDFA_syn() {}

void CoRDFA_syn::initializer(unique_ptr<SSNFA>& ssnfa){
  for(int i = 0; i < ssnfa->nbits; i++){
    BDD b = mgr->bddVar();
    ssnfa->bddvars.push_back(b);
  }
  BDD t = ssnfa->finalstatesBDD;
  W.push_back(ssnfa->finalstatesBDD);
  Wprime.push_back(ssnfa->finalstatesBDD);
  cur = 0;
}
