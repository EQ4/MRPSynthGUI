//
//  AudioController.cpp
//  MRP
//
//  Created by Jeff Gregorio on 6/23/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#include "AudioController.h"

AudioController::AudioController() : _synth(nullptr), _nOutputChannels(0), _fs(44100.0f), _streamIsOpen(false) {
    
    _globalAmp = SynthParameter("Global Amplitude", _fs, 1.0f, kAudioController_GlobalAmpRampTime);
    
    /* Initialize portaudio, get available devices, and initialize output stream info */
    paSetup();
}

AudioController::AudioController(PolySynth* synth) : _synth(synth), _nOutputChannels(0), _fs(44100.0f), _streamIsOpen(false) {
    
    _globalAmp = SynthParameter("Global Amplitude", _fs, 1.0f, kAudioController_GlobalAmpRampTime);
    
    /* Initialize portaudio, get available devices, and initialize output stream info */
    paSetup();
}

AudioController::~AudioController() {
    
    PaError error = paNoError;
    
    if (_streamIsOpen)
        Pa_AbortStream(_stream);
    
    error = Pa_Terminate();
    if (error != paNoError)
        printf("%s: PaError = %s\n", __PRETTY_FUNCTION__, Pa_GetErrorText(error));
}

#pragma mark - Private Methods
PaError AudioController::paSetup() {
    
    PaError error = paNoError;
    
    /* --------------------------- */
    /* === Initialze portaudio === */
    /* --------------------------- */
    error = Pa_Initialize();
    if (error != paNoError) {
        printf("%s: PaError = %s\n", __PRETTY_FUNCTION__, Pa_GetErrorText(error));
        return error;
    }
    
    /* ------------------------------------------ */
    /* === Initialze Output Stream Parameters === */
    /* ------------------------------------------ */
    _outputStreamParams.channelCount = 0;
    _outputStreamParams.device = paNoDevice;
    _outputStreamParams.hostApiSpecificStreamInfo = NULL;
    _outputStreamParams.sampleFormat = paFloat32; // | paNonInterleaved;
    _outputStreamParams.suggestedLatency = NULL;
    
    /* ------------------------------------------ */
    /* === Get available audio output devices === */
    /* ------------------------------------------ */
    int numDevs = Pa_GetDeviceCount();
    if (numDevs == 0)
        return paNoDevice;
    
    for (int i = 0; i < numDevs; i++) {
        _devices.push_back(Pa_GetDeviceInfo(i));
        if (_devices[i]->maxOutputChannels > 0)
            _outputDevices.push_back(std::pair<const PaDeviceInfo*, int>(_devices[i], i));
    }
    
    _theta = 0.0f;
    _thetaInc = 2*M_PI * 440.0f / _fs;
    
    return error;
}

#pragma mark - Portaudio Callback
int AudioController::renderCallback(const void* input, void* output,
                                    unsigned long frameCount,
                                    const PaStreamCallbackTimeInfo* timeInfo,
                                    PaStreamCallbackFlags statusFlags) {
    if (!_synth)
        return 1;
    
    /* Typecast and initialize the output to zeros */
    float* out = (float*)output;
    bzero(out, frameCount * _nOutputChannels * sizeof(float));
    
    /* Call the PolySynth to render single samples from the voice assigned to each channel */
    for (int i = 0; i < frameCount; i++) {
        for (int ch = 0; ch < _nOutputChannels; ch++) {
            *out++ = _globalAmp * _synth->renderSample(ch);
        }
    }
    
    return 0;
    
#pragma mark Fix Me: Non-interleaved audio
    
//    printf("frameCount = %lu\n", frameCount);
//    
//    /* Using _outputStreamParams.sampleFormat = paNonInterleaved makes void* output a list of pointers to separate non-interleaved channel buffers */
//    float** channelPtr = (float**)output;
//
//    /* For each output channel, call the PolySynth's render() method on each voice mapped to that channel */
//    for (int ch = 0; ch < _nOutputChannels; ch++) {
//    
//        float* chOut = (float*)channelPtr[ch];
//        memset(chOut, 0.0f, frameCount * sizeof(float));    // Initialize to zero
//    
//        /* Render from the synth voice mapped to this channel */
//        _synth->render(ch, chOut, (int)frameCount);
//    }
//
//    return 0;
}

#pragma mark - Interface Methods
/* Set a reference to the PolySynth object that handles voice allocation and rendering */
void AudioController::setPolySynth(PolySynth *synth) {
    
    _synth = synth;
    _synth->setSampleRate(_fs);
}

