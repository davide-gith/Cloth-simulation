# Physically based simulation
Physically based simulation (PBS) is a technique used to simulate the motion of dynamic systems based on physical forces such as gravity or friction. In particular, both external and internal forces acting on a system are computed according to Newton's second law of mation ($F=m \cdot a$) and a time integration method is then used to update the object's velocity and position, resulting in a realistic motion over time.  

# Particle systems
The simplest things to simulate are particles, which can be described simply by their position and velocity:
* $x_p(t)$: position of particle $p$ as a function of time $t$;
* $v_p(t) = \frac{dx_p(t)}{dt}$: velocity of particle $p$ as a function of time $t$, which is the derivative of its position with respect to time;
To predict the motion of a particle, it is necessary to incorporate Newton's second law of motion into the system:
* $f(t) = m \cdot a(t)$;
* $a_p(t) = \frac{dv_p(t)}{dt}$: acceleratio of a particle $p$ as a function of time $t$, which is the derivative of its velocity with respect to time (it's also true that: $a_p(t) = \frac{d^2x_p(t)}{d^2t}$);

So, the problem to solve is to determine the particle position over time given its mass, the forces that act on the particle, its initial position and its initial velocity.


## Mass-spring system

## Time integration

### Esplicit Euler

### Semi-implicit Euler

### Implicit Euler

#### Fast simulation of mass-spring systems
