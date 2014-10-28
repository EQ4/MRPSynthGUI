//
//  IOViewController.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 9/11/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "IOViewController.h"

@interface IOViewController ()

@end

@implementation IOViewController

@synthesize audioOutputDeviceSelector;

- (id)initWithNibName:(NSString *)nibNameOrNil
               bundle:(NSBundle *)nibBundleOrNil
                audio:(AudioController *)aCont
                 midi:(MIDIHandler *)mHandler {
    
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {

        audioController = aCont;
        midiHandler = mHandler;
        
        dispatch_async(dispatch_get_main_queue(), ^{
            [self audioSetup];
            [self setAudioOutputDevice:self];
            [self midiSetup];
            [self touchkeySetup];
        });
    }
    return self;
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    
    /* Make sure we properly close the touchkey device and any open log file */
    if (touchkeyController != NULL) {
        
        touchkeyController->closeDevice();
        
        if (touchkeyController->isLogging())
            touchkeyController->closeLogFile();
    }
}

- (void)audioSetup {
    
    /* Query devices */
    std::vector<const PaDeviceInfo*> audioDevs = audioController->getAvailableOutputDevices();

    /* Populate the output device list */
    for (int i = 0; i < audioDevs.size(); i++) {
        NSString* devName = [NSString stringWithFormat:@"%s", audioDevs[i]->name];
        [audioOutputDeviceSelector addItemWithTitle:devName];
        if ([devName isEqualToString:kDefaultAudioOutputDeviceName])
            [audioOutputDeviceSelector selectItemAtIndex:i];
    }
}

- (void)touchkeySetup {
    
    // TEMP:
    touchkeyCalibrationDirectoryPath = @"/Users/Jeff/Desktop/MRP/Calibration";
    touchkeyLogDirectoryPath = @"/Users/Jeff/Desktop/MRP/Log";
    
    keyboardController = new PianoKeyboard();
    touchkeyController = new TouchkeyDevice(*keyboardController);
    touchkeyController->setVerboseLevel(2);
    [self setAvailableTouchkeyDevices];
    
    /* Set callbacks */
    touchkeyController->setCentroidCallback(touchkeyCentroidCallback, (__bridge void*) self);
    touchkeyController->setAnalogCallback(touchkeyAnalogCallback, (__bridge void*) self);
}

- (void)midiSetup {
    
    /* Query input devices */
//    std::vector<std::string> midiDevs = midiController->getInputDeviceNames();
    std::vector<std::string> midiDevs = [midiHandler getInputDeviceNames];

    /* Populate the input devices list */
    [midiInputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"None"]];
    for (int i = 0; i < midiDevs.size(); i++)
        [midiInputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"%s", midiDevs[i].c_str()]];
    
    
    /* Select the first item on the list if one exists */
    if ([midiInputDeviceSelector numberOfItems] > 1)
        [midiInputDeviceSelector selectItemAtIndex:1];

    
    /* Open the selected device */
    [self setMIDIInputDevice:self];
    
    /* Query output devices */
//    midiDevs = midiController->getOutputDeviceNames();
    midiDevs = [midiHandler getOutputDeviceNames];
    
    /* Populate the output devices list */
    [midiOutputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"None"]];
    for (int i = 0; i < midiDevs.size(); i++)
        [midiOutputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"%s", midiDevs[i].c_str()]];
    
    /* No output device by default */
    [midiOutputDeviceSelector selectItemWithTitle:@"None"];

    /* Rescan for new devices on a 5-second timer */
    [NSTimer scheduledTimerWithTimeInterval:5.0
                                     target:self
                                   selector:@selector(rescanMIDIDevices)
                                   userInfo:nil
                                    repeats:true];
}

