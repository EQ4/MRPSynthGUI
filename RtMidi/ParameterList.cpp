//
//  ParameterList.cpp
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 8/5/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "ParameterList.h"

bool ParameterList::hasParameter(string name) {
    return _parameters.find(name) != _parameters.end();
}

vector<string> ParameterList::getParameterNames() {
    vector<string> names;
    for (map<string, SynthParameter*>::iterator it = _parameters.begin(); it != _parameters.end(); it++) {
        names.push_back((*it).second->name());
    }
    return names;
}

SynthParameter* ParameterList::getParameterWithName(string name) {
    /* Make sure we have a parameter with this name */
    if (_parameters.find(name) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, name.c_str());
        return nullptr;
    }
    
    return _parameters[name];
}

bool ParameterList::addParameter(SynthParameter* param) {
    
    /* Make sure we don't already have a parameter with this name */
    if (_parameters.find(param->name()) != _parameters.end()) {
        printf("%s: Duplicate parameter %s\n", __PRETTY_FUNCTION__, param->name().c_str());
        return false;
    }
    
    printf("%s: Adding parameter \"%s\"\n", __PRETTY_FUNCTION__, param->name().c_str());
    
    _parameters[param->name()] = param;
    
//    printf("%s: ""%s"" ", __PRETTY_FUNCTION__, param->name().c_str());
//    for (int i = 0; i < 30 - param->name().size(); i++)
//        printf(" ");
//    printf("[size = %lu]\n", _parameters.size());
    
    return true;
}

bool ParameterList::removeParameter(string name) {
    
    map<string, SynthParameter*>::iterator it = _parameters.find(name);
    
    if (it == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, name.c_str());
        return false;
    }
    
//    printf("%s: ""%s"" ", __PRETTY_FUNCTION__, name.c_str());
//    for (int i = 0; i < 30 - name.size(); i++)
//        printf(" ");
//    printf("[size = %lu]\n", _parameters.size());
    
    _parameters.erase(it);
    return true;
}

void ParameterList::clearParameterList() {
    _parameters.clear();
}

/* Ramp all parameters in the list for a single sample */
void ParameterList::parameterListRamp() {
    for (map<string, SynthParameter*>::iterator it = _parameters.begin(); it != _parameters.end(); ++it)
        (*it).second->ramp();
}

bool ParameterList::addMidiMapping(MidiMapping *mapping) {
    
    /* Make sure we have a parameter with this name */
    if (_parameters.find(mapping->parameterName) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, mapping->parameterName.c_str());
        return false;
    }
    
    /* Make sure the bytes are valid MIDI bytes */
    if (mapping->byte1 < 0 || mapping->byte1 >= 255) {
        printf("%s: Invalid MIDI byte %x. Specify a number 0x00-0x7f\n", __PRETTY_FUNCTION__, mapping->byte1);
        return false;
    }
    if (mapping->byte2 < 0 || mapping->byte2 >= 127) {
        printf("%s: Invalid MIDI byte %x. Specify a number 0x00-0x7f\n", __PRETTY_FUNCTION__, mapping->byte2);
        return false;
    }
    
    _midiListeners[(mapping->byte1 << 8) | mapping->byte2].push_back(mapping);
    
    return true;
}

bool ParameterList::addOscMapping(OscMapping mapping) {
    
    /* Make sure we have a parameter with this name */
    if (_parameters.find(mapping.parameterName) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, mapping.parameterName.c_str());
        return false;
    }
    
    _oscListeners[mapping.path].push_back(mapping);
    
    return true;
}

bool ParameterList::respondsToMidiEvent(int byte1) {
    return respondsToMidiEvent(byte1, 0x00);
}

bool ParameterList::respondsToMidiEvent(int byte1, int byte2) {
    return (_midiListeners.find((byte1 << 8) | byte2) != _midiListeners.end());
}

bool ParameterList::respondsToOscEvent(string path) {
    return (_oscListeners.find(path) != _oscListeners.end());
}

