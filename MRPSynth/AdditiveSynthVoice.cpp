//
//  AdditiveSynthVoice.cpp
//  MRP
//
//  Created by App Lab on 7/29/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "AdditiveSynthVoice.h"

/* Create voice with one harmonic (fundamental) */
AdditiveSynthVoice::AdditiveSynthVoice() :  _numHarmonics(1) {
    /* Basic parameters _f0, _amp, and _adsr params created by the parent (SynthVoice) constructor */
}

/* Create voice with a specified number of harmonics */
AdditiveSynthVoice::AdditiveSynthVoice(int numHarmonics) : _numHarmonics(numHarmonics) {
    
//    _fs = kSynthVoice_Default_fs;
    
    SynthParameter* p;
    for (int i = 0; i < _numHarmonics; i++) {
        
        char name[12];
        sprintf(name, "Harmonic %d", i+1);
        p = new SynthParameter(name, _fs, i == 0 ? 1.0f : 0.0f, 0.1f);
        p->setRange(0.0f, 1.0f);
        
        _harmonicAmps.push_back(p);             // Store harmonic amplitudes in a vector
        addParameter(p);
    }
}

/* Create voice with a specified number of harmonics and their amplitudes */
AdditiveSynthVoice::AdditiveSynthVoice(std::vector<float>harmonicAmps) : _numHarmonics((int)harmonicAmps.size()) {
    
//    _fs = kSynthVoice_Default_fs;
    
    SynthParameter *p;
    for (int i = 0; i < harmonicAmps.size(); i++) {
        
        char name[12];
        sprintf(name, "Harmonic %d", i+1);
        p = new SynthParameter(name, _fs, harmonicAmps[i], 0.1f);
        p->setRange(0.0f, 1.0f);
        
        _harmonicAmps.push_back(p);            // Store harmonic amplitudes in a vector
    }
}

AdditiveSynthVoice::AdditiveSynthVoice(const AdditiveSynthVoice* master) : SynthVoice(master), _numHarmonics(master->_numHarmonics) {
    
    _harmonicAmps.clear();
    
    SynthParameter* p;
    for (int i = 0; i < _numHarmonics; i++) {
        
        char name[12];
        sprintf(name, "Harmonic %d", i+1);
        p = new SynthParameter(*master->_harmonicAmps[i]);
        p->setRange(0.0f, 1.0f);
        
        _harmonicAmps.push_back(p);             // Store harmonic amplitudes in a vector
        addParameter(p);                        // Add to the public parameter list
    }
}

void AdditiveSynthVoice::setNumHarmonics(float num) {
    
    for (int i = 0; i < _numHarmonics; i++) {
        
        char name[12];
        sprintf(name, "Harmonic %d", i+1);
        removeParameter(name);
    }
    
    _numHarmonics = num;
    
    /* Clear the harmonic amplitudes vector, and clear any harmonic amplitude params from the parameter list and sample update and sample rate listeners */
    _harmonicAmps.clear();
    for (int i = 0; i < _sampleRateListeners.size(); i++) {
        
    }
    
    SynthParameter* p;
    for (int i = 0; i < _numHarmonics; i++) {
        
        char name[12];
        sprintf(name, "Harmonic %d", i+1);
        p = new SynthParameter(name, _fs, i == 0 ? 1.0f : 0.0f, 0.1f);
        p->setRange(0.0f, 1.0f);
        
        _harmonicAmps.push_back(p);            // Store harmonic amplitudes in a vector
        removeParameter(name);
        addParameter(p);
    }
}

void AdditiveSynthVoice::setHarmonicAmp(int harmonicNumber, float amp) {
    
    if (harmonicNumber >= 1 && harmonicNumber <= _numHarmonics)
        _harmonicAmps[harmonicNumber-1]->setValue(amp);
}

int AdditiveSynthVoice::renderSample(float *outSample) {
    
    sampleUpdate();     // Ramp update any parameters added to the vector _sampleUpdateListeners
    
    /* Add a value for each harmonic below 20kHz */
    for (int n = 0; n < _numHarmonics; n++) {
        _harmonicAmps[n]->ramp();        // Ramp the harmonic amplitudes if needed
        if (_f0->value() * (n+1) < 20000.0f)
            *outSample += (_harmonicAmps[n]->value() * sin((n+1) * _theta));
    }
    *outSample /= _numHarmonics;    // Normalize
    
    *outSample *= _velAmp->value();             // Scale by current MIDI velocity-mapped amplitude
    *outSample *= _adsr.currentAmplitude();     // Scale by current envelope amplitude
    *outSample *= _amp->value();                // Scale by current voice amplitude
   
    return _adsr.update();          // Update the ADSR envelope state
}













