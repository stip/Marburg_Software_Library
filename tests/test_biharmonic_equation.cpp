//#define _WAVELETTL_GALERKINUTILS_VERBOSITY 2

// choose which basis to use
#define BASIS_S 0
#define BASIS_P 1
#define BASIS_DS 2

#define BASIS BASIS_DS


#include <iostream>

#include <algebra/infinite_vector.h>
#include <algebra/vector_norms.h>
#include <algebra/matrix.h>
#include <algebra/sparse_matrix.h>
#include <algebra/matrix_norms.h>
#include <utils/random.h>
#include <ctime>

#if (BASIS == BASIS_S)
 #include <interval/s_basis.h>
 #include <interval/s_support.h>
 #include <interval/interval_evaluate.h>
 typedef WaveletTL::SBasis Basis;
 #define BASIS_NAME "s"
#elif (BASIS == BASIS_P)
 #include <interval/p_basis.h>
 #include <interval/p_support.h>
 #include <interval/p_evaluate.h>
 typedef WaveletTL::PBasis<4, 4> Basis;
 #define BASIS_NAME "p"
#elif (BASIS == BASIS_DS)
 #include <interval/ds_basis.h>
 #include <interval/ds_support.h>
 #include <interval/ds_evaluate.h>
 typedef WaveletTL::DSBasis<4, 4> Basis;
 #define BASIS_NAME "ds"
#endif

#include <numerics/bvp.h>
#include <galerkin/galerkin_utils.h>
#include <galerkin/biharmonic_equation.h>
#include <numerics/eigenvalues.h>
#include <numerics/iteratsolv.h>

using namespace std;
using namespace WaveletTL;
using namespace MathTL;


int main()
{
  cout << "Testing biharmonic equation ..." << endl;
  #if (BASIS == BASIS_S)
  Basis basis;
  #else
  Basis basis(2, 2);
  #endif
  Basis::Index lambda;
  set<Basis::Index> Lambda;
  
  int jmax = 12; //basis.j0()+3;
  for (lambda = basis.first_generator(basis.j0()); lambda <= basis.last_wavelet(jmax); ++lambda)
    Lambda.insert(lambda);

  Vector<double> value(1);
  value[0] = 384;
  ConstantFunction<1> const_fkt(value);
//  BiharmonicBVP<1> biharmonic(&const_fkt);
  BiharmonicEquation1D<Basis> discrete_biharmonic(basis, &const_fkt);

  cout << "Setting up full stiffness matrix ..." << endl;
  SparseMatrix<double> stiff(Lambda.size());

  setup_stiffness_matrix(discrete_biharmonic, Lambda, stiff, true);
//  cout << stiff << endl;
  cout << "Saving stiffness matrix to " << BASIS_NAME << "_stiff.m" << endl;
  ostringstream filename;
  filename << BASIS_NAME << "_stiff";
  stiff.matlab_output(filename.str().c_str(), "A", 1);
//  cout << "kappa = " << MathTL::CondSymm(stiff) << endl;
  double lambdamin, lambdamax;
  unsigned int iterations;
  LanczosIteration(stiff, 1e-6, lambdamin, lambdamax, 200, iterations);
  cout << "Extremal Eigenvalues of stiffness matrix: " << lambdamin << ", " << lambdamax << "; " << iterations << " iterations needed." << endl;
  cout << "Condition number of stiffness matrix: " << lambdamax/lambdamin << endl;

  cout << "Setting up full right-hand side ..." << endl;
  Vector<double> rh;
  setup_righthand_side(discrete_biharmonic, Lambda, rh);
//  cout << rh << endl;

  cout << "Computing solution ..." << endl;
  Vector<double> x(rh);
  CG(stiff, rh, x, 1e-4, 200, iterations);
  cout << "Solved linear equation system in " << iterations << " iterations. " << endl;
//  cout << "Solution:" << endl;
  InfiniteVector<double,Basis::Index> u;
  unsigned int i = 0;
  for (set<Basis::Index>::const_iterator it = Lambda.begin(); it != Lambda.end(); ++it, ++i)
    u.set_coefficient(*it, x[i]);
  u.scale(&discrete_biharmonic, -1); // undo preconditioning

  cout << "Writing solution to file biharmonic-solution.dat ..." << endl;
  SampledMapping<1> s(evaluate(basis, u, true, 7));
  std::ofstream fs("biharmonic-solution.dat");
  s.gnuplot_output(fs);
  fs.close();

  cout << "Maximal difference to exact solution: ";
  Polynomial<double> solution(Vector<double>(5, "0 0 16 -32 16")); // 16 x^2 (1-x)^2 = 16 (x^2 - 2 x^3 + x^4)
  SampledMapping<1> difference(Grid<1>(s.points()), solution);
  difference.add(-1.0,s);
  Array1D<double> err(difference.values());
  cout << linfty_norm(err) << endl;

  return 0;
}
