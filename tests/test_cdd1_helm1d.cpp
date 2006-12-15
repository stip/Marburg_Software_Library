#define _WAVELETTL_GALERKINUTILS_VERBOSITY 0
#define _WAVELETTL_CDD1_VERBOSITY 1

#include <iostream>
#include <map>
#include <time.h>

#include <algebra/infinite_vector.h>
#include "helmholtz_1d_solutions.h"

#include <galerkin/helmholtz_equation.h>
#include <galerkin/cached_problem.h>
#include <interval/spline_basis.h>
#include <interval/p_expansion.h>
#include <interval/spline_expansion.h>
#include <adaptive/cdd1.h>

using namespace std;
using namespace WaveletTL;

using MathTL::CG;

int main()
{
  cout << "Testing adaptive wavelet-Galerkin methods for the 1D Helmholtz equation ..." << endl;

  const int d  = 3;
  const int dT = 3;

  typedef SplineBasis<d,dT,P_construction> Basis;
  typedef Basis::Index Index;

  Basis basis("",1,1,0,0); // PBasis, complementary b.c.'s

  const unsigned int solution = 1;
  double kink = 0; // for Solution3;

  Function<1> *uexact = 0, *f = 0;
  switch(solution) {
  case 1:
    uexact = new Solution1();
    f = new RHS1();
    break;
  case 2:
    uexact = new Solution2();
    f = new RHS2();
    break;
  case 3:
    kink = 5./7.;
    uexact = new Solution3(kink);
    f = new RHS3_part(kink);
    break;
  default:
    break;
  }

  const int jmax = 9;

  typedef HelmholtzEquation1D<d,dT> Problem;
  Problem problem(basis, 1.0, InfiniteVector<double,Index>());

  InfiniteVector<double,Index> fcoeffs;
  expand(f, basis, true, jmax, fcoeffs);
  problem.set_rhs(fcoeffs);

//   CachedProblem<Problem> cproblem(&problem);

//   // initialization with some precomputed DSBasis eigenvalue bounds:
// //   CachedProblem<IntervalGramian<Basis> > cproblem(&problem, 1.4986, 47.7824); // d=2, dT=2 (no precond.)
// //   CachedProblem<IntervalGramian<Basis> > cproblem(&problem, 1.28638, 705.413); // d=3, dT=3 (no precond.)

//   // initialization with some precomputed PBasis eigenvalue bounds:
//   CachedProblem<IntervalGramian<Basis> > cproblem(&problem, 0.928217, 24.7998); // d=2, dT=2 (no precond.)
// //   CachedProblem<IntervalGramian<Basis> > cproblem(&problem, 0.978324, 19.8057); // d=2, dT=4 (no precond.)

//   double normA = problem.norm_A();
//   double normAinv = problem.norm_Ainv();

//   cout << "* estimate for normA: " << normA << endl;
//   cout << "* estimate for normAinv: " << normAinv << endl;

  InfiniteVector<double, Index> u_epsilon;
  CDD1_SOLVE(problem, 1e-4, u_epsilon, jmax);
//   CDD1_SOLVE(cproblem, 1e-4, u_epsilon, jmax);
//   CDD1_SOLVE(cproblem, 1e-6, u_epsilon, jmax);
// //   CDD1_SOLVE(cproblem, 1e-7, u_epsilon, 12);
// //   CDD1_SOLVE(cproblem, 1e-5, u_epsilon, 20);
// //   CDD1_SOLVE(cproblem, 1e-10, u_epsilon, 20);
// //   CDD1_SOLVE(cproblem, 1e-4, u_epsilon, 20);
// //   CDD1_SOLVE(cproblem, 1e-2, u_epsilon, 10);
// //   CDD1_SOLVE(cproblem, 1e-4, u_epsilon, 10);
// //   CDD1_SOLVE(cproblem, 1e-4, u_epsilon);

  // apply D^{-1} to obtain L_2 wavelet coefficients
  u_epsilon.scale(&problem, -1);

  // insert coefficients into a dense vector
  Vector<double> wcoeffs(problem.basis().Deltasize(jmax+1));
  for (InfiniteVector<double,Index>::const_iterator it(u_epsilon.begin()),
	 itend(u_epsilon.end()); it != itend; ++it) {
    // determine number of the wavelet
    typedef Vector<double>::size_type size_type;
    size_type number = 0;
    if (it.index().e() == 0) {
      number = it.index().k()-problem.basis().DeltaLmin();
    } else {
      number = problem.basis().Deltasize(it.index().j())+it.index().k()-problem.basis().Nablamin();
    }
    wcoeffs[number] = *it;
  }
  
  // switch to generator representation
  Vector<double> gcoeffs(wcoeffs.size(), false);
  if (jmax+1 == problem.basis().j0())
    gcoeffs = wcoeffs;
  else
    problem.basis().apply_Tj(jmax, wcoeffs, gcoeffs);
  
  // evaluate Galerkin solution
  const unsigned int N = 100;
  const double h = 1./N;
  Vector<double> ulambda_values(N+1);
  for (unsigned int i = 0; i <= N; i++) {
    const double x = i*h;
    SchoenbergIntervalBSpline_td<d> sbs(jmax+1,0);
    for (unsigned int k = 0; k < gcoeffs.size(); k++) {
      sbs.set_k(problem.basis().DeltaLmin()+k);
      ulambda_values[i] += gcoeffs[k] * sbs.value(Point<1>(x));
    }
  }

  // evaluate exact solution
  Vector<double> uexact_values(N+1);
  for (unsigned int i = 0; i <= N; i++) {
    const double x = i*h;
    uexact_values[i] = uexact->value(Point<1>(x));
  }
  
  const double Linfty_error = linfty_norm(ulambda_values-uexact_values);
  cout << "  L_infinity error on a subgrid: " << Linfty_error << endl;


  
  return 0;
}
