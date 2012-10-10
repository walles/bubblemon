//
//  BubblemonView.m
//  bubblemon
//
//  Created by Johan Walles on 2012-04-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "BubblemonView.h"

@implementation BubblemonView

- (void)setDockTile:(NSDockTile*)aDockTile {
    if (aDockTile) {
        dockTile = aDockTile;
        [dockTile setContentView:self];
    }
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
#ifdef DEBUG
        bubblemon_selftest();
#endif
        bubblemon = bubblemon_init();
        picture = NULL;
        
        // Inspired by http://stackoverflow.com/questions/1449035/how-do-i-use-nstimer
        [NSTimer scheduledTimerWithTimeInterval:(1.0 / 25.0)
                                         target:self
                                       selector:@selector(timerFired)
                                       userInfo:nil
                                        repeats:YES];

        // Load window frame graphics
        NSBundle *bundle = [NSBundle bundleForClass:[self class]];
        CFURLRef windowFrameUrl = (__bridge CFURLRef)[bundle URLForImageResource:@"window-frame"];
        CGDataProviderRef dataProvider = CGDataProviderCreateWithURL(windowFrameUrl);
        windowFrame = CGImageCreateWithPNGDataProvider(dataProvider, NULL, NO, kCGRenderingIntentDefault);
        CGDataProviderRelease(dataProvider);
    }
  
    return self;
}

- (void)timerFired
{
    // Compute a new image to display
    // The Dock won't tell us its size, so this is a guess at roughly how many pixels
    // the bubblemon will get on screen.
    bubblemon_setSize(bubblemon, 38, 38);
    picture = bubblemon_getPicture(bubblemon);
  
    NSString *tooltip = [[NSString alloc] initWithUTF8String:bubblemon_getTooltip(bubblemon)];
    [self setToolTip: tooltip];
    
    if (dockTile != nil) {
        [dockTile display];
    } else {
        [self setNeedsDisplay:YES];
    }
}

static void releaseDataProvider(void *info, const void *data, size_t size) {
    // No need to actually free anything here, the data is handled by
    // the bubblemon code.
}

- (CGImageRef)getMask
{
  if (mask != NULL) {
    return mask;
  }
  
  // Create a blank image of the right proportions
  CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceGray();
  CGContextRef context = CGBitmapContextCreate(NULL,
                                               picture->width, picture->height,
                                               8, 0,
                                               colorSpace,
                                               kCGImageAlphaNone);

  // Draw a black background
  CGRect rect = CGRectMake((CGFloat)0.0, (CGFloat)0.0,
                           (CGFloat)picture->width, (CGFloat)picture->height);
  CGContextSetGrayFillColor(context, (CGFloat)0.0, (CGFloat)1.0);
  CGContextFillRect(context, rect);
  
  // Draw a filled white circle
  CGContextSetGrayFillColor(context, (CGFloat)1.0, (CGFloat)1.0);
  CGContextFillEllipseInRect(context, rect);
  
  mask = CGBitmapContextCreateImage(context);
  
  CGColorSpaceRelease(colorSpace);
  CGContextRelease(context);
  
  return mask;
}

- (void)drawRect:(NSRect)dirtyRectIgnored
{
    if (!picture) {
        return;
    }
    
    // Redraw ourselves
    size_t imageDataSize = picture->width * picture->height * sizeof(bubblemon_color_t);
    
    CGDataProviderRef dataProviderRef = CGDataProviderCreateWithData(NULL, picture->pixels, imageDataSize, releaseDataProvider);
    
    const size_t bitsPerComponent = 8;
    const size_t bitsPerPixel = sizeof(bubblemon_color_t) * 8;
    const size_t bytesPerRow = picture->width * sizeof(bubblemon_color_t);
    const bool shouldInterpolate = false;
  
    CGColorSpaceRef rgb = CGColorSpaceCreateDeviceRGB();
    CGImageRef cgImageRef = CGImageCreate(picture->width,
                                          picture->height,
                                          bitsPerComponent,
                                          bitsPerPixel,
                                          bytesPerRow,
                                          rgb,
                                          kCGImageAlphaNoneSkipLast,
                                          dataProviderRef,
                                          NULL,
                                          shouldInterpolate,
                                          kCGRenderingIntentDefault);
    CGColorSpaceRelease(rgb);
    CGDataProviderRelease(dataProviderRef);
  
    CGImageRef maskedBubbles = CGImageCreateWithMask(cgImageRef, [self getMask]);
    CGImageRelease(cgImageRef);
    
    __strong NSGraphicsContext *nsGraphicsContext = [NSGraphicsContext currentContext];
    CGContextRef cgContextRef = (CGContextRef)[nsGraphicsContext graphicsPort];
  
    // Draw the bubblemon image
    CGRect bubbleViewRect = CGRectMake([self bounds].size.width  * 0.09f,
                                       [self bounds].size.height * 0.09f,
                                       [self bounds].size.width  * 0.83f,
                                       [self bounds].size.height * 0.83f);
    CGContextSetAlpha(cgContextRef, 0.8f);
    CGContextDrawImage(cgContextRef, bubbleViewRect, maskedBubbles);
    CGImageRelease(maskedBubbles);
  
    // Draw the window frame
    CGRect fullSizeRect = NSRectToCGRect([self bounds]);
    CGContextSetAlpha(cgContextRef, 1.0f);
    CGContextDrawImage(cgContextRef, fullSizeRect, windowFrame);
}

@end
