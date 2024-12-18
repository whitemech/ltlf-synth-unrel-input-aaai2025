#include <iostream>
#include <string>
#include <memory>
#include <chrono>
#include "syn.h"
#include "CoRDFA_syn.h"

using std::string;
using std::shared_ptr;
using std::make_shared;
using std::move;
using std::cout;
using std::endl;
namespace chrono = std::chrono;
using std::unique_ptr;
using std::make_unique;

/**
 * This function handles the main steps, to hide running MONA, etc. From the user. 
 * It takes in the name of the LTL file, the partition file and the algorithm to use (spec_type: 0 = direct, 1 = belief, 2 = mso).
 */
vector<string> get_DFAFiles(string LTLFile, string Partfile, int spec_type) {

  // For the MSO approach (Sect 7 paper / Chp 7 thesis) just run ltlf2fol with two file arguments, then it generates the translation 
  // of varphi_m & \forall U_1. \dots \forall U_n. \varphi_b as the output file.
  if(spec_type == 2) {
    // Generate the FOL file
    string FOL = LTLFile+".fol";
    // Translate to mso
    string LTLF2FOL = "./ltlf2fol NNF "+LTLFile+" "+Partfile+" >"+FOL;
    if(system(LTLF2FOL.c_str())) {
      cerr << "Error running ltlf2fol for MSO" << endl;
      exit(1);
    }

    string DFA = LTLFile+".dfa";
    // Create the DFA
    string FOL2DFA = "mona -u -xw "+FOL+" >"+DFA;
    if(system(FOL2DFA.c_str())) {
      cerr << "Error running mona for MSO" << endl;
      exit(1);
    }
    // We return only a single file-name, as this is the DFA we do  synthesis on.
    return {DFA};
  }
  if (spec_type == 3) {
    // Generate the FOL file
    string FOL = LTLFile+".fol";
    // Translate to mso
    string LTLF2FOL = "./qltlf2mso NNF "+LTLFile+ " > "+FOL;
    if(system(LTLF2FOL.c_str())) {
      cerr << "Error running ltlf2fol for MSO" << endl;
      exit(1);
    }

    string DFA = LTLFile+".dfa";
    // Create the DFA
    string FOL2DFA = "mona -u -xw "+FOL+" >"+DFA;
    if(system(FOL2DFA.c_str())) {
      cerr << "Error running mona for MSO" << endl;
      exit(1);
    }
    // We return only a single file-name, as this is the DFA we do  synthesis on.
    return {DFA};
  }
  // For the other two, we need to create two files
  // Read in the file and create the two files 
  string mainf = LTLFile+".main";
  string backupf = LTLFile+".backup";
  std::ofstream outputFile1(mainf), outputFile2(backupf);
  std::ifstream ltlfile(LTLFile);
  string line;

  if (std::getline(ltlfile, line)) outputFile1 << line << std::endl; else {
    cerr << "Expected two lines in .ltlf file" << endl;
    exit(1);
  };
  outputFile1.close();
  // For direct approach, as described in Sect 4 / Chp. 4, we need to create a DFA for \lnot \varphi_b
  if (std::getline(ltlfile, line)) {
    if(spec_type) {
      outputFile2 << "~(" <<  line << ")" << std::endl; 
    } else {
      outputFile2 << line << std::endl; 
    }
  } else {
    cerr << "Expected two lines in .ltlf file" << endl;
    exit(1);
  };
  outputFile2.close();
  // Generate the FOL files for main
  string FOLmain = mainf+".fol";
  string FOLBackup = backupf+".fol";
  // Run 
  string LTLF2FOLmain = "./ltlf2fol NNF "+mainf+" > "+FOLmain;
  cout << LTLF2FOLmain << endl;
  int x = 0;
  if(system(LTLF2FOLmain.c_str())) {
    cerr << "Error running ltlf2fol for main" << " " << x << endl;
    exit(1);
  }

  string LTLF2FOLbackup;
  // Depending on specification either translate the formula to FOL or the equivalent pure-past-ltlf formula to FOL
  if(spec_type) {
    LTLF2FOLbackup  = "./ltlf2pfol "+backupf+" >"+FOLBackup;
  }else{
    LTLF2FOLbackup  = "./ltlf2fol NNF "+backupf+" >"+FOLBackup;
  } 
  std::cout << LTLF2FOLbackup.c_str() << endl;
  if(system(LTLF2FOLbackup.c_str())) {
    cerr << "Error running ltlf2fol for backup" << endl;
    exit(1);
  }

  // Run MONA for both 
  string mainDFA = mainf+".mona";
  string monaMain = "mona -u -xw " +  FOLmain + " > "+mainDFA;
  if(system(monaMain.c_str())) {
    cerr << "Error running mona for main" << endl;
    exit(1);
  }
  string backupDFA = backupf+".mona";
  string monaBackup = "mona -u -xw " +  FOLBackup + " > "+backupDFA;
  std::cout << monaBackup << endl;
  if(system(monaBackup.c_str())) {
    cerr << "Error running mona for backup" << endl;
    exit(1);
  }
  // Clean Up: Remove all generated files but for the dfa 
  std::remove(mainf.c_str());
  std::remove(backupf.c_str());
  std::remove(FOLmain.c_str());
  std::remove(FOLBackup.c_str());
  // Return the two generated DFA (files)
  return {mainDFA, backupDFA};
}

