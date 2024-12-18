import sys 
if len(sys.argv) != 3:
    print("Expected two arguments")
    sys.exit(1)

input = sys.argv[1]
partfile = sys.argv[2]
# Ignore the line that has "unobservables".
newlines = []
variables = []
with open(partfile, "r") as partf:
    l = partf.readlines()
    for nl in l:
        if "unobservable" in nl:
            variables = nl.strip().split(" ")[1:]
        else:
            newlines.append(nl)
forma = None 
formb = None 
with open(input, "r") as inputf:
    forma = inputf.readline().strip()
    formb = inputf.readline().strip()

newform = forma + " && "
for x in variables:
    newform += "(ALL "+x
newform += " "+formb
for x in variables:
    newform += ") "

with open("out.qltlf", "w") as formfile:
    print(newform, file = formfile)

with open("out.part", "w") as partfile:
    partfile.writelines(newlines)