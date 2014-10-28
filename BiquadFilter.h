//
//  BiquadFilter.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 10/15/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRPSynthGUI__BiquadFilter__
#define __MRPSynthGUI__BiquadFilter__

#include <stdio.h>
#include <math.h>
#include <vector>

#include "SynthParameter.h"

#define kBiquad_DefaultSampleRate 44100.0f
#define kBiquad_ParameterRampDuration 0.1f

typedef enum BiquadFilterType {
    kBiquadFilterType_LowPass = 0,
    kBiquadFilterType_HighPass,
    kBiquadFilterType_BandPass
} BiquadFilterType;

class BiquadFilter {
    
private:
    
    float _fs;                  // Sample rate
    SynthParameter *_fc;        // Corner freq
    SynthParameter *_Q;         // Resonance
    
    BiquadFilterType _type;     // LPF/HPF/BPF
    
    /* Last two samples of the previous audio I/O buffers */
    float _x[2];        // Input,   {x[n-1], x[n-2]}
    float _y[2];        // Output,  {y[n-1], y[n-2]}
    
    /* Intermediate parameters */
    float _omega;
    float _alpha;
    
    /* Filter coefficients */
    float _a[3];
    float _b[3];
    
    /* Static parameter change listeners for Fc and Q */
    static void staticFcChanged(float targetVal, void *userData) {
        BiquadFilter *filter = (BiquadFilter *)userData;
        filter->fcChanged(targetVal);
    }
    void fcChanged(float targetVal);
    static void staticQChanged(float targetVal, void *userData) {
        BiquadFilter *filter = (BiquadFilter *)userData;
        filter->qChanged(targetVal);
    }
    void qChanged(float targetVal);
    
    void computeCoefficients();
    void printStabilityWarning();
   
public:
    
    BiquadFilter();
    BiquadFilter(BiquadFilterType type);
    BiquadFilter(BiquadFilterType type, float sampleRate);
    BiquadFilter(BiquadFilterType type, float sampleRate, float cornerFreq, float resonance);
    BiquadFilter(const BiquadFilter *o);
    
    void setFilterType(BiquadFilterType type);
    void setFc(float fc, bool doRamp);
    void setQ(float Q, bool doRamp);
    float fc()   { return _fc->value(); }
    float Q()    { return _Q->value(); }
    std::vector<SynthParameter*> getParameters();
    
    void filterSample(float *sample);
    void printCoefficients();
};

#endif /* defined(__MRPSynthGUI__BiquadFilter__) */
