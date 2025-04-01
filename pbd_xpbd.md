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
