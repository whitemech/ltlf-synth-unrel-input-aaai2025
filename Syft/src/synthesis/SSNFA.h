#include "Common.h"
#include "DFA.h"

#ifndef SSNFA_H
#define SSNFA_H

class SSNFA: public DFA
{
    public:
        SSNFA();
        SSNFA(std::shared_ptr<Cudd> m);
        virtual ~SSNFA();
        void initialize_mona(std::string filename, std::string partfile);
        //IDFA construction
        void construct_SSNFA_MONA();
	std::set<int> stateset;
	std::vector<vbdd> labels;
	void project_unobservables();

    private:
	vbdd con_node_mona(int index);
	std::vector<vbdd> labels_mona;
};

#endif // SSNFA_H
