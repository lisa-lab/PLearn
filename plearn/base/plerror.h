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
 * AUTHORS: Pascal Vincent & Yoshua Bengio
 * This file is part of the PLearn library.
 ******************************************************* */


/*! \file PLearnLibrary/PLearnCore/plerror.h */

#ifndef perror_INC
#define perror_INC

#include <assert.h>
#include "plexceptions.h"

namespace PLearn {

#ifndef USE_EXCEPTIONS
extern ostream* error_stream;
#endif

#define PLERROR   errormsg
#define PLWARNING warningmsg
#define PLDEPRECATED deprecationmsg

void errormsg(const char* msg, ...);
void warningmsg(const char* msg, ...);
void deprecationmsg(const char* msg, ...);
void exitmsg(const char* msg, ...);
void pl_assert_fail(const char* expr, const char* file, unsigned line,
                    const char* function, const char* message);

// Redefine the assert mechanism to throw an exception through PLERROR.  Also
// define a pl_assert which takes an explanation -- to be refined with ... a la
// errormsg.
#if defined(assert) && ! defined(PL_ASSERT_DEFINED)
#  undef assert
#endif

// When debugging, do nothing (do static cast as in GCC)
#ifdef  NDEBUG

#  define assert(expr) static_cast<void>(0)
#  define pl_assert(expr, message) static_cast<void>(0)

#else   // ! defined(NDEBUG)

#  define assert(expr)                                                      \
   static_cast<void>((expr) ? 0 :                                           \
                     (PLearn::pl_assert_fail(#expr, __FILE__, __LINE__,     \
                                             PL_ASSERT_FUNCTION, ""), 0))               

#  define pl_assert(expr, message)                                          \
   static_cast<void>((expr) ? 0 :                                           \
                     (PLearn::pl_assert_fail(#expr, __FILE__, __LINE__,     \
                                             PL_ASSERT_FUNCTION, (message)), 0))         

#  define PL_ASSERT_DEFINED

#endif  // NDEBUG


// Use the function prettification code present in GCC's assert.h include
#if defined __USE_GNU
#  define PL_ASSERT_FUNCTION	__PRETTY_FUNCTION__
#else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#    define PL_ASSERT_FUNCTION	__func__
#  else
#    define PL_ASSERT_FUNCTION	((__const char *) 0)
#  endif
#endif

    
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