#pragma mark - Audio Parameter Events
- (IBAction)setAudioOutputDevice:(id)sender {
    
//    if ([NSThread isMainThread])
//        NSLog(@"eff  = %d",(int)[audioOutputDeviceSelector indexOfSelectedItem]);
//    else dispatch_sync(dispatch_get_main_queue(), ^{
//        NSLog(@"eff = %d",(int)[audioOutputDeviceSelector indexOfSelectedItem]);
//    });
    
    int devIdx = (int)[audioOutputDeviceSelector indexOfSelectedItem];

    if (!audioController->setOutputDevice(devIdx)) {
        [audioOutputNumChannelsSelector setEnabled:false];
        [audioOutputSampleRateSelector setEnabled:false];
        [audioStartStopButton setEnabled:false];
    }

    std::vector<const PaDeviceInfo*> devices = audioController->getAvailableOutputDevices();

    /* Populate the number of output channels list */
    int maxNumChannels = devices[devIdx]->maxOutputChannels;
    for (int i = 1; i <= maxNumChannels; i++) {
        [audioOutputNumChannelsSelector addItemWithTitle:[NSString stringWithFormat:@"%d", i]];
        
        if (i == kDefaultAudioOutputNumChannels) {
            [audioOutputNumChannelsSelector selectItemAtIndex:i-1];
        }
    }

    /* Set the number of output channels to the default */
    [self setAudioOutputNumChannels:self];

    /* Popuate sample rate list */
    std::vector<float> sampleRates = audioController->getSupportedOutputSampleRates(devIdx);
    for (int i = 0; i < sampleRates.size(); i++) {
        [audioOutputSampleRateSelector addItemWithTitle:[NSString stringWithFormat:@"%.0f", sampleRates[i]]];
        
        if (sampleRates[i] == kDefaultAudioOutputSampleRate) {
            [audioOutputSampleRateSelector selectItemAtIndex:i];
        }
    }

    /* Set the output sample rate to the default */
    [self setAudioOutputSampleRate:self];

    if (audioController->streamIsOpen() && audioController->isRendering()) {
        audioController->stopAudioRender();
        audioController->closeStream();
    }

    else if (audioController->streamIsOpen())
        audioController->closeStream();

    /* Open the stream */
    if (!audioController->openStream()) {
        [audioStartStopButton setEnabled:false];
    }

}

- (IBAction)setAudioOutputNumChannels:(id)sender {
    
    if (!audioController->setNumOutputChannels([[[audioOutputNumChannelsSelector selectedItem] title] intValue])) {
        NSLog(@"Fuck no");
        [audioOutputSampleRateSelector setEnabled:false];
        [audioStartStopButton setEnabled:false];
    }
}

- (IBAction)setAudioOutputSampleRate:(id)sender {
    
    if (!audioController->setOutputSampleRate([[[audioOutputSampleRateSelector selectedItem] title] floatValue])) {
        [audioStartStopButton setEnabled:false];
    }
}

- (IBAction)toggleAudioOutput:(id)sender {
    
    if ([sender state] == NSOnState) {
        if (!audioController->startAudioRender()) {
            Pa_Terminate();
            [audioStartStopButton setEnabled:false];
        }
        else [audioStartStopButton setTitle:@"Stop"];
    }
    else {
        if (!audioController->stopAudioRender()) {
            Pa_Terminate();
            [audioStartStopButton setEnabled:false];
        }
        else [audioStartStopButton setTitle:@"Start"];
    }
}

#pragma mark - Touchkey Parameter Events
- (IBAction)touchkeyEnableInput:(NSButton *)sender {
    
    NSLog(@"%s",__func__);
}

- (IBAction)touchkeySelectDevice:(NSPopUpButton *)sender; {
    
    NSLog(@"Selected device %ld", (long)sender.indexOfSelectedItem);
}

- (IBAction)touchkeyChangeLowestOctave:(id)sender {
    
    int startingOctave = (int)[touchkeyLowestOctaveSelect indexOfSelectedItem];
    
//    keyboardDisplay->clearAllTouches();
    touchkeyController->setLowestMidiNote(12*(startingOctave+1));
}

