// -*- c++ -*-

// +--------------------------------------------------------------------+
// | This file is part of MathTL - the Mathematical Template Library    |
// |                                                                    |
// | Copyright (c) 2002-2005                                            |
// | Thorsten Raasch                                                    |
// +--------------------------------------------------------------------+

#ifndef _MATHTL_INFINITE_VECTOR_H
#define _MATHTL_INFINITE_VECTOR_H

#include <map>
#include <algorithm>
#include <iterator>

// external functionality, for convenience:
#include <algebra/vector_norms.h>
#include <algebra/vector_arithmetics.h>

namespace MathTL
{
  /*!
    A model class InfiniteVector<C,I> for inherently sparse,
    arbitrarily indexed vectors
      x = (x_i)_{i\in I}
    with entries from a (scalar) class C.
    InfiniteVector<C,I> may act as a realization of coefficient_type in the WaveletTL.
    
    Although internally, we will model InfiniteVector as a map<I,C>,
    the access to the vector entries takes place by a nested iterator
    class (compare the deal.II matrix classes) which is STL-compatible.
  */
  template <class C, class I>
  class InfiniteVector
    : protected std::map<I,C>
  {
  public:
    /*!
      default constructor: yields empty (zero) vector
    */
    InfiniteVector();

    /*!
      copy constructor
    */
    InfiniteVector(const InfiniteVector<C,I>& v);

    /*!
      STL-compliant const_iterator scanning the nontrivial entries
    */
    class const_iterator
      : protected std::map<I,C>::const_iterator
    {
    public:
      /*!
	constructs a const_iterator from a map::const_iterator
      */
      const_iterator(const typename std::map<I,C>::const_iterator& entry);

      /*!
	prefix increment of the const_iterator
      */
      const_iterator& operator ++ ();

      /*!
	postfix increment of the const_iterator
      */
      const_iterator operator ++ (int step);

      /*!
	dereference const_iterator
      */
      const C& operator * () const;

      /*!
	dereference const_iterator
      */
      const C* operator -> () const;

      /*!
	index of current iterator
	(maybe the only difference to an STL iterator)
      */
      I index() const;

      /*!
	compare positions of two iterators
      */
      bool operator == (const const_iterator& it) const;

      /*!
	non-equality test
      */
      bool operator != (const const_iterator& it) const;

      /*!
	comparison, corresponding to the order relation on I
       */
      bool operator < (const const_iterator& it) const;
    };

    /*!
      const_iterator pointing to the first nontrivial vector entry
    */
    const_iterator begin() const;

    /*!
      const_iterator pointing to one after the last nontrivial vector entry
    */
    const_iterator end() const;

    /*!
      STL-compliant const_reverse_iterator scanning the nontrivial entries
      in a reverse way
    */
    class const_reverse_iterator
      : protected std::reverse_iterator<typename std::map<I,C>::const_iterator>
    {
    public:
      /*!
	constructs a const_reverse_iterator from a map::const_iterator
      */
      const_reverse_iterator(const std::reverse_iterator<typename std::map<I,C>::const_iterator>& entry);

      /*!
	prefix increment of the const_reverse_iterator
      */
      const_reverse_iterator& operator ++ ();

      /*!
	postfix increment of the const_reverse_iterator
      */
      const_reverse_iterator operator ++ (int step);

      /*!
	dereference const_reverse_iterator
      */
      const C& operator * () const;

      /*!
	dereference const_reverse_iterator
      */
      const C* operator -> () const;

      /*!
	index of current iterator
	(maybe the only difference to an STL iterator)
      */
      I index() const;

      /*!
	compare positions of two iterators
      */
      bool operator == (const const_reverse_iterator& it) const;

      /*!
	non-equality test
      */
      bool operator != (const const_reverse_iterator& it) const;

      /*!
	comparison, corresponding to the order relation on I
       */
      bool operator < (const const_reverse_iterator& it) const;
    };

    /*!
      const_reverse_iterator pointing to the last nontrivial vector entry
    */
    const_reverse_iterator rbegin() const;

    /*!
      const_reverse_iterator pointing to one before the first last nontrivial vector entry
    */
    const_reverse_iterator rend() const;

    /*!
      assignment from another vector
    */
    InfiniteVector<C,I>& operator = (const InfiniteVector<C,I>& v);

    /*!
      swap components of two vectors
    */
    void swap (InfiniteVector<C,I>& v);

    /*!
      test emptyness
    */
    inline bool empty() const { return std::map<I,C>::empty(); }

    /*!
      set infinite vector to zero
    */
    void clear();
    
    /*!
      equality test
    */
    bool operator == (const InfiniteVector<C,I>& v) const;

    /*!
      non-equality test
    */
    bool operator != (const InfiniteVector<C,I>& v) const;

    /*!
      read-only access to the vector entries
    */
    C operator [] (const I& index) const;

    /*!
      read-only access to the vector entries
    */
    C get_coefficient(const I& index) const;

    /*!
      read-write access to the vector entries
    */
    C& operator [] (const I& index);

    /*!
      set a vector entry
    */
    void set_coefficient(const I& index, const C value);

    /*!
      number of nonzero entries
    */
    size_t size() const;

    /*!
      set all values with modulus below a threshold to zero
      (fabs<C> should exist)
    */
    void compress(const double eta = 1e-16);

    /*!
      in place summation *this += v
    */
    void add(const InfiniteVector<C,I>& v);

    /*!
      in place summation *this += s*v
    */
    void add(const C s, const InfiniteVector<C,I>& v);
    
    /*!
      in place summation *this = s*(*this) + v
      (AXPY level 1 BLAS routine)
    */
    void sadd(const C s, const InfiniteVector<C,I>& v);

    /*!
      in place scaling *this *= s
    */
    void scale(const C s);

    /*!
      in place summation
    */
    InfiniteVector<C,I>& operator += (const InfiniteVector<C,I>& v);

    /*!
      in place subtraction *this -= v
    */
    void subtract(const InfiniteVector<C,I>& v);

    /*!
      in place subtraction
    */
    InfiniteVector<C,I>& operator -= (const InfiniteVector<C,I>& v);

    /*!
      in place multiplication with a scalar
    */
    InfiniteVector<C,I>& operator *= (const C c);
    
    /*!
      in place division by a (nontrivial) scalar
    */
    InfiniteVector<C,I>& operator /= (const C c);

    /*!
      inner product
    */
    const C operator * (const InfiniteVector<C,I>& v) const;

    /*!
      helper struct to handle decreasing order in modulus
      for pairs, with respect to the first argument
    */
    struct decreasing_order
      : public std::binary_function<const std::pair<I,C>&,
				    const std::pair<I,C>&,
				    bool>
    {
      inline bool operator () (const std::pair<I,C>& p1,
			       const std::pair<I,C>& p2)
      {
	return (fabs(p1.second) > fabs(p2.second));
      }
    };

    /*!
      weak l_tau norm
    */
    double weak_norm(const double tau) const;

    /*!
      Computes optimal v such that \|*this-v\|_{\ell_2}\le\epsilon;
      "optimal" means taking the largest entries in modulus of *this.
      The vector v does not have to be initialized, it will be cleared
      at the beginning of the algorithm
    */
    void n_coarse(const double eps, InfiniteVector<C,I>& v) const;
  };

