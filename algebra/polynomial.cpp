// implementation of MathTL::Polynomial inline functions

#include <cassert>
#include <numerics/gauss_data.h>
#include <utils/tiny_tools.h>

namespace MathTL
{
  template <class C>
  Polynomial<C>::Polynomial()
    : Vector<C>(1), Function<1,C>()
  {
  }
  
  template <class C>
  Polynomial<C>::Polynomial(const Polynomial<C>& p)
    : Vector<C>(p)
  {
  }

  template <class C>
  Polynomial<C>::Polynomial(const Vector<C>& coeffs)
    : Vector<C>(coeffs)
  {
  }

  template <class C>
  Polynomial<C>::Polynomial(const C c)
    : Vector<C>(1)
  {
    set_coefficient(0, c);
  }

  template <class C>
  Polynomial<C>::~Polynomial()
  {
  }

  template <class C>
  inline
  unsigned int Polynomial<C>::degree() const
  {
    return Vector<C>::size()-1;
  }

  template <class C>
  inline
  C Polynomial<C>::get_coefficient(const unsigned int k) const
  {
    return Vector<C>::operator [] (k);
  }

  template <class C>
  inline
  void Polynomial<C>::get_coefficients(Vector<C>& coeffs) const
  {
    coeffs = *this;
  }
  
  template <class C>
  void Polynomial<C>::set_coefficient(const unsigned int k,
				      const C coeff)
  {
    if (k > degree())
      {
	Vector<C> help;
	help.swap(*this);
	Vector<C>::resize(k+1);
	std::copy(help.begin(), help.end(), Vector<C>::begin());
      }

    Vector<C>::operator [] (k) = coeff;
  }

  template <class C>
  void Polynomial<C>::set_coefficients(const Vector<C>& coeffs)
  {
    assert(coeffs.size() > 0);
    Vector<C>::operator = (coeffs);
  }

  template <class C>
  C Polynomial<C>::value(const C x) const
  {
    C value(Vector<C>::operator [] (degree()));

    for (unsigned int k(degree()); k > 0; k--)
      value = value * x + get_coefficient(k-1);

    return value;
  }

  template <class C>
  C Polynomial<C>::value(const C x, const unsigned int derivative) const
  {
    // storage for full Horner scheme
    Vector<C> horner(*this);

    for (unsigned row(0); row <= derivative; row++)
      {
	for (unsigned n(degree()); n > row; n--)
	  horner[n-1] += x*horner[n];
      }

    return horner[derivative] * faculty(derivative);
  }

  template <class C>
  inline
  C Polynomial<C>::value(const Point<1>& p,
			 const unsigned int component) const
  {
    return value(p[0]);
  }

  template <class C>
  inline
  void Polynomial<C>::vector_value(const Point<1> &p,
				   Vector<C>& values) const
  {
    values.resize(1, false);
    values[0] = value(p[0]);
  }

  template <class C>
  void Polynomial<C>::scale(const C s)
  {
    C factor(1.0);
    for (typename Vector<C>::iterator it(Vector<C>::begin()), itend(Vector<C>::end());
	 it != itend; ++it)
      {
	*it *= factor;
	factor *= s;
      }
  }
  
  template <class C>
  void Polynomial<C>::shift(const C s)
  {
    Vector<C> new_coeffs(*this);
    
    for (unsigned int d(1); d < new_coeffs.size(); d++)
      {
	unsigned int n(d);
	unsigned int binomial(1);
	C s_power(s);
	for (unsigned int k(0); k < d; k++)
	  {
	    binomial = (binomial*(n-k))/(k+1);
	    new_coeffs[d-k-1] +=
	      new_coeffs[d] * binomial * s_power;
	    s_power *= s;
	  }
      }
    
    swap(new_coeffs);
  }

  template <class C>
  void Polynomial<C>::chain(const Polynomial<C>& p)
  {
    if (degree() > 0)
      {
	// maybe the following can be optimized

  	Polynomial<C> q;
  	C a0(get_coefficient(0)); // not changed by substitution

  	Polynomial<C> r;
  	for (unsigned int expo = 1; expo <= degree(); expo++)
  	  {
  	    if (get_coefficient(expo) != 0)
  	      {
  		q = p;
  		for (unsigned int l = 2; l <= expo; l++)
  		  q *= p;
  		q *= get_coefficient(expo);

  		r += q;
  	      }
  	  }

 	Vector<C>::swap(r);
 	set_coefficient(0, get_coefficient(0) + a0);
      }
  }

  template <class C>
  Polynomial<C>& Polynomial<C>::operator = (const Polynomial<C>& p)
  {
    Vector<C>::operator = (p);
    return *this;
  }

  template <class C>
  Polynomial<C>& Polynomial<C>::operator = (const C c)
  {
    Vector<C>::resize(1, false);
    set_coefficient(0, c);
    return *this;
  }

  template <class C>
  void Polynomial<C>::add(const Polynomial<C>& p)
  {
    Vector<C> help(std::max(degree(), p.degree())+1);
    if (degree()+1 == help.size())
      {
	std::copy(p.begin(), p.end(), help.begin());
	help.add(*this);
      }
    else
      {
	std::copy(begin(), end(), help.begin());
	help.add(p);
      }
    swap(help);
  }

