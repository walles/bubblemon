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

// From: http://www.danandcheryl.com/tag/cocoa
- (BOOL)removeApplicationFromDock:(NSString *)name {
  NSDictionary *domain = [self persistentDomainForName:@"com.apple.dock"];
  NSArray *apps = [domain objectForKey:@"persistent-apps"];
  NSArray *newApps = [apps filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"not %K CONTAINS %@", @"tile-data.file-data._CFURLString", name]];
  if (![apps isEqualToArray:newApps]) {
    NSMutableDictionary *newDomain = [domain mutableCopy];
    [newDomain setObject:newApps forKey:@"persistent-apps"];
    [self setPersistentDomain:newDomain forName:@"com.apple.dock"];
    return [self synchronize];
  }
  return NO;
}

- (BOOL)dockHasApplication:(NSString *)path {
  NSDictionary *domain = [self persistentDomainForName:@"com.apple.dock"];
  NSArray *apps = [domain objectForKey:@"persistent-apps"];
  NSArray *matchingApps = [apps filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K CONTAINS[c] %@", @"tile-data.file-data._CFURLString", path]];
  return [matchingApps count] > 0;
}

- (NSString*)getRunningBubblemonPath {
  NSDictionary *domain = [self persistentDomainForName:@"com.apple.dock"];
  NSArray *apps = [domain objectForKey:@"persistent-apps"];
  NSArray *matchingApps = [apps filteredArrayUsingPredicate:[NSPredicate predicateWithFormat:@"%K CONTAINS %@", @"tile-data.bundle-identifier", @"com.gmail.walles.johan.Bubblemon"]];
  if ([matchingApps count] == 0) {
    return NULL;
  }
  NSDictionary *appDictionary = [matchingApps firstObject];
  NSDictionary *tileDictionary = [appDictionary objectForKey:@"tile-data"];
  NSDictionary *fileDictionary = [tileDictionary objectForKey:@"file-data"];
  NSString *urlString = [fileDictionary objectForKey:@"_CFURLString"];
  
  // Remove leading file://
  NSString *urlPrefix = @"file://";
  NSString *pathString = [urlString substringFromIndex:[urlPrefix length]];
  
  // Remove trailing /
  pathString = [pathString substringToIndex:[pathString length] - 1];

  return pathString;
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
  NSString *appPath = [[NSBundle mainBundle] bundlePath];
  NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
  NSString *runningBubblemonPath = [defaults getRunningBubblemonPath];
  if (runningBubblemonPath != NULL &&
      [runningBubblemonPath caseInsensitiveCompare:appPath] != NSOrderedSame)
  {
    NSLog(@"Other Bubblemon detected, removing:\n");
    NSLog(@"  %@ <- me\n", appPath);
    NSLog(@"  %@ <- the other guy\n", runningBubblemonPath);
    [defaults removeApplicationFromDock:runningBubblemonPath];
  }
  
  if ([defaults dockHasApplication:appPath]) {
    NSLog(@"Bubblemon already installed in the Dock, launching Activity Monitor...\n");
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
