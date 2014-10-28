//
//  AudioController.h
//  MRP
//
//  Created by Jeff Gregorio on 6/23/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#ifndef __MRP__AudioController__
#define __MRP__AudioController__

#include <iostream>
#include <vector>
#include <map>
#include <portaudio.h>
#include <assert.h>

#include "PolySynth.h"

#define kAudioController_GlobalAmpRampTime 0.1f
#define kAudioController_AudioBufferSizeFrames 1024

class AudioController {
    
    float _fs;                      // Sample rate
    SynthParameter _globalAmp;      // Global amplitude scalar
    
    PaStream* _stream;                                  // Port audio stream
    bool _streamIsOpen;                                 // Whether the audio output stream is open
    PaStreamParameters _outputStreamParams;             // Output audio stream parameters
    std::vector<const PaDeviceInfo*> _devices;          // Available audio devices
    std::vector<std::pair<const PaDeviceInfo*, PaDeviceIndex> > _outputDevices;    // Available audio output devices and their indices in the available I/O devices list
    
    int _nOutputChannels;                       // Number of output channels available
//    std::map<int, int> _channelVoiceMap;        // 1 to 1 mapping of audio channels to synth voices
    
    PolySynth* _synth;
    
    /* Temp */
    float _theta;
    float _thetaInc;
    
    
#pragma mark - Private Methods
    PaError paSetup();
    void printStreamParameters(PaStreamParameters _params, std::string title);
    
#pragma mark - Portaudio Callback
    /* Portaudio requires a static callback method, so the staticRenderCallback() passes control to the instance-specified renderCallback() */
    int renderCallback(const void* input, void* output,
					   unsigned long frameCount,
					   const PaStreamCallbackTimeInfo* timeInfo,
					   PaStreamCallbackFlags statusFlags);
	static int staticRenderCallback(const void* input, void* output,
									unsigned long frameCount,
									const PaStreamCallbackTimeInfo* timeInfo,
									PaStreamCallbackFlags statusFlags,
									void *userData) {
		return ((AudioController*)userData)
		->renderCallback(input, output, frameCount, timeInfo, statusFlags);
	}
    
#pragma mark - Public Methods
public:
    
    AudioController();
    AudioController(PolySynth* synth);
    ~AudioController();
    
    /* Set a reference to the PolySynth object that handles voice allocation and rendering */
    void setPolySynth(PolySynth* synth);
    
    /* Get/set available audio output devices */
    std::vector<const PaDeviceInfo*> getAvailableOutputDevices();
    bool setOutputDevice(int outputDeviceIdx);
    int getSelectedOutputDeviceIdx() { return _outputStreamParams.device; }
    
    /* Get/set supported sample rates for a specified device */
    std::vector<float> getSupportedOutputSampleRates(int outputDeviceIdx);
    bool setOutputSampleRate(float fs);
    float getOutputSampleRate() { return _fs; }
    
    /* Set the number of output channels to use, or specify a channel->voice map */
    bool setNumOutputChannels(int nChannels);
    bool setChannelVoiceMap(std::map<int, int> map);
//    std::map<int, int> getChannelVoiceMap() { return _channelVoiceMap; }
//    std::map<int, int> getVoiceChannelMap();
    
    /* Methods for opening/closing and starting/stoppping the audio stream */
    bool openStream();              // Always open before starting
    bool closeStream();
    bool startAudioRender();
    bool stopAudioRender();
    
    bool streamIsOpen() { return _streamIsOpen; }
    bool isRendering() { return Pa_IsStreamActive(&_stream); }
    
    void setGlobalAmplitude(float amp, bool doRamp);
};

#endif /* defined(__MRP__AudioController__) */









