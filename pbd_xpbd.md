# PBD/XPBD
Position-Based Dynamics (PBD) is a simulation technique that, unlike traditional physics simulations, which focus on forces and accelerations, directly manipulates the positions of particles in the simulation to satisfy constraints, ensuring realistic behavior without the need for expensive computations like solving differential equations.  
This approach is particularly efficient and stable because:
- works directly with positions;
- collisions are easily solvable. They are treated as constraints $C(p)>=0$, kinematic restrictions where particles are moved to valid positions;
- it solves the problem of excessive energy buildup in the system that causes it to explode;
- is closely related to implicit backward Euler integration schemes.

## Algorithm
The objects to be simulated are represented by a set of particles with position $x_i$ and velocity $v_i$ and a set of constraints with a stiffness parameter $k \in [0, 1]$ that defines the strength of the constraint. The simulation proceeds with a fixed timestep $\Delta t$, usually equal to $\frac{1}{60}$, as described by the following algorithm:
```
0) init data
1) simulation loop

2)   for all particles i
3)     v_i = v_i + dt * gravity
4)     prev_i = x_i
4)     x_i = x_i + dt * gravity

5)   for n interations
6)     for all constraints C
7)       for all particles i of C
8)         compute dx_i
9)         x_i = x_i + dx_i

10)  for all particles i
11)    v_i = (x_i - prev_i)\dt
```
Lines 2 to 4 perform a symplectic Euler integration step on the velocities and the positions, and the current pos $x_i$ is assigned to the previous position. The positions $x_i$ are not the final positions, but rather predictions that will be corrected to satisfy the constraint.  
The solver, in lines 5 to 9, iteratively corrects the predicted positions by resolving all constraints and computing the $\Delta x_i$ correction vector. In cases where the constraints are numerous, solving them one at a time per timestep will result in overly elastic objects; for this reason, the constraints are solved *n* times per timestep.  
Line 10 to 11 are used to update the positions and the velocities.  
This integretion system is very similar to the Verlet method.  

Instead of solving the same constraint multiple times, it is more effective to perform multiple substeps and solve each constraint only once, as this speeds up convergence.  
Therefore, we can rewrite the algorithm as follows:
```
0) init data with dt_s = dt/n
1) simulation loop
2)   for n substeps

3)     for all particles i
4)       v_i = v_i + dt_s * gravity
5)       prev_i = x_i
6)       x_i = x_i + dt_s * gravity

7)       for all constraints C
8)         for all particles i of C
9)           compute dx_i
10)          x_i = x_i + dx_i

11)    for all particles i
12)      v_i = (x_i - prev_i)\dt_s
```

## Solve general constraint
The goal of the solver is to correct the predicted positions of the particles such that they satisfy all constraints. More specifically, a general constraint *C* is a function that takes as input all the particles participating in the constraint and returns a constraint error c. The problem here is that even a simple distance constraint yelds a non-linear equation, making the resulting system of equations non-linear.  
So, PBD borrows the idea from the Gauss-Seidel algorithm (which can only handle linear systems) of solving each constraint independently, one after the other. In this way, the particles are projected into valid positions with respect to the given constraint alone. The fact that each constraint is linearized individually before its projection makes the solver more stable.  

In the solver, given *x*, it is necessary to find the correction vector $\Delta x$ such that $C(\Delta x + x) = 0$.  
If this vector is restricted to be in the direction of $\nabla C$ (which is also a requirement for linear and angular momentum conservation), it means that only a scalar $\lambda$ needs to be found to compute the correction $\Delta x$: 
<p align="center">
$\lambda = \frac{-C(x)}{w_1 \cdot \lVert \nabla C_1 \rVert^2 + w_2 \cdot \lVert \nabla C_2 \rVert^2 + ... + w_n \cdot \lVert \nabla C_n \rVert^2}$<p\>
<p align="center">
$\Delta x =\lambda w \nabla C$  
</p>  

$\lambda$ is the same for each particle participating in the constraint and $w$ is the inverse of the mass because, if a point does not move, it has infinite mass and $w=0$.

## Stiffness
The stiffness parameter $k$ can be included in the correction vectot, resulting in a hard-constraint if $k=1$, in a soft-constraint if $k \in (0, 1)$ and no-constraint if $k=0$:
<p align="center">
$\Delta x =k \cdot \lambda w \nabla C$  
</p>  

This is a simple approach, but for multiple iteration loops of the solver, the effect of $k$ is non-linear. The remaining error for a single distance constraint after $n_s$ solver iterations is $\Delta x(1-k)^{n_s}$ so, to get a linear relationship can be incorporated $k'=1-(1-k)^{1/n_s}$. Now the error becomes $\Delta x(1-k')^{n_s}=\Delta x(1-k)$ and becomes linearly dependent on $k$ and indipendent of $n_s$.  
However, the resulting system is still dependent on the timestep of the simulation, producing increasingly rigid systems with smaller timesteps.








<!--\begin{algorithm}
\caption{Calcolo del fattoriale}
\begin{algorithmic}[1]
\REQUIRE $n \geq 0$
\ENSURE $n!$
\IF{$n = 0$}
    \RETURN $1$
\ELSE
    \RETURN $n \times$ \textbf{factorial}($n-1$)
\ENDIF
\end{algorithmic}
\end{algorithm}
<span style="color:red">Testo in rosso</span>

$$ F = m \cdot a $$

La velocità è definita come **$v = \frac{\Delta x}{\Delta t}$**.

(1) for each frame  
(2)     for all particles *i*  
(3)         $v_t$ $\leftarrow$
    

-->
