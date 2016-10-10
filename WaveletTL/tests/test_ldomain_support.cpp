#include <iostream>

#include <algebra/infinite_vector.h>

#include <interval/ds_basis.h>
#include <interval/spline_basis.h>
#include <Ldomain/ldomain_basis.h>

using namespace std;
using namespace WaveletTL;

int main()
{
  cout << "Testing support computation for wavelet bases on the L-shaped domain..." << endl;

  const int d  = 2;
  const int dT = 2;

#if 1
  typedef DSBasis<d,dT,BernsteinSVD> Basis1D;
  Basis1D basis1d;
#else
  // this branch does not work at the moment!!!
  typedef SplineBasis<d,dT,DS_construction_bio5e> Basis1D;
  Basis1D basis1d;
#endif
  
  typedef LDomainBasis<Basis1D> Basis;
  Basis basis(basis1d);

  typedef Basis::Index Index;

#if 0
  cout << "- j0=" << basis.j0() << endl;
  cout << "- the default wavelet index: " << Index() << endl;
  cout << "- the default wavelet index w.r.t. the cube basis: " << Index(&basis) << endl;
  cout << "- first generator on the coarsest level: " << first_generator<Basis1D>(&basis, basis.j0()) << endl;
  cout << "- last generator on the coarsest level: " << last_generator<Basis1D>(&basis, basis.j0()) << endl;
  cout << "- first wavelet on the coarsest level: " << first_wavelet<Basis1D>(&basis, basis.j0()) << endl;
  cout << "- last wavelet on the coarsest level: " << last_wavelet<Basis1D>(&basis, basis.j0()) << endl;
#endif

#if 0
 {
   Basis::Support supp;
   for (Index lambda(basis.first_generator(basis.j0()));; ++lambda) {
     cout << "lambda=" << lambda << endl;
     support(basis, lambda, supp);
     
     cout << "  patch 0: 2^{-" << supp.j << "}"
	  << "[" << supp.xmin[0] << "," << supp.xmax[0]
	  << "]x[" << supp.ymin[0] << "," << supp.ymax[0] << "]";
     if (supp.xmin[0] == -1) cout << " (empty)";
     cout << endl;
     
     cout << "  patch 1: 2^{-" << supp.j << "}"
	  << "[" << supp.xmin[1] << "," << supp.xmax[1]
	  << "]x[" << supp.ymin[1] << "," << supp.ymax[1] << "]";
     if (supp.xmin[1] == -1) cout << " (empty)";
     cout << endl;
     
     cout << "  patch 2: 2^{-" << supp.j << "}"
	  << "[" << supp.xmin[2] << "," << supp.xmax[2]
	  << "]x[" << supp.ymin[2] << "," << supp.ymax[2] << "]";
     if (supp.xmin[2] == -1) cout << " (empty)";
     cout << endl;
     
//      if (lambda == basis.last_generator(basis.j0())) break;
     if (lambda == basis.last_wavelet(basis.j0())) break;
   }
 }
#endif
   
#if 0
  Index eta(first_generator<Basis1D>(&basis, basis.j0()));
//   for (; !(eta.p() == 1); ++eta);
//   for (; !(eta.p() == 2); ++eta);
  for (; !(eta.p() == 3); ++eta);
//   for (; !(eta.p() == 4); ++eta);

//   for (; eta.e()[0] != 0 || eta.e()[1] != 1; ++eta);
//   for (; !(eta.e()[0] == 0 && eta.e()[1] == 1 && eta.p() == 1); ++eta);

//   for (int i = 0; i < 3; i++, ++eta);

  cout << "- for eta=" << eta << ", the support of psi_eta looks as follows:" << endl;
  Basis::Support supp_eta;
  support(basis, eta, supp_eta);

  cout << "  patch 0: 2^{-" << supp_eta.j << "}"
       << "[" << supp_eta.xmin[0] << "," << supp_eta.xmax[0]
       << "]x[" << supp_eta.ymin[0] << "," << supp_eta.ymax[0] << "]"
       << endl;

  cout << "  patch 1: 2^{-" << supp_eta.j << "}"
       << "[" << supp_eta.xmin[1] << "," << supp_eta.xmax[1]
       << "]x[" << supp_eta.ymin[1] << "," << supp_eta.ymax[1] << "]"
       << endl;

  cout << "  patch 2: 2^{-" << supp_eta.j << "}"
       << "[" << supp_eta.xmin[2] << "," << supp_eta.xmax[2]
       << "]x[" << supp_eta.ymin[2] << "," << supp_eta.ymax[2] << "]"
       << endl;

  Index nu(first_generator<Basis1D>(&basis, basis.j0()+1));
//   for (; !(nu.p() == 1); ++nu);
//   for (; !(nu.p() == 2); ++nu);
//   for (; !(nu.p() == 3); ++nu);
//   for (; !(nu.p() == 4); ++nu);

//   for (; nu.e()[0] != 0 || nu.e()[1] != 1; ++nu);
//   for (; !(nu.e()[0] == 0 && nu.e()[1] == 1 && nu.p() == 1); ++nu);

  for (int i = 0; i < 2; i++, ++nu);
//   for (int i = 0; i < 3; i++, ++nu);

  cout << "- for nu=" << nu << ", the support of psi_nu looks as follows:" << endl;
  Basis::Support supp_nu;
  support(basis, nu, supp_nu);

  cout << "  patch 0: 2^{-" << supp_nu.j << "}"
       << "[" << supp_nu.xmin[0] << "," << supp_nu.xmax[0]
       << "]x[" << supp_nu.ymin[0] << "," << supp_nu.ymax[0] << "]"
       << endl;

  cout << "  patch 1: 2^{-" << supp_nu.j << "}"
       << "[" << supp_nu.xmin[1] << "," << supp_nu.xmax[1]
       << "]x[" << supp_nu.ymin[1] << "," << supp_nu.ymax[1] << "]"
       << endl;

  cout << "  patch 2: 2^{-" << supp_nu.j << "}"
       << "[" << supp_nu.xmin[2] << "," << supp_nu.xmax[2]
       << "]x[" << supp_nu.ymin[2] << "," << supp_nu.ymax[2] << "]"
       << endl;

  Basis::Support supp_intersect;
  bool inter = intersect_supports(basis, eta, nu, supp_intersect);
  cout << "- support intersection of psi_eta and psi_nu:" << endl;
  if (inter) {
    cout << "  patch 0: 2^{-" << supp_intersect.j << "}"
	 << "[" << supp_intersect.xmin[0] << "," << supp_intersect.xmax[0]
	 << "]x[" << supp_intersect.ymin[0] << "," << supp_intersect.ymax[0] << "]"
	 << endl;
    
    cout << "  patch 1: 2^{-" << supp_intersect.j << "}"
	 << "[" << supp_intersect.xmin[1] << "," << supp_intersect.xmax[1]
	 << "]x[" << supp_intersect.ymin[1] << "," << supp_intersect.ymax[1] << "]"
	 << endl;
    
    cout << "  patch 2: 2^{-" << supp_intersect.j << "}"
	 << "[" << supp_intersect.xmin[2] << "," << supp_intersect.xmax[2]
	 << "]x[" << supp_intersect.ymin[2] << "," << supp_intersect.ymax[2] << "]"
	 << endl;
  } else {
    cout << "  no!" << endl;
  }

#endif

#if 1
  cout << "- compute all intersecting wavelets:" << endl;
  for (Index lambda = first_generator(&basis, basis.j0());; ++lambda)
    {
      cout << "  * for lambda=" << lambda << ":" << endl;
      typedef std::list<Index> SupportList;
      SupportList nus;
      intersecting_wavelets(basis, lambda, basis.j0(), true, nus);
      for (SupportList::const_iterator it(nus.begin()); it != nus.end(); ++it) {
  	cout << "    nu=" << *it << endl;
      }
      for (int level = basis.j0(); level <= basis.j0()+1; level++) {
	nus.clear();
	intersecting_wavelets(basis, lambda, level, false, nus);
  	for (SupportList::const_iterator it(nus.begin()); it != nus.end(); ++it) {
  	  cout << "    nu=" << *it << endl;
  	}
      }
      
      if (lambda == last_wavelet(&basis, basis.j0())) break;
    }
#endif

}