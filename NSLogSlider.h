//
//  NSLogSlider.h
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 10/16/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSLogSlider : NSSlider {
    float _logValue;
}

- (float)logValue;
- (void)setLogValue:(float)logValue;

@end