- (IBAction)touchkeyInputStart:(NSButton *)sender {
    
    if(touchkeyController->isAutoGathering()) {
        // Currently running.  Stop data collection and close the device.
        
        touchkeyController->closeDevice();
        [touchkeyStartButton setTitle: @"Start"];
        [touchkeyStatusField setStringValue: @"Not Running"];
        
        [touchkeyCalibrateButton setEnabled: NO];
        [touchkeyCalibrationLoadButton setEnabled: NO];
        [touchkeyCalibrationSaveButton setEnabled: NO];
        [touchkeyCalibrationClearButton setEnabled: NO];
        [touchkeyCalibrationClearButton setTitle: @"Clear"];
        [touchkeyCalibrateButton setTitle: @"Calibrate"];
        [touchkeyCalibrationStatusField setStringValue: @"Not Running"];
    }
    else {
        //printf("starting 1\n");
        // Currently not running.  Open the device and start data collection.
        // Use the current selection from the devices menu to find the device path.
        if([touchkeyDeviceSelect numberOfItems] < 1)
            return;
        int selectedIndex = (int)[touchkeyDeviceSelect indexOfSelectedItem];
        if(selectedIndex >= touchkeyDevicePaths.size())
            return;
        
        // Make sure our MIDI mapping is current
        [self touchkeyChangeLowestOctave:self];
        
        // Try opening the device
        if(touchkeyController->openDevice(touchkeyDevicePaths[selectedIndex].c_str())) {
            // Device open successful.  First figure out whether this port is actually
            // a valid touchkey device
            
            printf("\t\tDevice = %s", touchkeyDevicePaths[selectedIndex].c_str());
            
            [touchkeyStatusField setStringValue: @"Waiting on device..."];
            
            int count = 0;
            while(1) {
                if(touchkeyController->checkIfDevicePresent(250))
                    break;
                if(++count >= 20) {	// Try for 5 seconds before giving up.
                    [touchkeyStatusField setStringValue: @"Failed to open"];
                    return;
                }
            }
            
//            // Set the display to reflect the number of octaves present
//            [self setDisplayRangeFromMidiNote:touchkeyController->lowestKeyPresentMidiNote() toMidiNote:touchkeyController->highestMidiNote()];
            
            // Set minimum size and scan rate before starting
            touchkeyController->setKeyMinimumCentroidSize(-1, -1, 32);
            touchkeyController->setScanInterval(1);
            
            // Now try starting data collection
            if(touchkeyController->startAutoGathering()) {
                // Success!
                [touchkeyStartButton setTitle: @"Stop"];
                [touchkeyStatusField setStringValue: @"Running"];
                [touchkeyCalibrateButton setEnabled: YES];
                [touchkeyCalibrationLoadButton setEnabled: YES];
                [touchkeyCalibrationSaveButton setEnabled: NO];
                [touchkeyCalibrationClearButton setEnabled: NO];
                [touchkeyCalibrationClearButton setTitle: @"Clear"];
                [touchkeyCalibrationStatusField setStringValue: @"Not Calibrated"];
                
                
            }
            else {
                touchkeyController->closeDevice();
                [touchkeyStatusField setStringValue: @"Failed to start"];
            }
        }
        else {
            // Failed
            [touchkeyStatusField setStringValue: @"Failed to open"];
        }
        
        NSString *pathToDefaultCal = [touchkeyCalibrationDirectoryPath stringByAppendingString:@"calibration.xml"];
        
        if (touchkeyController->calibrationLoadFromFile([pathToDefaultCal cStringUsingEncoding:NSASCIIStringEncoding])) {
            
            [touchkeyCalibrationStatusField setStringValue: @"Calibrated"];
            [touchkeyCalibrationLoadButton setEnabled: YES];
            [touchkeyCalibrationSaveButton setEnabled: YES];
            [touchkeyCalibrationClearButton setEnabled: YES];
            [touchkeyCalibrationClearButton setTitle: @"Clear"];
        }
        else {
            [touchkeyCalibrationStatusField setStringValue: @"Unable to load"];
            [touchkeyCalibrationSaveButton setEnabled: NO];
        }
    }
}

/* Begin touchkeyController's calibration sequence */
- (IBAction)touchkeyCalibrateDevice:(NSButton *)sender {
    
    if (!touchkeyController->isOpen()) // Sanity check
        return;
    if (touchkeyController->calibrationInProgress()) {
        // If a calibration is currently in progress, finish it
        touchkeyController->calibrationFinish();
        
        [touchkeyCalibrateButton setTitle: @"Recalibrate"];
        [touchkeyCalibrationStatusField setStringValue: @"Calibrated"];
        [touchkeyCalibrationLoadButton setEnabled: YES];
        [touchkeyCalibrationSaveButton setEnabled: YES];
        [touchkeyCalibrationClearButton setEnabled: YES];
        [touchkeyCalibrationClearButton setTitle: @"Clear"];
    }
    else {
        // Calibration not in progress: start one ( (0,0) means all keys )
        touchkeyController->calibrationStart(0);
        
        [touchkeyCalibrateButton setTitle: @"Finish"];
        [touchkeyCalibrationStatusField setStringValue: @"Calibrating"];
        [touchkeyCalibrationLoadButton setEnabled: NO];
        [touchkeyCalibrationSaveButton setEnabled: NO];
        [touchkeyCalibrationClearButton setEnabled: YES];
        [touchkeyCalibrationClearButton setTitle: @"Abort"];
    }
}