/* Return a list of devices that support output */
std::vector<const PaDeviceInfo*> AudioController::getAvailableOutputDevices() {
    
    std::vector<const PaDeviceInfo*> devices;
    
    for (int i = 0; i < _outputDevices.size(); i++) {
        
        /* Add it to the return list if the device supports output */
        if (_outputDevices[i].first->maxOutputChannels > 0)
            devices.push_back(_outputDevices[i].first);
    }
    
    return devices;
}

/* Set a device to use for output. Return true on success, false otherwise */
bool AudioController::setOutputDevice(int outputDeviceIdx) {
    
    /* Make sure the device index is valid */
    if (outputDeviceIdx < 0 || outputDeviceIdx >= _outputDevices.size()) {
        printf("%s: Invalid output device index. %lu available output devices\n", __PRETTY_FUNCTION__, _outputDevices.size());
        return false;
    }
    
    /* Make sure the chosen device supports output */
    if (_outputDevices[outputDeviceIdx].first->maxOutputChannels <= 0) {
        printf("%s: Device #%d (%s) doesn't support output\n", __PRETTY_FUNCTION__, outputDeviceIdx, _outputDevices[outputDeviceIdx].first->name);
        return false;
    }
    
    /* The index of the device in the output devices list may not be the same as the index in the devices list, so set the device using its actual PaDeviceIndex (second in std::pair<const PaDeviceInfo*, PaDeviceIndex>) */
    _outputStreamParams.device = _outputDevices[outputDeviceIdx].second;
    _outputStreamParams.suggestedLatency = _devices[_outputStreamParams.device]->defaultLowOutputLatency;
    
    _fs = _devices[_outputStreamParams.device]->defaultSampleRate;
    _globalAmp.setSampleRate(_fs);
    
    if (_synth) {
        _synth->setSampleRate(_fs);
    }
    
    printf("%s: Using output device %s\n", __PRETTY_FUNCTION__, _devices[_outputStreamParams.device]->name);
        
    return true;
}

std::vector<float> AudioController::getSupportedOutputSampleRates(int outputDeviceIdx) {
    
    std::vector<float> rates;
    
    /* Make sure the device index is valid */
    if (outputDeviceIdx < 0 || outputDeviceIdx >= _outputDevices.size()) {
        printf("%s: Invalid output device index. %lu available output devices\n", __PRETTY_FUNCTION__, _outputDevices.size());
        return rates;
    }
    
    /* Standard sample rates to test */
    static double testRates[] = {8000.0, 9600.0, 11025.0, 12000.0, 16000.0, 22050.0, 24000.0, 32000.0, 44100.0, 48000.0, 88200.0, 96000.0, 192000.0, -1.0};      // Negative-terminated array
    
    /* Test output stream parameters */
    PaStreamParameters params;
    params.device = _outputDevices[outputDeviceIdx].second;
    params.channelCount = _devices[params.device]->maxOutputChannels;
    params.sampleFormat = paFloat32;
    params.suggestedLatency = 0;    // Ignored by Pa_IsFormatSupported
    params.hostApiSpecificStreamInfo = NULL;
    
    /* Add the test rate to the output vector if supported */
    for (int i = 0; testRates[i] > 0; i++) {
        if (Pa_IsFormatSupported(NULL, &params, testRates[i]) == paFormatIsSupported)
            rates.push_back(testRates[i]);
    }
    
    return rates;
}