  template <class C>
  void Polynomial<C>::add(const C s, const Polynomial<C>& p)
  {
    Vector<C> help(std::max(degree(), p.degree())+1);
    if (degree()+1 == help.size())
      {
	std::copy(p.begin(), p.end(), help.begin());
	help.sadd(s, *this); // help <- s*help + *this
      }
    else
      {
	std::copy(begin(), end(), help.begin());
	help.add(s, p); // help <- help + s*p
      }
    swap(help);
  }

  template <class C>
  inline
  Polynomial<C>& Polynomial<C>::operator += (const Polynomial<C>& p)
  {
    add(p);
    return *this;
  }

  template <class C>
  inline
  Polynomial<C> Polynomial<C>::operator + (const Polynomial<C>& p) const
  {
    return (Polynomial<C>(*this) += p);
  }

  template <class C>
  inline
  Polynomial<C>& Polynomial<C>::operator -= (const Polynomial<C>& p)
  {
    add(C(-1), p);
    return *this;
  }

  template <class C>
  inline
  Polynomial<C> Polynomial<C>::operator - () const
  {
    return (Polynomial<C>() -= *this);
  }

  template <class C>
  inline
  Polynomial<C> Polynomial<C>::operator - (const Polynomial<C>& p) const
  {
    return (Polynomial<C>(*this) -= p);
  }

  template <class C>
  Polynomial<C>& Polynomial<C>::operator *= (const C c)
  {
    if (c == C(0))
      Vector<C>::resize(1); // p(x)=0, degree changes
    else
      Vector<C>::operator *= (c);

    return *this;
  }

  template <class C>
  inline
  Polynomial<C> Polynomial<C>::operator * (const C c) const
  {
    return (Polynomial(*this) *= c);
  }

  template <class C>
  Polynomial<C>& Polynomial<C>::operator *= (const Polynomial<C>& p)
  {
    Vector<C> coeffs(degree()+p.degree()+1);
    for (unsigned int n(0); n <= degree(); n++)
      for (unsigned int m(0); m <= p.degree(); m++)
	coeffs[n+m] += Vector<C>::operator [] (n) * p.get_coefficient(m);

    swap(coeffs);
    return *this;
  }

  template <class C>
  inline
  Polynomial<C> Polynomial<C>::operator * (const Polynomial<C>& p)
  {
    return (Polynomial(*this) *= p);
  }

  template <class C>
  Polynomial<C> Polynomial<C>::power(const unsigned int k) const
  {
    Polynomial<C> r;
    if (k == 0)
      r = 1;
    else
      {
	r = *this;
	for (unsigned int l(2); l <= k; l++)
	  r *= *this;
      }

    return r;
  }

  template <class C>
  Polynomial<C> Polynomial<C>::differentiate() const
  {
    Vector<C> coeffs(std::max(degree(),(unsigned int)1));

    for (unsigned int n(1); n <= degree(); n++)
      coeffs[n-1] = n * get_coefficient(n);

    return Polynomial<C>(coeffs);
  }

  template <class C>
  Polynomial<C> Polynomial<C>::integrate() const
  {
    Vector<C> coeffs(degree()+2);
    
    for (unsigned int n(0); n <= degree(); n++)
      coeffs[n+1] = get_coefficient(n) / (n + 1.0);

    return Polynomial<C>(coeffs);
  }

  template <class C>
  double Polynomial<C>::integrate(const double a,
				  const double b,
				  const bool quadrature) const
  {
    assert(a <= b);

    double r(0);

    if(quadrature)
      {
	const unsigned int N = (unsigned int)ceil((degree()+1)/2.0); // ensures 2*N-1>=degree()

	for(unsigned int i(0); i < N; i++)
	  r += GaussWeights[N-1][i] * value((a+b + (b-a)*GaussPoints[N-1][i])/2.0);
      
	r *= b-a;
      }
    else
      {
	Polynomial<C> P(integrate());
	r = P.value(b)-P.value(a);
      }

    return r;
  }

  template <class C>
  Polynomial<C> operator * (const C c, const Polynomial<C>& p)
  {
    return (Polynomial<C>(p) *= c);
  }

  template <class C>
  std::ostream& operator << (std::ostream &s, const Polynomial<C> &p)
  {
    double c;
    int oldprecision = s.precision();
    std::ios::fmtflags oldflags = s.flags();
    //  s.setf(ios::scientific, ios::floatfield);
    s.precision(12);

    bool first = true;
    for (unsigned int k = p.degree(); k >= 1; k--)
      {
	c = p.get_coefficient(k);
	if (c != 0)
	  {
	    if (!first)
	      {
		if (c >= 0)
		  s << "+";
	      }
	    if (fabs(c) != 1)
	      s << c;
	    else
	      if (c == -1)
		s << "-";
	    s << "x";
	    if (k > 1)
	      s << "^" << k;
	    first = false;
	  }
      }
  
    c = p.get_coefficient(0);

    if ((c!=0)||(p.degree()==0))
      {
	if (!first)
	  {
	    if (c >= 0)
	      s << "+";
	  }
	s << c;
      }
  
    s.setf(oldflags);
    s.precision(oldprecision);

    return s;
  }
}
