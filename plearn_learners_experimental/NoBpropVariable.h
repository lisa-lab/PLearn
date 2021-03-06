// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio, Rejean Ducharme and University of Montreal
// Copyright (C) 2001-2002 Nicolas Chapados, Ichiro Takeuchi, Jean-Sebastien Senecal
// Copyright (C) 2002 Xiangdong Wang, Christian Dorion

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
   * $Id: NoBpropVariable.h 2227 2004-08-21 14:42:05Z yoshua $
   * This file is part of the PLearn library.
   ******************************************************* */

#ifndef NoBpropVariable_INC
#define NoBpropVariable_INC

#include <plearn/var/UnaryVariable.h>

namespace PLearn {
using namespace std;


/*! * NoBpropVariable * */

/*!   Variable that copies its unique input, and can completely
  block the flow of gradient backward, or optionally let if flow
  partially (with an optional coefficient), i.e.
    fprop: output = input
    bprop: dC/dinput = gradient_scaling_factor * dC/doutput

*/
class NoBpropVariable: public UnaryVariable
{
  typedef UnaryVariable inherited;

public:
  real  * gradient_scaling_factor;

public:
  //!  Default constructor for persistence
  NoBpropVariable()
    : gradient_scaling_factor(0)
    {}
  NoBpropVariable(Variable* input, real  * the_gradient_scaling_factor);

  PLEARN_DECLARE_OBJECT(NoBpropVariable);
  static void declareOptions(OptionList &ol);

  virtual void build();

  virtual void recomputeSize(int& l, int& w) const;  
  virtual void fprop();
  virtual void bprop();

protected:
    void build_();
};

DECLARE_OBJECT_PTR(NoBpropVariable);

//!  copy its argument but block gradient completely or partially
inline Var no_bprop(Var v, real  *gradient_scaling_factor)
{ return new NoBpropVariable(v,gradient_scaling_factor); }

} // end of namespace PLearn

#endif 
