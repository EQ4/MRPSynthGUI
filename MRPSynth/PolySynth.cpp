//
//  PolySynth.cpp
//  MRP
//
//  Created by App Lab on 7/30/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "PolySynth.h"

PolySynth::PolySynth() : _masterVoice(NULL), _fs(44100.0f), _nVoices(0), _nActiveVoices(0), _noteCount(1) { }

PolySynth::PolySynth(SynthVoice* master, int numVoices) : _masterVoice(master), _fs(master->sampleRate()), _nVoices(numVoices), _nActiveVoices(0), _noteCount(1) {
    
    setMasterVoice(master);
}

void PolySynth::setMasterVoice(SynthVoice* master) {
    
    printf("%s: Setting master voice with parameters:", __PRETTY_FUNCTION__);
    master->parameterListPrint();
    
    _masterVoice = master;
    
    _voices.clear();
    
    /* Create the Note structs. Voices clone the master voice */
    for (int vc = 0; vc < _nVoices; vc++) {
        
        printf("%s: Creating instance voice %d from master voice\n", __PRETTY_FUNCTION__, vc);
       
        Voice note;
        note.v = new SynthVoice(_masterVoice);
        
        note.midiNum = -1;
        note.priority = 0;
        _voices.push_back(note);
    }
}

void PolySynth::setMasterVoice(AdditiveSynthVoice* master) {
    
    printf("%s: Setting master voice with parameters:", __PRETTY_FUNCTION__);
    
    _masterVoice = master;
    _masterVoice->parameterListPrint();
    
    _voices.clear();
    
    /* Create the Note structs. Voices clone the master voice */
    for (int vc = 0; vc < _nVoices; vc++) {
        
        printf("%s: Creating instance voice %d from master voice\n", __PRETTY_FUNCTION__, vc);
        
        Voice note;
        note.v = new AdditiveSynthVoice(master);
        
        note.midiNum = -1;
        note.priority = 0;
        _voices.push_back(note);
    }
}

void PolySynth::setMasterVoice(SubtractiveSynthVoice* master) {
    
    printf("%s: Setting master voice with parameters:", __PRETTY_FUNCTION__);
    
    _masterVoice = master;
    _masterVoice->parameterListPrint();
    
    _voices.clear();
    
    /* Create the Note structs. Voices clone the master voice */
    for (int vc = 0; vc < _nVoices; vc++) {
        
        printf("%s: Creating instance voice %d from master voice\n", __PRETTY_FUNCTION__, vc);
        
        Voice note;
        note.v = new SubtractiveSynthVoice(master);
        
        note.midiNum = -1;
        note.priority = 0;
        _voices.push_back(note);
    }
}

bool PolySynth::setMasterVoiceParam(string paramName, float value, bool doRamp) {
    
    /* Make sure the parameter exists */
    if (!_masterVoice->hasParameter(paramName)) {
        printf("%s: No parameter with name %s\n", __PRETTY_FUNCTION__, paramName.c_str());
        return false;
    }
    
    /* Set the value for the master voice and all channel voices */
    _masterVoice->setParameterValue(paramName, value, doRamp);
    for (int i = 0; i < _nVoices; i++) {
        _voices[i].v->setParameterValue(paramName, value, doRamp);
    }
    
    return true;
}

bool PolySynth::addMasterVoiceMidiMapping(MidiMapping *map) {
    
    /* Set the value for the master voice and all channel voices */
//    _masterVoice->setParameterValue(paramName, value, doRamp);
    _masterVoice->addMidiMapping(map);
    for (int i = 0; i < _nVoices; i++) {
        _voices[i].v->addMidiMapping(map);
    }
    
    return true;
}

bool PolySynth::removeMasterVoiceMidiMapping(MidiMapping *map) {
    
    _masterVoice->removeMidiMapping(map);
    for (int i = 0; i < _nVoices; i++) {
        _voices[i].v->removeMidiMapping(map);
    }
    
    return true;
}

void PolySynth::setSampleRate(float fs) {
    
    _fs = fs;
    for (int vc = 0; vc < _nVoices; vc++) {
        if (_voices[vc].v)
            _voices[vc].v->setSampleRate(_fs);
    }
}

void PolySynth::setNumVoices(int num) {
    
    _nVoices = num;
    
    if (_masterVoice)
        setMasterVoice(_masterVoice);
}

bool PolySynth::isSoundingMidiNote(int midiNum) {
    
    bool found = false;
    for (int vc = 0; vc < _nVoices; vc++) {
        if (_voices[vc].midiNum == midiNum && _voices[vc].priority > 0)
            found = true;
    }
    return found;
}

