import Cocoa

private func createContext(width: size_t, height: size_t) -> CGContext? {
  let bytesPerRow = width * 4

  // rgb alpha
  let rgb = CGColorSpaceCreateDeviceRGB()
  let zBitmapContextRef = CGContext(data: nil,
                                    width: width,
                                    height: height,
                                    bitsPerComponent: 8,
                                    bytesPerRow: bytesPerRow,
                                    space: rgb,
                                    bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue)
  return zBitmapContextRef
}

class BubblemonView: NSView, NSDockTilePlugIn {
  private var _bubblemon: UnsafeMutablePointer<bubblemon_t>
  private var _picture: UnsafePointer<bubblemon_picture_t>?
  private var _dockTile: NSDockTile?
  private var _dockMenu: NSMenu?
  private var _windowFrame: CGImage?
  private var _scaledWindowFrame: CGImage?

  func setDockTile(_ dockTile: NSDockTile?) {
    if dockTile != nil {
      _dockTile = dockTile
      _dockTile!.contentView = self
    }
  }

  func dockMenu() -> NSMenu? {
    Swift.print("Setting up dock menu\n")
    if _dockMenu != nil {
      return _dockMenu
    }

    let menu = NSMenu()
    let helpItem = NSMenuItem(title: "Help", action: #selector(self.openLegend), keyEquivalent: "")
    helpItem.target = self
    menu.addItem(helpItem)
    let aboutItem = NSMenuItem(title: "About", action: #selector(self.openAboutPanel), keyEquivalent: "")
    aboutItem.target = self
    menu.addItem(aboutItem)
    let shareItem = NSMenuItem(title: "Share on Facebook", action: #selector(self.shareOnFacebook), keyEquivalent: "")
    shareItem.target = self
    menu.addItem(shareItem)

    _dockMenu = menu
    return _dockMenu
  }

  @IBAction func openLegend(_ sender: Any) {
    Swift.print("Opening help in browser...\n")
    // From: http://lists.apple.com/archives/xcode-users/2016/Feb/msg00111.html
    let url = URL(string: "http://walles.github.io/bubblemon/")
    NSWorkspace.shared().open(url!)
  }

  @IBAction func shareOnFacebook(_ sender: Any) {
    Swift.print("Opening browser to share on Facebook...\n")
    // From: http://lists.apple.com/archives/xcode-users/2016/Feb/msg00111.html
    let url = URL(string: "https://www.facebook.com/sharer/sharer.php?u=http%3A//walles.github.io/bubblemon/")
    NSWorkspace.shared().open(url!)
  }

  @IBAction func openAboutPanel(_ sender: Any) {
    let credits = NSAttributedString(
      string: "http://walles.github.io/bubblemon",
      attributes: [
        NSLinkAttributeName: "http://walles.github.io/bubblemon"
      ]
    )

    // The Git hash and version get filled in by a "Run Script" build step
    let bundle = Bundle(for: BubblemonView.self)
    let icon = bundle.image(forResource: "icon.png")
    let gitHash = bundle.infoDictionary?["GitHash"] as? String
    let gitDescribe = bundle.infoDictionary?["GitDescribe"] as? String
    let copyright = bundle.infoDictionary?["NSHumanReadableCopyright"] as? String
    let aboutOptions: [String: Any] = [
      "Credits": credits,
      "ApplicationName": "Bubblemon",
      "ApplicationIcon": icon!,
      "Version": gitHash!,
      "Copyright": copyright!,
      "ApplicationVersion": gitDescribe!
    ]
    NSApplication.shared().activate(ignoringOtherApps: true)
    NSApplication.shared().orderFrontStandardAboutPanel(options: aboutOptions)
  }

  override init(frame: NSRect) {
    // Initialization code here.
#if DEBUG
    bubblemon_selftest()
#endif
    _bubblemon = bubblemon_init()
    bubblemon_setColors(_bubblemon, 0x75ceff00, 0x0066ff80, 0xff333340, 0xaa000080, 0x00ff0080, 0xffff40ff)
    _picture = nil
    // Load window frame graphics
    let bundle = Bundle(for: BubblemonView.self)
    let windowFrameUrl = bundle.urlForImageResource("window-frame")
    let windowFrameNsUrl = NSURL(string: (windowFrameUrl?.absoluteString)!)

    let dataProvider = CGDataProvider(url: windowFrameNsUrl!)
    _windowFrame = CGImage(
      pngDataProviderSource: dataProvider!,
      decode: nil,
      shouldInterpolate: false,
      intent: CGColorRenderingIntent.defaultIntent)

    super.init(frame: frame)

    // Inspired by http://stackoverflow.com/questions/1449035/how-do-i-use-nstimer
    Timer.scheduledTimer(
      timeInterval: (1.0 / 10.0),
      target: self,
      selector: #selector(self.timerTriggered),
      userInfo: nil,
      repeats: true)
  }

  required init?(coder: NSCoder) {
    fatalError("init(coder:) has not been implemented")
  }

  func getCachedWindowFrame() -> CGImage {
    let width = Int(round(bounds.size.width))
    let height = Int(round(bounds.size.height))

    // Can we use our cached window frame?
    if _scaledWindowFrame == nil {
      // No; we don't have one
    } else if _scaledWindowFrame!.width != width {
      // No; the width doesn't match
    } else if _scaledWindowFrame!.height != height {
      // No; the height doesn't match
    } else {
      // Yes!
      return _scaledWindowFrame!
    }

    let scaledContext = createContext(width: width, height: height)
    let rect = CGRect(x: 0, y: 0, width: width, height: height)
    scaledContext!.draw(_windowFrame!, in: rect)
    _scaledWindowFrame = scaledContext!.makeImage()
    return _scaledWindowFrame!
  }

  func timerTriggered() {
    // Compute a new image to display
    // The Dock won't tell us its size, so this is a guess at roughly how many pixels
    // the bubblemon will get on screen.
    bubblemon_setSize(_bubblemon, 50, 50)
    _picture = bubblemon_getPicture(_bubblemon)
    let tooltip = String(utf8String: bubblemon_getTooltip(_bubblemon))
    toolTip = tooltip
    if _dockTile != nil {
      _dockTile!.display()
    } else {
      needsDisplay = true
    }
  }

  override func draw(_ dirtyRect: NSRect) {
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

    let nsGraphicsContext = NSGraphicsContext.current()
    guard let cgContext = nsGraphicsContext?.cgContext else {
      fatalError("unable to get a CGContext, can't draw")
    }

    // Draw the bubblemon image
    let bubbleViewRect = CGRect(x: bounds.size.width * 0.08, y: bounds.size.height * 0.08, width: bounds.size.width * 0.84, height: bounds.size.height * 0.84)
    cgContext.setAlpha(1.0)
    cgContext.interpolationQuality = .none
    cgContext.draw(cgImage!, in: bubbleViewRect)
    // Draw the window frame
    let fullSizeRect = NSRectToCGRect(bounds)
    cgContext.setAlpha(1.0)
    cgContext.draw(getCachedWindowFrame(), in: fullSizeRect)
  }
}

func releaseDataProvider(_ context:UnsafeMutableRawPointer?, data:UnsafeRawPointer, size:Int) {
  // No need to actually free anything here, the data is handled by
  // the bubblemon code.
}
