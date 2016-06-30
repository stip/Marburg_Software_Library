// implementation for infinite_preconditioner.h

#include <cmath>

namespace WaveletTL
{
  template <class INDEX>
  InfinitePreconditioner<INDEX>::~InfinitePreconditioner() {}

  template <class INDEX>
  inline
  void
  InfiniteSymmetricPreconditioner<INDEX>::apply_left_preconditioner
  (const InfiniteVector<double,INDEX>& y,
   InfiniteVector<double,INDEX>& x) const
  {
    apply_preconditioner(y, x);
  }
    
  template <class INDEX>
  inline
  void
  InfiniteSymmetricPreconditioner<INDEX>::reverse_left_preconditioner
  (const InfiniteVector<double,INDEX>& x,
   InfiniteVector<double,INDEX>& y) const
  {
    reverse_preconditioner(x, y);
  }
    
  template <class INDEX>
  inline
  void
  InfiniteSymmetricPreconditioner<INDEX>::apply_right_preconditioner
  (const InfiniteVector<double,INDEX>& y,
   InfiniteVector<double,INDEX>& x) const
  {
    apply_preconditioner(y, x);
  }
  
  template <class INDEX>
  inline
  void
  InfiniteSymmetricPreconditioner<INDEX>::reverse_right_preconditioner
  (const InfiniteVector<double,INDEX>& x,
   InfiniteVector<double,INDEX>& y) const
  {
    reverse_preconditioner(x, y);
  }
  
  template <class INDEX>
  inline
  void
  FullyDiagonalPreconditioner<INDEX>::apply_preconditioner
  (const InfiniteVector<double,INDEX>& y,
   InfiniteVector<double,INDEX>& x) const
  {
    x = y;
    x.scale(this, -1);
  };
  
  template <class INDEX>
  inline
  void
  FullyDiagonalPreconditioner<INDEX>::reverse_preconditioner
  (const InfiniteVector<double,INDEX>& x,
   InfiniteVector<double,INDEX>& y) const
  {
    y = x;
    y.scale(this, 1);
  };
  
  template <class INDEX>
  inline
  double
  FullyDiagonalDyadicPreconditioner<INDEX>::diag(const INDEX& lambda) const
  {
    return pow(ldexp(1.0, lambda.j()), operator_order());
  }

#ifdef FRAME  
  template <class INDEX>
  inline
  double
  FullyDiagonalQuarkletPreconditioner<INDEX>::diag(const INDEX& lambda) const
  {

    return pow(ldexp(1.0, lambda.j())*pow(1+lambda.p(),4),operator_order()) * (1+lambda.p()); //2^j*(p+1)^5, falls operator_order()=1
    //return pow(sqrt(a(lambda, lambda))*pow(1+lambda.p(),4),operator_order()) * (1+lambda.p());
    ///return 1;
   //return ldexp(1.0, lambda.j());
  }
#endif

  template <class INDEX>
  inline
  double
  FullyDiagonalEnergyNormPreconditioner<INDEX>::diag(const INDEX& lambda) const
  {
    return sqrt(a(lambda, lambda));
    //return ldexp(1.0, lambda.j()); //ATTENTION!!! HAS TO BE CHANGED BACK; ONLY FOR EXPERIMENTING
  };

  template <class INDEX>
  inline
  double
  TrivialPreconditioner<INDEX>::diag(const INDEX& lambda) const
  {
    return 1;
  };
}