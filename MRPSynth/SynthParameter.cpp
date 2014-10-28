//
//  SynthParameter.cpp
//  MRP
//
//  Created by Jeff Gregorio on 7/28/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "SynthParameter.h"

#pragma mark - Constructors
SynthParameter::SynthParameter() : _name("None"), _fs(44100.0f), _value(0.0f), _rampDuration(0.1f), _targetValue(0.0f), _valueStep(0.0f), _maxValue(std::numeric_limits<float>::max()), _minValue(std::numeric_limits<float>::min()), _parameterChangeListener(nullptr), _parameterChangeListenerUserData(nullptr), _hasParameterChangeListener(false) { }

SynthParameter::SynthParameter(std::string name, float fs, float value, float rampDuration) : _name(name), _fs(fs), _value(value), _rampDuration(rampDuration), _targetValue(value), _valueStep(0.0f), _maxValue(std::numeric_limits<float>::max()), _minValue(std::numeric_limits<float>::min()), _parameterChangeListener(nullptr), _parameterChangeListenerUserData(nullptr), _hasParameterChangeListener(false) { }

//SynthParameter::SynthParameter() : _name("None"), _fs(44100.0f), _value(0.0f), _rampDuration(0.1f), _targetValue(0.0f), _valueStep(0.0f), _maxValue(std::numeric_limits<float>::max()), _minValue(std::numeric_limits<float>::min())  { }
//
//SynthParameter::SynthParameter(std::string name, float fs, float value, float rampDuration) : _name(name), _fs(fs), _value(value), _rampDuration(rampDuration), _targetValue(value), _valueStep(0.0f), _maxValue(std::numeric_limits<float>::max()), _minValue(std::numeric_limits<float>::min()) { }

#pragma mark - Setters
void SynthParameter::namePrepend(std::string prefix) {
    _name.insert(0, prefix);
}

void SynthParameter::setSampleRate(const float fs) {
    
    _fs = fs;
    _valueStep = (_targetValue - _value) / (_rampDuration * _fs);
}

void SynthParameter::setValue(const float value) {
    
    _targetValue = value;
    _targetValue = std::min(_targetValue, _maxValue);
    _targetValue = std::max(_targetValue, _minValue);
    _valueStep = (_targetValue - _value) / (_rampDuration * _fs);
}

void SynthParameter::setValue(const SynthParameter param) {
    
    _targetValue = param.value();
    _targetValue = std::min(_targetValue, _maxValue);
    _targetValue = std::max(_targetValue, _minValue);
    _valueStep = (_targetValue - _value) / (_rampDuration * _fs);
}

void SynthParameter::setRampDuration(const float rampDuration) {
    
    _rampDuration = rampDuration;
    _valueStep = (_targetValue - _value) / (_rampDuration * _fs);
}

void SynthParameter::setRange(float minVal, float maxVal) {
    _minValue = minVal;
    _maxValue = maxVal;
}

void SynthParameter::setParameterChangeListener(ParameterChangeListener callback, void *userData) {
    
    if (_hasParameterChangeListener) {
        printf("%s: Parameter \"%s\" already has a parameter change callback set\n", __PRETTY_FUNCTION__, _name.c_str());
        return;
    }
    
    if (!callback) {
        printf("%s: Invalid callback method\n", __PRETTY_FUNCTION__);
        return;
    }
    
    _parameterChangeListener = (void *)callback;
    _parameterChangeListenerUserData = userData;
    _hasParameterChangeListener = true;
}

void SynthParameter::removeParameterChangeListener() {
    
    if (!_hasParameterChangeListener) {
        printf("%s: Parameter \"%s\" has no parameter change listener to remove\n", __PRETTY_FUNCTION__, _name.c_str());
        return;
    }
    
    _parameterChangeListener = nullptr;
    _parameterChangeListenerUserData = nullptr;
    _hasParameterChangeListener = false;
}

#pragma mark - Updates
void SynthParameter::ramp() {
    
    if ((_valueStep > 0 && _value < _targetValue) || (_valueStep < 0 && _value > _targetValue))
        _value += _valueStep;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
}

void SynthParameter::ramp(int nSamples) {
    
    float modStep = _valueStep * nSamples;
    
    if ((modStep > 0.0f && _value < _targetValue) || (modStep < 0.0f && _value < _targetValue)) {
        
        _value += modStep;
        
        /* Notify the parameter listener if it exists */
        if (_hasParameterChangeListener) {
            ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
            callback(_targetValue, _parameterChangeListenerUserData);
        }
    }
    
    else
        return;
}

#pragma mark - Overloaded Operators (Members)
SynthParameter& SynthParameter::operator=(const float value) {
    
    _targetValue = value;
    _targetValue = std::min(_targetValue, _maxValue);
    _targetValue = std::max(_targetValue, _minValue);
    _value = _targetValue;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}

SynthParameter& SynthParameter::operator=(const SynthParameter param) {
    
    _fs = param._fs;
    _targetValue = param._value;
    _targetValue = std::min(_targetValue, _maxValue);
    _targetValue = std::max(_targetValue, _minValue);
    _value = _targetValue;
    _valueStep = 0.0f;
    _rampDuration = param._rampDuration;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}

SynthParameter& SynthParameter::operator+=(const float value) {

    _value += value;
    _value = std::min(_value, _maxValue);
    _targetValue = _value;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}

SynthParameter& SynthParameter::operator+=(const SynthParameter param) {
    
    _value += param._value;
    _value = std::min(_value, _maxValue);
    _targetValue = _value;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}

SynthParameter& SynthParameter::operator-=(const float value) {
    
    _value -= value;
    _value = std::min(_value, _maxValue);
    _value = std::max(_value, _minValue);
    _targetValue = _value;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}

SynthParameter& SynthParameter::operator-=(const SynthParameter param) {
    
    _value -= param._value;
    _value = std::min(_value, _maxValue);
    _value = std::max(_value, _minValue);
    _targetValue = _value;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}

SynthParameter& SynthParameter::operator*=(const float value) {
    
    _value *= value;
    _value = std::min(_value, _maxValue);
    _value = std::max(_value, _minValue);
    _targetValue = _value;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}


SynthParameter& SynthParameter::operator*=(const SynthParameter param) {
    
    _value *= param._value;
    _value = std::min(_value, _maxValue);
    _value = std::max(_value, _minValue);
    _targetValue = _value;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}

SynthParameter& SynthParameter::operator/=(const float value) {
    
    _value /= value;
    _value = std::min(_value, _maxValue);
    _value = std::max(_value, _minValue);
    _targetValue = _value;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}


SynthParameter& SynthParameter::operator/=(const SynthParameter param) {
    
    _value /= param._value;
    _value = std::min(_value, _maxValue);
    _value = std::max(_value, _minValue);
    _targetValue = _value;
    _valueStep = 0.0f;
    
    /* Notify the parameter listener if it exists */
    if (_hasParameterChangeListener) {
        ParameterChangeListener callback = (ParameterChangeListener)_parameterChangeListener;
        callback(_targetValue, _parameterChangeListenerUserData);
    }
    
    return *this;
}










