/* Open a panel to choose a calibration file */
- (IBAction)touchkeyLoadCalibration:(NSButton *)sender {
    
    // Sanity check
    if (!touchkeyController->isOpen() || touchkeyController->calibrationInProgress())
        return;
    
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setCanChooseDirectories:NO];
    NSInteger result = [openPanel runModal];
    
    if (result == NSOKButton) {
        
        NSString *file = [openPanel filename];
        
        // Try loading the file
        if (touchkeyController->calibrationLoadFromFile([file cStringUsingEncoding:NSASCIIStringEncoding])) {
            [touchkeyCalibrationStatusField setStringValue:@"Calibrated"];
            [touchkeyCalibrationLoadButton setEnabled:YES];
            [touchkeyCalibrationSaveButton setEnabled:YES];
            [touchkeyCalibrationClearButton setEnabled:YES];
            [touchkeyCalibrationClearButton setTitle:@"Clear"];
        }
        else {
            [touchkeyCalibrationStatusField setStringValue:@"Unable to load"];
            [touchkeyCalibrateButton setEnabled:NO];
        }
    }
}

/* Open a panel to save the current calibration to a file */
- (IBAction)touchkeySaveCalibration:(NSButton *)sender {
    
    if (!touchkeyController->isOpen() || touchkeyController->calibrationInProgress()
        || !touchkeyController->isCalibrated()) // Sanity check
        return;
    
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    [savePanel setAllowedFileTypes:[NSArray arrayWithObject: @"xml"]];
    NSInteger result = [savePanel runModal];
    
    if (result == NSOKButton) {
        
        NSString *file = [savePanel filename];
        
        // Try saving the file
        if(touchkeyController->calibrationSaveToFile([file cStringUsingEncoding:NSASCIIStringEncoding])) {
            [touchkeyCalibrationStatusField setStringValue: @"Calibrated (saved)"];
        }
        else {
            [touchkeyCalibrationStatusField setStringValue: @"Unable to save"];
        }
    }
}

/* Clear the current calibration */
- (IBAction)touchkeyClearCalibration:(NSButton *)sender {
    
    if (!touchkeyController->isOpen()) // Sanity check
        return;
    if (touchkeyController->calibrationInProgress()) {
        // Abort the current calibration
        touchkeyController->calibrationAbort();
        
        [touchkeyCalibrateButton setTitle: @"Clear"];
        
        // Now figure out whether there is still a calibration active
        if (touchkeyController->isCalibrated()) {
            [touchkeyCalibrateButton setEnabled: YES];
            [touchkeyCalibrateButton setTitle: @"Reccalibrate"];
            [touchkeyCalibrationLoadButton setEnabled: YES];
            [touchkeyCalibrationSaveButton setEnabled: YES];
            [touchkeyCalibrationClearButton setEnabled: YES];
            [touchkeyCalibrationStatusField setStringValue: @"Calibrated"];
        }
        else {
            [touchkeyCalibrateButton setEnabled: YES];
            [touchkeyCalibrateButton setTitle: @"Calibrate"];
            [touchkeyCalibrationLoadButton setEnabled: YES];
            [touchkeyCalibrationSaveButton setEnabled: NO];
            [touchkeyCalibrationClearButton setEnabled: NO];
            [touchkeyCalibrationStatusField setStringValue: @"Not Calibrated"];
        }
    }
    else if (touchkeyController->isCalibrated()) {
        touchkeyController->calibrationClear();
        
        [touchkeyCalibrateButton setEnabled: YES];
        [touchkeyCalibrateButton setTitle: @"Calibrate"];
        [touchkeyCalibrationLoadButton setEnabled: YES];
        [touchkeyCalibrationSaveButton setEnabled: NO];
        [touchkeyCalibrationClearButton setEnabled: NO];
        [touchkeyCalibrationClearButton setTitle: @"Clear"];
        [touchkeyCalibrationStatusField setStringValue: @"Not Calibrated"];
    }
    
    // If not calibrated, don't do anything (shouldn't happen)
}

