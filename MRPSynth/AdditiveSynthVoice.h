//
//  AdditiveSynthVoice.h
//  MRP
//
//  Created by App Lab on 7/29/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRP__AdditiveSynthVoice__
#define __MRP__AdditiveSynthVoice__

#include <iostream>
#include <vector>
#include "SynthVoice.h"

//! Monophonic additive synthesizer voice with specified number of harmonics
/*!

*/
class AdditiveSynthVoice : public SynthVoice {
    
    int _numHarmonics;                              // Number of harmonics
    std::vector<SynthParameter*> _harmonicAmps;     // Harmonic amplitudes
    
public:
    
    AdditiveSynthVoice();
    AdditiveSynthVoice(int numHarmonics);
    AdditiveSynthVoice(std::vector<float>harmonicAmps);
    AdditiveSynthVoice(const AdditiveSynthVoice *master);
    ~AdditiveSynthVoice() {};
    
    int numHarmonics() { return _numHarmonics; }
    
    void setNumHarmonics(float num);
    void setHarmonicAmp(int harmonicNumber, float amp);
    
    int renderSample(float *outSample);
    
};

#endif /* defined(__MRP__AdditiveSynthVoice__) */
