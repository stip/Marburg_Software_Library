// -*- c++ -*-

// +--------------------------------------------------------------------+
// | This file is part of FrameTL - the Wavelet Template Library      |
// |                                                                    |
// | Copyright (c) 2002-2005                                            |
// | Thorsten Raasch, Manuel Werner                                     |
// +--------------------------------------------------------------------+

#ifndef _FRAME_TL_MULTIPLICATIVE_SCHWARZ_H
#define _FRAME_TL_MULTIPLICATIVE_SCHWARZ_H

#include <algebra/infinite_vector.h>

namespace FrameTL
{
  template <class PROBLEM>
  void  multiplicative_Schwarz_SOLVE(const PROBLEM& P,  const double epsilon,
				     InfiniteVector<double, typename PROBLEM::Index>& u_epsilon_0,
				     InfiniteVector<double, typename PROBLEM::Index>& u_epsilon_1,
				     InfiniteVector<double, typename PROBLEM::Index>& u_epsilon);
//   /*!
//   */
//   template <class PROBLEM>
//   void multiplicative_Schwarz_SOLVE(const PROBLEM& P, const double epsilon,
// 				    InfiniteVector<double, typename PROBLEM::Index>& u_epsilon);
}

#include <multiplicative_Schwarz.cpp>

#endif
