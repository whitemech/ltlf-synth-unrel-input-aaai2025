#include "syn.h"

#include <memory>

using std::string;
using std::unordered_map;
using std::vector;
using std::shared_ptr;
using std::unique_ptr;
using std::make_unique;
using std::move;
using std::cout;
using std::endl;

syn::syn() {}
// This is standard synthesis, as used for the MSO technique.
syn::syn(shared_ptr<Cudd> m, string filename, string partfile, bool partial_observability)
{
    //ctor
    //Cudd *p = &mgr;
    bdd = make_unique<DFA>(m);

    bdd->initialize(filename, partfile, partial_observability);

    mgr = m;
    initializer();

    //bdd->bdd2dot();

}

/**
 * This implements the belief-state construction as described in Sect. 5 (paper) / Chp. 5 thesis.
 */
syn::syn(shared_ptr<Cudd> m, string backupfile, string mainfile, string partfile, bool partial_observability)
{
    //ctor
    mgr = m;
    // First initialize for main file
    bddMain = make_unique<DFA>(m);
    bddMain->initialize(mainfile, partfile, false);
    bddMain->dump_automaton("automaton_main.txt");
    //std::cout << "Initializing synthesis for  " << backupfile << std::endl;
    // Initialize for backup , suing belief state construction
    bddBackup = make_unique<DFA>(m);
    bddBackup->initialize(backupfile, partfile, true); 
    bddBackup->dump_automaton("automaton_backup.txt");
    auto bddt = make_unique<DFA>(m);
    // Combine using synchronous product
    bddt->init_from_cross_product(bddMain.get(), bddBackup.get()); 
    bddt->dump_automaton("automaton_cross.txt");
    bdd = move(bddt); 

    //bddtwo->print_full();
    initializer();
    #ifdef BUILD_DEBUG
     bdd->print_full();
    #endif // 
}


syn::syn(shared_ptr<Cudd> m, unique_ptr<DFA> d)
{
    bdd = move(d);
    mgr = move(m);
    initializer();

   // bdd->bdd2dot();
}

syn::~syn()
{
    //dtor
}

void syn::initializer(){
  #ifdef BUILD_DEBUG  
    std::cout << bdd->bddvars.size() << " <- size at inut" << std::endl;  
    int size = bdd->bddvars.size();
  #endif
  for(int i = 0; i < bdd->nbits; i++){
    BDD b = mgr->bddVar();
    bdd->bddvars.push_back(b);
    #ifdef BUILD_DEBUG
        std::cout << "added " << b << " " << "at " << bdd->bddvars.size() << endl;
    #endif
  }
    W.push_back(bdd->finalstatesBDD);
    Wprime.push_back(bdd->finalstatesBDD);
    cur = 0;
    #ifdef BUILD_DEBUG
        std::cout << "The BDD is now " << bdd->finalstatesBDD;
    #endif
}

BDD syn::state2bdd(int s){
    string bin = state2bin(s);
    BDD b = mgr->bddOne();
    int nzero = bdd->nbits - bin.length();
    //cout<<nzero<<endl;
    for(int i = 0; i < nzero; i++){
        b *= !bdd->bddvars[i];
    }
    for(int i = 0; i < bin.length(); i++){
        if(bin[i] == '0')
            b *= !bdd->bddvars[i+nzero];
        else
            b *= bdd->bddvars[i+nzero];
    }
    return b;

}

string syn::state2bin(int n){
    string res;
    while (n)
    {
        res.push_back((n & 1) + '0');
        n >>= 1;
    }

    if (res.empty())
        res = "0";
    else
        reverse(res.begin(), res.end());
   return res;
}

bool syn::fixpoint(){
    //std::cout << "FP :" <<  W[cur] << " " << W[cur-1] << std::endl;
    return (W[cur] == W[cur-1]);
}

void syn::printBDDSat(BDD b){

  std::cout<<"sat with: ";
  int max = bdd->nstates;
  
  for (int i=0; i<max; i++){
    if (b.Eval(state2bit(i)).IsOne()){
      std::cout<<i<<", ";
    }
  }
  std::cout<<std::endl;
}

