# Physically based simulation
Physically based simulation (PBS) is a technique used to simulate the motion of dynamic systems based on physical forces such as gravity or friction. In particular, both external and internal forces acting on a system are computed according to Newton's second law of mation ($F=m \cdot a$) and a time integration method is then used to update the object's velocity and position, resulting in a realistic motion over time.  

# Particle systems
The simplest things to simulate are particles, which can be described simply by their position and velocity:
* $x_p(t)$: position of particle $p$ as a function of time $t$;
* $v_p(t) = \frac{dx_p(t)}{dt}$: velocity of particle $p$ as a function of time $t$, which is the derivative of its position with respect to time;

To predict the motion of a particle, it is necessary to incorporate Newton's second law of motion into the system:
* $f_p(t) = m_p \cdot a_p(t)$;
* $a_p(t) = \frac{dv_p(t)}{dt}$: acceleration of a particle $p$ as a function of time $t$, which is the derivative of its velocity with respect to time (it's also true that: $a_p(t) = \frac{d^2x_p(t)}{d^2t}$);

So, the problem to solve is to determine the particle position over time given its mass, the forces that act on the particle, its initial position and its initial velocity.

## Mass-spring system
In addition to gravity, which is constantly applied to the system, additional internal and external forces influence the motion of the particles.  
Most real word object are kept together by internal forces that prevent objects from breaking under external forces. One simple approximation often used in graphics is to model internal forces as stiff springs that connect particles. The spring force is applied along the direction of the spring and is proportional to the difference between the particle distance and the spring rest length (linear force).
<p align="center">
  $f_i(x_{p_i}, x_{p_j}) = k(\lVert x_{p_i} - x_{p_j} \rVert- r)\frac{x_{p_i} - x_{p_j}}{\lVert x_{p_i} - x_{p_j} \rVert}$  
</p>

<p align="center">
$f_j(x_{p_j}, x_{p_i}) = -f_i(x_{p_i}, x_{p_j})$
</p>

where:
* $k$: spring coefficient;
* $\lVert x_{p_i} - x_{p_j} \rVert$: particle distance;
* $r$: rest length;
* $\frac{x_{p_i} - x_{p_j}}{\lVert x_{p_i} - x_{p_j} \rVert}$: force direction.

Like this, depends on the distance between particles, so depends on how dense is the object. It's possible to rewrite the spring force normalized in this way:  
<p align="center">
  $f_i(x_{p_i}, x_{p_j}) = k_s \left(\frac{\lVert x_{p_i} - x_{p_j} \rVert}{r} - 1 \right) \frac{x_{p_i} - x_{p_j}}{\lVert x_{p_i} - x_{p_j} \rVert}$
</p>

where:
* $k_s$: normalized spring coefficient;
* $\frac{\lVert x_{p_i} - x_{p_j} \rVert}{r}$: normalized distance;
* $\frac{x_{p_i} - x_{p_j}}{\lVert x_{p_i} - x_{p_j} \rVert}$: force direction.

To simulate loss of energy due to friction, it's possiible to include a damping term that decrease the particle velocity along the spring direction:
<p align="center">
  $f_i(x_{p_i}, x_{p_j}, v_{p_i}, v_{p_j}) = k_d \left(\frac{\lVert v_{p_i} - v_{p_j}\rVert}{r} \cdot \frac{x_{p_i} - x_{p_j}}{\lVert x_{p_i} - x_{p_j} \rVert} \right)\frac{x_{p_i} - x_{p_j}}{\lVert x_{p_i} - x_{p_j} \rVert}$
</p>

where:
* $\frac{\lVert v_{p_i} - v_{p_j}\rVert}{r}$: relative velocity;
* $\frac{x_{p_i} - x_{p_j}}{\lVert x_{p_i} - x_{p_j} \rVert}$: force direction.

## Time integration
Sicne the position of particles is exrpessed as a function of time, we obtain first-order differential equations (ODE).  
To solve this problem we discretize time at fixed, small (the smallest possible), intervals and write discrete approximations of the equations of motions:
* $v_p(t) = \frac{dx_p(t)}{dt} = \lim_{\Delta t \to 0} \frac{x_p(t+ \Delta t) - x_p(t)}{\Delta t}$;
* $a_p(t) = \frac{dv_p(t)}{dt} = \lim_{\Delta t \to 0} \frac{v_p(t+ \Delta t) - v_p(t)}{\Delta t}$.

From these equations, we derive approximate updates for position and velocity, depending on the integration scheme used (like explicit Euler, semi-implicit Euler and implicit Euler).

### Explicit Euler
Explicit Euler is a technique in which the update of the system's state at time $t + \Delta t$ is determined solely in terms of quantities computed at time $t$.
<p align="center">
  $x_p(t+ \Delta t) = x_p(t) + v_p(t) \cdot \Delta t$
</p>

<p align="center">
  $v_p(t+ \Delta t) = v_p(t) + a_p(t) \cdot \Delta t = v_p(t) + \frac{f_p(t)}{m_p \cdot \Delta t}$
</p>

Advantages:
* simple to implement;
* computationally lightweight and fast.

Disadvantages:
* low numerical stability, especially with large $\Delta t$ or very stiff systems;
* low accuracy;
* does not conserve energy.


### Semi-implicit Euler
Semi-implicit Euler is another time discretization method (the most commonly used) in which the position of the particles is evaluated using the velocity at time $t + \Delta t$. For this reason, the velocity is first computed and then used within the equation for updating the position of the particles.

<p align="center">
  $v_p(t+ \Delta t) = v_p(t) + a_p(t) \cdot \Delta t = v_p(t) + \frac{f_p(t)}{m_p \cdot \Delta t}$
</p>

<p align="center">
  $x_p(t+ \Delta t) = x_p(t) + v_p(t + \Delta t) \cdot \Delta t$
</p>

Advantages:
* numerically more stable than the explicit method;
* simple to implement.

Disadvantages:
* not as accurate as implicit methods;
* does not conserve energy well, but performs better than the explicit method.

### Implicit Euler
Implicit Euler is a technique in which the update of the system's state at time $t + \Delta t$ is determined in terms of quantities computed at time $t + \Delta t$, Which are not yet known. This creates a complex non-linear system that must be solved at each timestep (for example, using Newton's method).

<p align="center">
  $v_p(t+ \Delta t) = v_p(t) + \frac{f_p(t+ \Delta t)}{m_p \cdot \Delta t}$
</p>

<p align="center">
  $x_p(t+ \Delta t) = x_p(t) + v_p(t + \Delta t) \cdot \Delta t$
</p>

Advantages:
* numerically stable, even with large timesteps;
* accurate and suitable for long-term simulations;

Disadvantages:
* complex to implement because it requires solving a system;
* computationally expensive and slow;

#### Fast simulation of mass-spring systems
