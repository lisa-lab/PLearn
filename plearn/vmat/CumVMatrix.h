// -*- C++ -*-

// CumVMatrix.h
//
// Copyright (C) 2004 Yoshua Bengio 
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
   * $Id: CumVMatrix.h,v 1.1 2004/03/08 17:36:46 yoshua Exp $ 
   ******************************************************* */

// Authors: Yoshua Bengio

/*! \file CumVMatrix.h */


#ifndef CumVMatrix_INC
#define CumVMatrix_INC

#include "SourceVMatrix.h"

namespace PLearn {
using namespace std;

class CumVMatrix: public SourceVMatrix
{
public:
  typedef SourceVMatrix inherited;

protected:
  // *********************
  // * protected options *
  // *********************

  Mat accumulated_columns;
  Vec row;
  Vec previous_sourcerow;
  TVec<int> columns;

public:

  // ************************
  // * public build options *
  // ************************

  // ### declare public option fields (such as build options) here
  TVec<string> columns_to_accumulate;

  // ****************
  // * Constructors *
  // ****************

  // Default constructor, make sure the implementation in the .cc
  // initializes all fields to reasonable default values.
  CumVMatrix();

  // ******************
  // * Object methods *
  // ******************

private: 
  //! This does the actual building. 
  // (Please implement in .cc)
  void build_();

protected: 
  //! Declares this class' options
  // (Please implement in .cc)
  static void declareOptions(OptionList& ol);

public:
  //!  This is the only method requiring implementation
  virtual void getRow(int i, Vec v) const;

  // simply calls inherited::build() then build_() 
  virtual void build();

  //! Transforms a shallow copy into a deep copy
  virtual void makeDeepCopyFromShallowCopy(map<const void*, void*>& copies);

  //! Declares name and deepCopy methods
  PLEARN_DECLARE_OBJECT(CumVMatrix);

};
DECLARE_OBJECT_PTR(CumVMatrix);

} // end of namespace PLearn
#endif
