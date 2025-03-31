from sympy.integrals import laplace_transform
from sympy import sin 
from sympy.abc import t, s, a
 
gfg = laplace_transform(sin(a*t), t, s)
 
print(gfg)