// -*- C++ -*-

// ConditionalGaussMix.cc
//
// Copyright (C) 2004 Olivier Delalleau 
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
// 
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
// 
//  3. The name of the authors may not be used to endorse or promote
//     products derived from this software without specific prior written
//     permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
// NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
// TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// 
// This file is part of the PLearn library. For more information on the PLearn
// library, go to the PLearn Web site at www.plearn.org

/* *******************************************************      
   * $Id: ConditionalGaussMix.cc,v 1.2 2004/05/20 13:02:38 tihocan Exp $ 
   ******************************************************* */

// Authors: Olivier Delalleau

/*! \file ConditionalGaussMix.cc */


#include "ConditionalGaussMix.h"
#include "pl_erf.h"
#include "plapack.h"

namespace PLearn {
using namespace std;

//////////////////
// ConditionalGaussMix //
//////////////////
ConditionalGaussMix::ConditionalGaussMix() 
{}

PLEARN_IMPLEMENT_OBJECT(ConditionalGaussMix,
    "Models the conditional probability p(y|x), when p(x,y) is a mixture of Gaussians.",
    ""
);

////////////////////
// declareOptions //
////////////////////
void ConditionalGaussMix::declareOptions(OptionList& ol)
{
  // ### Declare all of this object's options here
  // ### For the "flags" of each option, you should typically specify  
  // ### one of OptionBase::buildoption, OptionBase::learntoption or 
  // ### OptionBase::tuningoption. Another possible flag to be combined with
  // ### is OptionBase::nosave

  // Build options.

  declareOption(ol, "gauss_mix", &ConditionalGaussMix::gauss_mix, OptionBase::buildoption,
      "The mixture of Gaussians trained on the joint probability.");

  // Learnt options.


  declareOption(ol, "eigenvalues_x", &ConditionalGaussMix::eigenvalues_x, OptionBase::learntoption,
      "The eigenvalues of the top left part of the covariance matrix for each Gaussian.\n"
      "The element (j,k) is the k-th eigenvalue of the j-th Gaussian.");

  declareOption(ol, "eigenvectors_x", &ConditionalGaussMix::eigenvectors_x, OptionBase::learntoption,
      "The eigenvectors of the top left part of the covariance matrix for each Gaussian.\n"
      "The j-th element is the matrix with eigenvectors of the j-th Gaussian, in rows.");

  declareOption(ol, "mu_x", &ConditionalGaussMix::mu_x, OptionBase::learntoption,
      "The average of x according to each Gaussian: the element (j,k) is the k-th\n"
      "component of the average of x according to Gaussian j.");

  declareOption(ol, "sigma", &ConditionalGaussMix::sigma, OptionBase::learntoption,
      "The global standard deviation when using spherical Gaussians.");

  // Now call the parent class' declareOptions().
  inherited::declareOptions(ol);
}

///////////
// build //
///////////
void ConditionalGaussMix::build()
{
  // ### Nothing to add here, simply calls build_().
  inherited::build();
  build_();
}

////////////
// build_ //
////////////
void ConditionalGaussMix::build_()
{
  // ### This method should do the real building of the object,
  // ### according to set 'options', in *any* situation. 
  // ### Typical situations include:
  // ###  - Initial building of an object from a few user-specified options
  // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
  // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
  // ### You should assume that the parent class' build_() has already been called.
}

/////////
// cdf //
/////////
double ConditionalGaussMix::cdf(const Vec& x) const
{
  PLERROR("cdf not implemented for ConditionalGaussMix"); return 0;
}

/////////////////
// expectation //
/////////////////
void ConditionalGaussMix::expectation(Vec& mu) const
{
  string type = gauss_mix->type;
  if (type == "spherical") {

  } else {
    PLERROR("In ConditionalGaussMix::expectation - Not implemented for this type");
  }

}

////////////
// forget //
////////////
void ConditionalGaussMix::forget()
{
  /*!
    A typical forget() method should do the following:
    - initialize a random number generator with the seed option
    - initialize the learner's parameters, using this random generator
    - stage = 0
   */
  gauss_mix->forget();
  stage = 0;
  eigenvalues_x = Mat();
  eigenvectors_x = TVec<Mat>();
  mu_x = Mat();
  sigma = Vec();
}

//////////////
// generate //
//////////////
void ConditionalGaussMix::generate(Vec& x) const
{
  PLERROR("generate not implemented for ConditionalGaussMix");
}

// ### Default version of inputsize returns learner->inputsize()
// ### If this is not appropriate, you should uncomment this and define
// ### it properly here:
// int ConditionalGaussMix::inputsize() const {}

/////////////////
// log_density //
/////////////////
double ConditionalGaussMix::log_density(const Vec& x) const
{
  PLERROR("density not implemented for ConditionalGaussMix"); return 0; 
}

/////////////////////////////////
// makeDeepCopyFromShallowCopy //
/////////////////////////////////
void ConditionalGaussMix::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
{
  inherited::makeDeepCopyFromShallowCopy(copies);

  // ### Call deepCopyField on all "pointer-like" fields 
  // ### that you wish to be deepCopied rather than 
  // ### shallow-copied.
  // ### ex:
  // deepCopyField(trainvec, copies);

  // ### Remove this line when you have fully implemented this method.
  PLERROR("ConditionalGaussMix::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

////////////////////
// resetGenerator //
////////////////////
void ConditionalGaussMix::resetGenerator(long g_seed) const
{
  PLERROR("resetGenerator not implemented for ConditionalGaussMix");
}

//////////////
// setInput //
//////////////
void ConditionalGaussMix::setInput(const Vec& input) const {
  // Let x = input. We precompute p(x) and the p_j(x):
  // p(x) = sum_j alpha_j p_j(x),
  // with p_j(x) ~= N(mu_j, C_j), where:
  //  - mu_j is the mean of the x part for the j-th Gaussian
  //  - C_j is the x part (the top-left corner) of the covariance matrix
  //    for the j-th Gaussian: sum_k (lambda_k - lambda0) (vx_k vx_k') + lambda0.I
  //    with vx_k the x part of the k-th eigenvector.
  int L = gauss_mix->L;
  int d = input_part_size;
  string type = gauss_mix->type;
  Vec log_pj_x(L);
  for (int j = 0; j < L; j++) {
    if (type == "spherical") {
      real p = 0.0;
      for (int k = 0; k < d; k++) {
        p += gauss_log_density(input[k], mu_x(j, k), sigma[j]);
#ifdef BOUNDCHECK
        if (isnan(p)) {
          PLWARNING("In ConditionalGaussMix::setInput - Density is nan");
        }
#endif
        log_pj_x[j] = p;
      }
    } else {
      PLERROR("In ConditionalGaussMix::setInput - Not implemented for this type") ;
    }
  }
}

/////////////////
// survival_fn //
/////////////////
double ConditionalGaussMix::survival_fn(const Vec& x) const
{
  PLERROR("survival_fn not implemented for ConditionalGaussMix"); return 0;
}

///////////
// train //
///////////
void ConditionalGaussMix::train()
{
  int d = input_part_size;
  int L = gauss_mix->L;
  string type = gauss_mix->type;
  // Resize some stuff.
  mu_x.resize(L,d);
  eigenvalues_x.resize(0,0);
  eigenvectors_x.resize(0);
  sigma.resize(0);
  if (type == "spherical") {
    sigma.resize(L);
  } else if (type == "general") {
    eigenvalues_x.resize(L,d);
    eigenvectors_x.resize(L);
    for (int j = 0; j < L; j++) {
      eigenvectors_x[j].resize(d,d);
    }
  } else {
    PLERROR("In ConditionalGaussMix::train - Not implemented for this type");
  }
  
  // We train the mixture on the joint probability.
  gauss_mix->nstages = this->nstages;
  gauss_mix->train();
  this->stage = gauss_mix->stage;

  if (type == "spherical") {
    sigma << gauss_mix->sigma;
  } else if (type == "general") {
    // We will need the eigendecomposition of the different parts of the
    // covariance matrix for each Gaussian.
    Mat cov_x(d,d);
    Mat eigenvec;
    Vec eigenval;
    real lambda0;
    int n_eigen;
    for (int j = 0; j < L; j++) {
      cov_x.fill(0);
      n_eigen = gauss_mix->getNEigenComputed();
      eigenvec = gauss_mix->getEigenvectors(j);
      eigenval = gauss_mix->getEigenvals(j);
      lambda0 = eigenval[n_eigen - 1];
      for (int k = 0; k < n_eigen - 1; k++) {
        cov_x += (eigenval[k] - lambda0) *
          productTranspose(columnmatrix(eigenvec(k)), rowmatrix(eigenvec(k)));
      }
      // Now add lambda0 on the diagonal.
      for (int i = 0; i < d; i++) {
        cov_x(i,i) += lambda0;
      }
      // And compute the eigendecomposition of this matrix.
      /* Vec x(0);
         real truc = eigenval[x]; */
      Vec eigenvals = eigenvalues_x(j);
      eigenVecOfSymmMat(cov_x, d, eigenvals, eigenvectors_x[j]);
    }
  } else {
    PLERROR("In ConditionalGaussMix::train - Not implemented for this type");
  }
}

//////////////
// variance //
//////////////
void ConditionalGaussMix::variance(Mat& covar) const
{
  PLERROR("variance not implemented for ConditionalGaussMix");
}

} // end of namespace PLearn

