// -*- C++ -*-

// SubsamplingDBN.h
//
// Copyright (C) 2006 Pascal Lamblin
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

// Authors: Pascal Lamblin

/*! \file SubsamplingDBN.h */


#ifndef SubsamplingDBN_INC
#define SubsamplingDBN_INC

#include <plearn_learners/generic/PLearner.h>
#include <plearn_learners/online/OnlineLearningModule.h>
#include <plearn_learners/online/CostModule.h>
#include <plearn_learners/online/NLLCostModule.h>
#include <plearn_learners/online/RBMClassificationModule.h>
#include <plearn_learners/online/RBMLayer.h>
#include <plearn_learners/online/RBMMixedLayer.h>
#include <plearn_learners/online/RBMConnection.h>
#include <plearn/misc/PTimer.h>
#include <plearn/sys/Profiler.h>

namespace PLearn {

/**
 * Neural net, learned layer-wise in a greedy fashion.
 * This version support different unit types, different connection types,
 * and different cost functions, including the NLL in classification.
 */
class SubsamplingDBN : public PLearner
{
    typedef PLearner inherited;

public:
    //#####  Public Build Options  ############################################

    //! The learning rate used during contrastive divergence learning
    real cd_learning_rate;

    //! The learning rate used during the gradient descent
    real grad_learning_rate;

    int batch_size;

    //! The decrease constant of the learning rate used during gradient descent
    real grad_decrease_ct;

    /* NOT IMPLEMENTED YET
    //! The weight decay used during the gradient descent
    real grad_weight_decay;
    */

    //! Number of classes in the training set
    //!   - 0 means we are doing regression,
    //!   - 1 means we have two classes, but only one output,
    //!   - 2 means we also have two classes, but two outputs summing to 1,
    //!   - >2 is the usual multiclass case.
    int n_classes;

    //! Number of examples to use during each phase of learning:
    //! first the greedy phases, and then the fine-tuning phase.
    //! However, the learning will stop as soon as we reach nstages.
    //! For example for 2 hidden layers, with 1000 examples in each
    //! greedy phase, and 500 in the fine-tuning phase, this option
    //! should be [1000 1000 500], and nstages should be at least 2500.
    //! When online = true, this vector is ignored and should be empty.
    TVec<int> training_schedule;

    //! If the first cost function is the NLL in classification,
    //! pre-trained with CD, and using the last *two* layers to get a better
    //! approximation (undirected softmax) than layer-wise mean-field.
    bool use_classification_cost;

    //! Minimize reconstruction error of each layer as an auto-encoder.
    //! This is done using cross-entropy between actual and reconstructed.
    //! This option automatically adds the following cost names:
    //! layerwise_reconstruction_error (sum over all layers)
    //!   layer0.reconstruction_error (only layers[0])
    //!   layer1.reconstruction_error (only layers[1])
    //!   etc.
    bool reconstruct_layerwise;

    //! The layers of units in the network
    TVec< PP<RBMLayer> > layers;

    //! The weights of the connections between the layers
    TVec< PP<RBMConnection> > connections;

    //! Optional module that takes as input the output of the last layer
    //! (layers[n_layers-1), and its output is fed to final_cost, and
    //! concatenated with the one of classification_cost (if present) as output
    //! of the learner.
    //! If it is not provided, then the last layer will directly be put as
    //! input of final_cost.
    PP<OnlineLearningModule> final_module;

    //! The cost function to be applied on top of the DBN (or of final_module
    //! if provided). Its gradients will be backpropagated to final_module,
    //! then combined with the one of classification_cost and backpropagated to
    //! the layers.
    PP<CostModule> final_cost;

    //! The different cost function to be applied on top of each layer
    //! (except the first one, which has to be null) of the RBM. These
    //! costs are not back-propagated to previous layers.
    TVec< PP<CostModule> > partial_costs;

    //! In an RBMLayer, do we want the bias during up and down propagations to
    //! be potentially different?
    bool independent_biases;

    //! Different subsampling modules, to be applied on top of RBMs when
    //! they're already learned. subsampling_modules[0] is null.
    TVec< PP<OnlineLearningModule> > subsampling_modules;

    //#####  Public Learnt Options  ###########################################
    //! The module computing the probabilities of the different classes.
    PP<RBMClassificationModule> classification_module;

    //! Number of layers
    int n_layers;

    //! The computed cost names
    TVec<string> cost_names;

    //! whether to do things by stages, including fine-tuning, or on-line
    bool online;

