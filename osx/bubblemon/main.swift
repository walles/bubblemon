//  Copyright (c) 2012, 2017 johan.walles@gmail.com. All rights reserved.

import Cocoa

// From: http://www.danandcheryl.com/tag/cocoa
extension UserDefaults {
  func addApplication(toDock path: String) -> Bool {
    let domain = persistentDomain(forName: "com.apple.dock")
    let apps = domain?["persistent-apps"] as? [Any] ?? [Any]()
    var newDomain = domain
    var newApps = apps
    let app = [
      "tile-data" : [
        "file-data" : [
          "_CFURLString" : path,
          "_CFURLStringType" : Int(0)
        ]
      ]
    ]
    newApps.append(app)
    newDomain?["persistent-apps"] = newApps
    setPersistentDomain(newDomain!, forName: "com.apple.dock")
    return synchronize()
  }

  // From: http://www.danandcheryl.com/tag/cocoa
  func removeApplication(fromDock removePath: String) -> Bool {
    let domain = persistentDomain(forName: "com.apple.dock")
    let apps = domain?["persistent-apps"] as? [Any] ?? [Any]()

    var newApps = [Any]()
    for app in apps {
      if getPath(appDictionary: app)?.caseInsensitiveCompare(removePath) == .orderedSame {
        // This is what we're removing, skip it
        continue
      }

      newApps.append(app)
    }

    if (apps as NSArray).isEqual(to: newApps) {
      // Nothing changed, bail saying nothing changed
      return false
    }

    var newDomain = domain
    newDomain?["persistent-apps"] = newApps
    setPersistentDomain(newDomain!, forName: "com.apple.dock")
    let result = synchronize()

    print("Killing com.apple.dock.extra to force it to unload the old Bubblemon...\n")
    let docks = NSRunningApplication.runningApplications(withBundleIdentifier: "com.apple.dock.extra")
    for dock in docks {
      dock.terminate()
    }
    return result
  }

  func dockHasApplication(_ appPath: String) -> Bool {
    let domain = persistentDomain(forName: "com.apple.dock")
    let apps = domain?["persistent-apps"] as? [Any] ?? [Any]()
    for appDictionary in apps {
      if getPath(appDictionary: appDictionary)?.caseInsensitiveCompare(appPath) == .orderedSame {
        return true
      }
    }
    return false
  }

  func getRunningBubblemonPath() -> String? {
    let domain = persistentDomain(forName: "com.apple.dock")
    let apps = domain?["persistent-apps"] as? [Any] ?? [Any]()
    let matchingApps = apps.filter {
      NSPredicate(
        format: "%K CONTAINS %@",
        "tile-data.bundle-identifier",
        "com.gmail.walles.johan.Bubblemon").evaluate(with: $0)
    }

    if matchingApps.count == 0 {
      return nil
    }

    return getPath(appDictionary: matchingApps.first!)
  }
}

// Return a normalized path from a Dock defaults app entry
//
// We accept an Any since the data comes from some struct on disk. If we can't
// parse it we just return nil.
private func getPath(appDictionary: Any) -> String? {
  let dictionary = appDictionary as? [String: Any]
  let tileDictionary = dictionary?["tile-data"] as? [String: Any]
  let fileDictionary = tileDictionary?["file-data"] as? [String: Any]
  let urlString = fileDictionary?["_CFURLString"] as? String
  if urlString == nil {
    return nil
  }

  let url = URL(string: urlString!)
  if url == nil {
    return nil
  }

  return url!.resolvingSymlinksInPath().path
}

private func launchActivityMonitor() {
  print("Launching Activity Monitor...\n")
  let launched = NSWorkspace.shared().launchApplication(
    withBundleIdentifier: "com.apple.ActivityMonitor",
    options: .default,
    additionalEventParamDescriptor: nil,
    launchIdentifier: nil)

  if !launched {
    print("Launching Activity Monitor failed\n")
  }
}

func main() -> Int32 {
  let appPath = URL(string: Bundle.main.bundlePath)!.resolvingSymlinksInPath().path
  let defaults = UserDefaults.standard

  let runningBubblemonPath = defaults.getRunningBubblemonPath()
  if runningBubblemonPath == nil {
    // No Bubblemon running, nothing to remove
  } else if runningBubblemonPath!.caseInsensitiveCompare(appPath) != .orderedSame {
    print("Removing old Bubblemon: \(runningBubblemonPath!)\n")

    if !defaults.removeApplication(fromDock: runningBubblemonPath!) {
      print("Removing old bubblemon failed\n")
    }
  }

  if defaults.dockHasApplication(appPath) {
    print("Bubblemon already installed in the Dock\n")
    launchActivityMonitor()
    return EXIT_SUCCESS
  }

  print("Not found, installing: \(appPath)\n")
  // Add ourselves to the dock
  if !defaults.addApplication(toDock: appPath) {
    print("Adding ourselves to the Dock failed, bailing...")
    return EXIT_FAILURE
  }

  print("Killing Dock to force it to reload its new Bubblemon-enabled configuration...\n")
  let docks = NSRunningApplication.runningApplications(withBundleIdentifier: "com.apple.dock")
  for dock in docks {
    dock.terminate()
  }

  return EXIT_SUCCESS
}

exit(main())
