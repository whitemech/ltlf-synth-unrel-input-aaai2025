#include "DFA.h"
#include <cmath>
using std::string;
using std::ifstream;
using std::vector;
using std::set;
using boost::algorithm::is_any_of;
using boost::algorithm::split;
using std::to_string;
using std::cout;
using std::endl;
using std::stoi;
using std::shared_ptr;
using std::make_shared;
using std::move;

//update test

DFA::DFA(shared_ptr<Cudd> m){
    //mgr = move(m);
    //ctor
    mgr = m;
}

DFA::DFA(){
    mgr = make_shared<Cudd>();
}

DFA::~DFA()
{
    //dtor
    //delete mgr;// = NULL;
}
void DFA::initialize(string filename, string partfile, bool partial_observability){
    //ctor
    read_from_file(filename);
    cout<<"The number of explicit states: "<<nstates<<endl;

    if (partial_observability) {
      nbits = nstates;

      read_partfile(partfile);
      construct_bdd_belief_state();

      initbv = vector<int>(nstates, 0);
      initbv[init] = 1;
    }
    else {
      nbits = state2bin(nstates-1).length();

      //get_bdd();
      //print_vec(bdd);
      //construct_bdd();
      construct_bdd_new();

      read_partfile(partfile);

      initbv = vector<int>(nbits);
      int temp = init;
      for (int i=nbits-1; i>=0; i--){
	initbv[i] = temp%2;
	temp = temp/2;
      }
    }

    cout<<"The number of state variables: "<<nbits<<endl;
}

void DFA::read_partfile(string partfile){
    ifstream f(partfile.c_str());
    vector<string> inputs;
    vector<string> outputs;
    vector<string> unobservables;
    string line;
    while(getline(f, line)){
        if(f.is_open()){
            if(strfind(line, "inputs")){
                split(inputs, line, is_any_of(" "));
                //print(inputs);
            }
            else if(strfind(line, "outputs")){
                split(outputs, line, is_any_of(" "));
                //print(outputs);
            }
	    else if(strfind(line, "unobservables")){
	        split(unobservables, line, is_any_of(" "));
	    }
            else{
                cout<<"read partfile error!"<<endl;
                cout<<partfile<<endl;
                cout<<line<<endl;
            }
	}
    }
    f.close();
    set<string> input_set;
    set<string> output_set;
    set<string> unobservable_set;

    for(int i = 1; i < inputs.size(); i++){
        string c = boost::algorithm::to_upper_copy(inputs[i]);
        input_set.insert(c);
    }
    for(int i = 1; i < outputs.size(); i++){
        string c = boost::algorithm::to_upper_copy(outputs[i]);
        output_set.insert(c);
    }
    for(int i = 1; i < unobservables.size(); i++){
        string c = boost::algorithm::to_upper_copy(unobservables[i]);
        unobservable_set.insert(c);
    }

    for(int i = 1; i < variables.size(); i++){
        if(input_set.find(variables[i]) != input_set.end())
        {
            if(unobservable_set.find(variables[i]) != unobservable_set.end())
	            unobservable.push_back(nbits+i-1);
            input.push_back(nbits+i-1);
        }        
        else if(output_set.find(variables[i]) != output_set.end())
            output.push_back(nbits+i-1);
	    else if(unobservable_set.find(variables[i]) != unobservable_set.end())
	        unobservable.push_back(nbits+i-1);
        else if(variables[i] == "ALIVE")
            output.push_back(nbits+i-1);
        else
            cout<<"error: "<<variables[i]<<endl;
    }
    //print_int(input);
    //print_int(output);

}

void DFA::init_from_DFA(DFA* d1) {
    nstates = d1->nstates;
    nbits = d1->nbits;
    nvars = d1->nvars;
    initbv = d1->initbv; 
    input = d1->input;
    output = d1->output; 
    bddvars = d1->bddvars;
    
    finalstates = d1->finalstates;
    finalstatesBDD = d1->finalstatesBDD;
    res = d1->res; 


}  

/**
 * This function constructs the synchronous product (in code called cross product) of two DFAs.
 * It implements Definition 7 from the paper / Definition 4.1 of thesis. 
 */