    // Coefficient between 0 and 1. If non-zero, run a background
    // Gibbs chain and use the visible-hidden statistics to
    // contribute in the negative phase update (in proportion
    // background_gibbs_update_ratio wrt the contrastive divergence
    // negative phase statistics). If = 1, then do not perform any
    // contrastive divergence negative phase (use only the Gibbs chain
    // statistics).
    real background_gibbs_update_ratio;

    //! Wether we do a step of joint contrastive divergence on top-layer
    //! Only used if online for the moment
    bool top_layer_joint_cd;

    //! after how many examples should we re-initialize the Gibbs chains
    //! (if == INT_MAX, the default then NEVER re-initialize except when
    //! stage==0)
    int gibbs_chain_reinit_freq;

    //! Layers of reduced size, to be put on top of subsampling modules
    //! If the subsampling module is null, it will be either the same that the
    //! one in 'layers' (default), or a copy of it (with independant biases)
    //! if 'independent_biases' is true.
    TVec< PP<RBMLayer> > reduced_layers;

    //#####  Not Options  #####################################################

    //! Timer for monitoring the speed
    PP<PTimer> timer;

    //! The module computing the classification cost function (NLL) on top of
    //! classification_module.
    PP<NLLCostModule> classification_cost;

    //! Concatenation of layers[n_layers-2] and the target layer (that is
    //! inside classification_module), if use_classification_cost
    PP<RBMMixedLayer> joint_layer;


public:
    //#####  Public Member Functions  #########################################

    //! Default constructor
    SubsamplingDBN();


    //#####  PLearner Member Functions  #######################################

    //! Returns the size of this learner's output, (which typically
    //! may depend on its inputsize(), targetsize() and set options).
    virtual int outputsize() const;

    //! (Re-)initializes the PLearner in its fresh state (that state may depend
    //! on the 'seed' option) and sets 'stage' back to 0 (this is the stage of
    //! a fresh learner!).
    // (PLEASE IMPLEMENT IN .cc)
    virtual void forget();

    //! The role of the train method is to bring the learner up to
    //! stage==nstages, updating the train_stats collector with training costs
    //! measured on-line in the process.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void train();

    //! Re-implementation of the PLearner test() for profiling purposes
    virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
                      VMat testoutputs=0, VMat testcosts=0) const;

    //! Computes the output from the input.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeOutput(const Vec& input, Vec& output) const;

    //! Computes the costs from already computed output.
    // (PLEASE IMPLEMENT IN .cc)
    virtual void computeCostsFromOutputs(const Vec& input, const Vec& output,
                                         const Vec& target, Vec& costs) const;

    //! Returns the names of the costs computed by computeCostsFromOutpus (and
    //! thus the test method).
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTestCostNames() const;

    //! Returns the names of the objective costs that the train method computes
    //! and  for which it updates the VecStatsCollector train_stats.
    // (PLEASE IMPLEMENT IN .cc)
    virtual TVec<std::string> getTrainCostNames() const;


    void onlineStep( const Vec& input, const Vec& target, Vec& train_costs );

    void onlineStep( const Mat& inputs, const Mat& targets, Mat& train_costs );

    void greedyStep( const Vec& input, const Vec& target, int index );

    //! Greedy step with mini-batches.
    void greedyStep(const Mat& inputs, const Mat& targets, int index, Mat& train_costs_m);

    void jointGreedyStep( const Vec& input, const Vec& target );

    void fineTuningStep( const Vec& input, const Vec& target,
                         Vec& train_costs );

    //! Fine tuning step with mini-batches.
    void fineTuningStep( const Mat& inputs, const Mat& targets,
                         Mat& train_costs );

    //! Perform a step of contrastive divergence, assuming that
    //! down_layer->expectation(s) is set.
    void contrastiveDivergenceStep( const PP<RBMLayer>& down_layer,
                                    const PP<RBMConnection>& connection,
                                    const PP<RBMLayer>& up_layer,
                                    int layer_index,
                                    bool nofprop=false);


    // *** SUBCLASS WRITING: ***
    // While in general not necessary, in case of particular needs
    // (efficiency concerns for ex) you may also want to overload
    // some of the following methods:
    // virtual void computeOutputAndCosts(const Vec& input, const Vec& target,
    //                                    Vec& output, Vec& costs) const;
    // virtual void computeCostsOnly(const Vec& input, const Vec& target,
    //                               Vec& costs) const;
    // virtual void test(VMat testset, PP<VecStatsCollector> test_stats,
    //                   VMat testoutputs=0, VMat testcosts=0) const;
    // virtual int nTestCosts() const;
    // virtual int nTrainCosts() const;
    // virtual void resetInternalState();
    // virtual bool isStatefulLearner() const;


