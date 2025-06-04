# Physically based simulation
Physically based simulation (PBS) is a technique used to simulate the motion of dynamic systems based on physical forces such as gravity or friction. In particular, both external and internal forces acting on a system are computed according to Newton's second law of mation ($F=m \cdot a$) and a time integration method is then used to update the object's velocity and position, resulting in a realistic motion over time.  

# Particle systems
The simplest things to simulate are particles, which can be described simply by their position and velocity:
* $x_p(t)$: position of particle p as a function of time t;
* $v_p(t) = \frac{dx_p(t)}{dt}$: velocity of particle p as a function of time t, which is the derivative of its position with respect to time t;

## Mass-spring system

## Time integration

### Esplicit Euler

### Semi-implicit Euler

### Implicit Euler

#### Fast simulation of mass-spring systems
