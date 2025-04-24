# CONSTRAINTS

## Distance constraint (stretching)
Distance constraint is one of the most fundamental types of constraints, used to enforce that two particles remain at a fixed distance from each other. Given two particles $x_1$ and $x_2$, the distance constraint ensures the current distance $l_c$ between them remains equal to some rest length $l_r$. So the distance constraint function is defined as: 
<p align="center">
$C(x_1, x_2) = \lVert x_1 - x_2 \rVert - l_r = l_c - l_r = 0$.
</p>

Knowing that $\nabla C_1$ and $\nabla C_2$ are:
<p align="center">
$\nabla C_1 = \frac{x_2 - x_1}{\lVert x_2 - x_1 \rVert}$
</p>
<p align="center">
$\nabla C_2 = -\frac{x_2 - x_1}{\lVert x_2 - x_1 \rVert}$
</p>
  
and that $\lVert \nabla C_1 \rVert = 1$ $\lVert \nabla C_2 \rVert = 1$, $\lambda$ and the correction vectors $\Delta x_1$ and $\Delta x_2$ are calculated as:
<p align="center">
$\lambda = \frac{-(l_c - l_r)}{w_1 \cdot 1 + w_2 \cdot 1}$
</p>
  
<p align="center">
$\Delta x_1 =\lambda w_1\ \nabla C_1 = \frac{w_1}{w_1+w_2} \cdot (l_c - l_r) \cdot \frac{x_2 - x_1}{\lVert x_2 - x_1 \rVert}$  
</p>  

<p align="center">
$\Delta x_2 =\lambda w_2\ \nabla C_2 = -\frac{w_2}{w_1+w_2} \cdot (l_c - l_r) \cdot \frac{x_2 - x_1}{\lVert x_2 - x_1 \rVert}$  
</p>

In the specific case of clothing, stretching needs to be handled in a particular way. Specifically:
- gravity rarely causes significant stretching;
- clothes stretch up to a certain limit, but beyond that, they no longer stretch despite increasing force;
- excessive stretching is an artifact.
What we want to reproduce, then, is an infinitely stiff material, which we can achieve by setting zero compliance distance constraints on the edges of the cloth.

## Bending constraint

