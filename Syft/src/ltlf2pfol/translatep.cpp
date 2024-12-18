#include "ltlf2pfol.h"
#include <assert.h>
#include <fstream>

#define MAXN 1000000
char in[MAXN];

using namespace std;

int main (int argc, char ** argv)
{
    string StrLine;
    string input;
    if(argc != 2){
        printf("Usage: ./ltlf2pfol filename\n");
        return 0;
  }
    input = argv[1];
    ifstream myfile(input);
    if (!myfile.is_open()) //判断文件是否存在及可读
    {
        printf("error!");
        return -1;
    }
    getline(myfile, StrLine);
    myfile.close(); //关闭文件
    strcpy (in, StrLine.c_str());
    printf ("#%s\n", in);

    ltl_formula *root = getAST (in);
    ltl_formula *newroot = bnf (root);  
    printf ("# past #%s\n", to_string (newroot).c_str ());
    
    ltlf2fol (input, newroot);

   
    destroy_formula (root);
    destroy_formula (newroot);
    
}
