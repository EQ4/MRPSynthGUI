//
//  BiquadFilter.cpp
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 10/15/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "BiquadFilter.h"

BiquadFilter::BiquadFilter() :
_fs(kBiquad_DefaultSampleRate), _type(kBiquadFilterType_LowPass),
_fc(new SynthParameter("Cutoff Freq", kBiquad_DefaultSampleRate, 10000.0f, kBiquad_ParameterRampDuration)),
_Q(new SynthParameter("Resonance", kBiquad_DefaultSampleRate, 1.0f, kBiquad_ParameterRampDuration)) {
    
    for (int i = 0; i < 2; i++)
        _x[i] = _y[i] = 0.0f;
    
    _fc->setRange(20.0f, 20000.0f);
    _Q->setRange(0.0f, 3.0f);
    
    computeCoefficients();
    
    /* Add parameter change listeners for Fc and Q so the filter is updated whenever the parameters are set or ramped */
    _fc->setParameterChangeListener(BiquadFilter::staticFcChanged, (void *)this);
    _Q->setParameterChangeListener(BiquadFilter::staticQChanged, (void *)this);
}

BiquadFilter::BiquadFilter(BiquadFilterType type) :
_fs(kBiquad_DefaultSampleRate), _type(type),
_fc(new SynthParameter("Cutoff Freq", kBiquad_DefaultSampleRate, 10000.0f, kBiquad_ParameterRampDuration)),
_Q(new SynthParameter("Resonance", kBiquad_DefaultSampleRate, 1.0f, kBiquad_ParameterRampDuration)) {
    
    _fc->setRange(20.0f, 20000.0f);
    _Q->setRange(0.0f, 3.0f);
    
    switch (_type) {
        case kBiquadFilterType_LowPass:
            break;
            
        case kBiquadFilterType_HighPass:
        case kBiquadFilterType_BandPass:
            *_fc = 20.0f;
            break;
            
        default:
            printf("%s: Invalid filter type\n", __PRETTY_FUNCTION__);
            break;
    }
    
    for (int i = 0; i < 2; i++)
        _x[i] = _y[i] = 0.0f;
    
    computeCoefficients();
    
    /* Add parameter change listeners for Fc and Q so the filter is updated whenever the parameters are set or ramped */
    _fc->setParameterChangeListener(BiquadFilter::staticFcChanged, (void *)this);
    _Q->setParameterChangeListener(BiquadFilter::staticQChanged, (void *)this);
}

BiquadFilter::BiquadFilter(BiquadFilterType type, float sampleRate, float cornerFreq, float resonance) :
_fs(sampleRate), _type(type),
_fc(new SynthParameter("Cutoff Freq", sampleRate, cornerFreq, kBiquad_ParameterRampDuration)),
_Q(new SynthParameter("Resonance", sampleRate, resonance, kBiquad_ParameterRampDuration)) {
    
    _fc->setRange(20.0f, 20000.0f);
    _Q->setRange(0.0f, 3.0f);
    
    for (int i = 0; i < 2; i++)
        _x[i] = _y[i] = 0.0f;
    
    computeCoefficients();
    
    /* Add parameter change listeners for Fc and Q so the filter is updated whenever the parameters are set or ramped */
    _fc->setParameterChangeListener(BiquadFilter::staticFcChanged, (void *)this);
    _Q->setParameterChangeListener(BiquadFilter::staticQChanged, (void *)this);
}

BiquadFilter::BiquadFilter(const BiquadFilter *o) : _fs(o->_fs), _fc(new SynthParameter(*o->_fc)), _Q(new SynthParameter(*o->_Q)), _type(o->_type) {
    
    _fc->setRange(20.0f, 20000.0f);
    _Q->setRange(0.0f, 3.0f);
    
    for (int i = 0; i < 2; i++) {
        _x[i] = o->_x[i];
        _y[i] = o->_y[i];
    }
    
    computeCoefficients();
    
    /* Add parameter change listeners for Fc and Q so the filter is updated whenever the parameters are set or ramped. First remove listeners set by the master voice */
    _fc->removeParameterChangeListener();
    _fc->setParameterChangeListener(BiquadFilter::staticFcChanged, (void *)this);
    _Q->removeParameterChangeListener();
    _Q->setParameterChangeListener(BiquadFilter::staticQChanged, (void *)this);
}

