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

- (NSMenu *)dockMenu {
  NSLog(@"Setting up dock menu\n");
  if (dockMenu != NULL) {
    return dockMenu;
  }
  
  dockMenu = [[NSMenu alloc] init];
  
  NSMenuItem *helpItem = [[NSMenuItem alloc] initWithTitle:@"Help" action:@selector(openLegend:) keyEquivalent:@""];
  [helpItem setTarget:self];
  [dockMenu addItem:helpItem];
  
  NSMenuItem *aboutItem = [[NSMenuItem alloc] initWithTitle:@"About" action:@selector(openAboutPanel:) keyEquivalent:@""];
  [aboutItem setTarget:self];
  [dockMenu addItem:aboutItem];
  
  return dockMenu;
}

- (IBAction)openLegend:(id)sender {
  NSLog(@"Opening help in browser...\n");
  [[NSWorkspace sharedWorkspace] openURL:[NSURL URLWithString:@"http://walles.github.io/bubblemon/"]];
}

- (IBAction)openAboutPanel:(id)sender {
  NSAttributedString *credits = [[NSAttributedString alloc]
                                 initWithString:@"http://walles.github.io/bubblemon"
                                 attributes: @{NSLinkAttributeName: @"http://walles.github.io/bubblemon"}];
  
  // The Git hash and version get filled in by a "Run Script" build step
  NSBundle *bundle = [NSBundle bundleForClass:[self class]];
  NSString *gitHash = [bundle infoDictionary][@"GitHash"];
  NSString *gitDescribe = [bundle infoDictionary][@"GitDescribe"];
  
  NSDictionary *aboutOptions = @{
                                 @"Credits": credits,
                                 @"ApplicationName": @"Bubblemon",
                                 // @"ApplicationIcon": FIXME,
                                 @"Version": gitHash,
                                 @"Copyright": @"Copyright 1999-2014 johan.walles@gmail.com",
                                 @"ApplicationVersion": gitDescribe
                                 };
  [[NSApplication sharedApplication] orderFrontStandardAboutPanelWithOptions: aboutOptions];
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
    bubblemon_setColors(bubblemon,
                        0x75ceff00u, 0x0066ff80u,
                        0xff333340u, 0xaa000080u,
                        0x00ff0080u, 0xffff40ffu);
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
  bubblemon_setSize(bubblemon, 50, 50);
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
                                        (CGBitmapInfo)kCGImageAlphaNoneSkipLast,
                                        dataProviderRef,
                                        NULL,
                                        shouldInterpolate,
                                        kCGRenderingIntentDefault);
  CGColorSpaceRelease(rgb);
  CGDataProviderRelease(dataProviderRef);
  
  __strong NSGraphicsContext *nsGraphicsContext = [NSGraphicsContext currentContext];
  CGContextRef cgContextRef = (CGContextRef)[nsGraphicsContext graphicsPort];
  
  // Draw the bubblemon image
  CGRect bubbleViewRect = CGRectMake([self bounds].size.width  * 0.08f,
                                     [self bounds].size.height * 0.08f,
                                     [self bounds].size.width  * 0.84f,
                                     [self bounds].size.height * 0.84f);
  CGContextSetAlpha(cgContextRef, 0.9f);
  CGContextDrawImage(cgContextRef, bubbleViewRect, cgImageRef);
  CGImageRelease(cgImageRef);
  
  // Draw the window frame
  CGRect fullSizeRect = NSRectToCGRect([self bounds]);
  CGContextSetAlpha(cgContextRef, 1.0f);
  CGContextDrawImage(cgContextRef, fullSizeRect, windowFrame);
}

@end