    //#####  PLearn::Object Protocol  #########################################

    // Declares other standard object methods.
    // ### If your class is not instantiatable (it has pure virtual methods)
    // ### you should replace this by PLEARN_DECLARE_ABSTRACT_OBJECT_METHODS
    PLEARN_DECLARE_OBJECT(SubsamplingDBN);

    // Simply calls inherited::build() then build_()
    virtual void build();

    //! Transforms a shallow copy into a deep copy
    // (PLEASE IMPLEMENT IN .cc)
    virtual void makeDeepCopyFromShallowCopy(CopiesMap& copies);

protected:

    int minibatch_size;

    //#####  Not Options  #####################################################

    // whether to re-initialize Gibbs chain next time around
    bool initialize_gibbs_chain;

    //! Stores the gradient of the cost wrt the activations
    //! (at the input of the layers)
    mutable TVec<Vec> activation_gradients;
    mutable TVec<Mat> activations_gradients; //!< For mini-batch.

    //! Stores the gradient of the cost wrt the expectations
    //! (at the output of the layers)
    mutable TVec<Vec> expectation_gradients;
    mutable TVec<Mat> expectations_gradients; //!< For mini-batch.

    mutable TVec<Vec> subsampling_gradients;

    mutable Vec final_cost_input;
    mutable Mat final_cost_inputs; //!< For mini-batch.

    mutable Vec final_cost_value;
    mutable Mat final_cost_values; //!< For mini-batch.

    mutable Vec final_cost_output;

    mutable Vec class_output;

    mutable Vec class_gradient;

    //! Stores the gradient of the cost at the input of final_cost
    mutable Vec final_cost_gradient;
    mutable Mat final_cost_gradients; //!< For mini-batch.

    //! buffers bottom layer activation during onlineStep 
    mutable Vec save_layer_activation;

    Mat save_layer_activations; //!< For mini-batches.

    //! buffers bottom layer expectation during onlineStep 
    mutable Vec save_layer_expectation;

    Mat save_layer_expectations; //!< For mini-batches.

    //! Does final_module exist and have a "learning_rate" option
    bool final_module_has_learning_rate;

    //! Does final_cost exist and have a "learning_rate" option
    bool final_cost_has_learning_rate;

    //! Store a copy of the positive phase values
    mutable Vec pos_down_val;
    mutable Vec pos_up_val;
    mutable Mat pos_down_vals;
    mutable Mat pos_up_vals;
    mutable Mat cd_neg_down_vals;
    mutable Mat cd_neg_up_vals;

    //! Store the state of the Gibbs chain for each RBM
    mutable TVec<Mat> gibbs_down_state;

    //! Used to store the costs optimized by the final cost module.
    Vec optimized_costs;

    //! Stores reconstruction costs
    mutable Vec reconstruction_costs;

    //! Keeps the index of the NLL cost in train_costs
    int nll_cost_index;

    //! Keeps the index of the class_error cost in train_costs
    int class_cost_index;

    //! Keeps the beginning index of the final costs in train_costs
    int final_cost_index;

    //! Keeps the beginning indices of the partial costs in train_costs
    TVec<int> partial_costs_indices;

    //! Keeps the beginning index of the reconstruction costs in train_costs
    int reconstruction_cost_index;

    //! Index of the cpu time cost (per each call of train())
    int training_cpu_time_cost_index;

    //! The index of the cumulative training time cost 
    int cumulative_training_time_cost_index;

    //! The index of the cumulative testing time cost
    int cumulative_testing_time_cost_index;

    //! Holds the total training (cpu)time
    real cumulative_training_time;

    //! Holds the total testing (cpu)time 
    mutable real cumulative_testing_time;

    //! Cumulative training schedule
    TVec<int> cumulative_schedule;

    mutable Vec layer_input;
    mutable Mat layer_inputs;

protected:
    //#####  Protected Member Functions  ######################################

    //! Declares the class options.
    static void declareOptions(OptionList& ol);

private:
    //#####  Private Member Functions  ########################################

    //! This does the actual building.
    void build_();

    void build_layers_and_connections();

    void build_costs();

    void build_classification_cost();

    void build_final_cost();

    void setLearningRate( real the_learning_rate );

private:
    //#####  Private Data Members  ############################################

    // The rest of the private stuff goes here
};

// Declares a few other classes and functions related to this class
DECLARE_OBJECT_PTR(SubsamplingDBN);

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
