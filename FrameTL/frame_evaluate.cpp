// implementation for frame_evaluate.h

#include <utils/array1d.h>
#include <utils/fixed_array1d.h>
#include <geometry/point.h>
#include <geometry/grid.h>
#include <geometry/sampled_mapping.h>
#include <numerics/gauss_data.h>

namespace FrameTL
{

//   template <class IBASIS>
//   double
//   EvaluateFrame<IBASIS,1,1> ::evaluate(const AggregatedFrame<IBASIS,1,1>& frame,
// 				       const typename AggregatedFrame<IBASIS,1,1>::Index& lambda,
// 				       const double x,
// 				       const bool primal,
// 				       const int resolution) const
//   {

//     double res 0.;

//     Point<1> x;
//     Point<1> x_patch;
//     Point<1> y;

//     xpatch[0] = x;
//     for (unsigned int p = 0;  < frame.n_p(); p++) {
//       frame.atlas()->charts()[p]->map_point_inv(x_patch,x);

//     }

//     if ( in_support(frame, lambda, x_patch) ) {
//       frame.atlas()->charts()[lambda.p()]->map_point_inv(x_patch,y);
//       double wav_val = WaveletTL::evaluate(*(frame.bases()[lambda.p()]->bases()[0]), 0,
// 					   typename IBASIS::Index(lambda.j(),
// 								  lambda.e()[0],
// 								  lambda.k()[0],
// 								  frame.bases()[lambda.p()]->bases()[0]),

// 					   y[0]);
//       values[i] = wav_val / frame.atlas()->charts()[lambda.p()]->Gram_factor(y);
//     }
//     else
//       values[i] = 0.;
//   }

//   return SampledMapping<1> (Grid<1>(grid),values);
// }



  template <class IBASIS>
  SampledMapping<1>
  EvaluateFrame<IBASIS,1,1> ::evaluate(const AggregatedFrame<IBASIS,1,1>& frame,
				       const typename AggregatedFrame<IBASIS,1,1>::Index& lambda,
				       const unsigned int patch,
				       const bool primal,
				       const int resolution) const
  {
    Point<1> x;
    Point<1> x_patch;
    Point<1> y;

    const unsigned int  n_points = (1 << resolution)+1;
    const double h = 1.0 / (n_points-1);
    Array1D<double> grid(n_points);
    Array1D<double> values(n_points);
    for (unsigned int i = 0; i < n_points; i++) {
      x[0] = h*i;
      frame.atlas()->charts()[patch]->map_point(x,x_patch);
      if (in_support(frame, lambda, x_patch)) {
	frame.atlas()->charts()[lambda.p()]->map_point_inv(x_patch,y);
	double wav_val = WaveletTL::evaluate(*(frame.bases()[lambda.p()]->bases()[0]), 0,
					     typename IBASIS::Index(lambda.j(),
								    lambda.e()[0],
								    lambda.k()[0],
								    frame.bases()[lambda.p()]->bases()[0]),

					     y[0]);
	values[i] = wav_val / frame.atlas()->charts()[lambda.p()]->Gram_factor(y);
      }
      else
	values[i] = 0.;
    }

    return SampledMapping<1> (Grid<1>(grid),values);
  }


 template <class IBASIS>
 SampledMapping<1>
 EvaluateFrame<IBASIS,1,1>::evaluate(const AggregatedFrame<IBASIS,1,1>& frame,
				     const typename AggregatedFrame<IBASIS,1,1>::Index& lambda,
				     const bool primal,
				     const int resolution) const
 {

   FixedArray1D<Array1D<double>,1> values; // point values of the factors within psi_lambda
   for (unsigned int i = 0; i < 1; i++) {
     values[i] = WaveletTL::evaluate(*(frame.bases()[lambda.p()]->bases()[i]),
			  typename IBASIS::Index(lambda.j(),
						 lambda.e()[i],
						 lambda.k()[i],
						 frame.bases()[lambda.p()]->bases()[i]),
			  primal,
			  resolution).values();

//      typename IBASIS::Index ind (lambda.j(), lambda.e()[i], lambda.k()[i], frame.bases()[lambda.p()]->bases()[i]);
//      values[i] = ((frame.bases()[lambda.p()]->bases()[i])->evaluate(ind, resolution)).values();
   }

   SampledMapping<1> result(*(frame.atlas()->charts()[lambda.p()]),
			    values,
			    resolution);

   return result;
 }


