// -*- c++ -*-

// +--------------------------------------------------------------------+
// | This file is part of MathTL - the Mathematical Template Library    |
// |                                                                    |
// | Copyright (c) 2002-2005                                            |
// | Thorsten Raasch                                                    |
// +--------------------------------------------------------------------+

#ifndef _MATHTL_TINY_TOOLS_H
#define _MATHTL_TINY_TOOLS_H

#include <cassert>

// some utility functions

//! absolute value of a real number
template <class R>
inline R abs(const R a)
{
  return  (a > 0 ? a : -a);
}

//! stable computation of hypotenuse
/*!
  stable computation of the hypotenuse length for scalars a and b;
  we do _not_ compute
    sqrt(a*a+b*b)
  but the more stable
    a*sqrt(1+b/a*b/a)
 */
template <class R>
inline R hypot(const R a, const R b)
{
  R r(0);
  if (a == 0)
    r = abs(b);
  else
    {
      R c(b/a);
      r = abs(a) * sqrt(1+c*c);
    }
  return r;
}

//! faculty of a (signed or unsigned) integer
template <class I>
I faculty (const I n)
{
  I r(1);
  for (I i(2); i <= n; r *= i, i++);
  return r;
}

//! binomial coefficient
inline int binomial (const int n, const int k)
{
  assert (k >= 0);
  
  int r(1);
  
  if (k > n)
    return 0;
  
  if (k == 0)
    return 1;
  
  for (int i(k+1); i <= n; i++)
    r *= i;
  
  for (int i(1); i <= n-k; i++)
    r /= i; // always possible without remainder
  
  return r;
}

//! (-1)^k
inline int minus1power(const int k)
{
  return (k%2==0 ? 1 : -1);
}

//! integer power n^k, 0^0=1
int intpower(const int n, const int k)
{
  assert(k >= 0);

  int r(1);
  for (int i(1); i <= k; i++)
    r *= n;

  return r;
}

#endif
