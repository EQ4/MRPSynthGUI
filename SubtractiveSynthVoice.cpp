//
//  SubtractiveSynthVoice.cpp
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 10/15/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "SubtractiveSynthVoice.h"


SubtractiveSynthVoice::SubtractiveSynthVoice() : _numHarmonics(1) {
    
}

SubtractiveSynthVoice::SubtractiveSynthVoice(int numHarmonics) : _numHarmonics(numHarmonics), _filter(BiquadFilter()), _filterEnv(ADSREnvelope()), _filterEnvInvert(false) {
    
    _fs = kSynthVoice_Default_fs;
    
    SynthParameter* p;
    for (int i = 0; i < _numHarmonics; i++) {
        
        char name[12];
        sprintf(name, "Harmonic %d", i+1);
        p = new SynthParameter(name, _fs, i == 0 ? 1.0f : 0.0f, 0.1f);
        p->setRange(0.0f, 1.0f);
        
        _harmonicAmps.push_back(p);             // Store harmonic amplitudes in a vector
        addParameter(p);                        // Add to the public parameter list
    }
    
    /* Get Fc and Q parameters from the filter to add to the parameter list */
    vector<SynthParameter*> params = _filter.getParameters();
    for (int i = 0; i < params.size(); i++)
        addParameter(params[i]);
    
    /* Get ADSR parameters from the filter envelope to add to the parameter list */
    params.clear();
    params = _filterEnv.getParameters();
    for (int i = 0; i < params.size(); i++) {
        params[i]->namePrepend("Filter ");
        addParameter(params[i]);
    }
}

SubtractiveSynthVoice::SubtractiveSynthVoice(std::vector<float>harmonicAmps) : _numHarmonics((int)harmonicAmps.size()), _filter(BiquadFilter()), _filterEnv(ADSREnvelope()), _filterEnvInvert(false) {
    
    _fs = kSynthVoice_Default_fs;
    
    SynthParameter* p;
    for (int i = 0; i < _numHarmonics; i++) {
        
        char name[12];
        sprintf(name, "Harmonic %d", i+1);
        p = new SynthParameter(name, _fs, i == 0 ? 1.0f : 0.0f, 0.1f);
        p->setRange(0.0f, 1.0f);
        
        _harmonicAmps.push_back(p);             // Store harmonic amplitudes in a vector
        addParameter(p);                        // Add to the public parameter list
    }
    
    /* Get Fc and Q parameters from the filter to add to the parameter list */
    vector<SynthParameter*> params = _filter.getParameters();
    for (int i = 0; i < params.size(); i++)
        addParameter(params[i]);
    
    /* Get ADSR parameters from the filter envelope to add to the parameter list */
    params.clear();
    params = _filterEnv.getParameters();
    for (int i = 0; i < params.size(); i++) {
        params[i]->namePrepend("Filter ");
        addParameter(params[i]);
    }
}

SubtractiveSynthVoice::SubtractiveSynthVoice(SubtractiveSynthVoice *master) : SynthVoice(master), _numHarmonics(master->_numHarmonics), _filter(BiquadFilter(&master->_filter)), _filterEnv(ADSREnvelope(&master->_filterEnv)), _filterEnvInvert(master->_filterEnvInvert) {
    
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
    
    /* Get Fc and Q parameters from the filter to add to the parameter list */
    vector<SynthParameter*> params = _filter.getParameters();
    for (int i = 0; i < params.size(); i++)
        addParameter(params[i]);
    
    /* Get ADSR parameters from the filter envelope to add to the parameter list */
    params.clear();
    params = _filterEnv.getParameters();
    for (int i = 0; i < params.size(); i++)
        addParameter(params[i]);
}

void SubtractiveSynthVoice::setNumHarmonics(float num) {
    
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

void SubtractiveSynthVoice::setHarmonicAmp(int harmonicNumber, float amp) {
    
    if (harmonicNumber >= 1 && harmonicNumber <= _numHarmonics)
        _harmonicAmps[harmonicNumber-1]->setValue(amp);
}

void SubtractiveSynthVoice::setFilterType(BiquadFilterType type) {
    
    _filter.setFilterType(type);
}

void SubtractiveSynthVoice::beginAttack() {
    
    _adsr.beginAttack();
    _filterEnv.beginAttack();
}

int SubtractiveSynthVoice::renderSample(float *outSample) {
    
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
    
    // TODO: Add a method to SynthParameter for setting deviations from the parameter's base value. This should allow the filter cutoff's value to be settable via slider/mapping while allowing the envelope (and later LFOs and add/multiply mappings) to introduce deviations without altering the base value.
    _filter.filterSample(outSample);
    
    _filterEnv.update();
    return _adsr.update();          // Update the ADSR envelope state
}


