int main(int argc, char ** argv){
    clock_t c_start = clock();
    auto t_start = chrono::high_resolution_clock::now();
    string filename;
    string filenamebackup;
    string partfile;
    string autfile;
    string starting_player;
    string observability;
    string spec_type;
    // Check for correct argument count
    if(argc != 5){
        cout<<"Usage: ./Syft LTLFFile Partfile Starting_player(0: system, 1: environment) SpecType(direct, belief, mso, qltlf)"<<endl;
        return 0;
    }
    else{
        filename = argv[1];
        partfile = argv[2];
        starting_player = argv[3];
	spec_type = argv[4];
    }

    bool partial_observability = true;

    int cordfa_spec;

    if (spec_type == "belief")
        cordfa_spec = 0;
    else if (spec_type == "direct")
      cordfa_spec = 1;
    else if (spec_type == "mso")
      cordfa_spec = 2;
    else if(spec_type == "qltlf")
      cordfa_spec = 3;
    else {
      cout << "SpecType should be one of: belief, direct, mso, qltlf" << endl;
      return 0;
    }
    #ifdef BUILD_DEBUG
    cout << "Creating DFA File" << endl;
    #endif
    vector<string> files = get_DFAFiles(filename, partfile, cordfa_spec);

    #ifdef BUILD_DEBUG
    cout << "Creating cudd mgr" << endl;
    #endif
    shared_ptr<Cudd> mgr = make_shared<Cudd>();

    unique_ptr<syn> test;

    // Initialize the synthesis object, depending on the specification type
    if(cordfa_spec == 0)
      test = make_unique<syn>(move(mgr), files[1], files[0], partfile, partial_observability); // belief-state
    else if(cordfa_spec == 1) 
      test = make_unique<CoRDFA_syn>(move(mgr), files[1], files[0], partfile);  // direct
    else if(cordfa_spec == 2 || cordfa_spec == 3)
      test = make_unique<syn>(move(mgr), files[0], partfile, false); // mso

    cout << "Syn initialised." << endl;

    clock_t c_dfa_end = clock();
    auto t_dfa_end = chrono::high_resolution_clock::now();
    std::cout << "DFA CPU time used: "
              << 1000.0 * (c_dfa_end-c_start) / CLOCKS_PER_SEC << " ms\n"
              << "DFA wall clock time passed: "
              << std::chrono::duration<double, std::milli>(t_dfa_end-t_start).count()
              << " ms\n";

    bool res = 0;
    std::unordered_map<unsigned, BDD> strategy;
    
    if(starting_player == "1")
        res = test->realizablity_env(strategy);
    else
        res = test->realizablity_sys(strategy);

    if(res)
        cout<<"Realizable"<<endl;
    else
        cout<<"Unrealizable"<<endl;
    clock_t c_end = clock();
    auto t_end = chrono::high_resolution_clock::now();
    std::cout << "Total CPU time used: "
              << 1000.0 * (c_end-c_start) / CLOCKS_PER_SEC << " ms\n"
              << "Total wall clock time passed: "
              << std::chrono::duration<double, std::milli>(t_end-t_start).count()
              << " ms\n";
    return 0;

}
//solveeqn
