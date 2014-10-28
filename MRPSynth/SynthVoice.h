//
//  SynthVoice.h
//  MRP
//
//  Created by Jeff Gregorio on 7/28/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRP__SynthVoice__
#define __MRP__SynthVoice__

#include <iostream>
#include "ParameterList.h"
#include "ADSREnvelope.h"

#define M_2PI 6.283185307f
#define kSynthVoice_Default_fs 44100.0f
#define kSynthVoice_Default_amp_RampTime 0.01f
#define kSynthVoice_Default_f0_RampTime 0.1f
#define kSynthVoice_Default_Atk 1.0f
#define kSynthVoice_Default_Dec 0.05f
#define kSynthVoice_Default_Sus 1.00f
#define kSynthVoice_Default_Rel 0.05f

//! Base Class for Single (monophonic) Synth Tones
/*!
    Contains the data and functionality common to any possible synth voice; including sample rate, fundamental frequency, and their setters/getters; phase and phase increment; and a virtual render() method required by any classes inheriting from SynthVoice. 
 
    Also includes an ADSR envelope whose parameters can be set (instantaneously or ramped) using SynthVoice::setAttack(float atk, bool doRamp), SynthVoice::setDecay(float dec, bool doRamp), etc.
*/
class SynthVoice : public ParameterList {
    
protected:
    
    float _fs;                  // Sampling rate
    
    SynthParameter *_velAmp;    // Velocity-scaled amplitude (private)
    SynthParameter *_amp;       // Global amplitude scalar (public)
    SynthParameter *_f0;        // Fundamental frequency
    float _theta;               // Current phase
    float _thetaStep;           // Phase increment
    
    ADSREnvelope _adsr;         // Amplitude envelope
    
    /* Any parameters added to the parameter list using addParameter() are automatically updated (ramped if needed) when sampleUpdate() is called by the render method. Any internal parameters that aren't added to the parameter list should be added to _sampleUpdateListeners to receive updates on each sample. */
    vector<SynthParameter*> _sampleUpdateListeners;
    
    /* When SynthVoice::setSampleRate() is called, any parameters added to the parameter list using addParameter() also update their sample rates automatically. Any internal parameters that aren't added to the parameter list should be added to _sampleRateListeners to receive updates when the sample rate changes */
    vector<SynthParameter*> _sampleRateListeners;
    
public:
    
    SynthVoice();
    SynthVoice(float fs);
    SynthVoice(const SynthVoice* master);
    ~SynthVoice();
    
    float sampleRate() { return _fs; }      // Get the sampling rate
    float f0() { return _f0->value(); }     // Query the current fundamental freq
    
    void setSampleRate(float fs);           // Set the sampling rate
    
    void setVelocityAmplitude(float amp, bool doRamp);
    void setAmplitude(float amp, bool doRamp);      // Set a new amplitude (directly or ramped)
    void setF0(float f0, bool doRamp);              // Set a new fundamental freq (directly or ramped)
    
    void setAttack(float atk, bool doRamp);         // Set ADSR envelope parameters (directly or ramped)
    void setDecay(float dec, bool doRamp);
    void setSustain(float sus, bool doRamp);
    void setRelease(float rel, bool doRamp);
    
    void beginAttack();         // Enable the note and reset its envelope to attack
    void beginRelease();        // Set the envelope to release
    
    void sampleUpdate();                // Update internal parameter ramps for a single sample
    
    /* Template for SynthVoice subclass render methods */
    virtual int renderSample(float *outSample) {
        
        /* ALWAYS call sampleUpdate() for each audio sample. This ramps the _amp and _f0 parameters, updates the current phase, and ramps any parameters added to the parameter list */
        sampleUpdate();
        
        /* ----- Insert rendering code here ----- */
        
        /* Write a sample to the output buffer at index i based on the current phase */
        *outSample = sinf(_theta);
        *outSample *= _adsr.currentAmplitude(); // Scale by current envelope amplitude
        *outSample *= _velAmp->value();         // Scale by MIDI velocity amplitude
        *outSample *= _amp->value();            // Scale by current voice amplitude
        
        /* -------------------------------------- */
        
        /* Update the ADSR envelope for the entire buffer. The ADSR envelope update() method returns 0 when we've exceeded the release time. This tells the PolySynth that calls SynthVoice::render() that we're finished rendering. Voices that inherit from SynthVoice should ALWAYS return _adsr.update() or the PolySynth's note priority system won't work properly and PolySynth will keep wastefully calling the Voice's render method after it has released. */
        return _adsr.update();
    }
};

#endif /* defined(__MRP__SynthVoice__) */
