#ifndef HEADERS_HPP
#define HEADERS_HPP

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <vector>
#include <algorithm>
#include <fstream>
#include <Eigen/Eigen>
#include <mpi.h>
#include <unistd.h>
#define WIDTH 15 

using namespace std;

//================================================================================
// SPECIAL TYPES
//================================================================================

// Types for dense storage
typedef Eigen::Matrix<double, Eigen::Dynamic, 1> Vector;
typedef Eigen::Matrix<int,    Eigen::Dynamic, 1> IntVector;
typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic> Matrix;
typedef Eigen::Matrix<int,    Eigen::Dynamic, Eigen::Dynamic> IntMatrix;

// Type for sparse storage
typedef Eigen::SparseMatrix<double> SpMatrix;

// Type for diagonal matrix
typedef Eigen::DiagonalMatrix<double, Eigen::Dynamic> DiagMatrix;

// Stucture for mesh
struct Mesh
{
  // Infos about nodes
  int nbOfNodes;  // number of nodes
  Matrix coords;  // coordinates of each node  (Size: nbOfNodes x 3)
  
  // Infos about elements
  int nbOfLin;                // number of lines (belonging to the boundary of the domain)
  int nbOfTri;                // number of triangles
  IntVector linNum;           // gmsh numerbing of lines      (Size: nbOfLin)
  IntVector triNum;           // gmsh numerbing of triangles  (Size: nbOfTri)
  IntMatrix linNodes;         // nodes to each lines          (Size: nbOfLin x 2)
  IntMatrix triNodes;         // nodes to each triangle       (Size: nbOfTri x 2)
  IntVector linPart;          // partition of lines           (Size: nbOfLin)
  IntVector triPart;          // partition of triangles       (Size: nbOfTri)

  // Infos for parallel computations
  int       numNodesPart;     // number of nodes belonging to the local part
  IntVector nodesPart;        // list of nodes belonging to the local part                                         (Size: numNodesPart )
  IntVector numNodesToExch;   // number of nodal values to exchanges between the current proc and each other proc  (Size: nbTasks)
  IntMatrix nodesToExch;      // list of nodal values to exchanges between the current proc and each other proc    (Size: nbTasks x max(numNodesToExch) )
  Vector nodesToCompute;      // list of nodal values to compute for L2 error
};

// Stucture for problem
struct Problem
{
  SpMatrix K;   // stiffness matrix
  SpMatrix M;   // mass matrix
  SpMatrix A;   // system matrix
  Vector b;     // RHS
};


// Right Hand Term types

enum F_type {
    CSTE = 0,
    COSCOS = 1
};

enum Print_type {
    PRINT = 1,
    NO_PRINT = 2
};

enum Solver_type {
    JACOBI = 0,
    CONJUGATEGRADIENT = 1
};

//================================================================================
// FUNCTIONS
//================================================================================

//==== Functions in 'mesh.cpp'

// Read the mesh from a gmsh-file (.msh) and store in a mesh-structure 'm'
void readMsh(Mesh& m, string fileName);

// Save a solution 'u' in in mesh file (.msh)
void saveToMsh(Vector& u, Mesh& m, string name, string fileName);

//==== Functions in 'parallel.cpp'

// Build the list of nodes for MPI communications
void buildListsNodesMPI(Mesh& m);

// Build the local numbering of nodes belonging to the current MPI process
void buildLocalNumbering(Mesh& m);

// MPI-parallel exchange/add the interface terms
void exchangeAddInterfMPI(Vector& vec, Mesh& m);

// Compute the local L2 error
double computeDotProd(const Vector& uNum, const Vector& uNum2, const Mesh& m, int print_types);
double computeL2Err(const Vector& uNum, const Vector& uExa, const Mesh& m, int print_types);

// Send res to 0
void sendResTo0(double res2);

// Get and sum res from other procs
void getAndSumRes(double& res2, Vector& otherRes,
		  std::vector<MPI_Request>& AllRequests);
//==== Functions in 'problem.cpp'

// Compute the matrices of the linear system
void buildLinearSystem(Problem& p, Mesh& m, double alpha, Vector& f);

// Special treatment of the matrices for a Dirichlet boundary condition
void buildDirichletBC(Problem& p, Mesh& m, Vector& uExa);

//==== Functions in 'solver.cpp'

// Solution of the system Au=b with Jacobi
void jacobi(SpMatrix& A, Vector& b, Vector& u, Mesh& m,
	    double tol, int totNbOfNodes, int maxit);
void conjugateGradient(SpMatrix& A, Vector& b, Vector& u, Mesh& m,
		       double tol, int totNbOfNodes, int maxit);
#endif /* HEADERS_HPP */
