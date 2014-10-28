//
//  SubtractiveSynthVoice.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 10/15/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRPSynthGUI__SubtractiveSynthVoice__
#define __MRPSynthGUI__SubtractiveSynthVoice__

#include <stdio.h>

#include "SynthVoice.h"
#include "BiquadFilter.h"

class SubtractiveSynthVoice : public SynthVoice {
    
    int _numHarmonics;                              // Number of harmonics
    std::vector<SynthParameter*> _harmonicAmps;     // Harmonic amplitudes
    
    BiquadFilter _filter;
    ADSREnvelope _filterEnv;
    bool _filterEnvInvert;
    
public:
    
    SubtractiveSynthVoice();
    SubtractiveSynthVoice(int numHarmonics);
    SubtractiveSynthVoice(std::vector<float>harmonicAmps);
    SubtractiveSynthVoice(SubtractiveSynthVoice *master);
    ~SubtractiveSynthVoice() {};
    
    int numHarmonics()  { return _numHarmonics; }
    
    void setNumHarmonics(float num);
    void setHarmonicAmp(int harmonicNumber, float amp);
    void setFilterType(BiquadFilterType type);
    
    void beginAttack();
    
    int renderSample(float *outSample);
};

#endif /* defined(__MRPSynthGUI__SubtractiveSynthVoice__) */
