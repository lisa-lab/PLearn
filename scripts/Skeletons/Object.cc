#include "DERIVEDCLASS.h"

namespace PLearn <%%
using namespace std;

DERIVEDCLASS::DERIVEDCLASS() 
  :Object()
/* ### Initialise all fields to their default value */
  {
    // ...

    // ### You may or may not want to call build_() to finish building the object
    // build_();
  }


  IMPLEMENT_NAME_AND_DEEPCOPY(DERIVEDCLASS);

  void DERIVEDCLASS::declareOptions(OptionList& ol)
  {
    // ### Declare all of this object's options here
    // ### For the "flags" of each option, you should typically specify  
    // ### one of OptionBase::buildoption, OptionBase::learntoption or 
    // ### OptionBase::tuningoption. Another possible flag to be combined with
    // ### is OptionBase::nosave

    // ### ex:
    // declareOption(ol, "myoption", &DERIVEDCLASS::myoption, OptionBase::buildoption,
    //               "Help text describing this option");
    // ...

    // Now call the parent class' declareOptions
    inherited::declareOptions(ol);
  }

  string DERIVEDCLASS::help() const
  {
    // ### Provide some useful description of what the class is ...
    return 
      "DERIVEDCLASS implements a ..."
      + optionHelp();
  }

  void DERIVEDCLASS::build_()
  {
    // ### This method should do the real building of the object,
    // ### according to set 'options', in *any* situation. 
    // ### Typical situations include:
    // ###  - Initial building of an object from a few user-specified options
    // ###  - Building of a "reloaded" object: i.e. from the complete set of all serialised options.
    // ###  - Updating or "re-building" of an object after a few "tuning" options have been modified.
    // ### You should assume that the parent class' build_() has already been called.
  }

  // ### Nothing to add here, simply calls build_
  void DERIVEDCLASS::build()
  {
    inherited::build();
    build_();
  }


  void DERIVEDCLASS::train(VMat training_set)
  { 
    if(training_set->width() != inputsize()+targetsize())
      PLERROR("In DERIVEDCLASS::train(VMat training_set) training_set->width() != inputsize()+targetsize()");

    setTrainingSet(training_set);

    // ### Please implement the actual training of the model.
    // ### For models with incremental training, to benefit 
    // ### from the "testing during training" and early-stopping 
    // ### mechanisms, you should make sure to call measure at 
    // ### every "epoch" (whatever epoch means for your algorithm).
    // ### ex:
    // if(measure(epoch,costvec)) 
    //     break; // exit training loop because early-stopping contditions were met
  }

  void DERIVEDCLASS::use(const Vec& input, Vec& output)
  {
    // ### You should redefine this method to compute the output
    // ### corresponfding to a new test input.
  }

  void DERIVEDCLASS::makeDeepCopyFromShallowCopy(map<const void*, void*>& copies)
  {
    Object::makeDeepCopyFromShallowCopy(copies);

    // ### Call deepCopyField on all "pointer-like" fields 
    // ### that you wish to be deepCopied rather than 
    // ### shallow-copied.
    // ### ex:
    // deepCopyField(trainvec, copies);

    // ### Remove this line when you have fully implemented this method.
    PLERROR("DERIVEDCLASS::makeDeepCopyFromShallowCopy not fully (correctly) implemented yet!");
  }

%%> // end of namespace PLearn
