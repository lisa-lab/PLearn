
// -*- C++ -*-

// RunCommand.cc
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
   * $Id: RunCommand.cc,v 1.7 2004/07/21 16:30:49 chrish42 Exp $ 
   ******************************************************* */

/*! \file RunCommand.cc */
#include "RunCommand.h"
#include <plearn/base/general.h>
#include <plearn/io/fileutils.h>      //!< For readFileAndMacroProcess.
#include <plearn/base/plerror.h>
#include <plearn/base/stringutils.h>    //!< For split_on_first.
#include <plearn/base/Object.h>

namespace PLearn {
using namespace std;

//! This allows to register the 'RunCommand' command in the command registry
PLearnCommandRegistry RunCommand::reg_(new RunCommand);

//! The actual implementation of the 'RunCommand' command 
void RunCommand::run(const vector<string>& args)
{
  string scriptfile = args[0];
  if(!file_exists(scriptfile))
    PLERROR("Non existant script file: %s\n",scriptfile.c_str());

  map<string, string> vars;
  // populate vars with the arguments passed on the command line
  for(unsigned int i=1; i<args.size(); i++)
    {
      string option = args[i];
      pair<string,string> name_val = split_on_first(option, "=");
      vars[name_val.first] = name_val.second;
    }
  PStream pout(&cout);
  //  pout << vars << endl;
      
  string script = readFileAndMacroProcess(scriptfile, vars);
  // cerr << script << endl;

  PIStringStream in(script);

  while(in)
    {
      PP<Object> o = readObject(in);
      o->run();
      in.skipBlanksAndCommentsAndSeparators();
      // cerr << bool(in) << endl;
      // cerr << in.peek() << endl;
    }
}

} // end of namespace PLearn

