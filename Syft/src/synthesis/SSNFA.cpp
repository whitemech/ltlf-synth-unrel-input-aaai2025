#include "SSNFA.h"
#include <string>
#include <memory>

using std::move;
using std::shared_ptr;
using std::string;
using std::vector;

SSNFA::SSNFA() : DFA() { }

SSNFA::SSNFA(shared_ptr<Cudd> m) : DFA(m) { }

SSNFA::~SSNFA() {}

void SSNFA::initialize_mona(string filename, string partfile){
        read_from_file(filename);
        #ifdef BUILD_DEBUG
            std::cout << "Printing the initial bitvector" << std::endl;
            for(int i = 0; i < initbv.size(); ++i) {
                std::cout << initbv[i] << " " << std::endl;
            }
            std::cout << "pinted" << std::endl;
        #endif // BUILD_DEBUG
	    init = 1;
        nbits = nstates;
        construct_SSNFA_MONA();
        for(int i = 0; i <= nstates-1; i++){
            labels.resize(nstates);
            for(int j = 0; j <= nstates-1; j++){
                labels[i].resize(nstates);
                labels[i][j] = labels_mona[behaviour[i]][j];
            }
	}

        read_partfile(partfile);
        #ifdef BUILD_DEBUG
            std::cout << "Unobservable has" << unobservable.size() << endl;
            std::cout << "Output has " << output.size() << endl;
        #endif // BUILD_DEBUG
        initbv = vector<int>(nbits);
        int j = 0;

        for(int i = 0; i < nstates; i++){
            if(j < finalstates.size() && i == finalstates[j]){
                j = j + 1;
                initbv[i] = 1;
            }
            else{
                initbv[i] = 0;
            }
        }
        BDD F = mgr->bddZero();
        F = F + bddvars[init];
        finalstatesBDD = F;
}

void SSNFA::construct_SSNFA_MONA(){

    for(int i = 0; i < nvars+nstates; i++){
        BDD b = mgr->bddVar();
        bddvars.push_back(b);
    }

    labels_mona.resize(smtbdd.size());
    for(int i = 0; i < labels_mona.size(); i++){
        if(labels_mona[i].size() == 0){
            vbdd b = con_node_mona(i);
        }
    }

}

vbdd SSNFA::con_node_mona(int index){
    if(labels_mona[index].size() != 0)
        return labels_mona[index];
    vbdd b;
    b.resize(nstates);
    if(smtbdd[index][0] == -1){
        int s = smtbdd[index][1];
        for(int i = 0; i < nstates; i++){
            b[i] = mgr->bddZero();
        }
        b[s] = mgr->bddOne();
        labels_mona[index] = b;

        return b;
    }
    else{
        int rootindex = smtbdd[index][0];
        int leftindex = smtbdd[index][1];
        int rightindex = smtbdd[index][2];
        BDD root = bddvars[nstates+rootindex];

        vbdd left = con_node_mona(leftindex);

        vbdd right = con_node_mona(rightindex);
        assert(left.size() == right.size());

        for(int i = 0; i < left.size(); i++){
            BDD tmp;
            tmp = root.Ite(right[i], left[i]);
            b[i] = tmp;
        }
        labels_mona[index] = b;

        return b;
    }
}

void SSNFA::project_unobservables() {
  BDD unobservable_cube = mgr->bddOne();

  for (int v : unobservable) {
    unobservable_cube &= bddvars[v];
  }
  for (int j = 0; j < nstates; ++j) {
    for (int i = 0; i < nstates; ++i) {
      labels[i][j] = labels[i][j].ExistAbstract(unobservable_cube);
    }
  }
}
