### Generate an example file for a very simple sheep - game 
import ltlf2dfa.ltlf2dfa
from pylogics.syntax.ltl import Atomic, Eventually, Always, WeakNext, Next, Until
from pylogics.syntax.base import And, Or, Not, Implies, Equivalence, TrueFormula
from ltlf2dfa.parser.ltlf import LTLfParser
from pylogics.utils.to_string import to_string

import utils
from generator import Generator

def leftvar(n):
    return Atomic("left_%d"%(n))


def generate_env_constraints(n, allowed_moves, disallowed_moves, fullSpec=True):
    move_spec = []
    left_vars = [leftvar(i) for i in range(n)]
    move_vars = [Atomic("move_%d" % i) for i in range(n)]
    # Movement specifications
    for i in range(n):
        # Sheep can only move if they are on the left side
        move_spec.append(Implies(WeakNext(Not(move_vars[i])) & left_vars[i], WeakNext(left_vars[i])))
        move_spec.append(Implies(WeakNext(Not(move_vars[i])) & Not(left_vars[i]), WeakNext(Not(left_vars[i]))))

        # At most two sheep can move at a time
        for j in range(i + 1, n):
            if (i, j) in disallowed_moves+ allowed_moves:
                move_spec.append(Implies(WeakNext(move_vars[i] & move_vars[j]  & Not(Atomic("disallow_%d_%d" % (i, j))))  & left_vars[i] & left_vars[j], WeakNext(And(Not(left_vars[i]), Not(left_vars[j])))))
                move_spec.append(Implies(WeakNext(move_vars[i] & move_vars[j] & (Atomic("disallow_%d_%d" % (i, j)))) & left_vars[i] & left_vars[j], WeakNext(And(left_vars[i], left_vars[j]))))
            else:    
                move_spec.append(Implies(WeakNext(move_vars[i] & move_vars[j]) & left_vars[i] & left_vars[j], WeakNext(And(Not(left_vars[i]), Not(left_vars[j])))))
        init = utils.simple_conj( left_vars, [])                 
        if fullSpec:
            for (i, j) in disallowed_moves:
                init &= Always((Atomic("disallow_%d_%d" % (i, j))))
            for (i, j) in allowed_moves:
                init &= Always(~(Atomic("disallow_%d_%d" % (i, j))))
    return Always(utils.conjunction(move_spec)) & init

# Example usage for n = 3

class SheepGenerator(Generator):
    # ASSUMES: pairs are ordered by numbers i.e. (2, 3) instead of (3, 2)
    def generate(self, n, partial = False, unreasonable_moves = [], reasonable_moves = [], uncertain_backup_goal = []):
        self.output = "sheep-%d" % (n)
        posvars = []
        env_partial_spec = []
        movevars = []

        # Generate input and output variables
        for i in range(n):
            self.outputvars.append(Atomic("move_%d" % (i)))
            movevars.append(self.outputvars[-1])
        for i in range(n):
            self.inputvars.append(Atomic("left_%d" % (i)))
            posvars.append(self.inputvars[-1])
        # Initially true variables
        itrue = []
        # Initially false variables
        ifalse = []
        for i , j in unreasonable_moves + reasonable_moves:
            if not(i < n and j < n):
                continue
            self.inputvars.append(Atomic("disallow_%d_%d" % (i, j)))
            self.partialvars.append(Atomic("disallow_%d_%d" % (i, j)))
            if (i, j) in unreasonable_moves:
                env_partial_spec.append((Atomic("disallow_%d_%d" % (i, j))))
                itrue.append(Atomic("disallow_%d_%d" % (i, j)))
            elif (i, j) in reasonable_moves:
                env_partial_spec.append((Not(Atomic("disallow_%d_%d" % (i, j)))))
                ifalse.append(Atomic("disallow_%d_%d" % (i, j)))
        
        only_two_moved = utils.exactly_k_of_simple(movevars, 2)
        correct_env = generate_env_constraints(n, reasonable_moves, unreasonable_moves)  
        system_constraints =   Always( only_two_moved)
        #print(system_constraints)
        self.formula =  system_constraints & Implies(correct_env , Eventually((utils.simple_conj([], posvars))))
        #print(to_string(self.formula).replace("X", "WX").replace("X[!]", "X"))

        if not partial:
            return 
        
        # Create partial specification 
        subset = []
        for x in uncertain_backup_goal: 
            subset.append(Atomic("left_%d" % (x)))
        backup_goal = Eventually(utils.simple_conj([], subset))
        correct_env = generate_env_constraints(n, reasonable_moves, unreasonable_moves, fullSpec=False) 
        self.backup = system_constraints  &  Implies(  correct_env , backup_goal)
        #self.backup = ( Always((simple_conj([], self.outputvars) | only_two_moved)) & Until( conjunction(player_cannot_move_disallowed) , Eventually(simple_conj(subset, [])) )) & Implies( (correct_env  & init) , Eventually(simple_conj(subset, [])))



