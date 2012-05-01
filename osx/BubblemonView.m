//
//  BubblemonView.m
//  bubblemon
//
//  Created by Johan Walles on 2012-04-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "BubblemonView.h"

@implementation BubblemonView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
        bubblemon = bubblemon_init();
        
        // Inspired by http://stackoverflow.com/questions/1449035/how-do-i-use-nstimer
        [NSTimer scheduledTimerWithTimeInterval:(1.0 / 25.0)
                                         target:self
                                       selector:@selector(timerFired)
                                       userInfo:nil
                                        repeats:YES];
    }
    
    return self;
}

- (void)timerFired
{
    // Invalidate ourselves
    [self setNeedsDisplay:YES];
    NSString *tooltip = [[NSString alloc] initWithUTF8String:bubblemon_getTooltip(bubblemon)];
    [self setToolTip: tooltip];
}

static void releaseDataProvider(void *info, const void *data, size_t size) {
    // No need to actually free anything here, the data is handled by
    // the bubblemon code.
}

- (void)drawRect:(NSRect)dirtyRectIgnored
{
    // Drawing code here.
    bubblemon_setSize(bubblemon, 
                      [self bounds].size.width,
                      [self bounds].size.height);
    const bubblemon_picture_t *picture = bubblemon_getPicture(bubblemon);
    
    size_t imageDataSize = [self bounds].size.width * [self bounds].size.height * sizeof(bubblemon_color_t);
    
    CGDataProviderRef dataProviderRef = CGDataProviderCreateWithData(NULL, picture->pixels, imageDataSize, releaseDataProvider);
    
    const size_t bitsPerComponent = 8;
    const size_t bitsPerPixel = sizeof(bubblemon_color_t) * 8;
    const size_t bytesPerRow = [self bounds].size.width * sizeof(bubblemon_color_t);
    const bool shouldInterpolate = false;
    
    CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
    CGImageRef cgImageRef = CGImageCreate([self bounds].size.width,
                                          [self bounds].size.height,
                                          bitsPerComponent,
                                          bitsPerPixel,
                                          bytesPerRow,
                                          rgb,
                                          kCGImageAlphaLast,
                                          dataProviderRef,
                                          NULL,
                                          shouldInterpolate,
                                          kCGRenderingIntentDefault);
    CGColorSpaceRelease(rgb);
    CGDataProviderRelease(dataProviderRef);
    
    __strong NSGraphicsContext *nsGraphicsContext = [NSGraphicsContext currentContext];
    CGContextRef cgContextRef = (CGContextRef)[nsGraphicsContext graphicsPort];
    
    CGRect cgRect = NSRectToCGRect([self bounds]);
    CGContextDrawImage(cgContextRef, cgRect, cgImageRef);
    CGImageRelease(cgImageRef);
}

@end
