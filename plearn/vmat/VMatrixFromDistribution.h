
// -*- C++ -*-

// VMatrixFromDistribution.h
//
// Copyright (C) 2003  Pascal Vincent 
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
   * $Id: VMatrixFromDistribution.h,v 1.3 2003/07/04 18:21:06 jkeable Exp $ 
   ******************************************************* */

/*! \file VMatrixFromDistribution.h */
#ifndef VMatrixFromDistribution_INC
#define VMatrixFromDistribution_INC

#include "VMatrix.h"
#include "PDistribution.h"

namespace PLearn <%
using namespace std;

class VMatrixFromDistribution: public VMatrix
{
protected:
  
  //! Will hold the data sampled from the distribution
  Mat data;  
    
public:

  typedef VMatrix inherited;

  // ************************
  // * public build options *
  // ************************

  PP<PDistribution> distr; // the distribution
  long generator_seed; // the generator_seed to initialize the distribution
  int nsamples; // number of samples to draw

  // for density mode:
  bool density_mode;
  bool logdensity_mode;
  Vec mins,maxs; // the min/max for each dimensions
  int samples_per_dim; // the number of samples per dimensions


  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  VMatrixFromDistribution();

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  void build_();

protected: 
  //! Declares this class' options
  static void declareOptions(OptionList& ol);

public:
  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Provides a help message describing this class
  static string help();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  // Declares other standard object methods
  //  If your class is not instantiatable (it has pure virtual methods)
  // you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS 
  PLEARN_DECLARE_OBJECT_METHODS(VMatrixFromDistribution, "VMatrixFromDistribution", VMatrix);


  // ******************
  // * VMatrix methods *
  // ******************

  virtual real get(int i, int j) const;
  virtual void getSubRow(int i, int j, Vec v) const;
  virtual void getRow(int i, Vec v) const;
  virtual void getColumn(int i, Vec v) const;
  virtual void getMat(int i, int j, Mat m) const;
  virtual Mat toMat() const;

};

// Declares a few other classes and functions related to this class
  DECLARE_OBJECT_PTR(VMatrixFromDistribution);
  
%> // end of namespace PLearn

#endif
