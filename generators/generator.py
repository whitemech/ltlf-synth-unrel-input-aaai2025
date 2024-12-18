
from pylogics.syntax.ltl import Formula
from pylogics.utils.to_string import to_string
import utils

class Generator():
    """
    Generator class for the partition file.
    """
    def __init__(self, partial = True):
        self.formula = None
        self.backup = None 
        
        self.inputvars = []
        self.outputvars = [] 
        self.partialvars = []
        
        self.partial = partial 
        

    def write(self, Syft = False, filename_overwrite = None):
        """

        Args:
            Syft (bool, optional): Whether to generate for Syft or SyftMax Syntax. Defaults to False.
            filename_overwrite (_type_, optional): Can be used to overwrite the output filename. Defaults to None.
        """
        if filename_overwrite != None:
            self.output = filename_overwrite
        if self.formula == None or (self.backup == None and self.partial):
            print("Need a formula")
            sys.exit(-1)
        
        #print("Writing the partition file ")
        with open(self.output+".part", "w") as f: 
            print(".inputs: "+" ".join((map(to_string, self.inputvars))), file=f)
            print(".outputs: "+" ".join(map(to_string, self.outputvars)), file=f)
            if self.partial:
                print(".unobservables: "+ " ".join(map(to_string, self.partialvars)), file=f)    
            with open(self.output+".ltlf", "w") as f: 
                if not Syft:
                    print(to_string(self.formula), file=f)
                else:
                    print(utils.toSyftInput(self.formula), file=f)
                if self.partial:
                    print(utils.toSyftInput(self.backup) , file=f)

