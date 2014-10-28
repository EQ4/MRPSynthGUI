//
//  ADSREnvelope.cpp
//  MRP
//
//  Created by App Lab on 7/29/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "ADSREnvelope.h"

#pragma mark - Constructors
ADSREnvelope::ADSREnvelope() : EffectBase("ADSR"), _state(kADSRPhase_Attack), _fs(kADSR_Default_fs), _timeInState(0.0f), _releaseAmp(0.0f), _amp(SynthParameter("Amplitude", kADSR_Default_fs, 0.0f, kADSR_ParameterRampDuration)), _atk(new SynthParameter("Attack", kADSR_Default_fs, 0.1f, kADSR_ParameterRampDuration)), _dec(new SynthParameter("Decay", kADSR_Default_fs, 0.1f, kADSR_ParameterRampDuration)), _sus(new SynthParameter("Sustain", kADSR_Default_fs, 1.0f, kADSR_ParameterRampDuration)), _rel(new SynthParameter("Release", kADSR_Default_fs, 0.1f, kADSR_ParameterRampDuration)) {
    
    _amp.setRange(0.0f, 1.0f);
    _atk->setRange(0.01f, 2.0f);
    _dec->setRange(0.01f, 2.0f);
    _sus->setRange(0.0f, 1.0f);
    _rel->setRange(0.01f, 2.0f);
}

ADSREnvelope::ADSREnvelope(float fs) : EffectBase("ADSR"), _state(kADSRPhase_Attack), _fs(fs), _timeInState(0.0f), _releaseAmp(0.0f), _amp(SynthParameter("Amplitude", fs, 0.0f, kADSR_ParameterRampDuration)), _atk(new SynthParameter("Attack", fs, 0.1f, kADSR_ParameterRampDuration)), _dec(new SynthParameter("Decay", fs, 0.1f, kADSR_ParameterRampDuration)), _sus(new SynthParameter("Sustain", fs, 1.0f, kADSR_ParameterRampDuration)), _rel(new SynthParameter("Release", fs, 0.1f, kADSR_ParameterRampDuration)) {
    
    _amp.setRange(0.01f, 1.0f);
    _atk->setRange(0.01f, 2.0f);
    _dec->setRange(0.01f, 2.0f);
    _sus->setRange(0.0f, 1.0f);
    _rel->setRange(0.01f, 2.0f);
}

ADSREnvelope::ADSREnvelope(float fs, float atk, float dec, float sus, float rel) : EffectBase("ADSR"), _fs(fs), _timeInState(0.0f), _releaseAmp(0.0f), _amp(SynthParameter("Amplitude", fs, 0.0f, kADSR_ParameterRampDuration)), _atk(new SynthParameter("Attack", fs, atk, kADSR_ParameterRampDuration)), _dec(new SynthParameter("Decay", fs, dec, kADSR_ParameterRampDuration)), _sus(new SynthParameter("Sustain", fs, sus, kADSR_ParameterRampDuration)), _rel(new SynthParameter("Release", fs, rel, kADSR_ParameterRampDuration)) {
    
    _amp.setRange(0.0f, 1.0f);
    _atk->setRange(0.01f, 2.0f);
    _dec->setRange(0.01f, 2.0f);
    _sus->setRange(0.0f, 1.0f);
    _rel->setRange(0.01f, 2.0f);
}

ADSREnvelope::ADSREnvelope(const ADSREnvelope* o) : EffectBase("ADSR"), _fs(o->_fs), _timeInState(o->_timeInState), _releaseAmp(o->_releaseAmp), _amp(SynthParameter(o->_amp)), _atk(new SynthParameter(*o->_atk)), _dec(new SynthParameter(*o->_dec)), _sus(new SynthParameter(*o->_sus)), _rel(new SynthParameter(*o->_rel)) {
    
    _amp.setRange(_amp.minVal(), _amp.maxVal());
    _atk->setRange(_atk->minVal(), _atk->maxVal());
    _dec->setRange(_atk->minVal(), _atk->maxVal());
    _sus->setRange(_atk->minVal(), _atk->maxVal());
    _rel->setRange(_atk->minVal(), _atk->maxVal());
}

