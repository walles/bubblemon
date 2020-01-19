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

    NSLog("Killing com.apple.dock.extra to force it to unload the old Bubblemon...")
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

  // We used to do .resolvingSymlinksInPath() on the path before returning it here,
  // but it turns out that actually *added* a symlink to the path:
  // * URL: file:///private/tmp/Bubblemon.app/
  // * Path: /private/tmp/Bubblemon.app
  // * Resolved path: /tmp/Bubblemon.app
  //
  // Since the value here is supposedly something we put here ourselves, we should
  // just go with that anyway. Don't resolve any paths here!!
  //
  // //NSLog("URL: %@, Path: %@, Resolved path: %@", url!.absoluteString, url!.path, url!.resolvingSymlinksInPath().path)
  return url!.path
}

private func launchActivityMonitor() {
  NSLog("Launching Activity Monitor...")
  let launched = NSWorkspace.shared.launchApplication(
    withBundleIdentifier: "com.apple.ActivityMonitor",
    options: .default,
    additionalEventParamDescriptor: nil,
    launchIdentifier: nil)

  if !launched {
    NSLog("Launching Activity Monitor failed")
  }
}

func isTranslocated() -> Bool {
  let appPath = URL(string: Bundle.main.bundlePath)!.resolvingSymlinksInPath().path

  // Example translocated path:
  // /private/var/folders/04/34ltylqn5yd9scvlrwjgmy580000gn/T/AppTranslocation/6E0A03F4-21EC-4559-87DF-77CB0DD7DB20/d/Bubblemon.app

  // Does our path say "AppTranslocation"?
  return appPath.range(of:"AppTranslocation") != nil
}

func showTranslocationWarning() {
  let alert = NSAlert()
  alert.messageText = "Move to Applications before running"
  alert.informativeText =
    "Please move Bubblemon into your Applications folder before running.\n\n" +
    "" +
    "Otherwise, because of App Translocation / Gatekeeper Path Randomization, " +
    "Bubblemon will run in a random path every time, and after your computer " +
    "reboots the Dock won't be able to find it any more."
  alert.alertStyle = NSAlert.Style.warning
  alert.addButton(withTitle: "OK")

  alert.runModal()
}

/** Returns the Dock launch timestamp, in seconds since the Epoch */
func getDockLaunchTimestamp() -> Double? {
  FIXME: This method does not work, see LaunchServices comment below

  // There should be only one of these
  let docks = NSRunningApplication.runningApplications(withBundleIdentifier: "com.apple.dock")

  for dock in docks {
    // FIXME: I get nil here.
    //
    // The docs say that: "This property is only available for applications that
    // were launched by LaunchServices".
    //
    // According to px:
    // kernel(0)        root
    //   launchd(1)     root
    // --> Dock(92561)  johan
    //
    // Apparently launchd and LaunchServices are two different things:
    // https://eclecticlight.co/2018/05/22/running-at-startup-when-to-use-a-login-item-or-a-launchagent-launchdaemon/
    //
    // See also "px launch"; which will show you both launchd and
    // launchservicesd, demonstrating they are different.
    let launchTimestamp = dock.launchDate?.timeIntervalSince1970
    if launchTimestamp == nil {
      NSLog("Unable to get launch timestamp from Dock")
      continue
    }

    return launchTimestamp!
  }

  return nil
}

/** Return when this build was made, in seconds since the Epoch */
func getBuildTimestamp() -> Double {
  let bundle = Bundle(for: BubblemonView.self)
  let buildTimestamp = bundle.infoDictionary?["BuildTimestamp"] as? Double
  return buildTimestamp!
}

/**
Given that we restart the Dock to install ourselves, if
the Dock was started before our build timestamp it means
that any Bubblemon running there is older than us.
*/
func isDockOlderThanUs() -> Bool? {
  let dockLaunchTimestamp = getDockLaunchTimestamp()
  if dockLaunchTimestamp == nil {
    return nil
  }

  return dockLaunchTimestamp! < getBuildTimestamp()
}

/**
If there are old Bubblemons around, remove them. "Old" in this case refers
to them being from another version.
*/
func removeOldBubblemon() {
  let defaults = UserDefaults.standard
  let runningBubblemonPath = defaults.getRunningBubblemonPath()
  if runningBubblemonPath == nil {
    // No Bubblemon running, nothing to remove
    return
  }

  let appPath = URL(string: Bundle.main.bundlePath)!.resolvingSymlinksInPath().path
  if runningBubblemonPath!.caseInsensitiveCompare(appPath) != .orderedSame {
    NSLog("Removing old Bubblemon with different path: %@", runningBubblemonPath!)

    if !defaults.removeApplication(fromDock: runningBubblemonPath!) {
      NSLog("Removing old bubblemon failed")
    }

    return
  }

  let dockIsOld = isDockOlderThanUs()
  if dockIsOld == nil {
    // Who knows? Do nothing to be safe.
    return
  }

  if dockIsOld! {
    NSLog("Removing Bubblemon started before we were built")

    if !defaults.removeApplication(fromDock: runningBubblemonPath!) {
      NSLog("Removing old bubblemon failed")
    }
  }
}

func main() -> Int32 {
  if (isTranslocated()) {
    // See: http://lapcatsoftware.com/articles/app-translocation.html
    // Or search the Internet for "Gatekeeper Path Randomization"
    showTranslocationWarning()

    return EXIT_FAILURE
  }

  removeOldBubblemon()

  let appPath = URL(string: Bundle.main.bundlePath)!.resolvingSymlinksInPath().path
  let defaults = UserDefaults.standard

  if defaults.dockHasApplication(appPath) {
    NSLog("Bubblemon already installed in the Dock")
    launchActivityMonitor()
    return EXIT_SUCCESS
  }

  NSLog("Not found, installing: %@", appPath)
  // Add ourselves to the dock
  if !defaults.addApplication(toDock: appPath) {
    NSLog("Adding ourselves to the Dock failed, bailing...")
    return EXIT_FAILURE
  }

  NSLog("Killing Dock to force it to reload its new Bubblemon-enabled configuration...")
  let docks = NSRunningApplication.runningApplications(withBundleIdentifier: "com.apple.dock")
  for dock in docks {
    dock.terminate()
  }

  return EXIT_SUCCESS
}

exit(main())