void DFA::init_from_cross_product(DFA* d1, DFA* d2) {
    // Idea: Just use d1->input, output and map the other variables 
    nstates = d1->nstates * d2->nstates; 
    nbits   = d1->nbits + d2->nbits;
    nvars   = d1->nvars;
    initbv = d1->initbv;
    initbv.insert(initbv.end(), d2->initbv.begin(), d2->initbv.end());

    input = d1->input;
    for(int i = 0; i < input.size(); ++i)
        input[i] += d2->nbits;
    output = d1->output;
    for(int i = 0; i < output.size(); ++i)
        output[i] += d2->nbits;
    

    for(int i = 0; i < d1->nbits; ++i) {
        bddvars.push_back(d1->bddvars[i]);
        #ifdef BUILD_DEBUG
        std::cout << "d1 bvar " << d1->bddvars[i] << endl;
        #endif // BUILD_DEBUG
    }
    for(int i = 0; i < d2->nbits; ++i) {
        bddvars.push_back(d2->bddvars[i]);
        #ifdef BUILD_DEBUG
        std::cout << "d2 bvar " << d2->bddvars[i] << endl;
        #endif
    }
    for(int i = d1->nbits; i < d1->bddvars.size(); ++i)
    {
        bddvars.push_back(d1->bddvars[i]);
    }
    // 
    vbdd swap, insert;
    // Compute swap 
    for(int j = 0; j < d2->output.size(); ++j) {
        swap.push_back(d1->bddvars[d1->output[j]]);
        insert.push_back(d2->bddvars[d2->output[j]]);
        #ifdef BUILD_DEBUG
        std::cout << "Output var (d1)" << d1->output[j] << "leads to swap" << d1->bddvars[d1->output[j]] << " " << d2->bddvars[d2->output[j]] << endl;
        #endif
    }
    for(int i = 0; i < d1->input.size(); ++i) {
        swap.push_back(d1->bddvars[d1->input[i]]);
        insert.push_back(d2->bddvars[d2->input[i]]);
        #ifdef BUILD_DEBUG
        std::cout << "Input var (d1)" << d1->input[i] << "leads to swap" << d1->bddvars[d1->input[i]] << " " << d2->bddvars[d2->input[i]] << endl;
        #endif
    }
    #ifdef BUILD_DEBUG
        std::cout << swap.size() << " " << insert.size() << " <<" << endl;
        assert(d2->input.size() == d1->input.size());   
    #endif 
    // Create res 
    res = d1->res;
    for(int i = 0; i < d2->res.size(); ++i) {
        auto tmp = d2->res[i];        
        res.push_back(d2->res[i].SwapVariables(insert, swap));
    }
    finalstatesBDD = d1->finalstatesBDD & (d2->finalstatesBDD.SwapVariables(insert, swap));
    #ifdef BUILD_DEBUG
    // Create function 
    std::cout << "Printing the completed autoamton" << std::endl;
    print_int(input);
    print_int(output);
    print_int(unobservable);
      std::cout << "Printing bdd vars" << std::endl;
    for(int i = 0; i < bddvars.size(); ++i)
                std::cout << bddvars[i] << std::endl;
        std::cout << "Printing input vars" << std::endl;
    for(int i = 0; i < input.size(); ++i)
                std::cout << bddvars[input[i]] << std::endl;
    std::cout << "Printing output vars" << std::endl;
    for(int i = 0; i < output.size(); ++i)
                std::cout << bddvars[output[i]]  << std::endl;
    std::cout << "State variables" << std::endl;
    for(int i = 0; i < nbits; ++i)
        std::cout << bddvars[i] << std::endl;
    std::cout << "Initial bit vector " << std::endl;
    print_int(initbv);
    std::cout << "Printing transition functions " << std::endl;
    for(int i = 0; i < nbits; ++i)
        std::cout << res[i] << std::endl;
    std::cout << "Final states" << std::endl;
    std::cout << finalstatesBDD << std::endl;
    #endif
}


