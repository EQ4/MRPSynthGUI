//
//  NSLogSlider.m
//  MRPSynthGUI
//
//  Created by Jeff Gregorio on 10/16/14.
//  Copyright (c) 2014 Jeff Gregorio. All rights reserved.
//

#import "NSLogSlider.h"

@implementation NSLogSlider

- (id)initWithCoder:(NSCoder *)aDecoder {
    
    self = [super initWithCoder:aDecoder];
    if (self) {
        
        [self sendActionOn:NSLeftMouseDownMask | NSLeftMouseDraggedMask | NSLeftMouseUpMask];
        return self;
    }
    else
        return nil;
}

- (void)mouseDown:(NSEvent *)theEvent {
    
    CGPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    loc.y = self.bounds.size.height - loc.y;
    [self setValueFromLocationInView:loc];
}

- (void)mouseDragged:(NSEvent *)theEvent {
    
    CGPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    loc.y = self.bounds.size.height - loc.y;
    [self setValueFromLocationInView:loc];
}

- (void)mouseUp:(NSEvent *)theEvent {
    
    CGPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
    loc.y = self.bounds.size.height - loc.y;
    [self setValueFromLocationInView:loc];
}

- (void)setValueFromLocationInView:(CGPoint)loc {
    
    CGFloat p, pMin, pMax;
    CGFloat linVal;
    
    if ([self isVertical]) {
        p = loc.y;
        pMin = self.bounds.origin.y;
        pMax = self.bounds.size.height;
    }
    else {
        p = loc.x;
        pMin = self.bounds.origin.x;
        pMax = self.bounds.size.width;
    }
    
    linVal = (p - pMin) / (pMax - pMin);
    linVal *= ([self maxValue] - [self minValue]);
    linVal += [self minValue];
    
    linVal = fmaxf(linVal, [self minValue]);
    linVal = fminf(linVal, [self maxValue]);
    
    _logValue = [self minValue] * expf(linVal * logf([self maxValue]/[self minValue]) /
                                       ([self maxValue]-[self minValue]));
    
    [self setFloatValue:linVal];
    [self sendAction:[self action] to:[self target]];
}

- (float)logValue {
    return _logValue;
}

- (void)setLogValue:(float)logValue {
    
    [self setFloatValue:(logf(logValue/[self minValue]) * ([self maxValue]-[self minValue])) / logf([self maxValue]/[self minValue])];
}

@end