bool AudioController::setOutputSampleRate(float fs) {
    
    /* Make sure we've already specified an output device to use */
    if (_outputStreamParams.device == paNoDevice) {
        printf("%s: Set an output device before specifying number of output channels\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    /* Make sure the device supports the requested sample rate */
    PaStreamParameters params;  // Test output stream parameters
    params.device = _outputStreamParams.device;
    params.channelCount = _devices[_outputStreamParams.device]->maxOutputChannels;
    params.sampleFormat = paFloat32;
    params.suggestedLatency = 0;    // Ignored by Pa_IsFormatSupported
    params.hostApiSpecificStreamInfo = NULL;
    
    if (Pa_IsFormatSupported(NULL, &params, fs) != paFormatIsSupported) {
        printf("%s: Device %s does not support sample rate %.0f\n", __PRETTY_FUNCTION__, _devices[_outputStreamParams.device]->name, fs);
        return false;
    }
    
    _fs = fs;
    _globalAmp.setSampleRate(_fs);
    
    if (_synth) {
        _synth->setSampleRate(_fs);
    }
    
    return true;
}

bool AudioController::setNumOutputChannels(int nChannels) {
    
    /* Make sure we've already specified an output device to use */
    if (_outputStreamParams.device == paNoDevice) {
        printf("%s: Set an output device before specifying number of output channels\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    /* Make sure we've set a PolySynth */
    if (!_synth) {
        printf("%s: Use setPolySynth() before setting the channel-voice map\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    if (nChannels == _nOutputChannels)
        return true;
    
    _nOutputChannels = nChannels;
    _outputStreamParams.channelCount = _nOutputChannels;
    _synth->setNumVoices(nChannels);
    
    bool wasActive = false;
    if (_streamIsOpen) {
        wasActive = Pa_IsStreamActive(_stream);
        closeStream();
    }
    
    openStream();
    
    if (wasActive)
        startAudioRender();
    
//    /* Set the default channel->voice map so that each voice renders on its associated channel index */
//    for (int i = 0; i < _nOutputChannels; i++) {
//        _channelVoiceMap[i] = i;
//    }
    
    return true;
}

//bool AudioController::setChannelVoiceMap(std::map<int, int> map) {
//    
//    /* Make sure we've already specified an output device to use */
//    if (_outputStreamParams.device == paNoDevice) {
//        printf("%s: Set an output device before specifying number of output channels\n", __PRETTY_FUNCTION__);
//        return false;
//    }
//    
//    /* Make sure we've set a PolySynth */
//    if (!_synth) {
//        printf("%s: Use setPolySynth() before setting the channel-voice map\n", __PRETTY_FUNCTION__);
//        return false;
//    }
//    
//    /* Test for valid channel and voice indices */
//    /* Keep track of channels and voices already set to enforce a one-to-one mapping */
//    std::vector<int> channels;
//    std::vector<int> voices;
//    for (std::map<int, int>::iterator it = map.begin(); it != map.end(); ++it) {
//        
//        int ch = (*it).first;   // Channel
//        int vc = (*it).second;  // Voice
//        
//        /* Make sure the channel is valid and the current output device supports output on this channel */
//        if (ch < 0 || ch >= _devices[_outputStreamParams.device]->maxOutputChannels) {
//            printf("%s: Invalid channel index %d. ", __PRETTY_FUNCTION__, ch);
//            printf("Device %s supports maximum %d output channels\n", _devices[_outputStreamParams.device]->name, _devices[_outputStreamParams.device]->maxOutputChannels);
//            return false;
//        }
//        
//        /* Make sure the voice index is valid */
//        if (vc < 0 || vc >= _synth->getNumVoices()) {
//            printf("%s: Invalid voice index %d. Current synth has %d voices.\n", __PRETTY_FUNCTION__, vc, _synth->getNumVoices());
//            return false;
//        }
//        
//        /* Make sure the channel index isn't a duplicate */
//        if (std::find(channels.begin(), channels.end(), ch) != channels.end()) {
//            printf("%s: Duplicate channel index entry %d. Mapping must be one-to-one\n", __PRETTY_FUNCTION__, ch);
//            return false;
//        }
//        
//        /* Make sure the voice index isn't a duplicate */
//        if (std::find(voices.begin(), voices.end(), vc) != voices.end()) {
//            printf("%s: Duplicate voice index entry %d. Mapping must be one-to-one\n", __PRETTY_FUNCTION__, vc);
//            return false;
//        }
//        
//        channels.push_back(ch);
//        voices.push_back(vc);
//    }
//    
//    _channelVoiceMap.clear();
//    _channelVoiceMap = map;
//    _nOutputChannels = (int)_channelVoiceMap.size();
//    _outputStreamParams.channelCount = _nOutputChannels;
//    _synth->setNumVoices(_nOutputChannels);
//    
//    return true;
//}

///* Get alternate representation where channels are accessed by the voice index */
//std::map<int, int> AudioController::getVoiceChannelMap() {
//    
//    std::map<int, int> vcMap;
//    
//    for (std::map<int, int>::iterator it = _channelVoiceMap.begin(); it != _channelVoiceMap.end(); ++it) {
//        int ch = (*it).first;   // Channel
//        int vc = (*it).second;  // Voice
//        vcMap[vc] = ch;
//    }
//    
//    return vcMap;
//}

bool AudioController::openStream() {
    
    /* Make sure we've already specified an output device to use */
    if (_outputStreamParams.device == paNoDevice) {
        printf("%s: Set an output device before specifying number of output channels\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    /* Make sure we have output channels allocated */
    if (_outputStreamParams.channelCount <= 0) {
        printf("%s: No output channels specified\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    printStreamParameters(_outputStreamParams, "\n== Opening output stream with parameters:");
    
    /* Open the stream, passing the static render callback method and output stream parameters */
    PaError error = Pa_OpenStream(&_stream,
                                  NULL,
                                  &_outputStreamParams,
                                  (double)_fs,
                                  kAudioController_AudioBufferSizeFrames,
                                  paNoFlag,
                                  AudioController::staticRenderCallback,
                                  this);
    if (error != paNoError) {
        printf("%s: PaError = %s\n", __PRETTY_FUNCTION__, Pa_GetErrorText(error));
        return false;
    }
    
    _streamIsOpen = true;
    
    return true;
}

bool AudioController::closeStream() {
    
    if (!_streamIsOpen) {
        printf("%s: Portaudio stream is not open\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    PaError error = Pa_CloseStream(_stream);
    if (error != paNoError) {
        printf("%s: PaError = %s\n", __PRETTY_FUNCTION__, Pa_GetErrorText(error));
        return false;
    }
    
    _streamIsOpen = false;
    
    return true;
}

bool AudioController::startAudioRender() {
    
    if (!_streamIsOpen) {
        printf("%s: Portaudio stream is not open. Use AudioController::openStream() first\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    PaError error = Pa_StartStream(_stream);
    if (error != paNoError) {
        printf("%s: PaError = %s\n", __PRETTY_FUNCTION__, Pa_GetErrorText(error));
        return false;
    }
    
    return true;
}

bool AudioController::stopAudioRender() {
    
    if (!Pa_IsStreamActive(_stream)) {
        printf("%s: Portaudio stream is not active\n", __PRETTY_FUNCTION__);
        return false;
    }
    
    PaError error = Pa_StopStream(_stream);
    if (error != paNoError) {
        printf("%s: PaError = %s\n", __PRETTY_FUNCTION__, Pa_GetErrorText(error));
        return false;
    }
    
    return true;
}

void AudioController::setGlobalAmplitude(float amp, bool doRamp) {
    
    if (doRamp)
        _globalAmp.setValue(amp);
    else
        _globalAmp = amp;
}

void AudioController::printStreamParameters(PaStreamParameters _params, std::string title) {
    
    printf("%s\n", title.c_str());
    for (int i = 0; i < title.size(); i++)
        printf("-");
    printf("\n");
    printf("device           = %s\n", _devices[_outputStreamParams.device]->name);
    printf("channelCount     = %d\n", _outputStreamParams.channelCount);
    printf("sampleFormat     = ");
    
    bool flagTrue = false;
    bool lastTrue = false;
    flagTrue = _outputStreamParams.sampleFormat & paFloat32;
    printf("%s", flagTrue ? "paFloat32" : "\b\b\b ");
    lastTrue = flagTrue;
    flagTrue = _outputStreamParams.sampleFormat & paInt32;
    printf("%s%s", lastTrue ? " | " : "", flagTrue ? "paInt32" : "");
    lastTrue = flagTrue;
    flagTrue = _outputStreamParams.sampleFormat & paInt24;
    printf("%s%s", lastTrue ? " | " : "", flagTrue ? "paInt24" : "");
    lastTrue = flagTrue;
    flagTrue = _outputStreamParams.sampleFormat & paInt16;
    printf("%s%s", lastTrue ? " | " : "", flagTrue ? "paInt16" : "");
    lastTrue = flagTrue;
    flagTrue = _outputStreamParams.sampleFormat & paInt8;
    printf("%s%s", lastTrue ? " | " : "", flagTrue ? "paInt8" : "");
    lastTrue = flagTrue;
    flagTrue = _outputStreamParams.sampleFormat & paCustomFormat;
    printf("%s%s", lastTrue ? " | " : "", flagTrue ? "paCustomFormat" : "");
    lastTrue = flagTrue;
    flagTrue = _outputStreamParams.sampleFormat & paNonInterleaved;
    printf("%s%s", lastTrue ? " | " : "", flagTrue ? "paNonInterleaved" : "");
    printf("\n");
    
    printf("suggestedLatency = %f\n", _outputStreamParams.suggestedLatency);
    printf("bufferSizeFrames = %d\n", kAudioController_AudioBufferSizeFrames);
    for (int i = 0; i < title.size(); i++)
        printf("-");
    printf("\n");
}