void DFA::read_from_file(string filename){
ifstream f(filename.c_str());
	if(f.is_open()){
		bool flag = 0;
		string line;
		item tmp; //item: vector<int>
        vector <string> fields; //temporary varibale

		while(getline(f, line)){
            if(flag == 0){
                if(strfind(line, "number of variables")){
                    split(fields, line, is_any_of(" "));
                    nvars = stoi(fields[3]);
                    //cout<<nvars<<endl;
                }
                if(strfind(line, "variables") && !strfind(line, "number")){
                    split(variables, line, is_any_of(" "));

                }
                else if(strfind(line, "states")){
                    split(fields, line, is_any_of(" "));
                    nstates = stoi(fields[1]);
                   // cout<<nstates<<endl;
                }
                else if(strfind(line, "initial")){
                    split(fields, line, is_any_of(" "));
                    init = stoi(fields[1]);
                    //cout<<init<<endl;
                }
                else if(strfind(line, "bdd nodes")){
                    split(fields, line, is_any_of(" "));
                    nodes = stoi(fields[2]);
                    //cout<<nodes<<endl;
                }
                else if(strfind(line, "final")){
                    split(fields, line, is_any_of(" "));
                    int i = 1; // start at 1 to ignore "final" token
                    while(i < fields.size()){
		      //if(fields[i] == "1")
		        if(fields[i] != "-1")
                            finalstates.push_back(i-1);
			else
			    nonfinalstates.push_back(i-1);
                        i = i + 1;
                    }
                    //print_int(finalstates);
                }
                else if(strfind(line, "behaviour")){
                    split(fields, line, is_any_of(" "));
                    int i = 1;
                    while(i < fields.size()){
                        behaviour.push_back(stoi(fields[i]));
                        i = i + 1;
                    }
                    //print_int(behaviour);
                }
                else if(strfind(line, "bdd:"))
                    flag = 1;
                else
                    continue;
            }
            else{
                if(strfind(line, "end"))
                    break;
                split(fields, line, is_any_of(" "));
                for(int i = 1; i < fields.size(); i++)
                    tmp.push_back(stoi(fields[i]));
                smtbdd.push_back(tmp);
                tmp.clear();
            }
		}

	}
	f.close();
    //print_vec(smtbdd);
}

void DFA::construct_from_comp_front(string filename){
  // Construct the BDD for the spec portion
  read_from_file(filename);
  nbits = state2bin(nstates-1).length();
  construct_bdd_new();
}

void DFA::construct_from_comp_back(vbdd& S2S, vbdd& S2P, vbdd& Svars, vbdd& Ivars, vbdd& Ovars, std::vector<int> IS){
  
  // substitute P from res, first create a substitution/projection vector, then use the batch substitution function
  vbdd subnProj;
  // task dfa states
  for (int i=0; i<nbits; i++){
    subnProj.push_back(bddvars[i]);
  }
  // propositions (aka task variables)
  assert(S2P.size()==nvars);
  for (int i=0; i<nvars; i++){
    // TODO: We need to make sure the variables line up!!!!
    subnProj.push_back(S2P[i]);
  }
  // domain state variables
  for (auto & v : Svars){
    subnProj.push_back(v);
  }
  // human action variables
  for (auto & v : Ivars){
    subnProj.push_back(v);
  }
  // robot action variables
  for (auto & v : Ovars){
    subnProj.push_back(v);
  }
  for (int i=0; i<res.size(); i++){
    res[i] = res[i].VectorCompose(subnProj);
  }  

  // append the propositions to res
  res.insert(res.end(), S2P.begin(), S2P.end());
  // append S2S to res
  res.insert(res.end(), S2S.begin(), S2S.end());
  
  // fix the other variables (nvars, nbits, init, etc)
  //std::cout<<"constructing bdd with "<<bddvars.size()<<"variables"<<std::endl;
  bddvars.insert(bddvars.end(), Svars.begin(), Svars.end());
  bddvars.insert(bddvars.end(), Ivars.begin(), Ivars.end());
  bddvars.insert(bddvars.end(), Ovars.begin(), Ovars.end());
  //std::cout<<"constructing bdd with "<<bddvars.size()<<"variables"<<std::endl;
  // make init bitvector (final states is a bdd, does not need change)
  initbv = vector<int>(nbits+nvars+Svars.size());
  int temp = init;
  for (int i=nbits-1; i>=0; i--){
    initbv[i] = temp%2;
    temp = temp/2;
  }
  for (int i=0; i<nvars; i++){
    initbv[i+nbits] = 0;
  }
  for (int i=0; i<IS.size(); i++){
    initbv[i+nbits+nvars] = IS[i];
  }
  nbits = nbits + nvars + Svars.size(); // TODO: can we eliminate the propositions completely?
  nvars = Ivars.size() + Ovars.size();

  // add indices for input and output
  input.clear();
  output.clear();
  for (int i=0; i<Ivars.size(); i++){
    input.push_back(i+nbits);
  }
  for (int i=0; i<Ovars.size(); i++){
    output.push_back(i+nbits+Ivars.size());
  }
    //bdd2dot();
}

