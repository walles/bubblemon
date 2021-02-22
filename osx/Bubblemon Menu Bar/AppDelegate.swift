import Cocoa

// OK: Make sure we don't show up in the alt-Tab list
// OK: Make sure we can (still) run in the Xcode debugger
// OK: Make sure people get an up-to-date tooltip on hover
// OK: Make sure the menu has: Help, About, Share on Facebook, Quit
// OK: Make sure the menu items work as expected: OK: About, OK: Share on Facebook, OK: Help, OK: Quit
// OK: Make sure launching externally doesn't activate the app
// OK: Make sure the application bundle has the right icon
// OK: Switch to a better name
// OK: Make sure we survive logout / login
// OK: Remove launch agent config file when user does Quit in the menu
// OK: Verify that we survive power-off / power-on
// OK: Add ourselves to the install script
// FIXME: Add ourselves to the web pages
// FIXME: Add ourselves to the README.md
// FIXME: Consider improving the names of the other Bubblemons as well?
// FIXME: After all points are done ^, remove this whole list
// FIXME: Figure out whether we can make the animation continue even when Bubblmon has been clicked and the menu is visible. Setting the alternateImage does not help.

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  private var statusItem: NSStatusItem?
  private let _bubblemon: UnsafeMutablePointer<bubblemon_t> = bubblemon_init()
  private var _picture: UnsafePointer<bubblemon_picture_t>? = nil

  private let LAUNCH_AGENT_PLIST_PATH = "~/Library/LaunchAgents/com.gmail.walles.johan.Bubblemon.plist"

  @IBOutlet weak var menu: NSMenu?

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    #if DEBUG
        bubblemon_selftest()
    #endif

    installLaunchAgent()

    // Inspiration: https://www.appcoda.com/macos-status-bar-apps/
    statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)

    if let menu = menu {
      statusItem?.menu = menu
    }
    statusItem?.button?.isBordered = false
    statusItem?.button?.imageScaling = NSImageScaling.scaleNone

    // Inspired by http://stackoverflow.com/questions/1449035/how-do-i-use-nstimer
    Timer.scheduledTimer(
      timeInterval: (1.0 / 10.0),
      target: self,
      selector: #selector(self.timerTriggered),
      userInfo: nil,
      repeats: true)
  }

  private func installLaunchAgent() {
    // Figure out absolute path to $0
    let bubblemonBinRelativePath = CommandLine.arguments[0]
    let bubblemonBinFileUrl = URL.init(fileURLWithPath: bubblemonBinRelativePath)
    let bubblemonBinAbsolutePath = bubblemonBinFileUrl.absoluteURL.path

    // From "man launchd.plist" and
    // https://rderik.com/blog/creating-a-launch-agent-that-provides-an-xpc-service-on-macos/
    let launchAgentPlistContents = """
      <?xml version="1.0" encoding="UTF-8"?>
      <!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
      <plist version="1.0">
        <dict>
          <key>Label</key>
            <string>com.gmail.walles.johan.Bubblemon</string>
          <key>RunAtLoad</key>
            <true/>
          <key>Program</key>
            <string>\(bubblemonBinAbsolutePath)</string>
        </dict>
      </plist>
      """

    // Save the file to where launchd can find it
    let launchAgentPlistPath =
      NSString(string: LAUNCH_AGENT_PLIST_PATH).expandingTildeInPath
    do {
      try launchAgentPlistContents.write(
        toFile: launchAgentPlistPath,
        atomically: true,
        encoding: String.Encoding.utf8)
    } catch {
      NSLog("Writing launch agent plist file failed: \(launchAgentPlistPath): \(error)")
      return
    }
    NSLog("Launch agent plist file written: \(launchAgentPlistPath)")
  }

  @IBAction func shutDown(_ sender: Any) {
    // Don't restart after logout / login any more after the user removed us
    let launchAgentPlistPath =
      NSString(string: LAUNCH_AGENT_PLIST_PATH).expandingTildeInPath
    do {
      try FileManager().removeItem(atPath: launchAgentPlistPath)
    } catch {
      NSLog("Removing launch agent plist file failed: \(launchAgentPlistPath): \(error)")
    }
    NSLog("Launch agent plist file removed: \(launchAgentPlistPath)")

    // Good bye
    NSRunningApplication.current.terminate()
  }

  @IBAction func openLegend(_ sender: Any) {
    NSLog("Opening help in browser...")
    // From: http://lists.apple.com/archives/xcode-users/2016/Feb/msg00111.html
    let url = URL(string: "https://walles.github.io/bubblemon/")
    NSWorkspace.shared.open(url!)
  }

  @IBAction func shareOnFacebook(_ sender: Any) {
    NSLog("Opening browser to share on Facebook...")
    // From: http://lists.apple.com/archives/xcode-users/2016/Feb/msg00111.html
    let url = URL(string: "https://www.facebook.com/sharer/sharer.php?u=http%3A//walles.github.io/bubblemon/")
    NSWorkspace.shared.open(url!)
  }

  @IBAction func openAboutPanel(_ sender: Any) {
    let credits = NSAttributedString(
      string: "https://walles.github.io/bubblemon",
      attributes: [
        NSAttributedString.Key.link: "https://walles.github.io/bubblemon"
      ]
    )

    // The Git hash and version get filled in by a "Run Script" build step
    let bundle = Bundle(for: AppDelegate.self)
    let version = bundle.infoDictionary?["CFBundleShortVersionString"] as? String
    let gitHash = bundle.infoDictionary?["GitHash"] as? String
    let icon = bundle.image(forResource: "icon.png.icns")
    let aboutOptions: [NSApplication.AboutPanelOptionKey: Any] = [
      NSApplication.AboutPanelOptionKey.applicationName: "Bubblemon",
      NSApplication.AboutPanelOptionKey.applicationVersion: version!,
      NSApplication.AboutPanelOptionKey.version: gitHash!,
      NSApplication.AboutPanelOptionKey.applicationIcon: icon!,
      NSApplication.AboutPanelOptionKey.credits: credits,
    ]
    NSApplication.shared.activate(ignoringOtherApps: true)
    NSApplication.shared.orderFrontStandardAboutPanel(options: aboutOptions)
  }

  @objc
  func timerTriggered() {
    // Compute a new image to display
    if statusItem == nil {
      return
    }

    // Image size in points. On high res displays, one point is 2x2 pixels.
    let height = statusItem!.button!.bounds.height
    let width = 2 * height

    bubblemon_setSize(_bubblemon, Int32(width), Int32(height))
    let _picture = bubblemon_getPicture(_bubblemon)
    if _picture == nil {
      return
    }
    let picture = _picture!.pointee

    // Redraw ourselves
    let imageDataSize = Int(picture.width * picture.height) * MemoryLayout<bubblemon_color_t>.size
    let dataProviderRef = CGDataProvider(
      dataInfo: nil,
      data: picture.pixels,
      size: imageDataSize,
      releaseData: releaseDataProvider)

    let bitsPerComponent: size_t = 8
    let bitsPerPixel = MemoryLayout<bubblemon_color_t>.size * 8
    let bytesPerRow = Int(picture.width) * MemoryLayout<bubblemon_color_t>.size
    let rgb = CGColorSpaceCreateDeviceRGB()
    let cgImage = CGImage(
      width: Int(picture.width), height: Int(picture.height),
      bitsPerComponent: bitsPerComponent,
      bitsPerPixel: bitsPerPixel,
      bytesPerRow: bytesPerRow,
      space: rgb,
      bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.noneSkipLast.rawValue),
      provider: dataProviderRef!,
      decode: nil,
      shouldInterpolate: false,
      intent: CGColorRenderingIntent.defaultIntent)

    let newImageSize = NSSize.init(width: width, height: height)
    let newImage = NSImage.init(cgImage: cgImage!, size: newImageSize)

    // Make it visible if we don't cover everything
    newImage.backgroundColor = NSColor.green

    statusItem?.button?.image = newImage
    statusItem?.toolTip = String(utf8String: bubblemon_getTooltip(_bubblemon))
  }
}

func releaseDataProvider(_ context:UnsafeMutableRawPointer?, data:UnsafeRawPointer, size:Int) {
  // No need to actually free anything here, the data is handled by
  // the bubblemon code.
}
