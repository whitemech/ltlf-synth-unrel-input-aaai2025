/*
 * added by Shufang ZHU on Jan 10th, 2017
 * translate ltlf formulas to fol, the input of MONA
*/

#include "ltlf2pfol.h"
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <set>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace std;
#define MAXN 1000000


void ltlf2fol (string ss, ltl_formula *root)
{
  int c = 1;
  string res;
  
  set<string> P = get_alphabet (root);
  if(!P.empty()){
    // cout<<"var2 $, ";
    cout<<"m2l-str;"<<endl;
    cout<<"var2 ";
    print_alphabet_no_comma(root);
    cout<<";"<<endl;
    // cout<<"allpos $;"<<endl;
    // cout<<"0 in $;"<<endl;
  }
  
  res = trans_fol(root, "max $", c);
  cout<<res<<";"<<endl;
  

}

void print_alphabet_no_comma (ltl_formula* root){
  set<string> P = get_alphabet (root);
  if(!P.empty()){
    set<string>::iterator it = P.begin ();
    // cout<<toupper(*it);
    cout<<up(*it);
    it++;
    while (it != P.end ()){
      cout<<", "<<up(*it);
      it++;
    }
  }
}

string alphabet_no_comma (ltl_formula* root){
  string res = "";
  set<string> P = get_alphabet (root);
  if(!P.empty()){
    set<string>::iterator it = P.begin ();
    // cout<<toupper(*it);
    res += up(*it);
    it++;
    while (it != P.end ()){
      res += ", "+up(*it);
      it++;
    }
  }
  return res;
}

void print_alphabet (ltl_formula* root){
  set<string> P = get_alphabet (root);
  if(!P.empty()){
    set<string>::iterator it = P.begin ();
    // cout<<toupper(*it);
    cout<<", "<<up(*it);
    it++;
    while (it != P.end ()){
      cout<<", "<<up(*it);
      it++;
    }
  }
}

void print_alphabet_not (ltl_formula* root){
  set<string> P = get_alphabet (root);
  if(!P.empty()){
    set<string>::iterator it = P.begin ();
    // cout<<toupper(*it);
    cout<<", "<<up(*it)<<"\\{p}";
    it++;
    while (it != P.end ()){
      cout<<", "<<up(*it);
      it++;
    }
  }
}

void printvars (ltl_formula* root){
  set<string> P = get_alphabet (root);
  if(!P.empty()){
    set<string>::iterator it = P.begin ();
    // cout<<toupper(*it);
    cout<<", var2 "<<up(*it);
    it++;
    while (it != P.end ()){
      cout<<", var2 "<<up(*it);
      it++;
    }
  }
}

std::string trans_fol(ltl_formula* root, string t, int& c){
  string ts;
  string exs, alls;
  string res;
  int intt;
  if (t == "max $"){
    ts = t;
    intt = 0;
  }
  else{
    ts = "x"+t;
    intt = stoi(t);
  }
  if (intt == 0)
    ts = "max $";
  switch(root->_type)
  {
        case eNOT:
          res = "~(";
          res += trans_fol(root->_right, t, c);
          res += ")";
          break;
        case eNEXT:
          exs = "x"+to_string(intt+1);
          res = "(ex1 "+exs+": (("+exs+"="+ts+"-1) & ("+ts+" > 0) & (";
          res += trans_fol(root->_right, to_string(intt+1), c);
          res += ")))";
          break;
        case eWNEXT:
          exs = "x"+to_string(intt+1);
          res = "((ex1 "+exs+": ("+exs+"="+ts+"-1 & ("+ts+" > 0) & (";
          res += trans_fol(root->_right, to_string(intt+1), c);
          res += "))) | ("+ts+" = 0))";
          break;
        case eFUTURE:
          exs = "x"+to_string(intt+1);
          res = "(ex1 "+exs+": ("+exs+" <= "+ts+" & 0 <= "+exs+" & (";
          res += trans_fol(root->_right, to_string(intt+1), c);
          res += ")))";
          break;
        case eUNTIL:
          exs = "x"+to_string(intt+1);
          alls = "x"+to_string(intt+2);
          res = "(ex1 "+exs+": ("+exs+" <= "+ts+" & 0 <= "+exs+" & (";
          res += trans_fol(root->_right, to_string(intt+1), c);
          res += ") & (all1 "+alls+": ("+alls+" <= "+ts+" & "+exs;
          res += " < "+alls+" => (";
          res += trans_fol(root->_left, to_string(intt+2), c);
          res += ")))))";
          break;
        case eOR:
          res += "(("+trans_fol(root->_right, to_string(intt), c);
          res += ") | (";
          res += trans_fol(root->_left, to_string(intt), c)+"))";
          break;
        case eAND:
          res += "(("+trans_fol(root->_right, to_string(intt), c);
          res += ") & (";
          res += trans_fol(root->_left, to_string(intt), c)+"))";
          break;
        case eTRUE:
          res += "(true)";
          break;
        case eFALSE:
          res += "(false)";
          break;
        case 3:
          res += "("+ts+" in ";
          res += alphabet_no_comma(root);
          res +=")";
          break;
        default:
          break;
  }
  // cout<<res<<endl;
  return res;
}

string up(string a){
  return boost::to_upper_copy<std::string>(a);
}