/* MIDI Note On handler method. Returns the index of the allocated channel so the MIDI handler can send the MRP routing message. */
int PolySynth::noteOn(int midiNum, int midiVel) {
    
    int vc, val;        // Loop variables
    bool stop = false;  // Loop control flag
    int idx = -1;       // Voice index to allocate
    
    if (midiNum < 0 || midiNum > 127)
        return idx;
    if (midiVel < 1 || midiVel > 127)
        return idx;
    
    _activeKeys.insert(midiNum);
    _noteCount++;
    
    /* Find the first free channel or lowest priority note to terminate */
    val = _noteCount;
    for (vc = 0; vc < _nVoices && !stop; vc++) {
        
        /* If we have an instance of the same MIDI note currently releasing, retrigger on the same voice */
        if (_voices[vc].midiNum == midiNum) {
            idx = vc;
            stop = true;
        }
        
        /* If we have a free channel, use it */
        else if (_voices[vc].priority == 0) {
            idx = vc;
            stop = true;
        }
        
        /* Otherwise, take the voice with the lowest priority */
        else if (_voices[vc].priority < val) {
            val = _voices[vc].priority;
            idx = vc;
        }
    }
    
    /* If we found a free voice or freed one and we have a note number in the piano range */
    if (idx != -1 && midiNum >= 21 && midiNum <= 108) {
        
        /* TODO: Send to pitch bend handler from here. Recording the note on time may be necessary for pitch bending. Possibly include a timestamp as an input argument to noteOn() and noteOff() */
        
        /* Set the note event priority */
        _voices[idx].priority = _noteCount;
        _voices[idx].midiNum = midiNum;
        
        /* Set the fundamental and start the ADSR envelope */
        _voices[idx].v->setF0(midiNoteToFreq(midiNum), false);
        _voices[idx].v->beginAttack();
        
        /* TODO: separate list of MIDI velocity listeners */
        _voices[idx].v->setVelocityAmplitude((float)midiVel/127.0f, false);
        
        printf("--- MIDI Note %d set to render on channel %d\n", midiNum, idx);
        printf("------ f0 = %f\n", _voices[idx].v->f0());
        _nActiveVoices++;
    }
    else
        printf("%s: Error allocating synth voice for MIDI Note %d\n", __PRETTY_FUNCTION__, midiNum);
    
    return idx;
}

/* MIDI Note off handler method */
int PolySynth::noteOff(int midiNum) {
    
    int vc = 0;
    bool stop = false;
    int idx = -1;       // Voice index to deallocate
    
    if (midiNum < 0 || midiNum > 127)
        return idx;
    
    _activeKeys.erase(midiNum);
    
    /* Find this note number in the voices list */
    while (!stop && vc < _nVoices) {
        
        if (midiNum == _voices[vc].midiNum) {
            
            _voices[vc].v->beginRelease();  // Start release envelope
            idx = vc;
            stop = true;
            
            _voices[vc].priority -= _activeKeys.size();     // Reduce priority of released notes
            printf("--- MIDI Note %d set to release on channel %d\n", midiNum, idx);
        }
        vc++;
    }
    
    /* If we have more or as many keys held as synth voices, then use the freed voice to re-trigger the currently held (non-sounding) voice with the highest priority */
    if (_activeKeys.size() >= _nVoices) {
        
        bool skip;
        
        /* For each currently active note (note: these are in order of arrival) */
        for (set<int>::iterator it = _activeKeys.begin(); it != _activeKeys.end(); it++) {
            
            skip = false;
            
            /* Skip this help note if it has a voice */
            for (int i = 0; i < _nVoices; i++) {
                if (_voices[i].midiNum == *it)
                    skip = true;
            }
            
            /* Otherwise, re-trigger the held note */
            if (!skip)
                noteOn(*it, 100);
        }
    }
    
    return idx;
}

vector<pair<string, float> > PolySynth::handleMidiControl(vector<unsigned char>* message) {
    
    /* Return a vector of parameter names and values for any parameters that were updated, allowing the Objective C MIDI handler to update UI elements */
    vector<pair<string, float> > updatedParams;
    
    /* Set the value for the master voice and all channel voices */
    updatedParams = _masterVoice->handleMidi(message);
    for (int i = 0; i < _nVoices; i++) {
       _voices[i].v->handleMidi(message);
    }
    
    return updatedParams;
}

float PolySynth::renderSample(int channel) {
    
    /* Make sure the channel/voice index is valid */
    if (channel < 0 || channel >= _nVoices)
        return 0.0f;
    
    /* Make sure we have a voice for this channel */
    if (!_voices[channel].v)
        return 0.0f;
    
    /* Only call the voices' render methods if their priority is > 0 */
    if (_voices[channel].priority <= 0)
        return 0.0f;
    
    /* Render a single sample from the synth voice assigned to this channel.  SynthVoice::renderSample() returns 1 if the note is to continue, and 0 if the note has released. The return value modifies the voice's priority. */
    float sample = 0.0f;
    _voices[channel].priority *= _voices[channel].v->renderSample(&sample);
    
    /* If we've deactivated this voice */
    if (_voices[channel].priority == 0) {
        printf("--- MIDI Note %d on channel %d has ended\n", _voices[channel].midiNum, channel);
        _voices[channel].midiNum = -1;
    }
    
    return sample;
}

void PolySynth::printAllVoiceParams() {
    
    printf("\n===============\n Master Voice:\n===============\n");
    _masterVoice->parameterListPrint();
    
    for (int i = 0; i < _voices.size(); i++) {
        printf("Instance Voice #%2d\n------------------\n", i);
        _voices[i].v->parameterListPrint();
    }
}











