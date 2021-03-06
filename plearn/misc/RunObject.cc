// -*- C++ -*-

// RunObject.cc
//
// Copyright (C) 2004-2005 Olivier Delalleau 
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
 ******************************************************* */

// Authors: Olivier Delalleau

/*! \file RunObject.cc */


#include "RunObject.h"
#include <plearn/io/load_and_save.h>

namespace PLearn {
using namespace std;

///////////////
// RunObject //
///////////////
RunObject::RunObject():
    run_objects(false)
{}

PLEARN_IMPLEMENT_OBJECT(RunObject,
    "Allows to build non-runnable objects or run multiple objects in scripts",
    "A RunObject can be used for:\n"
    " - Defining non-runnable objects in PLearn scripts, without PLearn\n"
    "   complaining (this assumes these objects do something interesting at\n"
    "   build time, e.g. a PrecomputedVMatrix will precompute its data)\n"
    " - Running multiple objects in PLearn scripts, when the 'run_objects'\n"
    "   option is set to 'true': this is especially useful when a script is\n"
    "   generated by a Python PyPLearn script\n"
    " - Saving the resulting objects to files\n"
);

void RunObject::declareOptions(OptionList& ol)
{
    TVec<PPath>         save_files;

    declareOption(ol, "objects", &RunObject::objects,
        OptionBase::buildoption,
        "The objects that need being built, run and / or saved.");

    declareOption(ol, "run_objects", &RunObject::run_objects,
        OptionBase::buildoption,
        "If set to 'true', objects will be run with this RunObject.");

    declareOption(ol, "save_files", &RunObject::save_files,
        OptionBase::buildoption,
        "If provided, the resulting objects will be saved. This vector must\n"
        "either have same length as 'objects', or be of length one, in which\n"
        "case each object 'i' will be saved as '<basename>_i.<extension>'.");

    declareOption(ol, "underlying_object", &RunObject::underlying_object,
        OptionBase::learntoption,
        "DEPRECATED: The underlying object to be built.");

    declareOption(ol, "save_object_name", &RunObject::save_object_name,
        OptionBase::learntoption,
        "DEPRECATED: If provided, the object will be saved to this file after it is built.");

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
}

void RunObject::build_()
{
    // Deal with deprecated options.
    if (underlying_object) {
        PLDEPRECATED("In RunObject::build_ - Option 'underlying_object' is now"
                     " deprecated, please use the 'objects' option (note that "
                     "this is still going to work as usual)");
        PLASSERT( objects.isEmpty() && !run_objects );
        run_objects = false;
        objects.resize(0);
        objects.append(underlying_object);
    }
    if (!save_object_name.isEmpty()) {
        PLDEPRECATED("In RunObject::build_ - Option 'save_object_name' is now"
                     "deprecated, please use the 'save_files' option (note "
                     "that this is still going to work as usual)");
        PLASSERT(save_files.isEmpty() && !run_objects && objects.length() == 1);
        save_files.resize(0);
        save_files.append(save_object_name);
    }
}

void RunObject::build()
{
    inherited::build();
    build_();
}

void RunObject::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("RunObject::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
}

/////////
// run //
/////////
void RunObject::run() {
    if (run_objects)
        for (int i = 0; i < objects.length(); i++)
            objects[i]->run();

    PLASSERT( save_files.isEmpty() || save_files.length() == 1
                                 || save_files.length() == objects.length() );
    if (!save_files.isEmpty()) {
        TVec<PPath> paths = save_files;
        if (paths.length() != objects.length()) {
            // Automatically generate paths with an increasing counter.
            PLASSERT( paths.length() == 1 );
            PPath basepath = paths[0];
            PPath no_ext = basepath.no_extension();
            string ext = basepath.extension(true);
            paths = TVec<PPath>(objects.length());
            for (int i = 0; i < paths.length(); i++)
                paths[i] = no_ext + "_" + tostring(i) + ext;
        }
        PLASSERT( objects.length() == paths.length() );
        for (int i = 0; i < objects.length(); i++)
            PLearn::save(paths[i], objects[i]);
    }
}

} // end of namespace PLearn


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