  template <class IBASIS>
  Array1D<SampledMapping<1> >
  EvaluateFrame<IBASIS,1,1>::evaluate(const AggregatedFrame<IBASIS,1,1>& frame,
				      const InfiniteVector<double,
				      typename AggregatedFrame<IBASIS,1,1>::Index>& coeffs,
				      const bool primal,
				      const int resolution) const
  {


    Array1D<SampledMapping<1> > result(frame.n_p());

    for (int i = 0; i < frame.n_p(); i++) {
      result[i] = SampledMapping<1>(*(frame.atlas()->charts()[i]),resolution);// all zero
    }

    typedef typename AggregatedFrame<IBASIS,1,1>::Index Index;
    for (typename InfiniteVector<double,Index>::const_iterator it(coeffs.begin()),
 	  itend(coeffs.end()); it != itend; ++it) {

      for (int i = 0; i < frame.n_p(); i++) {
	if ( i == it.index().p()) {
	  result[i].add(*it, evaluate(frame, it.index(), primal, resolution));
	}
	else
	  if ( frame.atlas()->get_adjacency_matrix().get_entry(i,it.index().p()) ) {
	    result[i].add(*it, evaluate(frame, it.index(), i, primal, resolution));
	  }
      }
    }
    return result;
  }

  template <class IBASIS>
  Array1D<SampledMapping<1> >
  EvaluateFrame<IBASIS,1,1>::evaluate_difference(const AggregatedFrame<IBASIS,1,1>& frame,
						 const InfiniteVector<double,
						 typename AggregatedFrame<IBASIS,1,1>::Index>& coeffs,
						 const Function<1>& f,
						 const int resolution) const
  {
    Array1D<SampledMapping<1> > result(frame.n_p());

    result = evaluate(frame, coeffs, true, resolution);

    for (int i = 0; i < frame.n_p(); i++) {
      result[i].add(-1.,SampledMapping<1>(result[i],f)); // SampledMapping<1> extends Grid<1> --> all fine
    }
    return result;
  }


