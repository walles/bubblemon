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

private func releaseDataProvider(info: Void, data: Void, size: size_t) {
  // No need to actually free anything here, the data is handled by
  // the bubblemon code.
}

class BubblemonView: NSView, NSDockTilePlugIn {
  private var bubblemon: bubblemon_t?
  private let picture: bubblemon_picture_t?
  private var dockTile: NSDockTile?
  private var dockMenu: NSMenu?
  private var windowFrame: CGImageRef = nil
  private var scaledWindowFrame: CGImageRef = nil

  func setDockTile(_ dockTile: NSDockTile?) {
    if aDockTile {
      _dockTile = aDockTile
      _dockTile.contentView = self
    }
  }

  func dockMenu() -> NSMenu? {
    print("Setting up dock menu\n")
    if _dockMenu != nil {
      return _dockMenu
    }
    _dockMenu = NSMenu()
    let helpItem = NSMenuItem(title: "Help", action: #selector(self.openLegend), keyEquivalent: "")
    helpItem.target = self
    _dockMenu.addItem(helpItem)
    let aboutItem = NSMenuItem(title: "About", action: #selector(self.openAboutPanel), keyEquivalent: "")
    aboutItem.target = self
    _dockMenu.addItem(aboutItem)
    let shareItem = NSMenuItem(title: "Share on Facebook", action: #selector(self.shareOnFacebook), keyEquivalent: "")
    shareItem.target = self
    _dockMenu.addItem(shareItem)
    return _dockMenu
  }

  @IBAction func openLegend(_ sender: Any) {
    print("Opening help in browser...\n")
    // From: http://lists.apple.com/archives/xcode-users/2016/Feb/msg00111.html
    let url = URL(string: "http://walles.github.io/bubblemon/")
    NSWorkspace.shared().open(url!)
  }

  @IBAction func share(onFacebook sender: Any) {
    print("Opening browser to share on Facebook...\n")
    // From: http://lists.apple.com/archives/xcode-users/2016/Feb/msg00111.html
    let url = URL(string: "https://www.facebook.com/sharer/sharer.php?u=http%3A//walles.github.io/bubblemon/")
    NSWorkspace.shared().open(url!)
  }

  @IBAction func openAboutPanel(_ sender: Any) {
    let credits = NSAttributedString(string: "http://walles.github.io/bubblemon", attributes: [NSLinkAttributeName: "http://walles.github.io/bubblemon"])
    // The Git hash and version get filled in by a "Run Script" build step
    let bundle = Bundle(for: BubblemonView)
    let icon: NSImage? = bundle.image(forResource: "icon.png")
    let gitHash: String? = bundle.infoDictionary["GitHash"]
    let gitDescribe: String? = bundle.infoDictionary["GitDescribe"]
    let copyright: String? = bundle.infoDictionary["NSHumanReadableCopyright"]
    let aboutOptions: [AnyHashable: Any] = ["Credits": credits, "ApplicationName": "Bubblemon", "ApplicationIcon": icon, "Version": gitHash, "Copyright": copyright, "ApplicationVersion": gitDescribe]
    NSApplication.shared().activate(ignoringOtherApps: true)
    NSApplication.shared().orderFrontStandardAboutPanel(options: aboutOptions as? [String : Any] ?? [String : Any]())
  }

  override init(frame frameRect: NSRect) {
    super.init(frame: frame)

    // Initialization code here.
f DEBUG
    bubblemon_selftest()
ndif
    bubblemon = bubblemon_init()
    bubblemon_setColors(bubblemon, 0x75ceff00, 0x0066ff80, 0xff333340, 0xaa000080, 0x00ff0080, 0xffff40ff)
    picture = nil
    // Inspired by http://stackoverflow.com/questions/1449035/how-do-i-use-nstimer
    Timer.scheduledTimer(timeInterval: (1.0 / 10.0), target: self, selector: #selector(self.timerTriggered), userInfo: nil, repeats: true)
    // Load window frame graphics
    let bundle = Bundle(for: BubblemonView)
    let windowFrameUrl: CFURLRef? = (bundle.urlForImageResource("window-frame") as? CFURLRef)
    let dataProvider: CGDataProviderRef = CGDataProviderCreateWithURL(windowFrameUrl)
    windowFrame = CGImageCreateWithPNGDataProvider(dataProvider, nil, false, CGColorRenderingIntent.defaultIntent)
    CGDataProviderRelease(dataProvider)

  }

  func getCachedWindowFrame() -> CGImageRef {
    let width: size_t = bounds.size.width
    let height: size_t = bounds.size.height
    if scaledWindowFrame != nil && CGImageGetWidth(scaledWindowFrame) == width && CGImageGetHeight(scaledWindowFrame) == height {
      return scaledWindowFrame
    }
    if scaledWindowFrame != nil {
      CGImageRelease(scaledWindowFrame)
    }
    let scaledContext: CGContext? = createContext(width, height)
    let rect = CGRect(x: 0, y: 0, width: width as? CGFloat ?? 0.0, height: height as? CGFloat ?? 0.0)
    scaledContext.draw(in: windowFrame, image: rect)
    scaledWindowFrame = scaledContext.makeImage()
    CGContextRelease(scaledContext)
    return scaledWindowFrame
  }

  func timerTriggered() {
    // Compute a new image to display
    // The Dock won't tell us its size, so this is a guess at roughly how many pixels
    // the bubblemon will get on screen.
    bubblemon_setSize(bubblemon, 50, 50)
    picture = bubblemon_getPicture(bubblemon)
    let tooltip = String(utf8String: bubblemon_getTooltip(bubblemon))
    toolTip = tooltip
    if dockTile != nil {
      dockTile?.display()
    }
    else {
      needsDisplay = true
    }
  }

  override func draw(_ dirtyRect: NSRect) {
    if picture == nil {
      return
    }
    // Redraw ourselves
    let imageDataSize: size_t? = picture?.width * picture?.height * MemoryLayout<bubblemon_color_t>.size
    let dataProviderRef: CGDataProviderRef? = CGDataProviderCreateWithData(nil, picture?.pixels, imageDataSize, releaseDataProvider)
    let bitsPerComponent: size_t = 8
    let bitsPerPixel: size_t = MemoryLayout<bubblemon_color_t>.size * 8
    let bytesPerRow: size_t? = picture?.width * MemoryLayout<bubblemon_color_t>.size
    let shouldInterpolate: Bool = false
    let rgb: CGColorSpace? = CGColorSpaceCreateDeviceRGB()
    let cgImageRef: CGImageRef? = CGImageCreate(picture?.width, picture?.height, bitsPerComponent, bitsPerPixel, bytesPerRow, rgb, (kCGImageAlphaNoneSkipLast as? CGBitmapInfo), dataProviderRef, nil, shouldInterpolate, CGColorRenderingIntent.defaultIntent)
    CGDataProviderRelease(dataProviderRef)
    let nsGraphicsContext = NSGraphicsContext.current()
    let cgContextRef: CGContext?? = (nsGraphicsContext?.graphicsPort as? CGContext?)
    // Draw the bubblemon image
    let bubbleViewRect = CGRect(x: bounds.size.width * 0.08, y: bounds.size.height * 0.08, width: bounds.size.width * 0.84, height: bounds.size.height * 0.84)
    cgContextRef.setAlpha(1.0)
    CGContextSetInterpolationQuality(cgContextRef, kCGInterpolationNone)
    cgContextRef.draw(in: cgImageRef, image: bubbleViewRect)
    CGImageRelease(cgImageRef)
    // Draw the window frame
    let fullSizeRect = NSRectToCGRect(bounds)
    cgContextRef.setAlpha(1.0)
    cgContextRef.draw(in: getCachedWindowFrame(), image: fullSizeRect)
  }
}