bool ParameterList::removeMidiMapping(string name) {
    
    /* Make sure we have a parameter with this name */
    if (_parameters.find(name) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, name.c_str());
        return false;
    }
    
    bool found = false;
    
    /* Keep track of any map entries whose size drops to zero so we can erase them later */
    int i = 0;
    int erase[_midiListeners.size()];
    for (int i = 0; i < _midiListeners.size(); i++)
        erase[i] = 0;
    
    /* Find instances of the specified parameter in the MIDI listeners map */
    for (map<int, vector<MidiMapping*> >::iterator mit = _midiListeners.begin();
         mit != _midiListeners.end(); ++mit) {
        
        /* Get the vector of SynthParameters that respond to this MIDI message */
        vector<MidiMapping*>& mappings = mit->second;
        for (vector<MidiMapping*>::iterator vit = mappings.begin(); vit != mappings.end(); ) {
            
            /* Erase the parameter if it has the target name */
            if (!(*vit)->parameterName.compare(name)) {
                mappings.erase(vit);
                found = true;
                
                /* See if we've erased the last element in the vector */
                if (mappings.size() == 0)
                    erase[i] = mit->first;  // Save the map key of the entry to erase
            }
            else
                ++vit;
        }
        i++;
    }
    
    /* If we saved any map keys for entries to erase, erase them */
    for (int i = 0; i < _midiListeners.size(); i++) {
        if (erase[i])
            _midiListeners.erase(erase[i]);
    }
    
    if (!found)
        printf("%s: Parameter %s does not respond to MIDI events\n", __PRETTY_FUNCTION__, name.c_str());
    
    return found;
}

bool ParameterList::removeMidiMapping(string name, int byte1) {
    return removeMidiMapping(name, byte1, 0x00);
}

bool ParameterList::removeMidiMapping(string name, int byte1, int byte2) {
    
    /* Make sure we have a parameter with this name */
    if (_parameters.find(name) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, name.c_str());
        return false;
    }
    
    /* Make sure at least one parameter responds to the specified event */
    if (!respondsToMidiEvent(byte1, byte2)) {
        printf("%s: No parameters respond to MIDI event with bytes (%x, %x)\n", __PRETTY_FUNCTION__, byte1, byte2);
        return false;
    }
    
    /* Get the vector of parameters that respond to the specified event */
    vector<MidiMapping*>& mappings = _midiListeners[(byte1 << 8) | byte2];
    
    /* Find the parameter with the specified name */
    bool found = false;
    for (vector<MidiMapping*>::iterator it = mappings.begin(); it != mappings.end(); ) {
        
        if (!(*it)->parameterName.compare(name)) {
            mappings.erase(it);
            found = true;
        }
        else
            ++it;
    }
    
    /* If we erased the last parameter for this map entry, erase the map entry */
    if (mappings.size() == 0)
        _midiListeners.erase(_midiListeners.find((byte1 << 8) | byte2));
    
    return true;
}

bool ParameterList::removeMidiMapping(MidiMapping *mapping) {
    
    /* Make sure we have a parameter with this name */
    if (_parameters.find(mapping->parameterName) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, mapping->parameterName.c_str());
        return false;
    }
    
    bool found = false;
    
    /* Keep track of any map entries whose size drops to zero so we can erase them later */
    int i = 0;
    int erase[_midiListeners.size()];
    for (int i = 0; i < _midiListeners.size(); i++)
        erase[i] = 0;
    
    /* Find instances of the specified parameter in the MIDI listeners map */
    for (map<int, vector<MidiMapping*> >::iterator mit = _midiListeners.begin();
         mit != _midiListeners.end(); ++mit) {
        
        /* Get the vector of SynthParameters that respond to this MIDI message */
        vector<MidiMapping*>& mappings = mit->second;
        for (vector<MidiMapping*>::iterator vit = mappings.begin(); vit != mappings.end(); ) {
            
            /* Erase the parameter if it has the target name */
            if (*vit == mapping) {
                mappings.erase(vit);
                found = true;
                
                /* See if we've erased the last element in the vector */
                if (mappings.size() == 0)
                    erase[i] = mit->first;  // Save the map key of the entry to erase
            }
            else
                ++vit;
        }
        i++;
    }
    
    /* If we saved any map keys for entries to erase, erase them */
    for (int i = 0; i < _midiListeners.size(); i++) {
        if (erase[i])
            _midiListeners.erase(erase[i]);
    }
    
    if (!found)
        printf("%s: Parameter %s does not respond to MIDI events\n", __PRETTY_FUNCTION__, mapping->parameterName.c_str());
    
    return found;
}

