//
//  AppDelegate.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 8/4/14.
//  Copyright (c) 2014 ___FULLUSERNAME___. All rights reserved.
//

#import "AppDelegate.h"

@implementation AppDelegate

@synthesize window;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {

    /* ------------------- */
    /* === Synth Voices == */
    /* ------------------- */
    
    synthVoice = new SynthVoice();
    [synthVoiceSelector addItemWithTitle:kSynthVoiceTitle];
    
    additiveSynthVoice = new AdditiveSynthVoice(kNumHarmonics);
    [synthVoiceSelector addItemWithTitle:kAdditiveSynthVoiceTitle];
    
    subtractiveSynthVoice = new SubtractiveSynthVoice(kNumHarmonics);
    [synthVoiceSelector addItemWithTitle:kSubtractiveSynthVoiceTitle];
    
    /* ------------------- */
    /* === Audio setup === */
    /* ------------------- */
    
    synth = new PolySynth();
    audioController = new AudioController(synth);
    [self setSynthVoice:self];
    
    /* ------------------ */
    /* === MIDI Setup === */
    /* ------------------ */
    
    midiHandler = [[MIDIHandler alloc] initWithPolySynth:synth];
    
    /* ------------------------------ */
    /* === Initialize I/O Menu UI === */
    /* ------------------------------ */
    
    /* Initialize the I/O menu from the .xib */
    ioViewController = [[IOViewController alloc] initWithNibName:@"IOViewController"
                                                          bundle:nil
                                                           audio:audioController
                                                            midi:midiHandler];
    
    /* Add it as the document view of the scroll view, which is constrained to have the same size as its parent tab view */
    [ioScrollView setDocumentView:[ioViewController view]];
    [ioScrollView setHorizontalScrollElasticity:NSScrollElasticityNone];
    [ioScrollView setVerticalScrollElasticity:NSScrollElasticityNone];
    
    NSDictionary *views, *metrics;
    
    /* Add a constraint on the scrollview so it doesn't become more narrow than the ioViewController's view */
    views   = @{@"view" : ioScrollView};
    metrics = @{@"width" : [NSNumber numberWithDouble:[ioViewController view].bounds.size.width]};
    [ioScrollView setTranslatesAutoresizingMaskIntoConstraints:false];
    [ioScrollView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:[view(width)]"
                                                                         options:0
                                                                         metrics:metrics
                                                                           views:views]];
    /* Make sure it's scrolled to the top */
    [[ioScrollView contentView] scrollToPoint:NSMakePoint([ioViewController view].bounds.size.width,
                                                          [ioViewController view].bounds.size.height)];
    
    /* ---------------------------------- */
    /* === Initialize Mapping Menu UI === */
    /* ---------------------------------- */
    mappingViewController = [[MappingViewController alloc] initWithNibName:@"MappingViewController"
                                                                    bundle:nil
                                                                     synth:synth];
    [mappingScrollView setDocumentView:[mappingViewController view]];
    [mappingScrollView setHorizontalScrollElasticity:NSScrollElasticityNone];
    [mappingScrollView setVerticalScrollElasticity:NSScrollElasticityNone];
    
    views = @{@"view" : mappingScrollView};
    metrics = @{@"width" : [NSNumber numberWithDouble:[mappingViewController view].frame.size.width]};
    [mappingScrollView setTranslatesAutoresizingMaskIntoConstraints:false];
    [mappingScrollView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:[view(width)]"
                                                                 options:0
                                                                 metrics:metrics
                                                                   views:views]];
    
    /* Make sure it's scrolled to the top */
    [[mappingScrollView contentView] scrollToPoint:NSMakePoint([mappingViewController view].bounds.size.width,
                                                               [mappingViewController view].bounds.size.height)];
    
    [midiHandler setMappingViewController:mappingViewController];
}

- (IBAction)setSynthVoice:(id)sender {
    
    /* Remove old view and constraints on the container */
    [[synthViewController view] removeFromSuperview];
    
    if ([synthViewWidthConstraints isNotEqualTo:nil])
        [synthViewContainer removeConstraints:synthViewWidthConstraints];
    if ([synthViewHeightConstraints isNotEqualTo:nil])
        [synthViewContainer removeConstraints:synthViewHeightConstraints];
    
    
    NSString* title = [synthVoiceSelector titleOfSelectedItem];
    
    if ([title isEqualToString:kSynthVoiceTitle]) {
        
        synth->setMasterVoice(synthVoice);
        
        synthViewController = [[SynthVoiceViewController alloc] initWithNibName:@"SynthVoiceViewController" bundle:nil synth:synth];
    }
    else if ([title isEqualToString:kAdditiveSynthVoiceTitle]) {
        
        synth->setMasterVoice(additiveSynthVoice);
        
        synthViewController = [[AdditiveSynthVoiceViewController alloc] initWithNibName:@"AdditiveSynthVoiceViewController" bundle:nil synth:synth numHarmonics:additiveSynthVoice->numHarmonics()];
    }
    else if ([title isEqualToString:kSubtractiveSynthVoiceTitle]) {
        
        synth->setMasterVoice(subtractiveSynthVoice);
        
        synthViewController = [[SubtractiveSynthVoiceViewController alloc] initWithNibName:@"SubtractiveSynthVoiceViewController" bundle:nil synth:synth numHarmonics:subtractiveSynthVoice->numHarmonics()];
    }
    
    [midiHandler setVoiceViewController:synthViewController];
    
    /* Add the new synth view */
    [synthViewContainer addSubview:[synthViewController view]];
    
    /* Constrain it's container view's minimum size to the size of the interface view in the .xib */
    NSDictionary *views, *metrics;
    views   = @{@"view" : synthViewContainer};
    metrics = @{@"width" : [NSNumber numberWithFloat:[synthViewController view].frame.size.width],
                @"height" : [NSNumber numberWithFloat:[synthViewController view].frame.size.height]};
    [synthViewContainer setTranslatesAutoresizingMaskIntoConstraints:false];
    synthViewWidthConstraints = [NSLayoutConstraint constraintsWithVisualFormat:@"H:[view(>=width)]"
                                                                        options:0
                                                                        metrics:metrics
                                                                          views:views];
    synthViewHeightConstraints = [NSLayoutConstraint constraintsWithVisualFormat:@"V:[view(>=height)]"
                                                                         options:0
                                                                         metrics:metrics
                                                                           views:views];
    /* Set the constraints */
    [synthViewContainer addConstraints:synthViewWidthConstraints];
    [synthViewContainer addConstraints:synthViewHeightConstraints];
    
    /* Set the synth UI to expand with its container view */
    [[synthViewController view] setTranslatesAutoresizingMaskIntoConstraints:false];
    views = @{@"view" : [synthViewController view]};
    [synthViewContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|[view]|"
                                                                                  options:0
                                                                                  metrics:nil
                                                                                    views:views]];
    [synthViewContainer addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[view]|"
                                                                                  options:0
                                                                                  metrics:nil
                                                                                    views:views]];
    
    /* Populate the parameter selector menu with the voice's parameters */
    std::vector<std::string> params = synth->masterVoice()->getParameterNames();
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    
    if (audioController)
        delete audioController;
    if (midiController)
        delete midiController;
//    if (masterVoice)
//        delete masterVoice;
    if (synth)
        delete synth;
}

@end