g = SheepGenerator()
for i in range(2,9):
    if i == 4:
        g.generate(i, partial=True, unreasonable_moves = [(0,1), (0,2)], reasonable_moves = [(0, 3)], uncertain_backup_goal = [1,2])
        g.write(Syft=True, filename_overwrite = "sheep-4-ext-solv")
        g.generate(i, partial=True, unreasonable_moves = [(0,1), (0,2)], reasonable_moves = [(0, 3)], uncertain_backup_goal = [0,2])
        g.write(Syft=True, filename_overwrite = "sheep-4-ext-unsolv")
        g.generate(i, partial=True, unreasonable_moves = [(0,1), (1,3), (0, 3)], reasonable_moves = [], uncertain_backup_goal = [0,1])
        g.write(Syft=True, filename_overwrite = "sheep-4-unsolv")
        
        g.generate(i, partial=True, unreasonable_moves = [(0,1), (1,2), (2, 3)], reasonable_moves = [(1, 3)], uncertain_backup_goal = [0,2])
        g.write(Syft=True, filename_overwrite = "sheep-4-twotalesa")
        g.generate(i, partial=True, unreasonable_moves = [(0,1), (1,2), (2, 3)], reasonable_moves = [(1, 3)], uncertain_backup_goal = [1,3])
        g.write(Syft=True, filename_overwrite = "sheep-4-twotalesb")

        g.generate(6, partial=True, unreasonable_moves = [(0,1), (1,3), (0, 3)], reasonable_moves = [], uncertain_backup_goal = [4,5])
        g.write(Syft=True, filename_overwrite = "sheep-6-solv")
        g.generate(8, partial=True, unreasonable_moves = [(0,1), (1,3), (0, 3)], reasonable_moves = [], uncertain_backup_goal = [4,5])
        g.write(Syft=True, filename_overwrite = "sheep-8-solv")
        g.generate(10, partial=True, unreasonable_moves = [(0,1), (1,3), (0, 3)], reasonable_moves = [], uncertain_backup_goal = [4,5])
        g.write(Syft=True, filename_overwrite = "sheep-10-solv")
        g.generate(2, partial=True, unreasonable_moves = [], reasonable_moves = [(0, 1)], uncertain_backup_goal = [0, 1])
        g.write(Syft=True, filename_overwrite = "sheep-2-ext-unsolv")
        g.generate(2, partial=True, unreasonable_moves = [], reasonable_moves = [], uncertain_backup_goal = [0, 1])
        g.write(Syft=True, filename_overwrite = "sheep-2-ext-solv")
        g.generate(3, partial=True, unreasonable_moves = [], reasonable_moves = [(0, 1)], uncertain_backup_goal = [0, 1])
        g.write(Syft=True, filename_overwrite = "sheep-3-ext-unsolv")

    #g.generate(i, partial=False)
    #g.write(Syft=True)



    
