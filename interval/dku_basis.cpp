// implementation for dku_basis.h

#include <cmath>
#include <iostream>

#include <utils/tiny_tools.h>
#include <algebra/matrix_norms.h>
#include <algebra/triangular_matrix.h>
#include <numerics/eigenvalues.h>
#include <numerics/matrix_decomp.h>

#include <Rd/haar_mask.h>
#include <Rd/cdf_mask.h>
#include <Rd/dm_mask.h>
#include <Rd/refinable.h>
#include <Rd/r_index.h>

using std::cout;
using std::endl;

namespace WaveletTL
{
  // helper routine to compute BetaL
  template <int d, int dT>
  double DKUBasis<d, dT>::betaL_(const int m, const int r) const {
    double result(0);
    
    for (int q((int)ceil((m-ell2T_)/2.0)); q <= ellT_-1; q++)
      result += Alpha_(q, r) * cdf_.aT().get_coefficient(MultiIndex<int, 1>(m-2*q)); // (3.2.31)
    
    return result * M_SQRT1_2;
  }
  
  // helper routine to compute BetaLT
  template <int d, int dT>
  double DKUBasis<d, dT>::betaLT_(const int m, const int r) const {
    double result(0);

    for (int q((int)ceil((m-ell2_)/2.0)); q <= ell_-1; q++)
      result += AlphaT_(q, r) * cdf_.a().get_coefficient(MultiIndex<int, 1>(m-2*q)); // (3.2.31)

    return result * M_SQRT1_2;
  }

