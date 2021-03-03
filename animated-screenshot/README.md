# Instructions for recreating

1. Apply the [bubblemon-recording-diff.diff](bubblemon-recording-diff.diff) to the
   source code.
1. Launch the `BubblemonWindow` target.
1. Record the 50x50 Bubblemon image. Use screen zooming to position the
   recording bounds correctly. You now have [a 100x100 clip](bubblemon-27s.mov).
   Why not 50x50? I don't know, this is what I got.
1. In [Blender](https://blender.org), or some other video editor of your choice,
   set output size to 100x100.
1. Cut the clip in the middle and switch places between the parts. Overlap the
   ends a bit and cross fade them. This gets you a loopable clip. When I saved I
   got [this](bubblemon-27s-crossfaded.mp4).
1. Inspired by [this Super User answer](https://superuser.com/a/556031/138661):
   ```
   ffmpeg -i animated-screenshot/bubblemon-27s-crossfaded.mp4 -vf "scale=50:50,fps=10,split[s0][s1];[s0]palettegen[p];[s1][p]paletteuse" -sws_flags neighbor animated-screenshot/bubblemon.gif
   ```
   This turns the `.mp4` into a 250kB, 50x50 `.gif`. Using Nearest Neighbor for
   scaling actually restores the 100x100 image to the 50x50 it should already
   have been, any fancier scaling algorithm than that will likely just introduce
   blurriness / artifacts.