void DFA::recur(int index, item tmp){
    if(smtbdd[index][0] == -1){
        while(tmp.size() < nbits + nvars)
            tmp.push_back(2); //0:false 1:true 2:don't care
        push_states(smtbdd[index][1], tmp);
        bdd.push_back(tmp);
        //print_vec(bdd);
        tmp.clear();
    }
    else{
        int left = smtbdd[index][1];
        int right = smtbdd[index][2];
        int v = smtbdd[index][0];
        recur_left(left, tmp, v);
        recur_right(right, tmp, v);
    }
}

void DFA::recur_left(int index, item tmp, int v){
	while(tmp.size() < (nbits + v))
		tmp.push_back(2); //0:false 1:true 2:don't care
	tmp.push_back(0);
	recur(index, tmp);
}

void DFA::recur_right(int index, item tmp, int v){
	while(tmp.size() < (nbits+v))//
		tmp.push_back(2); //0:false 1:true 2:don't care
	tmp.push_back(1);
	recur(index, tmp);
}

void DFA::get_bdd(){
    for(int i = 0; i < nstates; i++){
        int index = behaviour[i];
        item tmp;
        push_states(i, tmp);
        recur(index, tmp);
    }
}

void DFA::push_states(int i, item & tmp){
    string s = state2bin(i);
    for(int j = 0; j < nbits - s.length(); j++)
        tmp.push_back(0);
    for(int j = 0; j < s.length(); j++){
        int t = int(s[j]) - 48;
        tmp.push_back(t);
    }
}

void DFA::print( vector <string> & v )
{
  for (size_t n = 0; n < v.size(); n++)
    cout << v[ n ] << " ";
  cout << endl;
}

void DFA::print_int( vector <int> & v )
{
  for (size_t n = 0; n < v.size(); n++)
    cout<< v[ n ] << " ";
  cout << endl;
}

void DFA::print_vec(vector<item> & v){
    for (size_t n = 0; n < v.size(); n++)
        print_int(v[n]);
  cout << endl;
}

bool DFA::strfind(string str, string target){
	size_t found = str.find(target);
	if(found != string::npos)
		return true;
	else
		return false;
}

string DFA::state2bin(int n){
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
    //cout<<res<<endl;
   return res;
}

void DFA::bdd2dot(){
    for(int i = 0; i < res.size(); i++){
        string filename = "var_"+to_string(i);
        dumpdot(res[i], filename);
    }
}

void DFA::dump_automaton() {
    dump_automaton("automaton.txt");
}

void DFA::dump_automaton(const std::string& filename) {
    std::ofstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file for writing." << std::endl;
        return;
    }
    file << nbits << " " << input.size() << " " << output.size() << endl;
    // Inputs 
    for(int i = 0; i < input.size(); ++i)
           file << bddvars[input[i]] << " ";
    file << endl;  
    // Outputs 
    for(int i = 0; i < output.size(); ++i)
        file << bddvars[output[i]] << " ";
    file << endl;
    // Print state variables 
    for(int i = 0; i < nbits; ++i)
        file << bddvars[i] << " ";
    file << endl;

    // Dump the transition function (res)
    for(auto& bdd : res) {
        file << bdd << endl;
    }
    // Dump the finalstates BDD 
    file << finalstatesBDD << endl;
    // Dump the initial states 
    for(int i = 0; i < nbits; ++i)
        file << initbv[i] << " ";
    file << endl;
    
}

