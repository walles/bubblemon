//  Copyright (c) 2012, 2017 johan.walles@gmail.com. All rights reserved.

import Cocoa

// From: http://www.danandcheryl.com/tag/cocoa
extension UserDefaults {
  func addApplication(toDock path: String) -> Bool {
    let domain: [String: Any]? = persistentDomain(forName: "com.apple.dock")
    let apps = domain?["persistent-apps"] as? [Any] ?? [Any]()
    var newDomain: [String: Any]? = domain
    var newApps: [Any] = apps
    let app: [AnyHashable: Any] = [
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
    let domain: [String: Any]? = persistentDomain(forName: "com.apple.dock")
    let apps = domain?["persistent-apps"] as? [Any] ?? [Any]()
    var newApps = [Any]()
    for app: Any in apps {
      if getPath(appDictionary: app as! [String : Any]).caseInsensitiveCompare(removePath) == .orderedSame {
        // This is what we're removing, skip it
        continue
      }
      newApps.append(app)
    }
    if !(apps as NSArray).isEqual(to: newApps) {
      var newDomain: [String: Any]? = domain
      newDomain?["persistent-apps"] = newApps
      setPersistentDomain(newDomain!, forName: "com.apple.dock")
      let result: Bool = synchronize()
      print("Killing com.apple.dock.extra to force it to unload the old Bubblemon...\n")
      let docks: [Any] = NSRunningApplication.runningApplications(withBundleIdentifier: "com.apple.dock.extra")
      for dock: Any in docks {
        (dock as? NSRunningApplication)?.terminate()
      }
      return result
    }
    return false
  }
  
  func dockHasApplication(_ appPath: String) -> Bool {
    let domain: [String: Any]? = persistentDomain(forName: "com.apple.dock")
    let apps = domain?["persistent-apps"] as? [Any] ?? [Any]()
    for appDictionary: Any in apps {
      if getPath(appDictionary: appDictionary as! [String : Any]).caseInsensitiveCompare(appPath) == .orderedSame {
        return true
      }
    }
    return false
  }
  
  func getRunningBubblemonPath() -> String? {
    let domain: [String: Any]? = persistentDomain(forName: "com.apple.dock")
    let apps = domain?["persistent-apps"] as? [Any] ?? [Any]()
    let matchingApps: [Any] = apps.filter { NSPredicate(format: "%K CONTAINS %@", "tile-data.bundle-identifier", "com.gmail.walles.johan.Bubblemon").evaluate(with: $0) }
    if matchingApps.count == 0 {
      return nil
    }
    return getPath(appDictionary: matchingApps.first! as! [String : Any])
  }
}

// Return a normalized path from a Dock defaults app entry
private func getPath(appDictionary: [String: Any]) -> String {
  let tileDictionary = appDictionary["tile-data"] as? [String: Any] ?? [String: Any]()
  let fileDictionary = tileDictionary["file-data"] as? [String: Any] ?? [String: Any]()
  let urlString: String = fileDictionary["_CFURLString"] as? String ?? ""
  let pathString: String? = URL(string: urlString)?.path
  return pathString?.resolvingSymlinksInPath!
  }
  
  private func launchActivityMonitor() {
    print("Launching Activity Monitor...\n")
    let launched: Bool = NSWorkspace.shared().launchApplication(withBundleIdentifier: "com.apple.ActivityMonitor", options: .default, additionalEventParamDescriptor: nil, launchIdentifier: nil)
    if !launched {
      print("Launching Activity Monitor failed\n")
    }
  }
  
  func main(argc: Int, argv: [CChar]) -> Int {
    let appPath: String = Bundle.main.bundlePath.resolvingSymlinksInPath
    let defaults = UserDefaults.standard
    let runningBubblemonPath: String = defaults.getRunningBubblemonPath()
    if runningBubblemonPath != nil && runningBubblemonPath.caseInsensitiveCompare(appPath) != .orderedSame {
      print("Removing old Bubblemon: \(runningBubblemonPath)\n")
      defaults.removeApplication(fromDock: runningBubblemonPath)
    }
    if defaults.dockHasApplication(appPath) {
      print("Bubblemon already installed in the Dock\n")
      launchActivityMonitor()
    }
    else {
      print("Not found, installing: \(appPath)\n")
      // Add ourselves to the dock
      defaults.addApplication(toDock: appPath)
      print("Killing Dock to force it to reload its new Bubblemon-enabled configuration...\n")
      let docks: [Any] = NSRunningApplication.runningApplications(withBundleIdentifier: "com.apple.dock")
      for dock: Any in docks {
        (dock as? NSRunningApplication)?.terminate()
      }
    }
    exit(0)
  }
