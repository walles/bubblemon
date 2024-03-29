# `icon.png` and `icon.png.icns`

The `icon.png.icns` and the `icon.png` files are both based on the `icon.blend`
file.

To modify `icon.blend`, get Blender at http://www.blender.org/download.

To recreate `icon.png` from `icon.blend`:
1. Open `icon.blend` in Blender.
2. On the Render menu, do Render Image.
3. On the Image menu (it's half way down the screen), do Save as Image.
4. Double click on `icon.png` to save over it.

To recreate `icon.png.icns` from `icon.png`, use https://iconverticons.com/online/.

Please commit your changes to both `icon.blend`, `icon.png` and `icon.png.icns` in
the same commit.

# `macbookpro-with-bubblemon.png`

`macbookpro-with-bubblemon.png` has been done in [GIMP](https://gimp.org/) based on a screenshot, a
Touch Bar screenshot, and a photo of my Mac. GIMP XCF file and the other source
files can all be found in the `macbook-pro-with-bubblemon` directory.

After mixing the images in GIMP, I exported a `png`, and then compressed that
further (and interlaced it for an improved browser experience) using [pngquant](https://pngquant.org/) and [ImageMagick](https://imagemagick.org/):
```sh
pngquant --speed 1 --force pixmaps/macbookpro-with-bubblemon.png
magick mogrify -interlace PNG pixmaps/macbookpro-with-bubblemon-fs8.png
mv pixmaps/macbookpro-with-bubblemon-fs8.png pixmaps/macbookpro-with-bubblemon.png
```

# Notes
* The `dmg-preview.png` file is used in `dmg-background.blend` to make it
obvious when using Blender where the icons will be.
* `dmg-background.png` is generated from `dmg-background.blend`.