void BiquadFilter::setFilterType(BiquadFilterType type) {
    
    _type = type;
    computeCoefficients();
}

void BiquadFilter::setFc(float fc, bool doRamp) {
    
    if (fc <= 0.0f) {
        printf("%s: Invalid corner frequency %f\n", __PRETTY_FUNCTION__, fc);
        return;
    }
    
    if (doRamp)
        _fc->setValue(fc);
    else
        *_fc = fc;
    
    computeCoefficients();
}

void BiquadFilter::setQ(float Q, bool doRamp) {
    
    if (Q <= 0.0f) {
        printf("%s: Invalid filter Q %f\n", __PRETTY_FUNCTION__, Q);
        return;
    }
    
    if (doRamp)
        _Q->setValue(Q);
    else
        *_Q = Q;
    
    computeCoefficients();
}

void BiquadFilter::fcChanged(float targetVal) {
    computeCoefficients();
}


void BiquadFilter::qChanged(float targetVal) {
    computeCoefficients();
}

void BiquadFilter::computeCoefficients() {
    
    /* Intermediate parameters */
    _omega = 2 * M_PI * _fc->value() / _fs;
    _alpha = sin(_omega) / (2.0f * _Q->value());
    
    switch (_type) {
            
        case kBiquadFilterType_LowPass:
            
            _a[0] = 1.0f + _alpha;
            _a[1] = -2.0f * cos(_omega);
            _a[2] = 1.0f - _alpha;

            _b[0] = (1.0f - cos(_omega)) / 2.0f;
            _b[1] = 1.0f - cos(_omega);
            _b[2] = _b[0];
            
            break;
            
        case kBiquadFilterType_HighPass:
            
            _a[0] = 1.0f + _alpha;
            _a[1] = -2.0f * cos(_omega);
            _a[2] = 1.0f - _alpha;

            _b[0] = (1.0f + cos(_omega)) / 2.0f;
            _b[1] = -(1.0f + cos(_omega));
            _b[2] = _b[0];
            
            break;
            
        case kBiquadFilterType_BandPass:
            
            _a[0] = 1.0f + _alpha;
            _a[1] = -2.0f * cos(_omega);
            _a[2] = 1.0f - _alpha;

            _b[0] = _alpha;
            _b[1] = 0.0f;
            _b[2] = -_alpha;
            
            break;
            
        default:
            
            for (int i = 0; i < 3; i++)
                _a[i] = _b[i] = 0.0f;
            break;
            
    }
}

std::vector<SynthParameter*> BiquadFilter::getParameters() {
    
    std::vector<SynthParameter*> params;
    
    /* Make corner freq and resonance parameters accessible */
    params.push_back(_fc);
    params.push_back(_Q);
    
    return params;
}

void BiquadFilter::filterSample(float *sample) {
    
    _x[1] = _x[0];
    _x[0] = *sample;
    
    /* Direct form 1 difference equation */
    *sample =   (_b[0] * *sample +
                 _b[1] * _x[0] +
                 _b[2] * _x[1] -
                 _a[1] * _y[0] -
                 _a[2] * _y[1])
                / _a[0];
    
    _y[1] = _y[0];
    _y[0] = *sample;
}

void BiquadFilter::printStabilityWarning() {
    
    /* Stability of a biquad filter determined by the following conditions as noted in http://www.dafx.ca/proceedings/papers/p_057.pdf (Equation 2) */
    
    if (abs(_a[1]) >= (_a[2] + 1.0f))
        printf("=== Warning: a1 is unstable (|a1| >= a2 + 1)\n");
    
    if (abs(_a[2]) < 1)
        printf("=== Warning: a2 is unstable (|a2| >= 1)\n");
    
}

void BiquadFilter::printCoefficients() {
    
    printf("Biquad filter coefficients\n");
    printf("--------------------------\n");
    printf("b0 = %21f\n", _b[0]);
    printf("b1 = %21f\n", _b[1]);
    printf("b2 = %21f\n", _b[2]);
    printf("a0 = %21f\n", _a[0]);
    printf("a1 = %21f\n", _a[1]);
    printf("a2 = %21f\n", _a[2]);
    printf("--------------------------\n");
}









