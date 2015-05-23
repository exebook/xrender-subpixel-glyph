/* Demonstration of SUBPIXEL Xrender-based text rendering
  compile with:
    clang(or gcc) `pkg-config --cflags --libs freetype2 xrender` -o rendertext rendertext.c
    Subpixel handling is done by Yakov Sudeikin-Nivin.
*/

#define FONT_PATH "elfu.ttf"
#define TXT "hello subpixel world!"

#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>
#include "ft2build.h"
#include FT_FREETYPE_H

typedef unsigned char byte;
void freetype2xrender(byte *b, byte *a, int W, int H) {
    byte *end = b + (W * H * 4);
    int x = 0;
    int z = W & 3;
    while(1) {
        if (b >= end) break;
        b[0] = a[2]; // reverse RGB
        b[1] = a[1];
        b[2] = a[0];
        b[3] = 0;
        a += 3;
        b += 4;
        x++;
        if (x == W) {
            x = 0;
            a += z;
        }
    }
}

void load_glyph(Display *display, GlyphSet gs, FT_Face face, int charcode)
{
	Glyph gid;
	XGlyphInfo ginfo;
	
	int glyph_index=FT_Get_Char_Index(face, charcode);
	FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER
		|FT_LOAD_TARGET_LCD
		|FT_LOAD_PEDANTIC
		|FT_LOAD_FORCE_AUTOHINT
	);
	FT_Bitmap *bitmap=&face->glyph->bitmap;
	ginfo.x=-face->glyph->bitmap_left;
	ginfo.y=face->glyph->bitmap_top;
	ginfo.width=bitmap->width/3;
	ginfo.height=bitmap->rows;
	ginfo.xOff=face->glyph->advance.x/64;
	ginfo.yOff=face->glyph->advance.y/64;
	gid=charcode;
	int count = ginfo.width*ginfo.height*4;
	byte *A = (byte*)malloc(count);
	freetype2xrender(A, bitmap->buffer, ginfo.width, ginfo.height);
	XRenderAddGlyphs(display, gs, &gid, &ginfo, 1, (const char*)A, count);
	free(A);
	XSync(display, 0);
}

GlyphSet load_glyphset(Display *display, char *filename, int size)
{
	static int ft_lib_initialized=0;
	static FT_Library library;
	int n;
	XRenderPictFormat *fmt = XRenderFindStandardFormat(
		display, PictStandardARGB32);
	GlyphSet gs=XRenderCreateGlyphSet(display, fmt);
	if (!ft_lib_initialized) FT_Init_FreeType(&library);
	FT_Face face;
	FT_New_Face(library, filename, 0, &face);
	FT_Set_Char_Size(face, 0, size*64, 90, 90);
	int count = strlen(TXT);
	for(n=0; n<count; n++) load_glyph(display, gs, face, TXT[n]);
	FT_Done_Face(face);
	return gs;
}

Picture create_pen(Display *display, int red, int green, int blue, int alpha)
{
	XRenderColor color;
	color.red=red;
	color.green=green;
	color.blue=blue;
	color.alpha=alpha;
	XRenderPictFormat *fmt=XRenderFindStandardFormat(display, PictStandardARGB32);
	
	Window root=DefaultRootWindow(display);
	Pixmap pm=XCreatePixmap(display, root, 1, 1, 32);
	XRenderPictureAttributes pict_attr;
	pict_attr.repeat=1;
	Picture picture=XRenderCreatePicture(display, pm, fmt, CPRepeat, &pict_attr);
	XRenderFillRectangle(display, PictOpOver, picture, &color, 0, 0, 1, 1);
	XFreePixmap(display, pm);
	return picture;
}

int main(int argc, char **argv)
{
	int k;
	Display *display=XOpenDisplay(NULL);
	
	XRenderPictFormat *fmt=XRenderFindStandardFormat(display, PictStandardRGB24);
	int screen=DefaultScreen(display);
	Window root=DefaultRootWindow(display);
	
	Window window=XCreateWindow(display, root, 0, 0, 640, 480, 0,
		DefaultDepth(display, screen), InputOutput,
		DefaultVisual(display, screen), 
		0, NULL);
	XRenderPictureAttributes pict_attr;
	
	pict_attr.poly_edge=PolyEdgeSmooth;
	pict_attr.poly_mode=PolyModeImprecise;
	Picture picture=XRenderCreatePicture(display, window, fmt, CPPolyEdge|CPPolyMode, &pict_attr);
	
	XSelectInput(display, window, KeyPressMask|KeyReleaseMask|ExposureMask
		|ButtonPressMask|StructureNotifyMask);
	
	Picture fg_pen=create_pen(display, 0x4444,0x4444,0x4444,0xffff);
	
	GlyphSet font=load_glyphset(display, FONT_PATH, 14);
	
	XMapWindow(display, window);
	XRenderColor bg_color;
	bg_color.red=0xdddd; bg_color.green=0xdddd; bg_color.blue=0xdddd; bg_color.alpha=0xffff;
	
	while(1) {
		XEvent event;
		XNextEvent(display, &event);
		
		switch(event.type) {
			case KeyPress:
				k = event.xkey.keycode;
				if (k == 9) exit(0);
				break;
			case Expose:
				XRenderFillRectangle(display, PictOpOver,
					picture, &bg_color, 0, 0, 640, 480);
				XRenderCompositeString8(display, PictOpOver,
					fg_pen, picture, 0,
					font, 0, 0, 200, 240, TXT, 21);
				break;
			case DestroyNotify:
				return 0;
		}
	}
	
	return 0;
}
