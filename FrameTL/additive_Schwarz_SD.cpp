// implementation for steepest_descent.h

#include <cmath>
#include <set>
#include <utils/plot_tools.h>
#include <adaptive/apply.h>
#include <numerics/corner_singularity.h>
#include <frame_evaluate.h>
#include <poisson_1d_testcase.h>

using std::set;

namespace FrameTL
{

  template <class PROBLEM>
  void  additive_Schwarz_SD_SOLVE(const PROBLEM& P,  const double epsilon,
				  InfiniteVector<double, typename PROBLEM::Index>& u_epsilon)
{
    //typedef DSBasis<2,2> Basis1D;
    typedef PBasis<3,3> Basis1D;	

    Point<2> origin;
    origin[0] = 0.0;
    origin[1] = 0.0;
    
    CornerSingularity sing2D(origin, 0.5, 1.5);
    CornerSingularityRHS singRhs(origin, 0.5, 1.5);

    const int jmax = 4;
    typedef typename PROBLEM::Index Index;

    double a_inv     = P.norm_Ainv();
    double omega_i   = a_inv*P.F_norm();

    InfiniteVector<double, Index> u_k, u_k_1_2, r, tmp_r, help, help2, f, Av, r_exact;
    
    
    
    map<double,double> log_10_residual_norms;
    map<double,double> degrees_of_freedom;
    map<double,double> asymptotic;
    map<double,double> time_asymptotic;
    map<double,double> log_10_L2_error;

    EvaluateFrame<Basis1D,2,2> evalObj;

    double eta = 2;

    const int number_patches = P.basis().n_p();

    //const double alpha = 0.35;//pbasis 1D 3 3, 0.7x0.7
    double alpha = 0.19;

    unsigned int global_iterations = 0;
    double tmp = 5.;

    bool exit = 0;
    double time = 0;

    clock_t tstart, tend;
    tstart = clock();

    while (tmp > 0.0005) {

      cout << "reentering global loop " << endl;

//       //approximate residual
//       P.RHS(eta, f);
//       APPLY_COARSE(P, u_k, eta, help, 0.00000001, jmax, CDD1);
//       r = f - help;

      // compute exact residual
      P.RHS(0., f);
      APPLY(P, u_k, 0., help, jmax, CDD1);
      r_exact = f - help;

      r_exact.COARSE(eta, r);


      tmp = l2_norm(r_exact);
      cout << "residual norm = " << tmp  << endl;
      double tmp1 = log10(tmp);

      // ######################## OUTPUT #################################
      tend = clock();
      //time = (double)((tend-tstart)/CLOCKS_PER_SEC);
      time += ((double) (tend-tstart))/((double) CLOCKS_PER_SEC);
      time_asymptotic[log10(time)] = tmp1;


      if (u_k.size() != 0)
	asymptotic[log10( (double)u_k.size() )] = tmp1;
      //std::ofstream os3("add_schwarz_asymptotic_33_1D_1905.m");
      std::ofstream os3("add_schwarz_asymptotic_SD_33_2D_2605.m");
      matlab_output(asymptotic,os3);
      os3.close();

      //std::ofstream os4("add_schwarz_time_asymptotic_33_1D_1905.m");
      std::ofstream os4("add_schwarz_time_asymptotic_SD_33_2D_2605.m");
      matlab_output(time_asymptotic,os4);
      os4.close();
      
      tstart = clock();
      // ######################## END OUTPUT #############################

      // setup local index set
      set<Index> Lambda;
      r.support(Lambda);
      
      for (int i = 0; i < number_patches; i++) {
	cout << "doing patch " << i << endl;
	set<Index> local_index_set;

	typename set<Index>::const_iterator it = Lambda.begin();

	for (; it != Lambda.end(); ++it) {
	  if ((*it).p() == i) {
	    local_index_set.insert(*it);
	  }
	}
	
	if (local_index_set.size() == 0)
	  continue;

	// setup local stiffness matrix
	cout << "setting up full stiffness matrix..." << endl;
	SparseMatrix<double> A_Lambda_i;
	WaveletTL::setup_stiffness_matrix(P, local_index_set, A_Lambda_i);
	
	
	// setup local right hand side
	InfiniteVector<double, Index> r_i;
	typename InfiniteVector<double, Index>::const_iterator it2 = r.begin();
	for (; it2 != r.end(); ++it2) {
	  if ((it2.index()).p() == i) {
	    r_i.set_coefficient(it2.index(), *it2);
	  }
	}
	
	cout << "setting up full right hand side..." << endl;
	Vector<double> F_Lambda_i(r_i.size());
	unsigned int id = 0;
	typename InfiniteVector<double, Index>::const_iterator it3 = r_i.begin();
	for (; it3 != r_i.end(); ++it3, ++id) {
	  F_Lambda_i[id] = *it3;
	}
	
	// compute approximation to local problem
	Vector<double> xk(local_index_set.size());
	cout << "r_1 size = " << r_i.size() << endl;
	unsigned int iterations = 0;
	CG(A_Lambda_i, F_Lambda_i, xk, 0.0001, 300, iterations);
	cout << "CG done!!!!" << endl;           
	
	help.clear();
	
	id = 0;
	for (typename set<Index>::const_iterator it = local_index_set.begin(), itend = local_index_set.end();
	     it != itend; ++it, ++id)
	  help.set_coefficient(*it, xk[id]);


	//u_k = u_k + alpha*help;
	// setting up the current residual patch by patch
	tmp_r = tmp_r + help;
		
      }// end for loop over patches
      
      help.clear();
      
      // ###################### compute descent parameter ############################
      cout << "computing descent parameter..." << endl;
      APPLY_COARSE(P, tmp_r, eta/2., help, 0.00000001, jmax, CDD1);
      // next: apply M^-1 to help = A r
      // setup local index set
      Lambda.clear();
      help.support(Lambda);

      for (int i = 0; i < number_patches; i++) {
	cout << "doing patch " << i << endl;
	set<Index> local_index_set;

	typename set<Index>::const_iterator it = Lambda.begin();

	for (; it != Lambda.end(); ++it) {
	  if ((*it).p() == i) {
	    local_index_set.insert(*it);
	  }
	}
	
	if (local_index_set.size() == 0)
	  continue;

	// setup local stiffness matrix
	cout << "setting up full stiffness matrix..." << endl;
	SparseMatrix<double> A_Lambda_i;
	WaveletTL::setup_stiffness_matrix(P, local_index_set, A_Lambda_i);
	
	// setup local right hand side
	InfiniteVector<double, Index> r_i;
	typename InfiniteVector<double, Index>::const_iterator it2 = help.begin();
	for (; it2 != help.end(); ++it2) {
	  if ((it2.index()).p() == i) {
	    r_i.set_coefficient(it2.index(), *it2);
	  }
	}
	
	cout << "setting up full right hand side..." << endl;
	Vector<double> F_Lambda_i(r_i.size());
	unsigned int id = 0;
	typename InfiniteVector<double, Index>::const_iterator it3 = r_i.begin();
	for (; it3 != r_i.end(); ++it3, ++id) {
	  F_Lambda_i[id] = *it3;
	}
	
	// compute approximation to local problem
	Vector<double> xk(local_index_set.size());
	cout << "r_1 size = " << r_i.size() << endl;
	unsigned int iterations = 0;
	cout << "entering CG!!!!" << endl;           
	CG(A_Lambda_i, F_Lambda_i, xk, 0.0001, 300, iterations);
	cout << "CG done!!!!" << endl;           
	
	help2.clear();
	
	id = 0;
	for (typename set<Index>::const_iterator it = local_index_set.begin(), itend = local_index_set.end();
	     it != itend; ++it, ++id)
	  help2.set_coefficient(*it, xk[id]);


	//u_k = u_k + alpha*help;
	// setting up the current residual patch by patch
	help = help + help2;
	
	alpha = ((tmp_r * tmp_r)/(tmp_r * help));

      }// end for loop over patches
      cout << "finished computing descent parameter..." << endl;
      // ###################### end compute descent parameter ########################

      r = tmp_r;
      tmp_r.clear();
      help.clear();

      u_k = u_k + alpha*r;

      global_iterations++;
      
      eta *= 0.85;
          
      u_epsilon = u_k;
      
      cout << global_iterations <<" loops completed" << endl;
      
//       if (tmp < 0.0009) {
// 	u_epsilon = u_k;
// 	u_epsilon.scale(&P,-1);
// 	char filename1[50];
// 	char filename2[50];
	
// 	sprintf(filename1, "%s%d%s%d%s", "approx_sol_add_schwarz33_2D_out_", global_iterations, "_nactive_", u_k.size(),".m");
// 	sprintf(filename2, "%s%d%s%d%s", "error_add_schwarz_2D_out_", global_iterations, "_nactive_", u_k.size(),".m");
// 	cout << "...plotting approximate solution" << endl;
// 	Array1D<SampledMapping<2> > U = evalObj.evaluate(P.basis(), u_epsilon, true, 6);//expand in primal basis
	
// 	std::ofstream ofs5(filename1);
// 	matlab_output(ofs5,U);
// 	ofs5.close();



// 	Array1D<SampledMapping<2> > Error = evalObj.evaluate_difference(P.basis(), u_epsilon, sing2D, 6);
//  	cout << "...plotting error" << endl;
// 	std::ofstream ofs6(filename2);
// 	matlab_output(ofs6, Error);
// 	ofs6.close();
	
	
//       }
    }

 




  }
}