  template <int d, int dT>
  DKUBasis<d, dT>::DKUBasis(DKUBiorthogonalizationMethod bio)
    : ell1_(ell1<d>()),
      ell2_(ell2<d>()),
      ell1T_(ell1T<d,dT>()),
      ell2T_(ell2T<d,dT>()),
      GammaL_(dT, dT),
      CL_(dT, dT),
      CLT_(dT, dT)
  {
    ellT_ = ell2T<d,dT>(); // (3.2.10)
    ell_  = ellT_-(dT-d);  // (3.2.16)

    // setup Alpha
    // (offset in first argument: ell2T()-1)
    Alpha_.resize(ellT_+ell2T_-1, dT);
    for (unsigned int m(0); m < Alpha_.row_dimension(); m++)
      Alpha_(m, 0) = 1.0; // (5.1.1)
    for (int r(1); r < (int)Alpha_.column_dimension(); r++) {
      double dummy(0);
      for (int k(ell1_); k <= ell2_; k++) {
	double dummy1(0);
	for (int s(0); s <= r-1; s++)
	  dummy1 += binomial(r, s) * intpower(k, r-s) * Alpha_(ell2T_-1, s);
	dummy += cdf_.a().get_coefficient(MultiIndex<int, 1>(k)) * dummy1; // (5.1.3)
      }
      Alpha_(ell2T_-1, r) = dummy / (ldexp(1.0, r+1) - 2.0);
    }
    for (int r(1); r < (int)Alpha_.column_dimension(); r++) {
      for (int m(-ell2T_+1); m < 0; m++) {
	double dummy(0);
	for (int i(0); i <= r; i++)
	  dummy += binomial(r, i) * intpower(m, i) * Alpha_(ell2T_-1, r-i); // (5.1.2)
	Alpha_(m+ell2T_-1, r) = dummy;
      }
      for (int m(1); m <= ellT_-1; m++) {
	double dummy(0);
	for (int i(0); i <= r; i++)
	  dummy += binomial(r, i) * intpower(m, i) * Alpha_(ell2T_-1, r-i); // (5.1.2)
	Alpha_(m+ell2T_-1, r) = dummy;
      }
    }
    
    // setup AlphaT
    // (offset in first argument: ell2()-1)
    AlphaT_.resize(ell_+ell2_-1, d);
    for (unsigned int m(0); m < AlphaT_.row_dimension(); m++)
      AlphaT_(m, 0) = 1.0; // (5.1.1)
    for (int r(1); r < (int)AlphaT_.column_dimension(); r++) {
      double dummy(0);
      for (int k(ell1T_); k <= ell2T_; k++) {
	double dummy1(0);
	for (int s(0); s <= r-1; s++)
	  dummy1 += binomial(r, s) * intpower(k, r-s) * AlphaT_(ell2_-1, s); // (5.1.3)
	dummy += cdf_.aT().get_coefficient(MultiIndex<int, 1>(k)) * dummy1;
      }
      AlphaT_(ell2_-1, r) = dummy / (ldexp(1.0, r+1) - 2.0);
    }
    for (int r(1); r < (int)AlphaT_.column_dimension(); r++) {
      for (int m(-ell2_+1); m < 0; m++) {
	double dummy(0);
	for (int i(0); i <= r; i++)
	  dummy += binomial(r, i) * intpower(m, i) * AlphaT_(ell2_-1, r-i); // (5.1.2)
	AlphaT_(m+ell2_-1, r) = dummy;
      }
      for (int m(1); m <= ell_-1; m++) {
	double dummy(0);
	for (int i(0); i <= r; i++)
	  dummy += binomial(r, i) * intpower(m, i) * AlphaT_(ell2_-1, r-i); // (5.1.2)
	AlphaT_(m+ell2_-1, r) = dummy;
      }
    }
    
    // setup BetaL
    BetaL_.resize(ell2T_-ell1T_-1, dT);
    for (int r(0); r < dT; r++)
      for (int m(2*ellT_+ell1T_); m <= 2*ellT_+ell2T_-2; m++)
	BetaL_(m-2*ellT_-ell1T_, r) = betaL_(m, r);

    // setup BetaLT
    BetaLT_.resize(ell2_-ell1_-1, d);
    for (int r(0); r < d; r++)
      for (int m(2*ell_+ell1_); m <= 2*ell_+ell2_-2; m++)
	BetaLT_(m-2*ell_-ell1_, r) = betaLT_(m, r);
    
    // setup GammaL
    //
    //
    // 1. compute the integrals
    //      z(s,t) = \int_0^1\phi(x-s)\tilde\phi(x-t)\,dx
    //    exactly with the [DM] trick
    MultivariateRefinableFunction<DMMask2<HaarMask, CDFMask_primal<d>, CDFMask_dual<d, dT> >, 2> zmask;
    InfiniteVector<double, MultiIndex<int, 2> > zvalues(zmask.evaluate());
    //
    // 2. compute the integrals
    //      I(nu,mu) = \int_0^\infty\phi(x-\nu)\tilde\phi(x-\mu)\,dx
    //    exactly using the z(s,t) values
    Matrix<double> I(ell_-d+dT-1+ell2_, ellT_-1+ell2T_);
    for (int nu(-ell2_+1); nu <= -ell1_-1; nu++)
      for (int mu(-ell2T_+1); mu <= -ell1T_-1; mu++) {
	double help(0);
	int diff(mu - nu);
	for (int s(-ell2_+1); s <= nu; s++)
	  help += zvalues.get_coefficient(MultiIndex<int, 2>(s, s+diff));
	I(nu+ell2_-1, mu+ell2T_-1) = help; // (5.1.7)
      }
    for (int nu(-ell2_+1); nu <= ell_-d+dT-1; nu++)
      for (int mu(-ell2T_+1); mu <= ellT_-1; mu++) {
	if ((nu >= -ell1_) || ((nu <= ell_-1) && (mu >= -ell1T_)))
	  I(nu+ell2_-1, mu+ell2T_-1) = (nu == mu ? 1 : 0); // (5.1.6)
      }
    //
    // 3. finally, compute the Gramian GammaL
    //    (offsets: entry (r,k) <-> index (r+ellT()-dT,k+ellT()-dT) )
    for (int r(0); r <= d-1; r++)
      for (int k(0); k <= dT-1; k++) {
	double help(0);
	for (int nu(-ell2_+1); nu <= ell_-1; nu++)
	  for (int mu(-ell2T_+1); mu <= ellT_-1; mu++)
	    help += AlphaT_(nu+ell2_-1, r) * Alpha_(mu+ell2T_-1, k) * I(nu+ell2_-1, mu+ell2T_-1);
	GammaL_(r, k) = help; // (5.1.4)
      }
    for (int r(d); r <= dT-1; r++)
      for (int k(0); k <= dT-1; k++) {
	double help(0);
	for (int mu(-ell2T_+1); mu <= ellT_-1; mu++)
	  help += Alpha_(mu+ell2T_-1, k) * I(ell_-d+r+ell2_-1, mu+ell2T_-1);
	GammaL_(r, k) = help; // (5.1.5)
      }
    
    // setup CL and CLT
    // (offsets: entry (i,j) <-> index (i+ellT()-dT,j+ellT()-dT) )
    //
    if (bio == none) {
      for (unsigned int i(0); i < dT; i++)
	CL_(i, i) = 1.0;

      Matrix<double> CLGammaLInv;
      QRDecomposition<double>(CL_ * GammaL_).inverse(CLGammaLInv);
      CLT_ = transpose(CLGammaLInv);
    }

    if (bio == SVD) {
      MathTL::SVD<double> svd(GammaL_);
      Matrix<double> U, V;
      Vector<double> S;
      svd.getU(U);
      svd.getV(V);
      svd.getS(S);

      for (unsigned int i(0); i < dT; i++) {
	S[i] = 1.0 / sqrt(S[i]);
	for (unsigned int j(0); j < dT; j++) {
	  CL_(i, j) = S[i] * U(j, i);
	  CLT_(i, j) = S[i] * V(i, j);
	}
      }
    }

    if (bio == Bernstein) {
      double b(0);
      if (d == 2)
	b = 0.7; // cf. [DKU]
      else {
	b = 3.6;
      }

      // CL_ : transformation to Bernstein polynomials (triangular)
      //   (Z_b^m)_{r,l} = (-1)^{r-1}\binom{m-1}{r}\binom{r}{l}b^{-r}, r>=l, 0 otherwise
      for (unsigned int j(0); j < d; j++)
	for (unsigned int k(0); k <= j; k++)
	  CL_(k, j) = minus1power(j-k) * std::pow(b, -(int)j) * binomial(d-1, j) * binomial(j, k);
      for (unsigned int j(d); j < dT; j++)
	CL_(j, j) = 1.0;

      Matrix<double> CLGammaLInv;
      QRDecomposition<double>(CL_ * GammaL_).inverse(CLGammaLInv);
      CLT_ = transpose(CLGammaLInv);
    }

    if (bio == partialSVD) {
      MathTL::SVD<double> svd(GammaL_);
      Matrix<double> U, V;
      Vector<double> S;
      svd.getU(U);
      svd.getV(V);
      svd.getS(S);

      Matrix<double> GammaLInv;
      QRDecomposition<double>(GammaL_).inverse(GammaLInv);
      const double a = 1.0 / GammaLInv(0, 0);
      
      Matrix<double> R(dT, dT);
      for (unsigned int i(0); i < dT; i++)
	S[i] = sqrt(S[i]);
      for (unsigned int j(0); j < dT; j++)
	R(0, j) = a * V(0, j) / S[j];
      for (unsigned int i(1); i < dT; i++)
	for (unsigned int j(0); j < dT; j++)
	  R(i, j) = U(j, i) * S[j];

      for (unsigned int i(0); i < dT; i++)
	for (unsigned int j(0); j < dT; j++)
	  U(i, j) /= S[i];
      CL_ = R*U;

      Matrix<double> CLGammaLInv;
      QRDecomposition<double>(CL_ * GammaL_).inverse(CLGammaLInv);
      CLT_ = transpose(CLGammaLInv);
    }

    if (bio == BernsteinSVD) {
      double b(0);
      if (d == 2)
	b = 0.7; // cf. [DKU]
      else {
	b = 3.6;
      }

      // CL_ : transformation to Bernstein polynomials (triangular)
      //   (Z_b^m)_{r,l} = (-1)^{r-1}\binom{m-1}{r}\binom{r}{l}b^{-r}, r>=l, 0 otherwise
      for (unsigned int j(0); j < d; j++)
	for (unsigned int k(0); k <= j; k++)
	  CL_(k, j) = minus1power(j-k) * std::pow(b, -(int)j) * binomial(d-1, j) * binomial(j, k);
      for (unsigned int j(d); j < dT; j++)
	CL_(j, j) = 1.0;

      Matrix<double> GammaLNew(CL_ * GammaL_);

      MathTL::SVD<double> svd(GammaLNew);
      Matrix<double> U, V;
      Vector<double> S;
      svd.getU(U);
      svd.getV(V);
      svd.getS(S);

      Matrix<double> GammaLNewInv;
      QRDecomposition<double>(GammaLNew).inverse(GammaLNewInv);
      const double a = 1.0 / GammaLNewInv(0, 0);

      Matrix<double> R(dT, dT);
      for (unsigned int i(0); i < dT; i++)
	S[i] = sqrt(S[i]);
      for (unsigned int j(0); j < dT; j++)
	R(0, j) = a * V(0, j) / S[j];
      for (unsigned int i(1); i < dT; i++)
	for (unsigned int j(0); j < dT; j++)
	  R(i, j) = U(j, i) * S[j];

      for (unsigned int i(0); i < dT; i++)
	for (unsigned int j(0); j < dT; j++)
	  U(i, j) /= S[i];
      CL_ = R*U*CL_;

      Matrix<double> CLGammaLInv;
      QRDecomposition<double>(CL_ * GammaL_).inverse(CLGammaLInv);
      CLT_ = transpose(CLGammaLInv);
    }

#if 0
    // check biorthogonality of the matrix product CL * GammaL * (CLT)^T
    Matrix<double> check(CL_ * GammaL_ * transpose(CLT_));
    for (unsigned int i(0); i < check.row_dimension(); i++)
      check(i, i) -= 1;
    cout << "error for CLT: " << row_sum_norm(check) << endl;
#endif

    // setup CLA <-> AlphaT * (CL)^T
    // (offsets: for CLA entry (i,j) <-> index (i+1-ell2(),j+ellT()-dT) )
    CLA_.resize(ellT_+ell2_-1, dT);
    for (int i(1-ell2_); i <= ell_-1; i++) // the (3.2.25) bounds
      for (int r(ellT_-dT); r <= ellT_-1; r++) {
	double help(0);
	for (int m(ell_-d); m <= ell_-1; m++)
	  help += CL_(r-ellT_+dT, m-ellT_+dT) * AlphaT_(i+ell2_-1, m-ell_+d);
	CLA_(i-1+ell2_, r-ellT_+dT) = help;
      }
    for (int i(ell_); i <= ellT_-1; i++)
      for (int r(ellT_-dT); r <= ellT_-1; r++)
	CLA_(i-1+ell2_, r-ellT_+dT) = CL_(r-ellT_+dT, i-ellT_+dT);
    CLA_.compress();

    CLA_.mirror(CRA_);

    // setup CLAT <-> Alpha * (CLT)^T
    // (offsets: for CLAT entry (i,j) <-> index (i+1-ell2T(),j+ellT()-dT) )
    CLAT_.resize(ellT_+ell2T_-1, dT);
    for (int i(1-ell2T_); i <= ellT_-1; i++) // the (3.2.26) bounds
      for (int r(ellT_-dT); r <= ellT_-1; r++) {
	double help(0);
	for (int m(ellT_-dT); m <= ellT_-1; m++)
	  help += CLT_(r-ellT_+dT, m-ellT_+dT) * Alpha_(i+ell2T_-1, m-ellT_+dT);
  	CLAT_(i-1+ell2T_, r-ellT_+dT) = help;
      }
    CLAT_.compress();

    CLAT_.mirror(CRAT_);
  }

