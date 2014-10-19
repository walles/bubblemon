//
//  main.m
//  bubblemon
//
//  Created by Johan Walles on 2012-04-29.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

// Return a normalized path from a Dock defaults app entry
static NSString *getPath(NSDictionary *appDictionary) {
  NSDictionary *tileDictionary = [appDictionary objectForKey:@"tile-data"];
  NSDictionary *fileDictionary = [tileDictionary objectForKey:@"file-data"];
  NSString *urlString = [fileDictionary objectForKey:@"_CFURLString"];
  NSString *pathString = [[NSURL URLWithString:urlString] path];
  
  return [pathString stringByResolvingSymlinksInPath];
}

// From: http://www.danandcheryl.com/tag/cocoa
@implementation NSUserDefaults (Additions)
- (BOOL)addApplicationToDock:(NSString *)path {
  NSDictionary *domain = [self persistentDomainForName:@"com.apple.dock"];
  NSArray *apps = [domain objectForKey:@"persistent-apps"];

  NSMutableDictionary *newDomain = [domain mutableCopy];
  NSMutableArray *newApps = [apps mutableCopy];
  NSDictionary *app = [NSDictionary dictionaryWithObject:[NSDictionary dictionaryWithObject:[NSDictionary dictionaryWithObjectsAndKeys:path, @"_CFURLString", [NSNumber numberWithInt:0], @"_CFURLStringType", nil] forKey:@"file-data"] forKey:@"tile-data"];
  [newApps addObject:app];
  [newDomain setObject:newApps forKey:@"persistent-apps"];
  [self setPersistentDomain:newDomain forName:@"com.apple.dock"];
  return [self synchronize];
}

// From: http://www.danandcheryl.com/tag/cocoa
- (BOOL)removeApplicationFromDock:(NSString *)removePath {
  NSDictionary *domain = [self persistentDomainForName:@"com.apple.dock"];
  NSArray *apps = [domain objectForKey:@"persistent-apps"];
  NSMutableArray *newApps = [NSMutableArray array];
  for (id app in apps) {
    if ([getPath(app) caseInsensitiveCompare:removePath] == NSOrderedSame) {
      // This is what we're removing, skip it
      continue;
    }

    [newApps addObject:app];
  }
  
  if (![apps isEqualToArray:newApps]) {
    NSMutableDictionary *newDomain = [domain mutableCopy];
    [newDomain setObject:newApps forKey:@"persistent-apps"];
    [self setPersistentDomain:newDomain forName:@"com.apple.dock"];
    return [self synchronize];
  }
  return NO;
}

- (BOOL)dockHasApplication:(NSString *)appPath {
  NSDictionary *domain = [self persistentDomainForName:@"com.apple.dock"];
  NSArray *apps = [domain objectForKey:@"persistent-apps"];
  for (id appDictionary in apps) {
    if ([getPath(appDictionary) caseInsensitiveCompare:appPath] == NSOrderedSame) {
      return TRUE;
    }
  }
  
  return FALSE;
}

- (NSString*)getRunningBubblemonPath {
  NSDictionary *domain = [self persistentDomainForName:@"com.apple.dock"];
  NSArray *apps = [domain objectForKey:@"persistent-apps"];
  NSArray *matchingApps = [apps filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K CONTAINS %@", @"tile-data.bundle-identifier", @"com.gmail.walles.johan.Bubblemon"]];
  if ([matchingApps count] == 0) {
    return NULL;
  }
  return getPath([matchingApps firstObject]);
}
@end

static void launchActivityMonitor() {
  NSLog(@"Launching Activity Monitor...\n");
  BOOL launched = [[NSWorkspace sharedWorkspace] launchApplication:@"Activity Monitor"];
  if (!launched) {
    NSLog(@"Launching Activity Monitor failed\n");
  }
}

int main(int argc, char *argv[])
{
  NSString *appPath = [[[NSBundle mainBundle] bundlePath] stringByResolvingSymlinksInPath];
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSString *runningBubblemonPath = [defaults getRunningBubblemonPath];
  if (runningBubblemonPath != NULL &&
      [runningBubblemonPath caseInsensitiveCompare:appPath] != NSOrderedSame)
  {
    NSLog(@"Removing old Bubblemon: %@\n", runningBubblemonPath);
    [defaults removeApplicationFromDock:runningBubblemonPath];
  }
  
  if ([defaults dockHasApplication:appPath]) {
    NSLog(@"Bubblemon already installed in the Dock\n");
    launchActivityMonitor();
  } else {
    NSLog(@"Not found, installing: %@\n", appPath);
    
    // Add ourselves to the dock
    [defaults addApplicationToDock:appPath];
    
    NSLog(@"Killing Dock to force it to reload its new Bubblemon-enabled configuration...\n");
    NSArray *docks = [NSRunningApplication runningApplicationsWithBundleIdentifier:@"com.apple.dock"];
    for (id dock in docks) {
      [(NSRunningApplication*)dock terminate];
    }
  }

  exit(0);
}
