//
//  ADSREnvelope.h
//  MRP
//
//  Created by App Lab on 7/29/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRP__ADSREnvelope__
#define __MRP__ADSREnvelope__

#include <iostream>
#include <math.h>

#include "EffectBase.h"

#define kADSR_Default_fs 44100.0f
#define kADSR_ParameterRampDuration 0.1f    // Duration for ramped changes in A, D, S, or R

//! Timed amplitude envelope containing "Attack", "Decay", "Sustain", "Release", and "Amplitude" parameters
/*! 
 
 */
class ADSREnvelope : EffectBase {
    
    enum {
        kADSRPhase_Attack = 0,
        kADSRPhase_Decay,
        kADSRPhase_Sustain,
        kADSRPhase_Release,
    };
    
    int _state;                 // Current envelope state (A, D, S, or R)
    
    float _fs;                  // Sample rate
    float _timeInState;         // Time in current state
    
    SynthParameter _amp;        // Current amplitude (level)
    SynthParameter *_atk;       // Attack (time)
    SynthParameter *_dec;       // Decay (time)
    SynthParameter *_sus;       // Sustain (level)
    SynthParameter *_rel;       // Release (time)
    
    float _releaseAmp;
    
public:
    
#pragma mark - Constructors/Desctructors
    ADSREnvelope();
    ADSREnvelope(float fs);
    ADSREnvelope(float fs, float atk, float dec, float sus, float rel);
    ADSREnvelope(const ADSREnvelope* original);
    ~ADSREnvelope() {};
    
#pragma mark - Getters
    float currentAmplitude() const { return _amp.value(); }    // Get the current envelope amplitude
    float sampleRate() const { return _fs; }                    // Get the sampling rate
    float atk() const { return _atk->value(); }
    float dec() const { return _dec->value(); }
    float sus() const { return _sus->value(); }
    float rel() const { return _rel->value(); }
    
#pragma mark - Setters
    void setSampleRate(float fs);
    void setAttack(float atk, bool doRamp);
    void setDecay(float dec, bool doRamp);
    void setSustain(float sus, bool doRamp);
    void setRelease(float rel, bool doRamp);
    
#pragma mark - Updates
    /* Update the ramped parameters and envelope state. Return false if we've transitioned out of release */
    bool update();
    bool update(int nSamples);
    
#pragma mark - State control
    void beginAttack();
    void beginRelease();
    
#pragma mark - EffectBase Protocol Methods
    std::vector<SynthParameter*> getParameters();
    int process(float *procBuffer, int nFrames);
    
#pragma mark - Utility
    void printADSRValues();
};

#endif /* defined(__MRP__ADSREnvelope__) */
