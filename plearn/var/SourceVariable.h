// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 1998 Pascal Vincent
// Copyright (C) 1999-2002 Pascal Vincent, Yoshua Bengio and University of Montreal
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
 * $Id$
 * This file is part of the PLearn library.
 ******************************************************* */

#ifndef SourceVariable_INC
#define SourceVariable_INC

#include "VarArray.h"
#include <plearn/math/PRandom.h>

namespace PLearn {
using namespace std;

class SourceVariable: public Variable
{
    typedef Variable inherited;

public:
    // build options
    int build_length;
    int build_width;

    string random_type;

    real random_a;
    real random_b;

    bool random_clear_first_row;

private:
    void build_();
public:
    virtual void build();

protected:
    static void declareOptions(OptionList & ol);

public:
    //! Default constructor for persistence.
    SourceVariable();
    SourceVariable(int thelength, int thewidth, bool call_build_ = true);
    SourceVariable(const Vec& v, bool vertical=true, bool call_build_ = true);
    SourceVariable(const Mat& m, bool call_build_ = true);
    SourceVariable(int thelength, int thewidth, string random_type_, 
                   real random_a_=0, real random_b_=1, bool clear_first_row_=false,
                   bool call_build_ = true);

    virtual void setParents(const VarArray& parents)
    { PLERROR("In Variable::setParents  trying to set parents of a SourceVariable..."); }


    virtual bool markPath();
    virtual void buildPath(VarArray& proppath);
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);
    PLEARN_DECLARE_OBJECT(SourceVariable);
  
    //! Initializes the value of this variable from the given generator,
    //! according to options     
    virtual void randomInitialize(PP<PRandom> random_gen);
  
    virtual void fprop();
    virtual void bprop();
    virtual void bbprop();
    virtual void symbolicBprop();
    virtual void rfprop();
    virtual VarArray sources();
    virtual VarArray random_sources();
    virtual VarArray ancestors();
    virtual void unmarkAncestors();
    virtual VarArray parents();
    bool isConstant() { return true; }

    void printInfo(bool print_gradient) { 
        pout << getName() << "[" << (void*)this << "] " << *this << " = " << value;
        if (print_gradient) pout << " gradient=" << gradient;
        pout << endl; 
    }
};

DECLARE_OBJECT_PTR(SourceVariable);

} // end of namespace PLearn

#endif


/*
  Local Variables:
  mode:c++
  c-basic-offset:4
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0))
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:encoding=utf-8:textwidth=79 :