bool ParameterList::removeOscMapping(string name) {
    
    /* Make sure we have a parameter with this name */
    if (_parameters.find(name) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, name.c_str());
        return false;
    }
    
    bool found = false;
    
    /* Keep track of any map entries whose size drops to zero so we can erase them later */
    vector<string> erase;
    
    /* Find instances of the specified parameter in the MIDI listeners map */
    for (map<string, vector<OscMapping> >::iterator mit = _oscListeners.begin();
         mit != _oscListeners.end(); ++mit) {
        
        /* Get the vector of SynthParameters that respond to this MIDI message */
        vector<OscMapping>& mappings = mit->second;
        for (vector<OscMapping>::iterator vit = mappings.begin(); vit != mappings.end(); ) {
            
            /* Erase the parameter if it has the target name */
            if (!(*vit).parameterName.compare(name)) {
                mappings.erase(vit);
                found = true;
                
                /* See if we've erased the last element in the vector */
                if (mappings.size() == 0)
                    erase.push_back(mit->first);    // Save the map key of the entry to erase
                else
                    erase.push_back("");            // Save empty string to keep
            }
            else
                ++vit;
        }
    }
    
    /* If we saved any map keys for entries to erase, erase them */
    for (int i = 0; i < _oscListeners.size(); i++) {
        if (erase[i].size() > 0) {
            _oscListeners.erase(erase[i]);
        }
    }
    
    if (!found)
        printf("%s: Parameter %s does not respond to OSC events\n", __PRETTY_FUNCTION__, name.c_str());
    
    return found;
}

bool ParameterList::removeOscMapping(string name, string path) {
    
    /* Make sure we have a parameter with this name */
    if (_parameters.find(name) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, name.c_str());
        return false;
    }
    
    /* Make sure at least one parameter responds to the specified event */
    if (!respondsToOscEvent(path)) {
        printf("%s: No parameters respond to OSC event with path %s\n", __PRETTY_FUNCTION__, path.c_str());
        return false;
    }
    
    /* Get the vector of parameters that respond to the specified event */
    vector<OscMapping>& mappings = _oscListeners[path];
    
    /* Find the parameter with the specified name */
    bool found = false;
    for (vector<OscMapping>::iterator it = mappings.begin(); it != mappings.end(); ) {
        
        if (!(*it).parameterName.compare(name)) {
            mappings.erase(it);
            found = true;
        }
        else
            ++it;
    }
    
    /* If we erased the last parameter for this map entry, erase the map entry */
    if (mappings.size() == 0)
        _oscListeners.erase(_oscListeners.find(path));
    
    return true;
}

bool ParameterList::removeOscMapping(OscMapping *mapping) {
    
    return true;
}

bool ParameterList::setParameterValue(string name, float value, bool doRamp) {
    
    /* Make sure we have a parameter with this name */
    if (_parameters.find(name) == _parameters.end()) {
        printf("%s: Unknown parameter %s\n", __PRETTY_FUNCTION__, name.c_str());
        return false;
    }
    
    if (doRamp)
        (*_parameters.find(name)).second->setValue(value);
    else
        *(*_parameters.find(name)).second = value;
    
    return true;
}

