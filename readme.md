# Image Spectrumizer

![ScreenShot](https://raw.github.com/jarikomppa/img2spec/master/img2spec2.jpg)

This is a GUI tool to help convert normal images suitable to be used in ZX Spectrum (and similar devices).
Note that the resolution is 256x192. Feeding the tool smaller or bigger images works, but the internal
image size remains the same.

Instead of trying to be completely automated, this tool lets you interactively adjust the source image
while letting you see the results in real time. Any number of modifiers can be stacked on top of each other,
and their order can be changed.

Output can be saved as a PNG, or in raw, C header, or assembler include file.

## Binaries

http://iki.fi/sol/zip/img2spec_20.zip

## Source

https://github.com/jarikomppa/img2spec

##Tips:

- The window can be resized.
- Various helper windows can be opened from the window menu.
- Don't hesistate to play with conversion options.

##Typical workflow:

1. Load an image (file->load image)
2. Add modifiers (modifiers->...)
3. (optionally) open options (window->options) and change conversion options
4. Tweak modifiers until result is acceptable
5. Export result (file->export ...)

##Image editor interopration
If you keep the image file open in image spectrumizer and the image editor of your 
choise (such as photoshop), image spectrumizer can detect when the file has changed 
and reloads the image automatically. This way you can keep editing the source image 
with (potentially) better tools than image spectrumizer can offer, and see the 
results relatively quickly.

This feature is enabled by default, and can be disabled from the options.

##File formats:

The file formats for scr, h and inc are basically the same. Bitmap (in spectrum screen 
order) is followed by attribute data. In case of non-standard cell sizes, the attribute 
data is bigger, but still in linear order - it is up to the code that uses the data to 
figure out how to use it.

If you prefer the data to be in linear order (not in spectrum screen order), you can 
change that from the options.

.scr is a binary format, basically memory dump of the spectrum screen.

.h is C array, only data is included, so typical use would be:

```
    const myimagedata[]= {
    #include "myimagedata.h"
    };
```

.inc is assember .db lines.

##Command line arguments

Any image or workspace filenames given as arguments are loaded (in the order given). If 
multiple loadable images or workspaces are given, the last ones are the ones that are used. 

(Well, all of them do get loaded, they just replace the earlier ones in memory).

To save results on commandline, use the following flags:
- -p pngfilename.png
- -h headerfilename.h
- -i incfilename.inc
- -s scrfilename.scr

Example:

img2spec cat.png mush.isw -h cat.h

Output files are overwritten without a warning, but I trust you know what you're doing.

##3x64 mode
In this mode, two sets of attributes are calculated, and the application is expected to 
swap between the two every frame. This creates approximately 3*64 colors (due to double 
blacks, the actual number of colors is quite much lower). The effect flickers on emulators 
but works "fine" on CRTs. (Why 3x64 and not 192? because there's three sets of (about) 64 
color palettes, and each cell can use 2 from one of those).

Note that the color reproduction results in real hardware will differ from the simulated, 
depending on the capabilities of the display device, and may be a flickering nightmare. 

# License

Copyright (c) 2015 Jari Komppa, http://iki.fi/sol

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
claim that you wrote the original software. If you use this software
in a product, an acknowledgement in the product documentation would be
appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

