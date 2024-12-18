### utilities.py 
#   Contains helpers that help with generating formula
from os import system
import sys, argparse, itertools, ltlf2dfa
import ltlf2dfa.ltlf2dfa
from pylogics.syntax.ltl import Atomic, Eventually, Always, WeakNext, Next, Until
from pylogics.syntax.base import And, Or, Not, Implies, Equivalence, TrueFormula, FalseFormula
from ltlf2dfa.parser.ltlf import LTLfParser
from pylogics.utils.to_string import to_string

def conjunction(formulas, agg = And):
    if len(formulas) == 0:
        return TrueFormula() # TODO: Emoty conjunction == false?
    conj = formulas[0]
    for f in formulas[1:]:
        conj = agg(conj, f)
    return conj

def exactly_one_of_helper(formulas):
    at_least_one = conjunction(formulas, Or)
    not_the_same = []
    for forma, formb in itertools.combinations(formulas, 2):
            not_the_same.append(Or(Not(forma),Not(formb))) 
    return And(at_least_one, conjunction(not_the_same))

def at_most_one(formulas):
    not_the_same = []
    for forma, formb in itertools.combinations(formulas, 2):
            not_the_same.append(Or(Not(forma),Not(formb))) 
    return  conjunction(not_the_same)

def exactly_k_of_simple(formulas, k):
    disjuncts = []
    for s in itertools.combinations(formulas, k):
        l = list(s)
        disjuncts.append(simple_conj(l, list(set(formulas) - set(l))))
    return conjunction(disjuncts, Or)
    

def simple_conj(pos, neg):
    posform = conjunction(pos)
    negform = conjunction(list(map(Not, neg)))
    assert(not (len(pos) == 0 and len(neg) == 0))
    if len(pos) == 0:
        return negform
    if len(neg) == 0:
        return posform
    return And(posform, negform)

def Iter(k, op, formula):
    while k > 0:
        formula = op(formula)
        k = k - 1
    return formula

def IterWeakNext(k, formula):
    return Iter(k, WeakNext, formula)

def toSyftInput(formula):
    return to_string(formula).replace("X[!]", "Y").replace("X", "N").replace("Y", "X").replace("tt", "true").replace("ff", "false")