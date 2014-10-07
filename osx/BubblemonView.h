//
//  BubblemonView.h
//  bubblemon
//
//  Created by Johan Walles on 2012-04-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "bubblemon.h"

@interface BubblemonView : NSView <NSDockTilePlugIn> {
  bubblemon_t *bubblemon;
  const bubblemon_picture_t *picture;
  
  NSDockTile *dockTile;
  NSMenu *dockMenu;
  
  CGImageRef windowFrame;
}

@end
