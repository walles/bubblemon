//
//  main.m
//  bubblemon
//
//  Created by Johan Walles on 2012-04-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

// From: http://www.danandcheryl.com/tag/cocoa
@implementation NSUserDefaults (Additions)
- (BOOL)addApplicationToDock:(NSString *)path {
  NSDictionary *domain = [self persistentDomainForName:@"com.apple.dock"];
  NSArray *apps = [domain objectForKey:@"persistent-apps"];
  NSArray *matchingApps = [apps filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K CONTAINS %@", @"tile-data.file-data._CFURLString", path]];
  if ([matchingApps count] == 0) {
    NSMutableDictionary *newDomain = [domain mutableCopy];
    NSMutableArray *newApps = [apps mutableCopy];
    NSDictionary *app = [NSDictionary dictionaryWithObject:[NSDictionary dictionaryWithObject:[NSDictionary dictionaryWithObjectsAndKeys:path, @"_CFURLString", [NSNumber numberWithInt:0], @"_CFURLStringType", nil] forKey:@"file-data"] forKey:@"tile-data"];
    [newApps addObject:app];
    [newDomain setObject:newApps forKey:@"persistent-apps"];
    [self setPersistentDomain:newDomain forName:@"com.apple.dock"];
    return [self synchronize];
  }
  return NO;
}
@end

static BOOL isKeptInDock() {
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSDictionary *dock_settings = [defaults persistentDomainForName:@"com.apple.dock"];
  NSArray *persistent_apps = [dock_settings objectForKey:@"persistent-apps"];
  for (id persistent_app in persistent_apps) {
    NSDictionary *tile_data = [(NSDictionary*)persistent_app objectForKey:@"tile-data"];
    NSString *bundle_identifier = [tile_data objectForKey:@"bundle-identifier"];
    if ([@"com.gmail.walles.johan.BubblemonDocked" isEqualToString:bundle_identifier]) {
      return TRUE;
    }
  }
  return FALSE;
}

static void keepInDock(NSString *app_path) {
  NSLog(@"Adding Bubblemon as a Keep-in-Dock app...\n");
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  [defaults addApplicationToDock:app_path];
  
  NSLog(@"Killing Dock to force it to reload its new Bubblemon-enabled configuration...\n");
  NSArray *docks = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.dock"];
  for (id dock in docks) {
    [(NSRunningApplication*)dock terminate];
  }
}

static void launchActivityMonitor() {
  NSLog(@"Launching Activity Monitor...\n");
  BOOL launched = [[NSWorkspace sharedWorkspace] launchApplication:@"Activity Monitor"];
  if (!launched) {
    NSLog(@"Launching Activity Monitor failed\n");
  }
}

int main(int argc, char *argv[])
{
  if (isKeptInDock()) {
    NSLog(@"Bubblemon already installed in the Dock\n");
    launchActivityMonitor();
  } else {
    keepInDock([[NSBundle mainBundle] bundlePath]);
  }

  exit(0);
}