  /*!
    sum of two infinite vectors
    (you should avoid using this operator, since it requires one vector
    to be copied. Use += or add() instead!)
   */
  template <class C, class I>
  InfiniteVector<C,I> operator + (const InfiniteVector<C,I>& v1,
				  const InfiniteVector<C,I>& v2)
  {
    InfiniteVector<C,I> r(v1);
    r += v2;
    return r;
  }

  /*!
    difference of two infinite vectors
    (you should avoid using this operator, since it requires one vector
    to be copied. Use -= or sadd() instead!)
   */
  template <class C, class I>
  InfiniteVector<C,I> operator - (const InfiniteVector<C,I>& v1,
				  const InfiniteVector<C,I>& v2)
  {
    InfiniteVector<C,I> r(v1);
    r -= v2;
    return r;
  }

  //! sign
  template <class C, class I>
  InfiniteVector<C,I> operator - (const InfiniteVector<C,I>& v)
  {
    InfiniteVector<C,I> r(v);
    r -= C(-1);
    return r;
  }

  //! scalar multiplication
  template <class C, class I>
  InfiniteVector<C,I> operator * (const C c, const InfiniteVector<C,I>& v)
  {
     InfiniteVector<C,I> r(v);
     r *= c;
     return r;
  }

  /*!
    swap the values of two infinite vectors
  */
  template <class C, class I>
  void swap(InfiniteVector<C,I>& v1, InfiniteVector<C,I>& v2);

  /*!
    stream output for infinite vectors
  */
  template<class C, class I>
  std::ostream& operator << (std::ostream& os, const InfiniteVector<C,I>& v);
}

// include implementation of inline functions
#include <algebra/infinite_vector.cpp>

#endif
