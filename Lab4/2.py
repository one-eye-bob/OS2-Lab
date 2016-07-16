#!/bin/python
from z3 import *

#Create goal which 'stores' all constraints
g = Goal()
length = Int('len')
arr = Array('arr', IntSort(), IntSort())

#initialize len and arr
g.add(length == 10 )
for i in xrange(10):
    pass

oldRes = Int('res0')
g.add(oldRes == 0)

for i in xrange(10):

    newI = Int ('i' + str(i))
    newRes = Int('res' + str(i+1))

    g.add(And(newRes == oldRes + arr[newI], newI == i+1 ))    
    g.add(arr[newI] == newI)
    
    
    oldI    = newI
    oldRes = newRes 

#add assertion, 
#g.add(10*(10+1)/2 == oldRes)
print g.as_expr()
solve(And (length + (length+1)/2 == oldRes, Not(g.as_expr())))
