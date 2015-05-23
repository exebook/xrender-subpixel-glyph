#XRender subpixel glyph hello world.

This is a modification of FreeType+XRender example I found on GitHub.
Original example did not utilize **subpixel** rendering. I added it here.
I cannot credit the original author, because the same example is found in many repositories and there is no person name anywhere.

The trick is to use ARGB32. which was suggested by Andrey Sidorov [here on StackOVerflow][1].

[1]:http://stackoverflow.com/questions/30399674/how-to-subpixel-render-glyphs-with-xrender-extension/30409074#30409074

