mechanoChem: Modeling solid mechanics, chemistry, and their interactions

Developed by the Computational Physics Group at the University of Michigan. 
http://www.umich.edu/~compphys/index.html

List of contributors:
Greg Teichert (Lead Developer)
Shiva Rudraraju
Krishna Garikipati

<B>Code documentation:</B> https://goo.gl/VNAt2Y <br>

Overview
=======================================================================
The mechanoChem code is an isogeometric analysis based code used to solve the partial differential equations describing solid mechanics (including gradient elasticity) and chemistry (including the Cahn-Hilliard phase field model). It is built on the PetIGA [https://bitbucket.org/dalcinl/petiga/] and PETSc [https://www.mcs.anl.gov/petsc/] libraries, and it uses the automatic differentiation capabilities of the Sacado package from the Trilino library [https://trilinos.org/packages/sacado/].


Version information
=======================================================================
This is version 0.1, the intial release of the code.


License
=======================================================================
GNU Lesser General Public License (LGPL). Please see the file LICENSE for details.


Acknowledgements
=======================================================================
This code has been developed under the support of the following:

NSF DMREF grant: DMR1436154 "DMREF: Integrated Computational Framework for Designing Dynamically Controlled Alloy-Oxide Heterostructures"
NSF CDI Type I grant: CHE1027729 "Meta-Codes for Computational Kinetics"
DOE BES, Division of Materials Sciences and Engineering: Award #DE-SC0008637 that funds the PRedictive Integrated Structural Materials Science (PRISMS) Center at University of Michigan


Referencing this code
=======================================================================
If you write a paper using results obtained with the help of this code,  please consider citing one or more of the following:

"A variational treatment of material configurations with application to interface motion and microstructural evolution" (under review)
G. Teichert, S. Rudraraju, K. Garikipati 

@Article{Teichert2016a,
  Title                    = {A variational treatment of material configurations with application to interface motion and microstructural evolution},
  Author                   = {G. Teichert and S. Rudraraju and K. Garikipati},
  Journal                  = {ArXiv e-prints},
  archivePrefix            = "arXiv",
  eprint                   = {1608.05355},
  Year                     = {2016},
  Month                    = {Jul},
  Url                      = {https://arxiv.org/abs/1608.05355}
}

"A comparison of Redlich-Kister polynomial and cubic spline representations of the chemical potential in phase field computations" (under review)
G. Teichert, H. Gunda, S. Rudraraju, A. Natarajan, B. Puchala, K. Garikipati, A. Van der Ven

@Article{Teichert2016b,
  Title                    = {A comparison of Redlich-Kister polynomial and cubic spline representations of the chemical potential in phase field computations},
  Author                   = {G. Teichert and N. S. H. Gunda and S. Rudraraju and A. R. Natarajan and B. Puchala and A. Van der Ven and K. Garikipati},
  Journal                  = {ArXiv e-prints},
  archivePrefix            = "arXiv",
  eprint                   = {1609.00704},
  Year                     = {2016},
  Month                    = {Aug},
  Url                      = {https://arxiv.org/abs/1609.00704}
}

"Mechano-chemical spinodal decomposition: A phenomenological theory of phase transformations in multi-component, crystalline solids" (Nature npj Computational Materials)
S. Rudraraju, A. Van der Ven, K. Garikipati 

@Article{Rudraraju2016,
  Title                    = {Phenomenological treatment of chemo-mechanical spinodal decomposition},
  Author                   = {S. Rudraraju and A. Van der Ven and K. Garikipati},
  Journal                  = {npj Computational Materials},
  Year                     = {2016},
  Volume                   = {2},
  Doi                      = {10.1038/npjcompumats.2016.12}
}


Installation
=======================================================================
1) Install PETSc:

-Download and extract PETSc source code.
-Quick installation as follows (the symbol $ denotes the command prompt):

	$ ./configure --with-cc=gcc --with-cxx=g++ --with-fc=gfortran --download-fblaslapack --download-mpich
	$ make all test

-Set the appropriate PETSC_DIR and PETSC_ARCH environment variables, e.g.

	$ export PETSC_DIR=/path/to/petsc-3.6.4
	$ export PETSC_ARch=arch-linux2-c-debug

Note that this also installs mpich. It is possible to use an existing version of mpi by including a flag to its directory. If you will be using the local PETSc installation of mpich, set the following:

	$ alias mpirun=$PETSC_DIR/$PETSC_ARCH/bin/mpirun

If you would like to use a parallel direct solver, we recommend using superlu_dist. Add the following flags when configuring PETSc: --download-metis --download-parmetis --download-superlu_dist

Download: http://www.mcs.anl.gov/petsc/download/index.html
Installation instructions: http://www.mcs.anl.gov/petsc/documentation/installation.html


2) Install PetIGA:

-Download and extract or clone the PetIGA source code.

-Enter the PetIGA top directory and install using make:

	$ make all
	$ make test

-Export path to PetIGA directory:

	$ export PETIGA_DIR=/path/to/petiga/

Download: https://bitbucket.org/dalcinl/petiga/downloads (or clone the bitbucket repository)
Installation instructions: https://bitbucket.org/dalcinl/petiga/


3) Install CMake:

Download: https://cmake.org/download/


4) Install the Sacado package from Trilinos (version 11.10.2 recommended)

-Download and extract or clone the Trilinos source code.
-Simple installation of Sacado as follows (from the Trilinos top directory), with the desire /path/to/trilinos/installation/:

	$ mkdir build
	$ cd build
	$ cmake -DTrilinos_ENABLE_Sacado=ON -DTrilinos_ENABLE_Teuchos=OFF -DCMAKE_INSTALL_PREFIX=/path/to/trilinos/installation/ ../
	$ make install

-Export path to Trilinos:

	$ export TRILINOS_DIR=/path/to/trilinos/installation/

Download: http://trilinos.org/oldsite/download/
Installation instructions: https://trilinos.org/docs/files/TrilinosBuildReference.html


Usage
=======================================================================
To run the example initial boundary value problems, navigate to the desired example folder. Create a makefile using cmake:

	$ cmake CMakeLists.txt

To compile the code (default is debug mode):

	$ make

To switch the compilation to release mode:

	$ make release

To switch the compilation to debug mode:

	$ make debug

To run the code (replace "nproc" with the number of processors to be use), with some recommended flags:

	$ mpirun -np nproc ./main -ts_monitor -snes_type newtontr -ksp_type fgmres
