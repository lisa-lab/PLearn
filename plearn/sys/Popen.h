// -*- C++ -*-

// PLearn (A C++ Machine Learning Library)
// Copyright (C) 2001 Pascal Vincent, Yoshua Bengio and University of Montreal
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
   * $Id: Popen.h,v 1.6 2005/01/21 22:22:36 chrish42 Exp $
   * This file is part of the PLearn library.
   ******************************************************* */

// This file contains useful functions for string manipulation
// that are used in the PLearn Library


/*! \file PLearnLibrary/PLearnCore/Popen.h */

#ifndef Popen_INC
#define Popen_INC

#include <plearn/base/general.h>
#include <plearn/io/PStream.h>

#define USE_NSPR
class PRProcess;

namespace PLearn {
using namespace std;

  class Popen: public PPointable
  {
  protected:
    //! Multi-argument variant: the arguments are passed in a vector.
    void launch(const string& command, const vector<string>&
                commandoptions);
    //! Full text variant. All arguments are passed together in a string.
    //! @deprecated Use the other version of launch instead.
    void launch(const string& commandline); 

    bool verbose;
    bool process_alive;

    PRProcess *process;
    
  public:
    PStream in; //should these be only one I/O PStream? -xsm
    PStream out;
    
    Popen(const string& command, 
	  bool the_verbose = false) 
      { verbose = the_verbose; launch(command); }

    Popen(const string& command, const vector<string>& commandoptions, 
	  bool the_verbose = false) 
      { verbose = the_verbose; launch(command,commandoptions); }

    /** Wait for process termination and return exit value.
        @note This must be called after all output from the program
        has been read. (Some progs block until their output is read.)
    */
    int wait();
    
    ~Popen();
  };

  
/*!     Returns the full output of the command as a vector of strings,
    containing the lines of the answer (with any newline character removed). 
    The command must not be waiting for input on its standard input 
    or this call will never return.
*/
  vector<string> execute(const string& command);

} // end of namespace PLearn

#endif
