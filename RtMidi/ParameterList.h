//
//  ParameterList.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 8/5/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRPSynthGUI__ParameterList__
#define __MRPSynthGUI__ParameterList__

#include <iostream>
#include <vector>
#include <map>
#include <math.h>

#include "SynthParameter.h"

//! List of SynthParameter types indexable by the parameter name, or any MIDI messages or OSC paths the parameter responds to. This class should not be instantiated on its own, as the addParameter() method is protected. Classes that contain a list of SynthParameter types should inherit from ParameterList.
/*! Classes that inherit from ParameterList can use the protected member addParameter() to add their SynthParameter types to the parameter list, which makes the parameters accessible by name.
 
    Since a SynthVoice inherits from ParameterList, a user interface can query the SynthVoice for its registered parameters using getParameterNames(). The interface can then allow the user to map MIDI/OSC messages to the parameter using addMidiEventListener() and addOscEventListener().
 
    Any parameters registered as MIDI event listeners with addMidiEventListener() are accesible by the MIDI message (first byte or first two bytes) using getMidiListeners()
 
    Any parameters registered as OSC event listeners with addOscEventListener() are accessible by the OSC path using getOscListeners()
 */

using namespace std;

typedef enum MappingType {
    kMappingTypeAssign = 0,
    kMappingTypeAdd,
    kMappingTypeMultiply
} MappingType;

typedef enum MappingScale {
    kMappingScaleLinear = 0,
    kMappingScaleLogarithmic
} MappingScale;

typedef struct MidiMapping {
    int byte1, byte2;
    string parameterName;
    MappingType type;
    bool ramp;
    float min, max;
    MappingScale scale;
} MidiMapping;

typedef struct OscMapping {
    string path;
    string parameterName;
    MappingType type;
    bool ramp;
} OscMapping;

class ParameterList {
    
    /* Accessible parameters indexed by name (one-to-one) */
    map<string, SynthParameter*> _parameters;
    
    /* Mappings indexed by a MIDI or OSC message. Multiple mappings may respond to the same message */
    map<int, vector<MidiMapping*> > _midiListeners;
    map<string, vector<OscMapping> > _oscListeners;
    
protected:
    
//    public:
    bool addParameter(SynthParameter* param);
    bool removeParameter(string name);
    void clearParameterList();
    void parameterListRamp();
    
public:
    
    /* Get a list of accessible parameter names */
    bool hasParameter(string name);
    vector<string> getParameterNames();
    SynthParameter* getParameterWithName(string name);
    
    /* Adding mappings */
    bool addMidiMapping(MidiMapping *mapping);
    bool addOscMapping(OscMapping mapping);
    
    /* Querying if any parameters in the list respond to a message type */
    bool respondsToMidiEvent(int byte1);
    bool respondsToMidiEvent(int byte1, int byte2);
    bool respondsToOscEvent(string path);
    
    /* Removing mappings by name or by name and message */
    bool removeMidiMapping(string name);
    bool removeMidiMapping(string name, int byte1);
    bool removeMidiMapping(string name, int byte1, int byte2);
    bool removeMidiMapping(MidiMapping *mapping);
    bool removeOscMapping(string name);
    bool removeOscMapping(string name, string path);
    bool removeOscMapping(OscMapping *mapping);
    
    /* Parameter update methods */
    bool setParameterValue(string name, float value, bool doRamp);
    
    /* DONE: Have handleMidi() return a vector<pair<string, float>> with the names and new values of any parameters that were updated with a call to handleMidi() so the UI can be updated accordingly. The new objective C MIDI class can call the synth voice UI to update its sliders, which will have to be key-value associated with the parameter name. If the call to update the slider's value comes from a source other than the actual NSSlider, we can have the method not update the synth parameter since handleMidi() already updates it. */
    
    /* Related note: we can also have the objective C MIDI class forward its incoming messages to the selected mapping item view controller so it can automatically populate the MIDI message parameters with the most recent MIDI control message. */
    vector<pair<string, float> > handleMidi(vector<unsigned char>* message);
    void handleOsc();
    
    /* Display mapped parameters */
    void parameterListPrint();
};

#endif /* defined(__MRPSynthGUI__ParameterList__) */
