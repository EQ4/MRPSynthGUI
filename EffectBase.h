//
//  EffectBase.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 8/6/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRPSynthGUI__EffectBase__
#define __MRPSynthGUI__EffectBase__

#include <iostream>
#include <vector>
#include "SynthParameter.h"

//! Abstract base class for implementing effects.
/*! EffectBase defines the protocol that all effects should follow. Effects should implement a method that returns pointers to any SynthParameter types that should be added to the SynthVoice's parameter list. Parameters returned by this method will be accessible by name and can be registered to receive updates via MIDI and OSC.
 
    Effects should also contain a process() method that applies the effect to a monaural, non-interleaved buffer of audio samples. The int return value on the process() method can be used to indicate a change in effect state (example: an ADSR envelope can return 1 while the note is in the attack, decay, sustain, or release states, but return 0 when release is finished to tell the synth voice to stop rendering).
 */
class EffectBase {
    
protected:
    
    EffectBase(const std::string name) : _name(name) {}
    std::string _name;
    
public:
    
    std::string name() { return _name; }
    
    virtual std::vector<SynthParameter*> getParameters() = 0;
    virtual int process(float *procBuffer, int nFrames) = 0;
};

#endif /* defined(__MRPSynthGUI__EffectBase__) */
