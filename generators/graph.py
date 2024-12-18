### gridgen.py
from collections import defaultdict
from math import log, ceil
import sys, argparse, itertools, ltlf2dfa
import ltlf2dfa.ltlf2dfa
from pylogics.syntax.ltl import Atomic, Eventually, Always, WeakNext, Next, Until
from pylogics.syntax.base import And, Or, Not, Implies, Equivalence, TrueFormula
from ltlf2dfa.parser.ltlf import LTLfParser
from pylogics.utils.to_string import to_string
import utils 
from generator import Generator
    

class Trap:
    def __init__(self, edge1, to1, edge2, to2) -> None:
        self.behaviourTrue = {
            "edge": edge1,
            "to": to1
        }

        self.behaviourFalse = {
            "edge": edge2,
            "to": to2
        }

        

class GraphGenerator(Generator):
    def fromState(self, n: int):
        get_bin = lambda x, n: format(x, 'b').zfill(n)
        conj = []
        for i, x in enumerate(get_bin(n, self.nbits)):
            conj.append((~Atomic("pos_%d"%(i))) if int(x) == 0 else Atomic("pos_%d"%(i)))
        return conj
    def generateEnvironmentFormula(self, edges, traps, usedInTrap, withInit = True):
        sourceDict = defaultdict(lambda : 0)
        self.conditions = []
        for i in range(len(edges)):
            edge = edges[i]
            left = True
            if sourceDict[edge[0]] == 1:
                left = False 
            elif sourceDict[edge[0]] > 1:
                assert(False)
            sourceDict[edge[0]] = sourceDict[edge[0]] + 1
            # Check wheter it is a left or right edge:
            #print("Edge {} is a {}".format(edge, left))
            # Check if this edge has a trap behaviour 
            trap = (None, None)
            for j, t in enumerate(traps): 
                if t.behaviourTrue["edge"] == i:
                    trap = ("+", j, t.behaviourTrue["to"])
                if t.behaviourFalse["edge"] == i:
                    trap = ("-", j, t.behaviourFalse["to"])
            # Generate Box (pos_1 _ _ & t_1 ) -> X()
            if trap == (None, None):
                fromvars = self.fromState(edge[0])
                tovars = self.fromState(edge[1])
                movevar = Atomic("left")
                if not left:
                    movevar = ~movevar
                self.conditions.append(Always(Implies(utils.conjunction(fromvars) & WeakNext(movevar), WeakNext(utils.conjunction(tovars)))))
            else:
                fromvars = self.fromState(edge[0])
                tovarsA = self.fromState(edge[1])
                tovarsB = self.fromState(trap[2])
                movevar = Atomic("left")
                if not left:
                    movevar = ~movevar
                trapOff = Atomic("t_%d"%(trap[1]))
                trapOn = ~Atomic("t_%d"%(trap[1]))
                if trap[0] != '-':
                    z = trapOn 
                    trapOn = trapOff
                    trapOff = z 
                self.conditions.append(Always(Implies(utils.conjunction(fromvars) & WeakNext(movevar) & trapOff, (WeakNext(utils.conjunction(tovarsA)) ))))
                self.conditions.append(Always(Implies(utils.conjunction(fromvars) & WeakNext(movevar) & trapOn, (WeakNext(utils.conjunction(tovarsA)) | WeakNext(utils.conjunction(tovarsB)) ))))
        for i in range(self.n):
            fromvars = self.fromState(i)
            if sourceDict[i] == 0:
                self.conditions.append(Always(Implies(utils.conjunction(fromvars) & WeakNext(Atomic("left")) , (WeakNext(utils.conjunction(fromvars)) ))))
                self.conditions.append(Always(Implies(utils.conjunction(fromvars) & WeakNext(~Atomic("left")) , (WeakNext(utils.conjunction(fromvars)) ))))
            if sourceDict[i] == 1:
                self.conditions.append(Always(Implies(utils.conjunction(fromvars) & WeakNext(~Atomic("left")) , (WeakNext(utils.conjunction(fromvars)) ))))
        # Add condiion that A -> Box(A) for each trap variable 
        for i, t in enumerate(traps):
            self.conditions.append(Always(Implies(Atomic("t_%d"%(i)), WeakNext(Atomic("t_%d"%(i))))))
            self.conditions.append(Always(Implies(~Atomic("t_%d"%(i)), WeakNext(~Atomic("t_%d"%(i))))))
        initialstate = utils.conjunction(self.fromState(0))
        #print(initialstate)
        self.conditions.append(initialstate)
        return utils.simple_conj(self.conditions, [])

    def generate(self, n, edges, traps, goals, safety, usedInTrap, partial = False):
        self.inputvars = []
        self.outputvars = []
        self.partialvars = []
        self.backup = []
        self.partial = partial
        self.output = "example"
        self.n = n
        # Create (log2(n)) many field variables 
        self.fieldvars = []
        self.nbits = (ceil(log(n, 2)))
        for i in range(self.nbits):
            self.fieldvars.append(Atomic("pos_%d"%(i)))
        for i in range(len(traps)):
            self.inputvars.append(Atomic("t_%d"%(i)))
            self.partialvars.append(Atomic("t_%d"%(i)))
        self.inputvars.extend(self.fieldvars)
        #self.partialvars.extend(self.fieldvars)

        #(self.fieldvars)
        phi_env = self.generateEnvironmentFormula(edges, traps, usedInTrap)
        # Create agent variables (left / right)
        self.outputvars.append(Atomic("left"))
        goalf = []
        for g in goals:
            goalf.append(utils.conjunction(self.fromState(g)))
        goalf = Eventually(utils.conjunction(goalf, Or))
        securityGoal = []
        for g in goals:
            securityGoal.append(utils.conjunction(self.fromState(g)))
        for g in safety:
            securityGoal.append(utils.conjunction(self.fromState(g)))
        securityGoal = Eventually(utils.conjunction(securityGoal, Or))


        self.formula = Implies(phi_env, goalf)
        if self.partial:
            self.backup = Implies(self.generateEnvironmentFormula(edges, traps, usedInTrap, False), securityGoal)

if __name__ == "__main__":
    filename = sys.argv[1]
    print("Reading from file {}".format(filename))
    edges = []
    goals = []
    safety = []
    usedInTrap = []
    traps = []
    with open(filename) as f:
        n, m, t = [int(x) for x in next(f).split(";")[0].split()] # read first line
        usedInTrap = [False] * m
        array = []
        goals = [int(x) for x in next(f).split(";")[0].split()]
        safety = [int(x) for x in next(f).split(";")[0].split()]
        print(goals)
        print(safety)
        for i in range(m):
            line = [int(x) for x in next(f).split(";")[0].split()]
            print(line)
            assert(len(line) == 2)
            edges.append(tuple(line))
        for i in range(t):
            line = [int(x) for x in next(f).split(";")[0].split()]
            print(line)
            e1, t1, e2, t2 = line
            assert(len(line) == 4)
            usedInTrap[e1] = i
            usedInTrap[e2] = i
            print("Read trap ",e1,t1,e2,t2)
            print(edges[e1])
            print(edges[e2])
            traps.append(Trap(e1, t1, e2, t2))
            

        g = GraphGenerator()
        g.generate(n, edges, traps, goals, safety, usedInTrap, partial = True)
        g.write(Syft = True)
        