  //Riemann approximation of pointwise error
  template <class IBASIS>
  double
  EvaluateFrame<IBASIS,1,1>::L_2_error(const AggregatedFrame<IBASIS,1,1>& frame,
				       const InfiniteVector<double,
				       typename AggregatedFrame<IBASIS,1,1>::Index>& coeffs,
				       const Function<1>& f,
				       const int resolution,
				       const double a, const double b) const
  {
    typedef typename AggregatedFrame<IBASIS,1,1>::Index Index;
    const unsigned int N = (1 << resolution);//number of points where difference has to be evaluated
    const double dx = (b-a)/N;
    double res = 0;
    //evaluate exact solution
    //SampledMapping<1> exact_solution(Grid<1>(0, 1, N+1),f);

    const unsigned int N_Gauss = 2;

    Array1D<double> gauss_points, gauss_weights;
    gauss_points.resize(N_Gauss*N);
    gauss_weights.resize(N_Gauss*N);

    for (unsigned int k = 0; k < N ; k++) {
      for (unsigned int n = 0; n < N_Gauss ; n++) {
	double u = k*dx;
	//double v = (k+1)*dx;
	gauss_points[k*N_Gauss+n]
	  = (GaussPoints[N_Gauss-1][n]+1)*0.5*dx+u;
	gauss_weights[k*N_Gauss+n]
	  = dx*GaussWeights[N_Gauss-1][n];
      }
    }

    double approx = 0.;
    for (unsigned int k = 0; k < N ; k++) {
      for (unsigned int n = 0; n < N_Gauss ; n++) {
	Point<1> p(gauss_points[k*N_Gauss+n]);
	//cout << "gp = " << gauss_points[k*N_Gauss+n] << endl;
	for(typename InfiniteVector<double,Index>::const_iterator it(coeffs.begin()),
	      itend(coeffs.end()); it != itend; ++it) {
	  if (in_support(frame, it.index(), p)) {
	    Point<1> p_0_1;
	    frame.atlas()->charts()[it.index().p()]->map_point_inv(p,p_0_1);
	    //cout << "point_int = " << p_0_1 << endl;
	    double wavVal = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[0]), 0,
						typename IBASIS::Index(it.index().j(),
								       it.index().e()[0],
								       it.index().k()[0],
								       frame.bases()[it.index().p()]->bases()[0]),

						p_0_1[0]);

	    approx += (*it)*(wavVal / frame.atlas()->charts()[(it.index()).p()]->Gram_factor(p_0_1));
	  }// end if
	}// end for

	approx -= f.value(p);
	approx *= approx;

	res += gauss_weights[k*N_Gauss+n]*approx;
	approx = 0;
      }// end for
    }//end for

    return sqrt(res);
  }


  //################################## 2D case starts here ######################################
  template <class IBASIS>
  SampledMapping<2>
  EvaluateFrame<IBASIS,2,2> ::evaluate(const AggregatedFrame<IBASIS,2,2>& frame,
				       const typename AggregatedFrame<IBASIS,2,2>::Index& lambda,
				       const unsigned int patch,
				       const bool primal,
				       const int resolution) const
  {

    Point<2> x;
    Point<2> x_patch;
    Point<2> y;

    const unsigned int  n_points = (1 << resolution)+1;
    const double h = 1.0 / (n_points-1);

    Matrix<double> gridx;
    Matrix<double> gridy;
    // setup grid

    Matrix<double> values;
    values.resize(n_points,n_points);

    gridx.resize(n_points,n_points);
    gridy.resize(n_points,n_points);

    for (unsigned int i = 0; i < n_points; i++)
    {
      x[1] = h*i;
      for (unsigned int j = 0; j < n_points; j++)
      {
        x[0] = h*j;
        frame.atlas()->charts()[patch]->map_point(x,x_patch);
        gridx.set_entry(i,j,x_patch[0]);
        gridy.set_entry(i,j,x_patch[1]);
        if ( in_support(frame, lambda, x_patch) )
        {
          frame.atlas()->charts()[lambda.p()]->map_point_inv(x_patch,y);
          double wav_val_x = WaveletTL::evaluate(*(frame.bases()[lambda.p()]->bases()[0]), 0,
						 typename IBASIS::Index(lambda.j(),
									lambda.e()[0],
									lambda.k()[0],
									frame.bases()[lambda.p()]->bases()[0]),
						 y[0]);
          double wav_val_y = WaveletTL::evaluate(*(frame.bases()[lambda.p()]->bases()[1]), 0,
						 typename IBASIS::Index(lambda.j(),
									lambda.e()[1],
									lambda.k()[1],
									frame.bases()[lambda.p()]->bases()[1]),
						 y[1]);
          values.set_entry(i,j,(wav_val_x*wav_val_y) / frame.atlas()->charts()[lambda.p()]->Gram_factor(y));
        }
        else
        {
          values.set_entry(i,j,0.);
        }
      }
    }
    return SampledMapping<2> (Grid<2>(gridx,gridy),values);
  }

 template <class IBASIS>
 SampledMapping<2>
 EvaluateFrame<IBASIS,2,2>::evaluate(const AggregatedFrame<IBASIS,2,2>& frame,
				     const typename AggregatedFrame<IBASIS,2,2>::Index& lambda,
				     const bool primal,
				     const int resolution) const
 {
   FixedArray1D<Array1D<double>,2> values; // point values of the factors within psi_lambda
   for (unsigned int i = 0; i < 2; i++)
     values[i] = WaveletTL::evaluate(*(frame.bases()[lambda.p()]->bases()[i]),
			  typename IBASIS::Index(lambda.j(),
						 lambda.e()[i],
						 lambda.k()[i],
						 frame.bases()[lambda.p()]->bases()[i]),
			  primal,
			  resolution).values();

   SampledMapping<2> result(*(frame.atlas()->charts()[lambda.p()]),
				values,
				resolution);

   return result;
 }

  template <class IBASIS>
  Array1D<SampledMapping<2> >
  EvaluateFrame<IBASIS,2,2>::evaluate(const AggregatedFrame<IBASIS,2,2>& frame,
                      const InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>& coeffs,
				      const bool primal,
				      const int resolution) const
  {

    Array1D<SampledMapping<2> > result(frame.n_p());

    for (int i = 0; i < frame.n_p(); i++) {
      result[i] = SampledMapping<2>(*(frame.atlas()->charts()[i]),resolution);// all zero
    }

    typedef typename AggregatedFrame<IBASIS,2,2>::Index Index;
    for (typename InfiniteVector<double,Index>::const_iterator it(coeffs.begin()),
 	  itend(coeffs.end()); it != itend; ++it)
    {

      for (int i = 0; i < frame.n_p(); i++)
      {
        if ( i == it.index().p())
        {
          result[i].add(*it, evaluate(frame, it.index(), primal, resolution));
        }
        else
        if ( frame.atlas()->get_adjacency_matrix().get_entry(i,it.index().p()) )
          result[i].add(*it, evaluate(frame, it.index(), i, primal, resolution));
      }
    }

    return result;
  }

  template <class IBASIS>
  Array1D<SampledMapping<2> >
  EvaluateFrame<IBASIS,2,2>::evaluate_difference(const AggregatedFrame<IBASIS,2,2>& frame,
						 const InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>& coeffs,
						 const Function<2>& f,
						 const int resolution) const
  {
    Array1D<SampledMapping<2> > result(frame.n_p());

    result = evaluate(frame, coeffs, true, resolution);

    for (int i = 0; i < frame.n_p(); i++) {
      result[i].add(-1.,SampledMapping<2>(result[i],f)); // SampledMapping<2> extends Grid<2> --> all fine
    }
    return result;

  }


  // will only work for our standard L-shaped domain
  template <class IBASIS>
  double
  EvaluateFrame<IBASIS,2,2>::L_2_error(const AggregatedFrame<IBASIS,2,2>& frame,
				       const InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>& coeffs,
				       const Function<2>& f,
				       const int resolution,
				       const double a, const double b) const
  {
    double res = 0.;
    typedef typename AggregatedFrame<IBASIS,2,2>::Index Index;
    const unsigned int N = (1 << resolution);//number of points where difference has to be evaluated
    const double dx = 1./N;

    const unsigned int N_Gauss = 2;

    // loop over all non overlapping squares forming the L-shaped domain
    for (unsigned int helpPatch = 0; helpPatch < 3; helpPatch++)
    {
      double a1, b1, a2, b2;
      switch(helpPatch)
      {
          case 0:
          {
            a1 = 0.;
            b1 = 1.;
            a2 = -1.;
            b2 = 0.;
            break;
          }
          case 1:
          {
            a1 = -1.;
            b1 = 0.;
            a2 = -1.;
            b2 = 0.;
            break;
          }
          case 2:
          {
            a1 = -1;
            b1 = 0.;
            a2 = 0.;
            b2 = 1.;
            break;
          }
      }

      FixedArray1D<Array1D<double>,2> gauss_points, gauss_weights;
      for (unsigned int i = 0; i < 2; i++)
      {
	    gauss_points[i].resize(N_Gauss*N);
	    gauss_weights[i].resize(N_Gauss*N);
      }

      for (unsigned int k = 0; k < N ; k++)
      {
	    for (unsigned int n = 0; n < N_Gauss ; n++)
	    {
          double u = a1 + k*dx;
          double v = a1 + (k+1)*dx;
          gauss_points[0][k*N_Gauss+n] = (GaussPoints[N_Gauss-1][n]+1)*0.5*dx+u;
          gauss_weights[0][k*N_Gauss+n] = dx*GaussWeights[N_Gauss-1][n];
	    }
      }

      for (unsigned int k = 0; k < N ; k++)
      {
	    for (unsigned int n = 0; n < N_Gauss ; n++)
	    {
          double u = a2 + k*dx;
          double v = a2 + (k+1)*dx;
          gauss_points[1][k*N_Gauss+n] = (GaussPoints[N_Gauss-1][n]+1)*0.5*dx+u;
          gauss_weights[1][k*N_Gauss+n] = dx*GaussWeights[N_Gauss-1][n];
	    }
      }
      //##################################################

      double approx = 0.;
      for (unsigned int k1 = 0; k1 < N ; k1++)
      {
	    for (unsigned int n1 = 0; n1 < N_Gauss ; n1++)
	    {
	      for (unsigned int k2 = 0; k2 < N ; k2++)
	      {
	        for (unsigned int n2 = 0; n2< N_Gauss ; n2++)
	        {
              Point<2> p(gauss_points[0][k1*N_Gauss+n1],gauss_points[1][k2*N_Gauss+n2]);
              for(typename InfiniteVector<double,Index>::const_iterator it(coeffs.begin()),
                  itend(coeffs.end()); it != itend; ++it)
              {
                if (in_support(frame, it.index(), p))
                {
                  Point<2> p_0_1;
                  frame.atlas()->charts()[it.index().p()]->map_point_inv(p,p_0_1);
                  double wav_val_x = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[0]), 0,
                                     typename IBASIS::Index(it.index().j(),
                                                it.index().e()[0],
                                                it.index().k()[0],
                                                frame.bases()[it.index().p()]->bases()[0]),
                                     p_0_1[0]);
                  double wav_val_y = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[1]), 0,
                                     typename IBASIS::Index(it.index().j(),
                                                it.index().e()[1],
                                                it.index().k()[1],
                                                frame.bases()[it.index().p()]->bases()[1]),
                                     p_0_1[1]);
                  approx += (*it)*(wav_val_x*wav_val_y) / frame.atlas()->charts()[it.index().p()]->Gram_factor(p_0_1);
                }// end if
              }// end for over coeffs
              approx -= f.value(p);
              approx *= approx;
              res += gauss_weights[0][k1*N_Gauss+n1]*gauss_weights[1][k2*N_Gauss+n2]*approx;
              approx = 0;
	        }
	      }
	    }
      }
      cout << "done patch " << helpPatch << endl;
    }//end for helpPatch

    return sqrt(res);
  }



    template <class IBASIS>
    double
    EvaluateFrame<IBASIS,2,2>::evaluate(const AggregatedFrame<IBASIS,2,2>& frame,
			const InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>& coeffs,
			const Point<2> x) const
    {
      double r = 0.0;

      for (typename InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>::const_iterator it(coeffs.begin()), itend(coeffs.end()); it != itend; ++it)
      {

        r += ( (*it) *  frame.evaluate(it.index(), x) );

      }

      return r;
    }


    template <class IBASIS>
    Matrix<double>
    EvaluateFrame<IBASIS,2,2>::evaluate(const AggregatedFrame<IBASIS,2,2>& frame,
			const typename AggregatedFrame<IBASIS,2,2>::Index& lambda,
			const FixedArray1D<Array1D<double>,2>& grid) const
    {

      Matrix<double> r(grid[0].size(), grid[1].size());

      Point<2> x;
      Point<2> x_patch;

      x[0] = 0.0;
      x[1] = 0.0;

      const double gram_factor = frame.atlas()->charts()[lambda.p()]->Gram_factor(x);

      // values of Psi_x on x-grid
      MathTL::Array1D<double> values_x(grid[0].size());
      x_patch[1] = 0.0;
      for (unsigned int i(0); i < values_x.size(); i++)
      {
        x_patch[0] = grid[0][i];
        frame.atlas()->charts()[lambda.p()]->map_point_inv(x_patch, x);

        values_x[i] = WaveletTL::evaluate(*(frame.bases()[lambda.p()]->bases()[0]), 0,
						 typename IBASIS::Index(lambda.j(),
									lambda.e()[0],
									lambda.k()[0],
									frame.bases()[lambda.p()]->bases()[0]),
						 x[0]) / gram_factor;
      }

      // values of Psi_y on y-grid
      MathTL::Array1D<double> values_y(grid[1].size());
      x_patch[0] = 0.0;
      for (unsigned int i(0); i < values_y.size(); i++)
 	  {
        x_patch[1] = grid[1][i];
        frame.atlas()->charts()[lambda.p()]->map_point_inv(x_patch, x);

        values_y[i] = WaveletTL::evaluate(*(frame.bases()[lambda.p()]->bases()[1]), 0,
						 typename IBASIS::Index(lambda.j(),
									lambda.e()[1],
									lambda.k()[1],
									frame.bases()[lambda.p()]->bases()[1]),
						 x[1]);
      }

      // compute tensor product
      for (unsigned int m(0); m < r.row_dimension(); m++)
        for (unsigned int n(0); n < r.column_dimension(); n++)
	      r(m,n) = values_x[m] * values_y[n];

      return r;

    }



    template <class IBASIS>
    Matrix<double>
    EvaluateFrame<IBASIS,2,2>::evaluate(const AggregatedFrame<IBASIS,2,2>& frame,
             const InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>& coeffs,
             const FixedArray1D<Array1D<double>,2>& grid) const
    {
      Matrix<double> r(grid[0].size(), grid[1].size());

      MathTL::Array1D<double> values_x(grid[0].size());
      MathTL::Array1D<double> values_y(grid[1].size());

      Point<2> x;
      Point<2> x_patch;

      x[0] = 0.0;
      x[1] = 0.0;

      typedef typename AggregatedFrame<IBASIS,2,2>::Index Index;

      //! iterate over all frames in 'coeffs'
      for (typename InfiniteVector<double,Index>::const_iterator it(coeffs.begin()),
	       itend(coeffs.end()); it != itend; ++it)
      {

        double gram_factor = frame.atlas()->charts()[it.index().p()]->Gram_factor(x);   // = const, i.e. independent of x

        // values of Psi_x on x-grid
        x_patch[1] = 0.0;
        for (unsigned int i(0); i < values_x.size(); i++)
        {
          x_patch[0] = grid[0][i];
          frame.atlas()->charts()[it.index().p()]->map_point_inv(x_patch, x);

          values_x[i] = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[0]), 0,
						 typename IBASIS::Index(it.index().j(),
									it.index().e()[0],
									it.index().k()[0],
									frame.bases()[it.index().p()]->bases()[0]),
						 x[0]) / gram_factor;
        }

        // values of Psi_y on y-grid
        x_patch[0] = 0.0;
        for (unsigned int i(0); i < values_y.size(); i++)
 	    {
          x_patch[1] = grid[1][i];
          frame.atlas()->charts()[it.index().p()]->map_point_inv(x_patch, x);

          values_y[i] = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[1]), 0,
						 typename IBASIS::Index(it.index().j(),
									it.index().e()[1],
									it.index().k()[1],
									frame.bases()[it.index().p()]->bases()[1]),
						 x[1]);
        }


        for (unsigned int m(0); m < r.row_dimension(); m++)
        {
          for (unsigned int n(0); n < r.column_dimension(); n++)
          {
	        r(m,n) += ( (*it) * values_x[m] * values_y[n] );
          }
        }

      }

      return r;
    }


    template <class IBASIS>
    void
    EvaluateFrame<IBASIS,2,2>::evaluate_deriv(const AggregatedFrame<IBASIS,2,2>& frame,
			const InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>& coeffs,
			const Point<2> x,
			FixedArray1D<double,2>& deriv_values) const
    {
      Vector<double> share(2);
      deriv_values[0] = 0.0;
      deriv_values[1] = 0.0;

      for (typename InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>::const_iterator it(coeffs.begin()), itend(coeffs.end()); it != itend; ++it)
      {
        if ( (x[0] > frame.all_patch_supports[it.index().number()].a[0]) && (x[0] < frame.all_patch_supports[it.index().number()].b[0]) &&
             (x[1] > frame.all_patch_supports[it.index().number()].a[1]) && (x[1] < frame.all_patch_supports[it.index().number()].b[1]) )
        {
          frame.evaluate_gradient(it.index(), x, share);
          deriv_values[0] += ( (*it) *  share[0] );
          deriv_values[1] += ( (*it) *  share[1] );
        }
      }
    }



    //! tuned version: ATTENTION: ONLY FOR THE CASE KAPPA = SimpleAffineLinear

    template <class IBASIS>
    void
    EvaluateFrame<IBASIS,2,2>::evaluate_deriv_tuned(const AggregatedFrame<IBASIS,2,2>& frame,
			const InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>& coeffs,
			const Point<2> x,
			FixedArray1D<double,2>& deriv_values) const
    {
      deriv_values[0] = 0.0;
      deriv_values[1] = 0.0;
      Point<2> p_d;
      Chart<2,2>* chart;
      double gram_factor;
      Vector<double> cube_wavelet_components(2);
      Vector<double> gradient_cube_wavelet(2);
      FixedArray1D<IBASIS*,2> bases1D_lambda = frame.bases()[0]->bases();
      typename AggregatedFrame<IBASIS,2,2>::Index lambda;
      int la_number;

      for (typename InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>::const_iterator it(coeffs.begin()), itend(coeffs.end()); it != itend; ++it)
      {
        la_number = it.index().number();
        if ( (x[0] > frame.all_patch_supports[la_number].a[0]) && (x[0] < frame.all_patch_supports[la_number].b[0]) &&
             (x[1] > frame.all_patch_supports[la_number].a[1]) && (x[1] < frame.all_patch_supports[la_number].b[1]) )
        {
          lambda = it.index();
          // get the parametric mapping corresponding to the frame element
          chart = frame.atlas()->charts()[lambda.p()];

          // pull back the point x from the domain into the unit cube
          chart->map_point_inv(x, p_d);


          // evaluate the tensor factors of the reference wavelet
          cube_wavelet_components[0] = WaveletTL::evaluate(*(bases1D_lambda[0]), 0, lambda.j(), lambda.e()[0], lambda.k()[0], p_d[0]);
          cube_wavelet_components[1] = WaveletTL::evaluate(*(bases1D_lambda[1]), 0, lambda.j(), lambda.e()[1], lambda.k()[1], p_d[1]);

          // compute the gradient of the reference cube wavelet
          gradient_cube_wavelet[0] = WaveletTL::evaluate(*(bases1D_lambda[0]), 1, lambda.j(), lambda.e()[0], lambda.k()[0], p_d[0]) * cube_wavelet_components[1];
          gradient_cube_wavelet[1] = WaveletTL::evaluate(*(bases1D_lambda[1]), 1, lambda.j(), lambda.e()[1], lambda.k()[1], p_d[1]) * cube_wavelet_components[0];


          // setup the gradient of the wavelet frame element
          // we use quotient and chain rule to derive the gradient of eq. (2.3.9)
          gram_factor = (chart->Gram_factor(p_d));

          deriv_values[0] += ( (*it) *  gradient_cube_wavelet[0] * chart->Dkappa_inv(0, 0, p_d) / gram_factor );
          deriv_values[1] += ( (*it) *  gradient_cube_wavelet[1] * chart->Dkappa_inv(1, 1, p_d) / gram_factor );
        }

      }

    }


    template <class IBASIS>
    void
    EvaluateFrame<IBASIS,2,2>::evaluate(const AggregatedFrame<IBASIS,2,2>& frame,
             const InfiniteVector<double, typename AggregatedFrame<IBASIS,2,2>::Index>& coeffs,
             const FixedArray1D<Array1D<double>,2>& grid,
             Matrix<double>& deriv_x, Matrix<double>& deriv_y) const
    {

      deriv_x.resize(grid[0].size(), grid[1].size());
      deriv_y.resize(grid[0].size(), grid[1].size());

      MathTL::Array1D<double> values_x(grid[0].size());
      MathTL::Array1D<double> values_y(grid[1].size());

      MathTL::Array1D<double> deriv_values_x(grid[0].size());
      MathTL::Array1D<double> deriv_values_y(grid[1].size());


      Point<2> x;
      Point<2> x_patch;

      x[0] = 0.0;
      x[1] = 0.0;

      typedef typename AggregatedFrame<IBASIS,2,2>::Index Index;

      //! iterate over all frames in 'coeffs'
      for (typename InfiniteVector<double,Index>::const_iterator it(coeffs.begin()),
	       itend(coeffs.end()); it != itend; ++it)
      {

        double gram_factor = frame.atlas()->charts()[it.index().p()]->Gram_factor(x);   // = const, i.e. independent of x
        double D_Kappa_inv_00 = frame.atlas()->charts()[it.index().p()]->Dkappa_inv(0, 0, x);    // = const, i.e. independent of x
        double D_Kappa_inv_11 = frame.atlas()->charts()[it.index().p()]->Dkappa_inv(1, 1, x);    // = const, i.e. independent of x

        // values of Psi_x on x-grid
        x_patch[1] = 0.0;
        for (unsigned int i(0); i < values_x.size(); i++)
        {
          x_patch[0] = grid[0][i];
          frame.atlas()->charts()[it.index().p()]->map_point_inv(x_patch, x);

          values_x[i] = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[0]), 0,
						 typename IBASIS::Index(it.index().j(),
									it.index().e()[0],
									it.index().k()[0],
									frame.bases()[it.index().p()]->bases()[0]),
						 x[0]);

          deriv_values_x[i] = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[0]), 1,
						 typename IBASIS::Index(it.index().j(),
									it.index().e()[0],
									it.index().k()[0],
									frame.bases()[it.index().p()]->bases()[0]),
						 x[0]) * D_Kappa_inv_00 / gram_factor;

        }


        // values of Psi_y on y-grid
        x_patch[0] = 0.0;
        for (unsigned int i(0); i < values_y.size(); i++)
 	    {
          x_patch[1] = grid[1][i];
          frame.atlas()->charts()[it.index().p()]->map_point_inv(x_patch, x);

          values_y[i] = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[1]), 0,
						 typename IBASIS::Index(it.index().j(),
									it.index().e()[1],
									it.index().k()[1],
									frame.bases()[it.index().p()]->bases()[1]),
						 x[1]);

          deriv_values_y[i] = WaveletTL::evaluate(*(frame.bases()[it.index().p()]->bases()[1]), 1,
						 typename IBASIS::Index(it.index().j(),
									it.index().e()[1],
									it.index().k()[1],
									frame.bases()[it.index().p()]->bases()[1]),
						 x[1]) * D_Kappa_inv_11 / gram_factor;

        }


        for (unsigned int m(0); m < deriv_x.row_dimension(); m++)
        {
          for (unsigned int n(0); n < deriv_x.column_dimension(); n++)
          {
	        deriv_x(m,n) += ( (*it) * deriv_values_x[m] * values_y[n]  );
	        deriv_y(m,n) += ( (*it) * values_x[m] * deriv_values_y[n] );
          }
        }

      }




    }


}
