#include "ltlf2fol.h"
#include "partfile_util.hpp"
#include <assert.h>
#include <fstream>

#define MAXN 1000000
char in[MAXN];
char in2[MAXN];

using namespace std;

int main (int argc, char ** argv)
{
  
		string StrLine;
		std::string input;
    std::string format;
    std::string partfile;
		assert(argc == 3 || argc == 4);
    // if argc == 4 ==> preprocess for both formulas 
		input = argv[2];
    format = argv[1];
    if(argc == 4) 
      partfile = argv[3];
		ifstream myfile(input);
    ltl_formula *newroot = NULL;
		if (!myfile.is_open()) //判断文件是否存在及可读
		{
		    printf("unreadable file!");
		    return -1;
		}
		getline(myfile, StrLine);
    strcpy (in, StrLine.c_str());
    if(argc == 4) {
      getline(myfile, StrLine);
      strcpy (in2, StrLine.c_str());
    }
		myfile.close(); //关闭文件
    printf ("#%s\n", in);
    if(argc == 4)
      printf ("#%s\n", in2);

    ltl_formula *root = getAST (in);
    printf("# Got AST \n");
    ltl_formula *bnfroot = bnf (root);
    if(argc == 4) {
      
      ltl_formula *sndroot = getAST(in2);
      ltl_formula *nnfscnd = nnf (bnf (sndroot));
      printf("# Translating a file with special treatment \n");
      // Read in the relevant part of the partitioning file 
      auto unobservable = get_unobservables(partfile);

      ltlf2fol_with_projection(nnf(bnfroot), nnfscnd, unobservable);

    } else { 
      printf ("# %s\n", to_string (bnfroot).c_str ());
      if(format == "NNF"){
        printf ("#NNF format\n");
        newroot = nnf (bnfroot) ;   
      }
      else{
        printf ("#BNF format\n");
        newroot = bnfroot;
      }
      
      printf ("#%s\n", to_string (newroot).c_str ());
      ltlf2fol (newroot);
    }  
    
    

    // printf ("%s\n", res.c_str ());
    destroy_formula (root);
    destroy_formula (newroot);
    //destroy_formula (nnfroot);
}