vector<pair<string, float> > ParameterList::handleMidi(vector<unsigned char>* message) {
    
    vector<pair<string, float> > updatedParams;
    
    int statusByte = (int)message->at(0);
    
    int key;
    int value = 0;
    
    if (message->size() == 2) {
        key = (statusByte << 8) | 0x00;
        value = message->at(1);
    }
    else if (message->size() == 3) {
        key = (statusByte << 8) | message->at(1);
        value = message->at(2);
    }
    else
        return updatedParams;
    
    /* If we have mappings for this message */
    if (_midiListeners.find(key) != _midiListeners.end()) {
        
        /* Return the vector of mappings and iterate over each one */
        vector<MidiMapping*>& mappings = _midiListeners[key];
        for (int i = 0; i < mappings.size(); i++) {
            
            /* Get the parameter for this mapping */
            SynthParameter* param = _parameters[mappings[i]->parameterName];
            
            /* Scale the 0-127 MIDI value to the range specified by the mapping */
            float fval = (float)value / 127.0f;
            fval *= (mappings[i]->max - mappings[i]->min);
            fval += mappings[i]->min;
            
            /* If the mapping is logarithmic, scale it again */
            if (mappings[i]->scale == kMappingScaleLogarithmic) {
                fval = mappings[i]->min * expf(fval * logf(mappings[i]->max / mappings[i]->min) /
                                               (mappings[i]->max - mappings[i]->min));
            }
            
            switch (mappings[i]->type) {
                    
                case kMappingTypeAssign:
                    if (mappings[i]->ramp) param->setValue(fval);
                    else *param = fval;
                    break;
                    
                case kMappingTypeAdd:
                    if (mappings[i]->ramp) param->setValue(*param + fval);
                    else *param += fval;
                    break;
                    
                case kMappingTypeMultiply:
                    if (mappings[i]->ramp) param->setValue(*param * fval);
                    else *param *= fval;
                    break;
                    
                default:
                    break;
            }
            
            /* Store the updated parameter's name and value for the return value */
            updatedParams.push_back(pair<string, float>(mappings[i]->parameterName, param->value()));
        }
    }
    
    return updatedParams;
}

void ParameterList::handleOsc() {
    
}


void ParameterList::parameterListPrint() {
    
    int cnt = 0;
    printf("\nParameter List:\n---------------\n");
    for (map<string, SynthParameter*>::iterator it = _parameters.begin(); it != _parameters.end(); ++it) {
        printf("#%d: it = %d; ", cnt, (int&)*it);
        printf("  %24s = %f\n", (*it).second->name().c_str(), (*it).second->value());
        cnt++;
    }
    
    printf("\nMIDI Event Listeners:\n---------------------\n");
    for (map<int, vector<MidiMapping*> >::iterator mit = _midiListeners.begin();
         mit != _midiListeners.end(); ++mit) {
        
        int byte1 = ((*mit).first & 0xFF00) >> 8;
        int byte2 = (*mit).first & 0xFF;
        
        printf("Event %x %x: { ", byte1, byte2);
        
        vector<MidiMapping*>& mappings = (*mit).second;
        for (vector<MidiMapping*>::iterator vit = mappings.begin(); vit != mappings.end(); ++vit) {
            
            if (vit != mappings.end()-1)
                printf("%s, ", (*vit)->parameterName.c_str());
            else
                printf("%s }\n", (*vit)->parameterName.c_str());
        }
    }
    
    printf("\nOSC Event Listeners:\n--------------------\n");
    for (map<string, vector<OscMapping> >::iterator mit = _oscListeners.begin();
         mit != _oscListeners.end(); ++mit) {
        
        printf("Event %s: { ", (*mit).first.c_str());
        
        vector<OscMapping>& mappings = (*mit).second;
        for (vector<OscMapping>::iterator vit = mappings.begin(); vit != mappings.end(); ++vit) {
            
            if (vit != mappings.end()-1)
                printf("%s, ", (*vit).parameterName.c_str());
            else
                printf("%s }\n", (*vit).parameterName.c_str());
        }
    }
    printf("\n");
}

        
        
        
        
        
        
        
        
        
        
        
        
        
        
