#!/usr/bin/env python3
### Generate an example file for a very simple sheep - game 
from pylogics.syntax.ltl import Atomic, Eventually, Always, WeakNext, Next, Until
from pylogics.syntax.base import And, Or, Not, Implies, Equivalence, TrueFormula, FalseFormula
from pylogics.utils.to_string import to_string
import utils 
from generator import Generator

class HikerGenerator(Generator):

    def generate(self, n, k, haveHerbs, partial = True):
        print("Start")
        """Generates a test case for the hiker.

        Args:
            n (int):
            k (int): At what distance from the end the herbs (if haveHerbs = True) shall appear.
            haveHerbs (): Controls whether there are herbs (i.e., depending on the result, we either have a realisable or unrealisable instance)
            partial (bool, optional): _description_. Defaults to True.
        """
        self.output = "hiker-%d-%s"%(n, "solv" if haveHerbs else "unsolv")
        # Environment variables
        # Variables
        berry = Atomic("berry")
        poison = Atomic("poison")
        herbs = Atomic("herbs")
        sick = Atomic("sick")
        eot = Atomic("eot")
        inbag = Atomic("inbag")

        self.inputvars = [poison, berry, sick, eot, herbs, inbag]
        self.partialvars = [poison]

        # Agent variables
        eat = Atomic("eat")
        takeherbs = Atomic("takeherbs")
        collectherbs = Atomic("collectherbs")
        self.outputvars = [eat, collectherbs, takeherbs]

        # Constraints
        berry_const =  Always(poison >> berry) & Always(berry >> ~herbs)
        # Successor state axioms
        #sick_ssa = Always((sick) >> WeakNext(sick)) & Always((berry & poison & WeakNext(eat) & ~sick) >> WeakNext(sick))
        sick_ssa = Always(Equivalence(Next(sick),  Next(TrueFormula()) & ((Next(eat) & berry & poison) | (sick & ~(inbag & takeherbs)))))
        inbag_ssa = Always(Equivalence(Next(inbag), (Next(TrueFormula()) & ((herbs & Next(collectherbs)) |  (inbag & ~takeherbs)))))

        # End of trail constraint
        eot_constraint = utils.IterWeakNext(n, eot) & utils.IterWeakNext(n+1, eot & Always(eot)) & Always(eot >> (~berry))
        for i in range(n):
            eot_constraint &= utils.IterWeakNext(i, ~eot)
        # Medication constraint: Ensure herbs appear at least once before the end
        medication_constraint = utils.IterWeakNext(n - k, herbs) if haveHerbs else TrueFormula()

        # Initial condition
        init = ~sick & ~inbag   #& WeakNext(Always(~start))

        # Environment formula
        phi_env =  sick_ssa  & init & eot_constraint   & berry_const & inbag_ssa & medication_constraint & init
        agent_goal =  Always(((berry) & ~poison) >> WeakNext(eat)) & Eventually(eot)
        backup_goal = Eventually(eot & (~sick)) 

        self.formula =   (phi_env >> agent_goal)
        self.backup =  (phi_env >> backup_goal)
        #print(to_string(phi_env).replace("X[!]", "Y").replace("X", "WX").replace("Y", "X").replace("ff", "false").replace("tt", "true"))        

print("Starting to generate")
for i in range(3, 5, 1):
    g = HikerGenerator()
    g.generate(i, 1, False)
    g.write(Syft=True)
    g.generate(i, 1, True)
    g.write(Syft=True)

import random
for i in range(5, 60, 5):
    g = HikerGenerator()
    g.generate(i, random.randint(2, i - 1), False)
    g.write(Syft=True)
    g.generate(i, 1, True)
    g.write(Syft=True)
print("Generated successfully")