bool syn::realizablity_sys(unordered_map<unsigned int, BDD>& IFstrategy){
    int iteration = 0;
    //std::cout << "Starting iteration" << std::endl;
    while(true){
        iteration = iteration + 1;
        BDD I = mgr->bddOne();
        int index;
        //std::cout << bdd->input.size() << std::endl;
        for(int i = 0; i < bdd->input.size(); i++){
            index = bdd->input[i];
            I *= bdd->bddvars[index];
        }
        BDD tmp = W[cur] + univsyn_sys(I);
        //std::cout << "After iteration, this is bdd" << tmp << endl;
        W.push_back(tmp);
        cur++;

        BDD O = mgr->bddOne();
        for(int i = 0; i < bdd->output.size(); i++){
            index = bdd->output[i];
            O *= bdd->bddvars[index];
        }

        Wprime.push_back(existsyn_sys(O));
        //std::cout << "COMPUTED NEW STATE " << Wprime[Wprime.size() - 1] << endl;
        if(fixpoint())
            break;

    }

    // Create the vector to evaluate with 
    // have as many as state variables

    // Compute maximum input variable 
    unsigned int mv = bdd->initbv.size();
    for(int i = 0; i < bdd->initbv.size(); ++i) {
        mv = std::max(mv, bdd->bddvars[i].NodeReadIndex());
    }
    std::vector<int> initialbits(mv, 0);
    for(int i = 0; i < bdd->nbits; ++i) {
        if(bdd->initbv[i] == 1) {
            initialbits[bdd->bddvars[i].NodeReadIndex()] = 1;
        }
    }    

    if(Wprime[cur-1].Eval(initialbits.data()).IsOne()){
        BDD O = mgr->bddOne();
        for(int i = 0; i < bdd->output.size(); i++){
            O *= bdd->bddvars[bdd->output[i]];
        }
    
        InputFirstSynthesis IFsyn(*mgr);
        IFstrategy = IFsyn.synthesize(W[cur], O);
        vector<BDD> tmp = bdd->res;
        tmp.push_back(bdd->finalstatesBDD);
        cout<<"Iteration: "<<iteration<<endl;
        cout<<"BDD nodes: "<<mgr->nodeCount(tmp)<<endl;
        //std::cout<<"realizable, winning set: "<<std::endl;
        //std::cout<<Wprime[Wprime.size()-1]<<std::endl;
        return true;
    }
    //std::cout<<"unrealizable, winning set: "<<std::endl;
    //std::cout<<Wprime[Wprime.size()-1]<<std::endl;
    // assert(false);
    vector<BDD> tmp = bdd->res;
    tmp.push_back(bdd->finalstatesBDD);
    cout<<"Iteration: "<<iteration<<endl;
    cout<<"BDD nodes: "<<mgr->nodeCount(tmp)<<endl;

    return false;
}

bool syn::realizablity_env(std::unordered_map<unsigned, BDD>& IFstrategy){
    BDD transducer;
    while(true){
        int index;
        BDD O = mgr->bddOne();
        for(int i = 0; i < bdd->output.size(); i++){
            index = bdd->output[i];
            O *= bdd->bddvars[index];
        }

        BDD tmp = W[cur] + existsyn_env(O, transducer);
        W.push_back(tmp);
        cur++;  

        BDD I = mgr->bddOne();
        for(int i = 0; i < bdd->input.size(); i++){
            index = bdd->input[i];
            I *= bdd->bddvars[index];
        }

        Wprime.push_back(univsyn_env(I));
        if(fixpoint())
            break;

    }
    unsigned int mv = bdd->initbv.size();
    for(int i = 0; i < bdd->initbv.size(); ++i) {
        mv = std::max(mv, bdd->bddvars[i].NodeReadIndex());
    }
    std::vector<int> initialbits(mv, 0);
    for(int i = 0; i < bdd->nbits; ++i) {
        if(bdd->initbv[i] == 1) {
            initialbits[bdd->bddvars[i].NodeReadIndex()] = 1;
        }
    }    
    if((Wprime[cur-1].Eval(initialbits.data())).IsOne()){
        BDD O = mgr->bddOne();
        for(int i = 0; i < bdd->output.size(); i++){
            O *= bdd->bddvars[bdd->output[i]];
        }
        O *= bdd->bddvars[bdd->nbits];

        InputFirstSynthesis IFsyn(*mgr);
        IFstrategy = IFsyn.synthesize(transducer, O);
        vector<BDD> tmp = bdd->res;
        tmp.push_back(bdd->finalstatesBDD);

        cout<<"BDD nodes: "<<mgr->nodeCount(tmp)<<endl;

        return true;
    }
    vector<BDD> tmp = bdd->res;
    tmp.push_back(bdd->finalstatesBDD);

    cout<<"BDD nodes: "<<mgr->nodeCount(tmp)<<endl;
    return false;
}