void ADSREnvelope::setSampleRate(float fs) {
    
    _fs = fs;
    _amp.setSampleRate(_fs);
    _atk->setSampleRate(_fs);
    _dec->setSampleRate(_fs);
    _sus->setSampleRate(_fs);
    _rel->setSampleRate(_fs);
}

void ADSREnvelope::setAttack(float atk, bool doRamp) {
    
    if (doRamp)
        _atk->setValue(atk);
    else
        *_atk = atk;
}

void ADSREnvelope::setDecay(float dec, bool doRamp) {

    if (doRamp)
        _dec->setValue(dec);
    else
        *_dec = dec;
}

void ADSREnvelope::setSustain(float sus, bool doRamp) {
    
    if (doRamp)
        _sus->setValue(sus);
    else
        *_sus = sus;
}

void ADSREnvelope::setRelease(float rel, bool doRamp) {
    
    if (doRamp)
        _rel->setValue(rel);
    else
        *_rel = rel;
}

#pragma mark - Updates
/* Update the ramped parameters and envelope state. Return true if we haven't yet transitioned out of release */
bool ADSREnvelope::update() {
    
    bool cont = true;   // Return value indicating whether the voice should continue rendering
    
    /* Ramp the parameters if updates are needed */
    _amp.ramp();
    _atk->ramp();
    _dec->ramp();
    _sus->ramp();
    _rel->ramp();
    
    /* Keep track of time spent in the current state (seconds) */
    _timeInState += (1.0f / _fs);
    
    /* ADSR state machine */
    switch (_state) {
            
            /* Update the current amplitude directly in non-sustain phases */
        case kADSRPhase_Attack:
            _amp += (1.0f / *_atk) / _fs;            // Amplitude update
            if (_timeInState > *_atk) {              // State change condition
                _state = kADSRPhase_Decay;
                _timeInState = 0.0f;
            }
            break;
            
        case kADSRPhase_Decay:
            _amp += ((*_sus - 1.0f) / *_dec) / _fs;       // Amplitude update
            if (_timeInState > *_dec) {                  // State change condition
                _state = kADSRPhase_Sustain;
                _timeInState = 0.0f;
            }
            break;
            
        case kADSRPhase_Release:
            _amp += ((0.0f - _releaseAmp) / *_rel) / _fs;    // Amplitude update
            if (_timeInState > *_rel) {                      // State change condition
                cont = false;
                _timeInState = 0.0f;
            }
            break;
            
            /* If the sustain level changes while we're in the state, ramp the amplitude to the sustain level */
        case kADSRPhase_Sustain:
            if ((fabs(_amp - *_sus) > 0.02))
                _amp.setValue(*_sus);
            break;
            
        default:
            break;
    }
    
    return cont;
}

#pragma mark - State control
void ADSREnvelope::beginAttack() {
    
    _state = kADSRPhase_Attack;
    _timeInState = 0.0f;
}

void ADSREnvelope::beginRelease() {
    
    /* Get the current amplitude so we can properly computer the ramp for release */
    _releaseAmp = _amp.value();
    _state = kADSRPhase_Release;
    _timeInState = 0.0f;
}

#pragma mark - EffectBase Protocol Methods
std::vector<SynthParameter*> ADSREnvelope::getParameters() {
    
    std::vector<SynthParameter*> params;
    
    /* Make A, D, S, and R parameters accessible, but keep amplitude private */
    params.push_back(_atk);
    params.push_back(_dec);
    params.push_back(_sus);
    params.push_back(_rel);
    
    return params;
}

int ADSREnvelope::process(float *procBuffer, int nFrames) {
    
    bool cont = true;
    for (int i = 0; i < nFrames; i++) {
        cont = update();
        *procBuffer++ *= _amp.value();
    }
    return cont;    // update() returns 0 (false) when release has finished
}

#pragma mark - Utility
void ADSREnvelope::printADSRValues() {
    
    printf("ADSR Envelope has parameters:----------------------------\n");
    printf("attack  = %f\n", _atk->value());
    printf("decay   = %f\n", _dec->value());
    printf("sustain = %f\n", _sus->value());
    printf("release = %f\n", _rel->value());
    printf("----------------------------\n");
}













