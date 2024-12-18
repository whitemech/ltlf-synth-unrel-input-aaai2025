#include "Common.h"

#ifndef DFA_H
#define DFA_H

typedef std::vector<int> item;
typedef std::vector<BDD> vbdd;
class DFA
{
    public:
        DFA(std::shared_ptr<Cudd> m);
	DFA();
        virtual ~DFA();

        std::shared_ptr<Cudd> mgr;

        void initialize(std::string filename, std::string partfile, bool partial_observability);
        //void initialize(std::string filename, std::string partfile, Cudd& manager);
        void init_from_cross_product(DFA* d1, DFA* d2);
        std::vector<item> bdd;
        void print_vec( std::vector <item> & v );
        void construct_bdd();
        void bdd2dot();
        void dumpdot(BDD &b, std::string filename);
	BDD state2bdd(int s);
        int nbits;
        int init;
	std::vector<int> initbv;
        int nstates;

        int nvars;
        std::vector<int> finalstates;
	std::vector<int> nonfinalstates;
	BDD finalstatesBDD;
        std::vector<BDD> res;
        std::vector<BDD> bddvars;
        std::vector<int> input;
        std::vector<int> output;
	std::vector<int> unobservable;

	std::vector<std::string> variables;
	
        //new bdd constructer
        void construct_bdd_new();
	void construct_bdd_belief_state();

	// domain-spec separate construction
	// Front need to be called before variable construction for domain
	// back is called after the components are constructed
	void construct_from_comp_front(std::string filename);
	void construct_from_comp_back(vbdd& S2S, vbdd& S2P, vbdd& Svars, vbdd& Ivars, vbdd& Ovars, std::vector<int> IS);
	void complement();
        void init_from_DFA(DFA *d1);
        void dump_automaton();
        void print_full();

        void dump_automaton(const std::string &filename);

        // A function to construct the product of two DFAs 

    protected:
	std::vector<int> behaviour;
	std::vector<item> smtbdd;
        void read_from_file(std::string filename); //read the ltlf formula
        void read_partfile(std::string partfile); //read the partfile

    private:
        int nodes;
        //		std::vector<std::string> variables;
        void get_bdd();
        void recur(int index, item tmp);
        void recur_left(int index, item tmp, int v);
        void recur_right(int index, item tmp, int v);
        void print( std::vector <std::string> & v );
        void print_int( std::vector <int> & v );
        bool strfind(std::string str, std::string target);
        void push_states(int i, item& tmp);
        std::string state2bin(int n);
        BDD var2bddvar(int v, int index);

        //new bdd constructer
        std::vector<vbdd> tBDD;
        vbdd try_get(int index, bool partial_observability);

};

#endif // DFA_H
