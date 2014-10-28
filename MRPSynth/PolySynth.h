//
//  PolySynth.h
//  MRP
//
//  Created by App Lab on 7/30/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRP__PolySynth__
#define __MRP__PolySynth__

#include <iostream>
#include <vector>
#include <set>

//#include "MidiController.h"
#include "SynthVoice.h"
#include "AdditiveSynthVoice.h"
#include "SubtractiveSynthVoice.h"

/* To Do: Poly synth should handle incoming MIDI messages in a raw format.
 
    Use a Note event struct and a Note map indexed by the MIDI note number
 
    typedef struct Note {
        SynthVoice* voice;
        int priority;
        int audioChannel;
    }
 
    std::map<int, Note> _noteMap;
 
    NOTE_ON messages generate Note structs by memcpy-ing the master voice, setting the fundamental frequency, setting the priority, and assigning an audio channel. For generality's sake, we don't store the note map by audio channel because we want to allow multiple voices to render to the same channel or to different channels.
 
    Idea: keep a list of master synth voices, with a note range assigned to each. When a NOTE_ON event happens, have instance voices copy parameters from the master voice that covers the note number. We could possibly keep a map of MIDI note numbers to voice pointers so we can just update the original pointer's value.
 
    NOTE_OFF messages call _noteMap[midiNum].voice->beginRelease()
 
    The PolySynth::render() method calls note.voice->render() for every note with priority > 0. The voice's render method returns a continue flag (inverse return value of the voice's _adsr.ramp() method, which returns true if the note has been fully released). The Note's priority is multiplied by the continue flag (keep priority or zero out). Ended notes do not render and are of highest priority for replacement.
 
    MIDI Control (global) messages should query the master voice's ParameterList for associated listeners and update the listeners for the master voice, followed by iterating through the _noteMap and updating listeners for all instance voices.
 
    MIDI Control (poly) should query the _noteMap[midiNote].voice's ParameterList and update any listeners
 
 */

//! Abstract Base Class for Polyphonic Synthesis
/*!
 
*/
class PolySynth {
    
protected:
    
    float _fs;          // Sampling rate
    
    /* Master (non-sounding) voice. Sounding voices copy parameters from the master voice when noteOn() is called */
    SynthVoice *_masterVoice;
    
    typedef struct Voice {
        SynthVoice *v;          // Actual synth voice
        int midiNum;            // MIDI note number (-1) if unused
        int priority;           // Inverse priority for replacement (0 if unused)
    } Voice;
    
    int _nVoices;               // Number of synth voices (polyphony), or audio channels in the case of the MRP
    int _nActiveVoices;         // Number of voices currently rendering
    std::vector<Voice> _voices; // Array of voice info structs (one for each audio channel)
    std::set<int> _activeKeys;  // MIDI note numbers of keys currently held
    int _noteCount;             // Number of noteOn() events since object instantiation
    
    inline float midiNoteToFreq(int midiNote) { return powf(2.0f, (midiNote-69.0f)/12.0f) * 440.0f; }

public:
    
#pragma mark - Constructors
    PolySynth();
    PolySynth(SynthVoice *master, int numVoices);
    
#pragma mark - Setters
    /* TODO: generalize setting master voices derived from SynthVoice (virtual constructor?).
     Perhaps a virtual SynthVoice::clone() method that can be overridden by derived classes */
    void setMasterVoice(SynthVoice *master);
    void setMasterVoice(AdditiveSynthVoice *master);
    void setMasterVoice(SubtractiveSynthVoice *master);
    
    bool setMasterVoiceParam(string paramName, float value, bool doRamp);
    bool addMasterVoiceMidiMapping(MidiMapping *map);
    bool removeMasterVoiceMidiMapping(MidiMapping *map);
    void setSampleRate(float fs);
    void setNumVoices(int num);
    
#pragma mark - Getters
    SynthVoice* masterVoice() { return _masterVoice; }
    int sampleRate() { return _fs; }
    int numVoices() { return _nVoices; }
    bool isSoundingMidiNote(int midiNum);
    
#pragma mark - Event Handlers
    /* Note: PolySynth::noteOn() and PolySynth::noteOff() do the work of enabling synth voices, setting envelope states, and setting voice frequencies from the MIDI note nubmers. Any derived class that overrides these methods should directly call PolySynth::noteOn() and PolySynth::noteOff() and do additional work before or after these calls
    */
    virtual int noteOn(int midiNum, int midiVel);       // Returns index of allocated voice or -1 on failure
    virtual int noteOff(int midiNum);                   // Returns index of deallocated voice or -1 on failure
    vector<pair<string, float> > handleMidiControl(vector<unsigned char>* message);
    
    /* Call the SynthVoice render methods to render a single sample for any Note events with priority > 0 */
    virtual float renderSample(int channel);
    
#pragma mark - Debug
    void printAllVoiceParams();
};

#endif /* defined(__MRP__PolySynth__) */
