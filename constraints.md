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



<!--Anche i vestiti si stretchano, ma tipicamente hanno un forte limite di stretch. Si applica una forza che allunga il vestito, ma poi questo stretching non va oltre all'aumentare della forza
La gravità raramente è causa di stretching che si nota
Troppo stretching è un artefatto
Quello che vorremmo simulare è un infinitely stiff material -> use zero compliance distance constraints on cloth mesh edges 
L'unico -->
