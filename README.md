# 👕 Cloth Simulation
This repository implements a **cloth simulator** built in C++ and OpenGL, combining both **physically-based** and **position-based** simulation techniques.  
The cloth interacts with a variety of colliders, including **spheres**, **boxes**, **capsules**, **swept-sphere lines**, and **swept-sphere triangles**, allowing the study of complex fabric dynamics in different scenarios.

## 📖 Documentation
Theoretical details and notes are provided in the [Doc](./Doc) folder:  
- [Physically-based simulation](./Doc/Phisycally_based.md)
- [Position-based dynamics](./Doc/pbd_xpbd.md)  
- [Constraints](./Doc/constraints.md)  

## 📂 Assets
Assets like meshes and materials are available at this link: https://univr-my.sharepoint.com/:f:/g/personal/davide_garavaso_studenti_univr_it/Ei4lNXu88ctCsmt0umZpFS4BCHFwZbXAy8eCDnDObxTfRA?e=CJEwc6

## 🎥 Demo
- [Cloth Simulation Algorithms (Explicit Euler, Symplectic Euler, PBD, XPBD)](https://www.youtube.com/watch?v=ohieZQnSpEU)  
- [Cloth Simulation Collisions (sphere, capsule, swept-sphere line & triangle)](https://www.youtube.com/watch?v=yGHgXt2FPfw)


Esplicit Euler [Press et al. (2007)]:
```
@book{press2007numerical,
  title     = {Numerical Recipes: The Art of Scientific Computing},
  author    = {Press, William H. and Teukolsky, Saul A. and Vetterling, William T. and Flannery, Brian P.},
  year      = {2007},
  edition   = {3},
  publisher = {Cambridge University Press},
  address   = {New York, NY, USA},
  isbn      = {978-0521880688}
}
```

Semi-implicit Euler [Baraff and Witkin (1998)]:
```
@inproceedings{baraff1998large,
  title     = {Large Steps in Cloth Simulation},
  author    = {Baraff, David and Witkin, Andrew},
  booktitle = {Proceedings of the 25th Annual Conference on Computer Graphics and Interactive Techniques (SIGGRAPH 1998)},
  pages     = {43--54},
  year      = {1998},
  publisher = {ACM},
  doi       = {10.1145/280814.280821}
}
```

Fast simulation of mass-spring systems [Liu et al. (2013)]:
```
@article{liu2013fast,
  title     = {Fast Simulation of Mass-Spring Systems},
  author    = {Liu, Tiantian and Bargteil, Adam and O'Brien, James and Kavan, Ladislav},
  journal   = {ACM Transactions on Graphics (TOG)},
  volume    = {32},
  number    = {6},
  pages     = {1--7},
  year      = {2013},
  publisher = {ACM},
  doi       = {10.1145/2508363.2508410}
}
```