void DFA::print_full() {
    cout << bddvars.size() << " " << nvars;
    cout << " " << nbits << " ";
    cout <<  nstates << " " << input.size() << " " << output.size() << endl;
    // Inputs 
    for(int i = 0; i < input.size(); ++i)
           cout << bddvars[input[i]] << " ";
    cout << endl;  
    // Outputs 
    for(int i = 0; i < output.size(); ++i)
        cout << bddvars[output[i]] << " ";
    cout << endl;
    // Print state variables 
    for(int i = 0; i < nbits; ++i)
        cout << bddvars[i] << " ";
    cout << endl;

    // Dump the transition function (res)
    for(auto& bdd : res) {
        cout << bdd << endl;
    }
    // Dump the finalstates BDD 
    cout << finalstatesBDD << endl;
    // Dump the initial states 
    for(int i = 0; i < nbits; ++i)
        cout << initbv[i] << " ";
    cout << endl;
}

//return positive or nagative bdd variable index
BDD DFA::var2bddvar(int v, int index){
    if(v == 0){
        return !bddvars[index];
    }
    else{
        return bddvars[index];
    }
}


void DFA::construct_bdd(){

    for(int i = 0; i < nbits+nvars; i++){
        BDD b = mgr->bddVar();
        bddvars.push_back(b);
    }

    for(int i = 0; i < nbits; i++){
        BDD d = mgr->bddZero();
        res.push_back(d);
    }
    //cout<<"bddvars.length: "<<bddvars.size()<<endl;

    for(int i = 0; i < bdd.size(); i++){
        for(int j = 0; j < nbits; j++){
            if(bdd[i][nbits+nvars+j] == 1){
                BDD tmp = mgr->bddOne();
                for(int m = 0; m < nbits+nvars; m++)
                {
                    if(bdd[i][m] != 2){
                        tmp *= var2bddvar(bdd[i][m], m);
                    }
                }
                res[j] += tmp;
            }
        }
    }
}

void DFA::construct_bdd_new(){
    for(int i = 0; i < nbits+nvars; i++){
        BDD b = mgr->bddVar();
        bddvars.push_back(b);
        //dumpdot(b, to_string(i));
    }
    #ifdef BUILD_DEBUG
        std::cout<<"constructing bdd with "<<bddvars.size()<<"variables"<<std::endl;
    #endif
    for(int i = 0; i < nbits; i++){
        BDD d = mgr->bddZero();
        res.push_back(d);
    }
    tBDD.resize(smtbdd.size());
    for(int i = 0; i < tBDD.size(); i++){
        if(tBDD[i].size() == 0){
            //dumpdot(tBDD[i][0], "test");
	    vbdd b = try_get(i, false);
        }
    }


    for(int i = 0; i < nbits; i++){
        for(int j = 0; j < nstates; j++){
            BDD tmp = mgr->bddOne();
            string bins = state2bin(j);
            int offset = nbits - bins.size();
            for(int m = 0; m < offset; m++){
                tmp = tmp * var2bddvar(0, m);
            }
            for(int m = 0; m < bins.size(); m++){
                tmp = tmp * var2bddvar(int(bins[m])-48, m + offset);
            }
            //dumpdot(tmp, "res-state "+to_string(behaviour[j])+to_string(i));
            //dumpdot(tBDD[behaviour[j]][i], "res-bdd "+to_string(behaviour[j])+to_string(i));
            tmp = tmp * tBDD[behaviour[j]][i];
            res[i] = res[i] + tmp;
            //dumpdot(res[i], "res "+to_string(i));
        }
        dumpdot(res[i], "res "+to_string(i));
    }

    finalstatesBDD = mgr->bddZero();
    for(int i = 0; i < finalstates.size(); i++){
        BDD ac = state2bdd(finalstates[i]);
        finalstatesBDD += ac;
    }
}