void syn::strategy(vector<BDD>& S2O){
    vector<BDD> winning;
    for(int i = 0; i < S2O.size(); i++){
        //dumpdot(S2O[i], "S2O"+to_string(i));
        for(int j = 0; j < bdd->output.size(); j++){
            int index = bdd->output[j];
            S2O[i] = S2O[i].Compose(bdd->bddvars[index], mgr->bddOne());
        }
    }
}

int** syn::outindex(){
    int outlength = bdd->output.size();
    int outwidth = 2;
    int **out = 0;
    out = new int*[outlength];
    for(int l = 0; l < outlength; l++){
        out[l] = new int[outwidth];
        out[l][0] = l;
        out[l][1] = bdd->output[l];
    }
    return out;
}

int* syn::state2bit(int n){
    int* s = new int[bdd->nbits];
    for (int i=bdd->nbits-1; i>=0; i--){
      s[i] = n%2;
      n = n/2;
    }
    return s;
}


BDD syn::univsyn_sys(BDD univ){

    BDD tmp = Wprime[cur];
    int offset = bdd->nbits + bdd->nvars;
    // << "offset is " << offset << " " << bdd->bddvars.size() << " " << std::endl;
    tmp = prime(tmp);
    //std::cout << "after prime': " << tmp << " iters: " << bdd->nbits << std::endl;
    for(int i = 0; i < bdd->nbits; i++){

        //cout << "ITER " << i << endl << endl;
        //tmp = tmp.Compose(bdd->res[i], offset+i);
        //std::cout <<   bdd->bddvars[i+offset] << " " << i << std::endl;
        //std::cout << bdd->res[i] << std::endl;
        tmp = tmp.Compose(bdd->res[i], bdd->bddvars[i+offset].NodeReadIndex());
        //std::cout << "now tmp is " << tmp << endl; 
    }

    tmp *= !Wprime[cur];
    //std::cout << "before elim " << tmp << std::endl;
    BDD eliminput = tmp.UnivAbstract(univ);
    //std::cout << "after elim" << eliminput << std::endl;
    return eliminput;

}

BDD syn::existsyn_env(BDD exist, BDD& transducer){
    BDD tmp = Wprime[cur];
    int offset = bdd->nbits + bdd->nvars;

    //dumpdot(I, "W00");
    tmp = prime(tmp);
    for(int i = 0; i < bdd->nbits; i++){
        tmp = tmp.Compose(bdd->res[i], bdd->bddvars[offset+i].NodeReadIndex());
        //tmp = tmp.Compose(bdd->res[i], offset+i);
    }
    transducer = tmp;
    tmp *= !Wprime[cur];
    BDD elimoutput = tmp.ExistAbstract(exist);
    return elimoutput;
}

BDD syn::univsyn_env(BDD univ){

    BDD tmp = W[cur];
    BDD elimuniv = tmp.UnivAbstract(univ);
    return elimuniv;

}

BDD syn::prime(BDD orign){
    //std::cout << "orign is" << orign << endl;
    int offset = bdd->nbits + bdd->nvars;
    BDD tmp = orign;
    //std::cout << "*** " << bdd->input.size() << "   " << bdd->output.size() << " " << bdd->nbits << " " << std::endl;
    for(int i = 0; i < bdd->nbits; i++){
       // std::cout << bdd->bddvars[i]<< " " << bdd->bddvars[i+offset] << " " << bdd->bddvars[i].NodeReadIndex() << endl ;
        //tmp = tmp.Compose(bdd->bddvars[i+offset], bdd->bddvars[i]); // Slice bdd->bddVars[i+offset] into slot occupied by i
        tmp = tmp.Compose(bdd->bddvars[i+offset], bdd->bddvars[i].NodeReadIndex()); // 
        //tmp = tmp.Compose(bdd->bddvars[i+offset], i);
        //std::cout << tmp << endl;
    }
    //std::cout << "origin was " << orign << std::endl;
    //std::cout << "tmp is" << tmp << std::endl;
    return tmp;
}

BDD syn::existsyn_sys(BDD exist){
    //std::cout << "abstracting " << exist << endl;
    BDD tmp = W[cur];
    BDD elimoutput = tmp.ExistAbstract(exist);
    return elimoutput;

}

void syn::dumpdot(BDD &b, string filename){
    FILE *fp = fopen(filename.c_str(), "w");
    vector<BDD> single(1);
    single[0] = b;
    this->mgr->DumpDot(single, NULL, NULL, fp);
    fclose(fp);
}


