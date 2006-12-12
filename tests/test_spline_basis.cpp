#include <iostream>
#include <fstream>
#include <algebra/vector.h>
#include <interval/spline_basis_data.h>
#include <interval/spline_basis.h>

using namespace std;
using namespace MathTL;
using namespace WaveletTL;

int main()
{
  cout << "Testing setup of SplineBasisData objects..." << endl;

#if 1
  SplineBasisData<2,2> sd22nobc("P","",0,0,0,0); // PBasis, no b.c.'s
  sd22nobc.check();

  SplineBasisData<2,2> sd22("P","",1,1,0,0); // PBasis, complementary b.c.'s
  sd22.check();

  SplineBasisData<2,2> sd22bcleft("P","",1,0,0,0); // PBasis, complementary b.c.'s at x=0
  sd22bcleft.check();

  SplineBasisData<2,2> sd22bcright("P","",0,1,0,0); // PBasis, complementary b.c.'s at x=1
  sd22bcright.check();

  SplineBasisData<3,3> sd33("P","",1,1,0,0); // PBasis, complementary b.c.'s
  sd33.check();
#endif

#if 0
  cout << "Testing SplineBasis..." << endl;
  SplineBasis<3,3> basis("P","",1,1,0,0); // PBasis, complementary b.c.'s

  const int j0 = basis.j0();
  Vector<double> x(basis.Deltasize(j0+1));
  x[0] = 1;
  cout << "* a vector x=" << x << endl;
  Vector<double> y(basis.Deltasize(j0+1));
  basis.apply_Mj(j0, x, y);
  cout << "* applying Mj=(Mj0 Mj1) to x yields y=Mj*x=" << y << endl;
  basis.apply_Gj(j0, y, x);
  cout << "* applying Gj=(Mj0T Mj1T)^T to y yields x=Gj*y=" << x << endl;
  basis.apply_Gj_transposed(j0, x, y);
  cout << "* applying Gj^T to x yields y=Gj^T*x=" << y << endl;
  basis.apply_Mj_transposed(j0, x, y);
  cout << "* applying Mj^T to y yields x=Mj^T*y=" << x << endl;

  x.scale(0); y.scale(0);
  x[0] = 1;
  basis.apply_Tj(j0, x, y);
  cout << "* applying T_{j0} to x yields y=" << y << endl;
  x.resize(basis.Deltasize(j0+2));
  x[4] = 1;
  cout << "* x on the next level: " << x << endl;
  y.resize(basis.Deltasize(j0+2));
  basis.apply_Tj(j0+1, x, y);
  cout << "* applying T_{j0+1} to x yields y=" << y << endl;
  basis.apply_Tjinv(j0+1, y, x);
  cout << "* applying T_{j0+1}^{-1} to y yields x=" << x << endl;
  x.resize(basis.Deltasize(j0+3));
  x[4] = 1;
  cout << "* x on the next plus 1 level: " << x << endl;
  y.resize(basis.Deltasize(j0+3));
  basis.apply_Tj(j0+2, x, y);
  cout << "* applying T_{j0+2} to x yields y=" << y << endl;
  basis.apply_Tjinv(j0+2, y, x);
  cout << "* applying T_{j0+2}^{-1} to y yields x=" << x << endl;


  cout << "* point evaluation of spline wavelets:" << endl;
  typedef SplineBasis<3,3>::Index Index;
  int N = 16;
  Array1D<double> points(N+1), values(N+1), dervalues(N+1);
  double h = 1.0/N;
  for (int i = 0; i <= N; i++) points[i] = i*h;
  for (Index lambda(first_generator(&basis, basis.j0()));; ++lambda) {
    cout << lambda << endl;
    evaluate(basis, lambda, points, values, dervalues);
    cout << "values: " << values << endl;
    cout << "values of first derivative: " << dervalues << endl;
    if (lambda == last_generator(&basis, basis.j0())) break;
  }
#endif
  
  return 0;
}
