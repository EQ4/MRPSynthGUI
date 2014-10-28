//
//  SynthVoice.cpp
//  MRP
//
//  Created by Jeff Gregorio on 7/28/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "SynthVoice.h"

#pragma mark - Constructors
SynthVoice::SynthVoice() : _fs(kSynthVoice_Default_fs), _theta(0.0f), _amp(nullptr), _f0(nullptr), _adsr(ADSREnvelope(kSynthVoice_Default_fs, kSynthVoice_Default_Atk, kSynthVoice_Default_Dec, kSynthVoice_Default_Dec, kSynthVoice_Default_Rel)) {
    
    clearParameterList();
    
    _velAmp = new SynthParameter("VelAmp", kSynthVoice_Default_fs, 1.0f, kSynthVoice_Default_amp_RampTime);
    _velAmp->setRange(0.0f, 1.0f);
    _amp = new SynthParameter("Amplitude", kSynthVoice_Default_fs, 1.0f, kSynthVoice_Default_amp_RampTime);
    _amp->setRange(0.0f, 1.0f);
    _f0 = new SynthParameter("Fundamental Frequency", kSynthVoice_Default_fs, 440.0f, kSynthVoice_Default_f0_RampTime);
    _f0->setRange(20.0f, 20000.0f);
    _thetaStep = M_2PI * _f0->value() / _fs;
    
    /* Add public parameters to the ParameterList */
    addParameter(_amp);
    addParameter(_f0);
    
    vector<SynthParameter*> params = _adsr.getParameters();
    for (int i = 0; i < params.size(); i++) {
        addParameter(params[i]);
    }
}

SynthVoice::SynthVoice(float fs) : _fs(fs), _theta(0.0f), _amp(nullptr), _f0(nullptr), _adsr(ADSREnvelope(fs, kSynthVoice_Default_Atk, kSynthVoice_Default_Dec, kSynthVoice_Default_Dec, kSynthVoice_Default_Rel)) {
    
    clearParameterList();
    
    _velAmp = new SynthParameter("VelAmp", fs, 1.0f, kSynthVoice_Default_amp_RampTime);
    _velAmp->setRange(0.0f, 1.0f);
    _amp = new SynthParameter("Amplitude", fs, 1.0f, kSynthVoice_Default_amp_RampTime);
    _amp->setRange(0.0f, 1.0f);
    _f0 = new SynthParameter("Fundamental Frequency", fs, 440.0f, kSynthVoice_Default_f0_RampTime);
    _f0->setRange(20.0f, 20000.0f);
    _thetaStep = M_2PI * _f0->value() / _fs;
    
    /* Add parameters to the ParameterList */
    addParameter(_amp);
    addParameter(_f0);
    
    vector<SynthParameter*> params = _adsr.getParameters();
    for (int i = 0; i < params.size(); i++) {
        addParameter(params[i]);
    }
}

SynthVoice::SynthVoice(const SynthVoice* master) : _fs(master->_fs), _theta(master->_theta), _amp(nullptr), _f0(nullptr), _adsr(ADSREnvelope(&master->_adsr)) {

    clearParameterList();
    
    _velAmp = new SynthParameter(*master->_velAmp);
    _velAmp->setRange(0.0f, 1.0f);
    _amp = new SynthParameter(*master->_amp);
    _amp->setRange(0.0f, 1.0f);
    _f0 = new SynthParameter(*master->_f0);
    _f0->setRange(20.0f, 20000.0f);
    _thetaStep = M_2PI * _f0->value() / _fs;
    
    /* Add parameters to the ParameterList */
    addParameter(_amp);
    addParameter(_f0);
    
    vector<SynthParameter*> params = _adsr.getParameters();
    for (int i = 0; i < params.size(); i++) {
        addParameter(params[i]);
    }
}

SynthVoice::~SynthVoice() {
    
    if (_velAmp)
        delete _velAmp;
    if (_amp)
        delete _amp;
    if (_f0)
        delete _f0;
}

#pragma mark - Inerface Methods
void SynthVoice::setSampleRate(float fs) {
    
    _fs = fs;
    _amp->setSampleRate(_fs);
    _f0->setSampleRate(_fs);
    _adsr.setSampleRate(_fs);
    
    // TODO: Update sample rate of parameter list params
    
    for (int i = 0; i < (int)_sampleRateListeners.size(); i++) {
        _sampleRateListeners[i]->setSampleRate(_fs);
    }
}

void SynthVoice::setVelocityAmplitude(float amp, bool doRamp) {
    
    if (doRamp)
        _velAmp->setValue(amp);
    else
        *_velAmp = amp;
}

void SynthVoice::setAmplitude(float amp, bool doRamp) {
    
    if (doRamp)
        _amp->setValue(amp);
    else
        *_amp = amp;
}

void SynthVoice::setF0(float f0, bool doRamp) {
    
    if (doRamp)
        _f0->setValue(f0);
    else
        *_f0 = f0;
}

void SynthVoice::setAttack(float atk, bool doRamp) {
    _adsr.setAttack(atk, doRamp);
}

void SynthVoice::setDecay(float dec, bool doRamp) {
    _adsr.setDecay(dec, doRamp);
}

void SynthVoice::setSustain(float sus, bool doRamp) {
    _adsr.setSustain(sus, doRamp);
}

void SynthVoice::setRelease(float rel, bool doRamp) {
    _adsr.setRelease(rel, doRamp);
}

void SynthVoice::beginAttack() {
    
    _adsr.beginAttack();
    
    /* TODO: Automated parameter curves or sweeps to target values like below */
    
//    _f0->setRampDuration(1.0f);
//    _f0->setValue(*_f0 - 100.0f);
}

void SynthVoice::beginRelease() {
    _adsr.beginRelease();
}

void SynthVoice::sampleUpdate() {
    
    parameterListRamp();        // Ramp any parameters in the parameter list
    
    _thetaStep = M_2PI * _f0->value() / _fs;    // Recomputer the phase increment
    _theta += _thetaStep;                       // Update the phase
    if (_theta >= M_2PI)                        // Wrap to interval [0, 2*pi]
        _theta -= M_2PI;
}














