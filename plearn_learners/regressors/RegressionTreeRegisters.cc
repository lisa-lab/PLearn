// -*- C++ -*-

// RegressionTreeRegisters.cc
// Copyright (c) 1998-2002 Pascal Vincent
// Copyright (C) 1999-2002 Yoshua Bengio and University of Montreal
// Copyright (c) 2002 Jean-Sebastien Senecal, Xavier Saint-Mleux, Rejean Ducharme
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


/* **********************************************************************************    
 * $Id: RegressionTreeRegisters.cc, v 1.0 2004/07/19 10:00:00 Bengio/Kegl/Godbout *
 * This file is part of the PLearn library.                                       *
 ********************************************************************************** */

#include "RegressionTreeRegisters.h"
#include <plearn/vmat/TransposeVMatrix.h>
#include <plearn/vmat/MemoryVMatrixNoSave.h>
#include <plearn/vmat/SubVMatrix.h>
#include <limits>

namespace PLearn {
using namespace std;

PLEARN_IMPLEMENT_OBJECT(RegressionTreeRegisters,
                        "Object to maintain the various registers of a regression tree", 
                        "It is used first, to sort the learner train set on all dimensions of the input samples.\n"
                        "It keeps matrices of row indices to navigate thru the training set in ascending value order fo each variable.\n"
                        "Missing values are sorted at the beginning of the column.\n"
                        "It also keeps registers of which leave, a row belongs to as the tree is built.\n"
                        "It is also used to maintain the next available leave id.\n"
    );

RegressionTreeRegisters::RegressionTreeRegisters():
    report_progress(0),
    verbosity(0),
    next_id(0),
    do_sort_rows(true),
    mem_tsource(true)
{
    build();
}

RegressionTreeRegisters::RegressionTreeRegisters(VMat source_,
                                                 TMat<RTR_type> tsorted_row_,
                                                 VMat tsource_,
                                                 bool report_progress_,
                                                 bool verbosity_,
                                                 bool do_sort_rows_,
                                                 bool mem_tsource_):
    report_progress(report_progress_),
    verbosity(verbosity_),
    next_id(0),
    do_sort_rows(do_sort_rows_),
    mem_tsource(mem_tsource_)
{
    source = source_;
    tsource = tsource_;
    tsorted_row = tsorted_row_;
    build();
}

RegressionTreeRegisters::RegressionTreeRegisters(VMat source_,
                                                 bool report_progress_,
                                                 bool verbosity_,
                                                 bool do_sort_rows_,
                                                 bool mem_tsource_):
    report_progress(report_progress_),
    verbosity(verbosity_),
    next_id(0),
    do_sort_rows(do_sort_rows_),
    mem_tsource(mem_tsource_)
{
    source = source_;
    build();
}

RegressionTreeRegisters::~RegressionTreeRegisters()
{
}

void RegressionTreeRegisters::declareOptions(OptionList& ol)
{ 
    declareOption(ol, "report_progress", &RegressionTreeRegisters::report_progress, OptionBase::buildoption,
                  "The indicator to report progress through a progress bar\n");
    declareOption(ol, "verbosity", &RegressionTreeRegisters::verbosity, OptionBase::buildoption,
                  "The desired level of verbosity\n");
    declareOption(ol, "tsource", &RegressionTreeRegisters::tsource,
                  OptionBase::learntoption | OptionBase::nosave,
                  "The source VMatrix transposed");

    declareOption(ol, "source", &RegressionTreeRegisters::source,
                  OptionBase::buildoption,
                  "The source VMatrix");

    declareOption(ol, "next_id", &RegressionTreeRegisters::next_id, OptionBase::learntoption,
                  "The next id for creating a new leave\n");
    declareOption(ol, "leave_register", &RegressionTreeRegisters::leave_register, OptionBase::learntoption,
                  "The vector identifying the leave to which, each row belongs\n");

    declareOption(ol, "do_sort_rows", &RegressionTreeRegisters::do_sort_rows,
                  OptionBase::buildoption,
                  "Do we generate the sorted rows? Not usefull if used only to test.\n");

    declareOption(ol, "mem_tsource", &RegressionTreeRegisters::mem_tsource,
                  OptionBase::buildoption,
                  "Do we put the tsource in memory? default to true as this"
                  " give an great speed up for the trainning of RegressionTree.\n");

    //too big to save
    declareOption(ol, "tsorted_row", &RegressionTreeRegisters::tsorted_row, OptionBase::nosave,
                  "The matrix holding the sequence of samples in ascending value order for each dimension\n");

    inherited::declareOptions(ol);
}

void RegressionTreeRegisters::makeDeepCopyFromShallowCopy(CopiesMap& copies)
{
    inherited::makeDeepCopyFromShallowCopy(copies);
    deepCopyField(tsorted_row, copies);
    deepCopyField(leave_register, copies);
//tsource should be deep copied, but as currently when it is deep copied
// the copy is not used anymore to train. To save memory we don't do it.
// It is deep copied eavily by HyperLearner and HyperOptimizer
//    deepCopyField(tsource,copies);
//no need to deep copy source as we don't reuse it after initialization
//    deepCopyField(source,copies);
}

void RegressionTreeRegisters::build()
{
    inherited::build();
    build_();
}

void RegressionTreeRegisters::build_()
{
    if(!source)
        return;
    //check that we can put all the examples of the train_set
    //with respect to the size of RTR_type who limit the capacity
    PLCHECK(source.length()>0 
            && (unsigned)source.length()
            <= std::numeric_limits<RTR_type>::max());
    PLCHECK(source->targetsize()==1);
    PLCHECK(source->weightsize()<=1);
    PLCHECK(source->inputsize()>0);

    if(!tsource){
        tsource = VMat(new TransposeVMatrix(new SubVMatrix(
                                                source, 0,0,source->length(),
                                                source->inputsize())));
        if(do_sort_rows){
            PP<MemoryVMatrixNoSave> tmp = new MemoryVMatrixNoSave(tsource);
            tsource = VMat(tmp);
        }
    }
    setMetaInfoFrom(source);
    weightsize_=1;
    targetsize_=1;
    target_weight.resize(source->length());
    if(source->weightsize()<=0){
        width_++;
        for(int i=0;i<source->length();i++){
            target_weight[i].first=source->get(i,inputsize());
            target_weight[i].second=1.0 / length();
        }
    }else
        for(int i=0;i<source->length();i++){
            target_weight[i].first=source->get(i,inputsize());
            target_weight[i].second=source->get(i,inputsize()+targetsize());
        }
    leave_register.resize(length());
    sortRows();
}

void RegressionTreeRegisters::reinitRegisters()
{
    next_id = 0;

    //in case we don't save the sorted data
    sortRows();
}
void RegressionTreeRegisters::getAllRegisteredRow(RTR_type_id leave_id, int col,
                                                  TVec<RTR_type> &reg,
                                                  Vec &target,
                                                  Vec &weight, Vec &value) const
{
    getAllRegisteredRow(leave_id,col,reg);
    target.resize(reg.length());
    weight.resize(reg.length());
    value.resize(reg.length());    
    if(weightsize() <= 0){
        weight.fill(1.0 / length());
        for(int i=0;i<reg.length();i++){            
            target[i] = target_weight[int(reg[i])].first;
            value[i]  = tsource->get(col, reg[i]);
        }
    } else {
        //It is better to do multiple pass for memory access.
        for(int i=0;i<reg.length();i++){
            target[i] = target_weight[int(reg[i])].first;
            weight[i] = target_weight[int(reg[i])].second;
        }
        for(int i=0;i<reg.length();i++)
            value[i]  = tsource->get(col, reg[i]);
    }
}

//! reg.size() == the number of row that we will put in it.
void RegressionTreeRegisters::getAllRegisteredRow(RTR_type_id leave_id, int col,
                                                  TVec<RTR_type> &reg) const
{

    int idx=0;
    int n=reg.length();
    int i;
    RTR_type* preg = reg.data();
    RTR_type* ptsorted_row = tsorted_row[col];
    RTR_type_id* pleave_register = leave_register.data();
    for( i=0;i<length() && n> idx;i++)
    {
        PLASSERT(ptsorted_row[i]==tsorted_row(col, i));
        int srow = ptsorted_row[i];
        if ( pleave_register[srow] == leave_id){
            PLASSERT(leave_register[srow] == leave_id);
            PLASSERT(preg[idx]==reg[idx]);
            preg[idx++]=srow;
        }
    }
    PLASSERT(idx==reg->size());
}

void RegressionTreeRegisters::sortRows()
{
    next_id = 0;
    if(!do_sort_rows)
        return;
    if (tsorted_row.length() == inputsize() && tsorted_row.width() == length())
    {
        verbose("RegressionTreeRegisters: Sorted train set indices are present, no sort required", 3);
        return;
    }
    verbose("RegressionTreeRegisters: The train set is being sorted", 3);
    tsorted_row.resize(inputsize(), length());
    PP<ProgressBar> pb;
    if (report_progress)
    {
        pb = new ProgressBar("RegressionTreeRegisters : sorting the train set on input dimensions: ", inputsize());
    }
    for(int row=0;row<tsorted_row.length();row++)
        for(int col=0;col<tsorted_row.width(); col++)
            tsorted_row(row,col)=col;
            
//     for (int each_train_sample_index = 0; each_train_sample_index < length(); each_train_sample_index++)
//     {
//         sorted_row(each_train_sample_index).fill(each_train_sample_index);
//     }
#pragma omp parallel for default(none) shared(pb)
    for (int sample_dim = 0; sample_dim < inputsize(); sample_dim++)
    {
        sortEachDim(sample_dim);
        if (report_progress) pb->update(sample_dim);
    }
}
  
void RegressionTreeRegisters::sortEachDim(int dim)
{
    int start_index = 0;
    int end_index = length() - 1;
    int forward_index;
    int backward_index;
    int stack_index = -1;
    TVec<int> stack(50);
    for (;;)
    {
        if ((end_index - start_index) < 7)
        {
            if (end_index > start_index)
            {
                sortSmallSubArray(start_index, end_index, dim);
            }
            if (stack_index < 0)
            {
                break;
            }
            end_index = stack[stack_index--];
            start_index = stack[stack_index--];
        }
        else
        {
            swapIndex(start_index + 1, (start_index + end_index) / 2, dim);
            if (compare(tsource->get(dim, tsorted_row(dim, start_index)),
                        tsource->get(dim, tsorted_row(dim, end_index))) > 0.0)
                swapIndex(start_index, end_index, dim);
            if (compare(tsource->get(dim, tsorted_row(dim, start_index + 1)),
                        tsource->get(dim, tsorted_row(dim, end_index))) > 0.0)
                swapIndex(start_index + 1, end_index, dim);
            if (compare(tsource->get(dim, tsorted_row(dim, start_index)),
                        tsource->get(dim, tsorted_row(dim, start_index + 1))) > 0.0)
                swapIndex(start_index, start_index + 1, dim);
            forward_index = start_index + 1;
            backward_index = end_index;
            real sample_feature = tsource->get(dim, tsorted_row(dim, start_index + 1));
            for (;;)
            {
                do forward_index++; while (compare(tsource->get(dim, tsorted_row(dim, forward_index)), sample_feature) < 0.0);
                do backward_index--; while (compare(tsource->get(dim, tsorted_row(dim, backward_index)), sample_feature) > 0.0);
                if (backward_index < forward_index)
                {
                    break;
                }
                swapIndex(forward_index, backward_index, dim);
            }
            swapIndex(start_index + 1, backward_index, dim);
            stack_index += 2;
            if (stack_index > 50)
                PLERROR("RegressionTreeRegistersVMatrix: the stack for sorting the rows is too small");
            if ((end_index - forward_index + 1) >= (backward_index - start_index))
            {
                stack[stack_index] = end_index;
                stack[stack_index - 1] = forward_index;
                end_index = backward_index - 1;
            }
            else
            {
                stack[stack_index] = backward_index - 1;
                stack[stack_index - 1] = start_index;
                start_index = forward_index;
            }
        }
    }
}
  
void RegressionTreeRegisters::sortSmallSubArray(int the_start_index, int the_end_index, int dim)
{
    for (int next_train_sample_index = the_start_index + 1;
         next_train_sample_index <= the_end_index;
         next_train_sample_index++)
    {
        int saved_index = tsorted_row(dim, next_train_sample_index);
        real sample_feature = tsource->get(dim,saved_index);
        int each_train_sample_index;
        for (each_train_sample_index = next_train_sample_index - 1;
             each_train_sample_index >= the_start_index;
             each_train_sample_index--)
        {
            if (compare(tsource->get(dim,tsorted_row(dim, each_train_sample_index)), sample_feature) <= 0.0)
            {
                break;
            }
            tsorted_row(dim, each_train_sample_index + 1) = tsorted_row(dim, each_train_sample_index);
        }
        tsorted_row(dim, each_train_sample_index + 1) = saved_index;
    }  
}

void RegressionTreeRegisters::swapIndex(int index_i, int index_j, int dim)
{
    int saved_index = tsorted_row(dim, index_i);
    tsorted_row(dim, index_i) = tsorted_row(dim, index_j);
    tsorted_row(dim, index_j) = saved_index;
}

real RegressionTreeRegisters::compare(real field1, real field2)
{
    if (is_missing(field1) && is_missing(field2)) return 0.0;
    if (is_missing(field1)) return -1.0;
    if (is_missing(field2)) return +1.0;
    return field1 - field2;
}

void RegressionTreeRegisters::printRegisters()
{
    cout << " register:  ";
    for (int ii = 0; ii < leave_register.length(); ii++) 
        cout << " " << tostring(leave_register[ii]);
    cout << endl;
}

void RegressionTreeRegisters::verbose(string the_msg, int the_level)
{
    if (verbosity >= the_level)
        cout << the_msg << endl;
}

void RegressionTreeRegisters::getExample(int i, Vec& input, Vec& target, real& weight)
{
#ifdef BOUNDCHECK
    if(inputsize_<0)
        PLERROR("In RegressionTreeRegisters::getExample, inputsize_ not defined for this vmat");
    if(targetsize_<0)
        PLERROR("In RegressionTreeRegisters::getExample, targetsize_ not defined for this vmat");
    if(weightsize()<0)
        PLERROR("In RegressionTreeRegisters::getExample, weightsize_ not defined for this vmat");
#endif
    tsource->getColumn(i,input);

    target[0]=target_weight[i].first;
    weight = target_weight[i].second;
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