  template <int d, int dT>
  typename DKUBasis<d, dT>::Index
  DKUBasis<d, dT>::firstGenerator(const int j) const
  {
    assert(j >= j0());
    return Index(j, 0, DeltaLmin(), this);
  }

  template <int d, int dT>
  typename DKUBasis<d, dT>::Index
  DKUBasis<d, dT>::lastGenerator(const int j) const
  {
    assert(j >= j0());
    return Index(j, 0, DeltaRmax(j0()), this);
  }

  template <int d, int dT>
  typename DKUBasis<d, dT>::Index
  DKUBasis<d, dT>::firstWavelet(const int j) const
  {
    assert(j >= j0());
    return Index(j, 1, Nablamin(), this);
  }

  template <int d, int dT>
  typename DKUBasis<d, dT>::Index
  DKUBasis<d, dT>::lastWavelet(const int j) const
  {
    assert(j >= j0());
    return Index(j, 1, Nablamax(j), this);
  }

  template <int d, int dT>
  SampledMapping<1>
  DKUBasis<d, dT>::evaluate(const Index& lambda,
			    const bool primal,
			    const int resolution) const
  {
    if (lambda.e() == 0) { // generator
      if (primal) {
	if (lambda.k() <= DeltaLTmax()) {
	    // left boundary generator
	    InfiniteVector<double, RIndex> coeffs;
	    for(int i(0); i < ellT_+ell2_-1; i++) {
	      double v(CLA_(i, lambda.k()-ellT_+dT));
	      if (v != 0)
		coeffs.set_coefficient(RIndex(lambda.j(), 0, i+1-ell2_), v);
	    }
	    return cdf_.evaluate(0, coeffs, primal, 0, 1, resolution);
	} else {
	  if (lambda.k() >= DeltaRTmin(lambda.j())) {
	    // right boundary generator
	    InfiniteVector<double, RIndex> coeffs;
	    for (int i(0); i < ellT_+ell2_-1; i++) {
	      double v(CRA_(i, lambda.k()+dT-1-DeltaRmax(lambda.j())));
	      if (v != 0)
		coeffs.set_coefficient(RIndex(lambda.j(), 0, DeltaRmax(lambda.j())-ellT_-ell2_+2+i), v);
	    }
	    return cdf_.evaluate(0, coeffs, primal, 0, 1, resolution);
	  } else {
	    // inner generator
	    return cdf_.evaluate(0, RIndex(lambda.j(), 0, lambda.k()),
				 primal, 0, 1, resolution);
	  }
	}
      } else {
	// dual
	if (lambda.k() <= DeltaLTmax()) {
	  // left boundary generator
	  InfiniteVector<double, RIndex> coeffs;
	  for (int i(0); i < ellT_+ell2T_-1; i++) {
	    double v(CLAT_(i, lambda.k()-ell_+d));
	    if (v != 0)
	      coeffs.set_coefficient(RIndex(lambda.j(), 0, i+1-ell2T_), v);
	  }
	  return cdf_.evaluate(0, coeffs, primal, 0, 1, resolution);
	} else {
	  if (lambda.k() >= DeltaRTmin(lambda.j())) {
	    // right boundary generator
	    InfiniteVector<double, RIndex> coeffs;
	    for (int i(0); i < ellT_+ell2T_-1; i++) {
	      double v(CRAT_(i, lambda.k()+dT-1-DeltaRmax(lambda.j())));
	      if (v != 0)
		coeffs.set_coefficient(RIndex(lambda.j(), 0, DeltaRmax(lambda.j())-ellT_-ell2_+2+i), v);
	    }
	    return cdf_.evaluate(0, coeffs, primal, 0, 1, resolution);
	  } else {
	    // inner generator
	    return cdf_.evaluate(0, RIndex(lambda.j(), 0, lambda.k()),
				 primal, 0, 1, resolution);
	  }
	}
      }
    } else {
      // expand wavelet as generators of a higher scale
      // TODO!!!
    }
    
    return SampledMapping<1>(); // dummy return for the compiler
  }

//   template <int d, int dT>
//   SampledMapping<1> evaluate(const InfiniteVector<double, Index>& coeffs,
// 			     const bool primal,
// 			     const int resolution) const;


}
