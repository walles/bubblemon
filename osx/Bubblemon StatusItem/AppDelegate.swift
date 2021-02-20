import Cocoa

@NSApplicationMain
class AppDelegate: NSObject, NSApplicationDelegate {
  private var statusItem: NSStatusItem?
  private let _bubblemon: UnsafeMutablePointer<bubblemon_t> = bubblemon_init()
  private var _picture: UnsafePointer<bubblemon_picture_t>? = nil

  @IBOutlet weak var menu: NSMenu?

  func applicationDidFinishLaunching(_ aNotification: Notification) {
    #if DEBUG
        bubblemon_selftest()
    #endif

    // Inspiration: https://www.appcoda.com/macos-status-bar-apps/
    statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
    statusItem?.button?.title = "Johan"

    if let menu = menu {
      statusItem?.menu = menu
    }

    // Inspired by http://stackoverflow.com/questions/1449035/how-do-i-use-nstimer
    Timer.scheduledTimer(
      timeInterval: (1.0 / 10.0),
      target: self,
      selector: #selector(self.timerTriggered),
      userInfo: nil,
      repeats: true)
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

  @IBAction func showLoadNumbers(_ sender: Any) {
    let alert = NSAlert()

    alert.messageText = String(cString: bubblemon_getTooltip(_bubblemon)!)

    alert.alertStyle = NSAlert.Style.informational
    alert.addButton(withTitle: "OK")

    // This might freeze the physics, but give us the numbers. Call it an MVP!
    alert.runModal()
  }

  @objc
  func timerTriggered() {
    // Compute a new image to display

    // FIXME: What should these be?
    let width = 50
    let height = 50

    // FIXME: Use the right size here, whatever that is
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
    let shouldInterpolate: Bool = false
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
      shouldInterpolate: shouldInterpolate,
      intent: CGColorRenderingIntent.defaultIntent)

    let fixmeWhatShouldThisBe = NSSize.init(width: width, height: height);
    statusItem?.button?.image = NSImage.init(cgImage: cgImage!, size: fixmeWhatShouldThisBe);
  }
}

func releaseDataProvider(_ context:UnsafeMutableRawPointer?, data:UnsafeRawPointer, size:Int) {
  // No need to actually free anything here, the data is handled by
  // the bubblemon code.
}