/* Populate the pop-up menu of available touchkey devices */
- (void)setAvailableTouchkeyDevices
{
    /* Clear all existing items */
    [touchkeyDeviceSelect removeAllItems];
    
    NSString *dev = @"/dev";
    NSArray *dirContents = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:dev error:nil];
    NSArray *touchkeyDevices = [dirContents filteredArrayUsingPredicate:
                                [NSPredicate predicateWithFormat:@"self BEGINSWITH 'cu.usbmodem'"]];
    
    /* If we don't have any touchkey devices, disable the controls */
    if([touchkeyDevices count] == 0) {
        [touchkeyDeviceSelect addItemWithTitle: @"No devices found"];
        [touchkeyDeviceSelect            setEnabled:NO];
        [touchkeyInputEnable             setEnabled:NO];
        [touchkeyLowestOctaveSelect      setEnabled:NO];
        [touchkeyStartButton             setEnabled:NO];
        [touchkeyCalibrateButton         setEnabled:NO];
        [touchkeyCalibrationLoadButton   setEnabled:NO];
        [touchkeyCalibrationSaveButton   setEnabled:NO];
        [touchkeyCalibrationClearButton  setEnabled:NO];
    }
    /* Otherwise, populate the drop-down menu with available devices */
    else {
        for(int i = 0; i < [touchkeyDevices count]; i++) {
            [touchkeyDeviceSelect addItemWithTitle: [touchkeyDevices objectAtIndex: i]];
            touchkeyDevicePaths.push_back([[dev stringByAppendingPathComponent:[touchkeyDevices objectAtIndex: i]]
                                            cStringUsingEncoding:NSASCIIStringEncoding]);
        }
        [touchkeyDeviceSelect selectItemAtIndex:0];
        [touchkeyDeviceSelect            setEnabled:YES];
        [touchkeyInputEnable             setEnabled:YES];
        [touchkeyLowestOctaveSelect      setEnabled:YES];
        [touchkeyStartButton             setEnabled:YES];
        [touchkeyCalibrateButton         setEnabled:YES];
        [touchkeyCalibrationLoadButton   setEnabled:YES];
        [touchkeyCalibrationSaveButton   setEnabled:YES];
        [touchkeyCalibrationClearButton  setEnabled:YES];
    }
}

#pragma mark - Touchkey Callbacks
void touchkeyCentroidCallback(timestamp_type timeStamp, int midiNote, KeyTouchFrame frame, void *userData) {
    
    printf("[t = %12.6f]\t\tnote: %3d\n---------------------------------\n", (float)timeStamp, midiNote);
    
    /* For each touch on this key */
    for (int i = 0; i < frame.count; i++)
        printf("touch %d (%d):\t\tloc.y = %f;\t\t size = %f\n",
               i, frame.ids[i], frame.locs[i], frame.sizes[i]);
    
    if (frame.white)
        printf("loc.x = %f\n\n", frame.locH);
    else
        printf("\n\n");
}

void touchkeyAnalogCallback(timestamp_type timeStamp, int midiNote, float position, void *userData) {
    
    if (!isnan(position) && position > 0.1)
        printf("[t = %12.6f]\t\tnote: %3d\t\tpos = %f\n", (float)timeStamp, midiNote, position);
}

#pragma mark - MIDI Parameter Events
- (IBAction)setMIDIInputDevice:(id)sender {
    
    if ([midiHandler isRunning]) {
        [midiHandler openSecondaryInputDevice:((int)[midiInputDeviceSelector indexOfSelectedItem]-1)];
        return;
    }
    
    if ([midiHandler setInputDevice:((int)[midiInputDeviceSelector indexOfSelectedItem]-1)]) {

    }
    else {
        
    }
}

- (IBAction)setMIDIOutputDevice:(id)sender {
    
    if ([midiHandler setOutputDevice:((int)[midiOutputDeviceSelector indexOfSelectedItem]-1)]) {
    
    }
    else {
        
    }
}

- (void)rescanMIDIDevices {
    
    std::vector<std::string> midiDevs;
    
    /* If the number of input devices has changed */
//    if (midiController->rescanInputDevices()) {
    if ([midiHandler rescanInputDevices]) {
    
//        midiDevs = midiController->getInputDeviceNames();
        midiDevs = [midiHandler getInputDeviceNames];
        
        /* Automatically set the first device as the MIDI input device if we went from zero available devices to one */
        bool selectFirst = midiDevs.size() == 1 && [midiInputDeviceSelector numberOfItems] == 1;
        
        [midiInputDeviceSelector removeAllItems];
        
        /* Populate the input devices list */
        [midiInputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"None"]];
        for (int i = 0; i < midiDevs.size(); i++) {
            [midiInputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"%s", midiDevs[i].c_str()]];
        }
        
        if (selectFirst) {
            [midiInputDeviceSelector selectItemAtIndex:1];
            [self setMIDIInputDevice:self];
        }
        
    }
    
    /* If the number of output devices has changed */
//    if (midiController->rescanOutputDevices()) {
    if ([midiHandler rescanOutputDevices]) {
    
        [midiOutputDeviceSelector removeAllItems];
        
        /* Populate the output devices list */
//        midiDevs = midiController->getOutputDeviceNames();
        midiDevs = [midiHandler getOutputDeviceNames];
        [midiOutputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"None"]];
        for (int i = 0; i < midiDevs.size(); i++) {
            [midiOutputDeviceSelector addItemWithTitle:[NSString stringWithFormat:@"%s", midiDevs[i].c_str()]];
        }
    }
}

@end