void DFA::construct_bdd_belief_state(){
    #ifdef BUILD_DEBUG
        std::cout << "constructing belief state now" << std::endl;
        std::cout << "have " << output.size() << " many output variables" << std::endl;
    #endif
    // belief-state space has one state variable per state of the automaton
    for(int i = 0; i < nstates+nvars; i++){
        BDD b = mgr->bddVar();
        bddvars.push_back(b);
    }

    // review
    for(int i = 0; i < nstates; i++){
        BDD d = mgr->bddZero();
        res.push_back(d);
    }
    tBDD.resize(smtbdd.size());
    for(int i = 0; i < tBDD.size(); i++){
        if(tBDD[i].size() == 0){
	  vbdd b = try_get(i, true);
        }
    }

    BDD unobservable_cube = mgr->bddOne();
    for(int i = 0; i < unobservable.size(); i++){
        unobservable_cube *= bddvars[unobservable[i]];
    }

    for(int i = 0; i < nstates; i++){
        for(int j = 0; j < nstates; j++){
	  BDD tmp = bddvars[j] * tBDD[behaviour[j]][i];
	  res[i] = res[i] + tmp.ExistAbstract(unobservable_cube);
        }
    }

    finalstatesBDD = mgr->bddOne();
    for(int i = 0; i < nonfinalstates.size(); i++){
      finalstatesBDD *= !bddvars[nonfinalstates[i]];
    }
}

BDD DFA::state2bdd(int s){
    string bin = state2bin(s);
    BDD b = mgr->bddOne();
    int nzero = nbits - bin.length();
    //cout<<nzero<<endl;
    for(int i = 0; i < nzero; i++){
        b *= !bddvars[i];
    }
    for(int i = 0; i < bin.length(); i++){
        if(bin[i] == '0')
            b *= !bddvars[i+nzero];
        else
            b *= bddvars[i+nzero];
    }
    return b;

}

vbdd DFA::try_get(int index, bool partial_observability){
  if(tBDD[index].size() != 0)
    return tBDD[index];
  vbdd b;
  if(smtbdd[index][0] == -1){
    int s = smtbdd[index][1];
    if (partial_observability) {
	b = vbdd(nstates, mgr->bddZero());
	b[s] = mgr->bddOne();
    }
    else {
      string bins = state2bin(s);
      for(int m = 0; m < nbits - bins.size(); m++){
	b.push_back(mgr->bddZero());
      }
      for(int i = 0; i < bins.size(); i++){
	if(bins[i] == '0')
	  b.push_back(mgr->bddZero());
	else if(bins[i] == '1')
	  b.push_back(mgr->bddOne());
	else
	  cout<<"error binary state"<<endl;
      }
    }
    tBDD[index] = b;
    return b;
  }
  else{
    int rootindex = smtbdd[index][0];
    int leftindex = smtbdd[index][1];
    int rightindex = smtbdd[index][2];
    BDD root = bddvars[rootindex+nbits];
    //dumpdot(root, "test");
    vbdd left = try_get(leftindex, partial_observability);
    //for(int l = 0; l < left.size(); l++)
    // dumpdot(left[l], "left"+to_string(l));
    vbdd right = try_get(rightindex, partial_observability);
    //for(int l = 0; l < left.size(); l++)
    // dumpdot(right[l], "right"+to_string(l));
    assert(left.size() == right.size());
    for(int i = 0; i < left.size(); i++){
      BDD tmp;
      tmp = root.Ite(right[i], left[i]);//Assume this is correct
      //dumpdot(tmp, "tmp");
      b.push_back(tmp);
    }
    tBDD[index] = b;
    return b;
  }
}

void DFA::dumpdot(BDD &b, string filename){
    FILE *fp = fopen(filename.c_str(), "w");
    vector<BDD> single(1);
    single[0] = b;
	this->mgr->DumpDot(single, NULL, NULL, fp);
	fclose(fp);
}

void DFA::complement() {
  finalstatesBDD = !finalstatesBDD;
}
