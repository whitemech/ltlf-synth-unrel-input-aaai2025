#ifndef CORDFA_SYN_H
#define CORDFA_SYN_H
#include <memory>
#include "Common.h"
#include "DFA.h"
#include "SSNFA.h"
#include "syn.h"

class CoRDFA_syn : public syn
{
 public:
  CoRDFA_syn(std::shared_ptr<Cudd> m, std::string filename, std::string secondfile, std::string partfile);
  virtual ~CoRDFA_syn();
  void initializer(std::unique_ptr<SSNFA>& ssnfa);
 private:
	std::unique_ptr<SSNFA> bddBackups;

  
};

#endif // CORDFA_SYN_H
