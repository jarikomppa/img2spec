/*
 *	Copyright (c) 2015 Jari Komppa, http://iki.fi/sol
 *
 *	This software is provided 'as-is', without any express or implied
 *	warranty. In no event will the authors be held liable for any damages
 *	arising from the use of this software.
 *
 *	Permission is granted to anyone to use this software for any purpose,
 *	including commercial applications, and to alter it and redistribute it
 *	freely, subject to the following restrictions:
 *
 *	1. The origin of this software must not be misrepresented; you must not
 *	claim that you wrote the original software. If you use this software
 *	in a product, an acknowledgement in the product documentation would be
 *	appreciated but is not required.
 *	2. Altered source versions must be plainly marked as such, and must not be
 *	misrepresented as being the original software.
 *	3. This notice may not be removed or altered from any source distribution.
 */

/*
Note that this is (mostly) one evening hack, so the
code quality leaves a lot to be desired..
Still, if you find it useful, great!
*/

#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define VERSION "1.1"

#define SPEC_Y(y)  ((((y>>0)&7)<< 3) | (((y >> 3)&7) <<0) | ((y >> 6) & 3) << 6)


int gSpeccyPalette[]
{
	0x000000, // black
	0xc00000, // blue
	0x0000c0, // red
	0xc000c0, // purple
	0x00c000, // green
	0xc0c000, // cyan
	0x00c0c0, // yellow
	0xc0c0c0, // white

	0x000000, // bright
	0xff0000,
	0x0000ff,
	0xff00ff,
	0x00ff00,
	0xffff00,
	0x00ffff,
	0xffffff
};

// used to avoid id collisions in ImGui
int gUniqueValueCounter = 0;

bool gWindowAttribBitmap = false;
bool gWindowHistograms = false;
bool gWindowConvOptions = true;
int gOptAttribOrder = 0;
int gOptBright = 2;
int gOptPaper = 0;
int gOptCellSize = 0;

// Texture handles
GLuint gTextureOrig, gTextureProc, gTextureSpec, gTextureAttr, gTextureBitm; 

// Bitmaps for the textures
unsigned int gBitmapOrig[256 * 192];
unsigned int gBitmapProc[256 * 192];
unsigned int gBitmapSpec[256 * 192];
unsigned int gBitmapAttr[256 * 192];
unsigned int gBitmapBitm[256 * 192];

// Floating point version of the processed bitmap, for processing
float gBitmapProcFloat[256 * 192 * 3];

// Spectrum format data
unsigned char gSpectrumAttributes[32 * 24];
unsigned char gSpectrumBitmap[32 * 192];

// Histogram arrays
float gHistogramR[256];
float gHistogramG[256];
float gHistogramB[256];


class Modifier;
// Modifier stack
Modifier *gModifierRoot = 0;


class Modifier
{
public:
	int mUnique;
	bool mEnabled;
	Modifier * mNext;

	Modifier()
	{ 
		gUniqueValueCounter++; 
		mUnique = gUniqueValueCounter; 
		mEnabled = true; 
	}

	int common()
	{
		int ret = 0;
		ImGui::Checkbox("Enabled", &mEnabled);
		ImGui::SameLine();
		if (ImGui::Button("Remove")) ret = -1;
		ImGui::SameLine();
		if (ImGui::Button("Move down")) ret = 1;
		return ret;
	}

	virtual int process() = 0;	
};

class RGBModifier : public Modifier
{
public:
	float mR, mG, mB;
	int mOnce;

	RGBModifier() 
	{ 
		mR = mG = mB = 0; 
		mOnce = 0;
	}
	virtual int process()
	{
		int ret = 0;
		ImGui::PushID(mUnique);
	
		if (!mOnce) 
		{
			ImGui::OpenNextNode(1); 
			mOnce = 1;
		}
		
		if (ImGui::CollapsingHeader("RGB Modifier"))
		{
			ret = common();
			ImGui::SliderFloat("Red  ", &mR, -1, 1); ImGui::SameLine();	
			if (ImGui::Button("Reset red   ")) mR = 0;

			ImGui::SliderFloat("Green", &mG, -1, 1); ImGui::SameLine();	
			if (ImGui::Button("Reset green ")) mG = 0;

			ImGui::SliderFloat("Blue ", &mB, -1, 1); ImGui::SameLine();	
			if (ImGui::Button("Reset blue  ")) mB = 0;
		}

		int i;
		if (mEnabled)
		{
			for (i = 0; i < 256 * 192; i++)
			{
				gBitmapProcFloat[i * 3 + 0] += mB;
				gBitmapProcFloat[i * 3 + 1] += mG;
				gBitmapProcFloat[i * 3 + 2] += mR;
			}
		}
		ImGui::PopID();
		return ret;
	}
	
};


class NoiseModifier : public Modifier
{
public:
	float mV;
	int mSeed;
	bool mColornoise;
	bool mR_en, mG_en, mB_en;
	int mOnce;

	NoiseModifier()
	{
		mV = 0; 
		mSeed = 0; 
		mColornoise = false; 
		mOnce = 0;
		mR_en = true;
		mG_en = true;
		mB_en = true;
	}
	
	virtual int process()
	{
		int ret = 0;
		ImGui::PushID(mUnique);
	
		if (!mOnce) 
		{
			ImGui::OpenNextNode(1); 
			mOnce = 1;
		}

		if (ImGui::CollapsingHeader("Noise Modifier"))
		{
			ret = common();			
			ImGui::SliderFloat("Strength", &mV, 0, 0.25f);	ImGui::SameLine(); 
			if (ImGui::Button("Reset strength   ")) mV = 0;			

			ImGui::SliderInt("Seed    ", &mSeed, 0, 65535); ImGui::SameLine(); 
			if (ImGui::Button("Reset seed       ")) mSeed = 0; ImGui::SameLine();  
			if (ImGui::Button("Randomize ")) mSeed = (SDL_GetTicks() * 74531) % 65535;

			ImGui::Checkbox("Color noise", &mColornoise);   ImGui::SameLine(); 
			ImGui::Checkbox("Red enable", &mR_en); ImGui::SameLine(); 
			ImGui::Checkbox("Green enable", &mG_en); ImGui::SameLine(); 
			ImGui::Checkbox("Blue enable", &mB_en);
		}

		int i;
		if (mEnabled)
		{
			srand(mSeed);
			for (i = 0; i < 256 * 192; i++)
			{

				float r = (((rand() % 1024) - 512) / 512.0f) * mV;
				float g = (((rand() % 1024) - 512) / 512.0f) * mV;
				float b = (((rand() % 1024) - 512) / 512.0f) * mV;
				if (!mColornoise)
				{
					r = g = b;
				}
				if (mB_en) gBitmapProcFloat[i * 3 + 0] += b;
				if (mG_en) gBitmapProcFloat[i * 3 + 1] += g;
				if (mR_en) gBitmapProcFloat[i * 3 + 2] += r;
			}
		}
		ImGui::PopID();
		return ret;
	}
};


class ContrastModifier : public Modifier
{
public:
	float mC, mB;
	int mOnce;

	ContrastModifier()
	{
		mB = 0;
		mC = 1;
		mOnce = 0;
	}

	virtual int process()
	{
		int ret = 0;
		ImGui::PushID(mUnique);
		
		if (!mOnce) {
			ImGui::OpenNextNode(1); 
			mOnce = 1;
		}

		if (ImGui::CollapsingHeader("Contrast Modifier"))
		{
			ret = common();
			
			ImGui::SliderFloat("Contrast  ", &mC, 0, 5);  ImGui::SameLine();	
			if (ImGui::Button("Reset contrast   ")) mC = 1;

			ImGui::SliderFloat("Brightness", &mB, -2, 2); ImGui::SameLine();	
			if (ImGui::Button("Reset brightness ")) mB = 0;
		}

		int i;
		if (mEnabled)
		{
			for (i = 0; i < 256 * 192 * 3; i++)
			{
				gBitmapProcFloat[i] = (gBitmapProcFloat[i] - 0.5f) * mC + 0.5f + mB;
			}
		}
		ImGui::PopID();
		return ret;
	}
};


class HSVModifier : public Modifier
{
public:
	float mH, mS, mV;
	int mOnce;
	
	HSVModifier()
	{
		mH = 0;
		mS = 0;
		mV = 0;
		mOnce = 0;
	}

	void rgb2hsv(float r, float g, float b, float &h, float &s, float &v)
	{
		float      min, max, delta;

		min = r < g ? r : g;
		min = min  < b ? min : b;

		max = r > g ? r : g;
		max = max > b ? max : b;

		v = max;                                // v
		delta = max - min;
		if (delta < 0.00001)
		{
			s = 0;
			h = 0; // undefined, maybe nan?
			return;
		}
		if (max > 0.0) { // NOTE: if Max is == 0, this divide would cause a crash
			s = (delta / max);                  // s
		}
		else {
			// if max is 0, then r = g = b = 0              
			// s = 0, v is undefined
			s = 0.0;
			h = 0.0;                            // its now undefined
			return;
		}
		if (r >= max)                           // > is bogus, just keeps compilor happy
			h = (g - b) / delta;        // between yellow & magenta
		else
			if (g >= max)
				h = 2.0f + (b - r) / delta;  // between cyan & yellow
			else
				h = 4.0f + (r - g) / delta;  // between magenta & cyan

		h *= 60.0f;                              // degrees

		if (h < 0.0f)
			h += 360.0f;
	}


	void hsv2rgb(float h, float s, float v, float &r, float &g, float &b)
	{
		float      hh, p, q, t, ff;
		long        i;

		if (s <= 0.0f) {       // < is bogus, just shuts up warnings
			r = v;
			g = v;
			b = v;
			return;
		}
		hh = h;
		if (hh >= 360.0f) hh = 0.0f;
		hh /= 60.0f;
		i = (long)floor(hh);
		ff = hh - i;
		p = v * (1.0f - s);
		q = v * (1.0f - (s * ff));
		t = v * (1.0f - (s * (1.0f - ff)));

		switch (i) {
		case 0:
			r = v;
			g = t;
			b = p;
			break;
		case 1:
			r = q;
			g = v;
			b = p;
			break;
		case 2:
			r = p;
			g = v;
			b = t;
			break;

		case 3:
			r = p;
			g = q;
			b = v;
			break;
		case 4:
			r = t;
			g = p;
			b = v;
			break;
		case 5:
		default:
			r = v;
			g = p;
			b = q;
			break;
		}
	}

	virtual int process()
	{
		int ret = 0;
		ImGui::PushID(mUnique);

		if (!mOnce) 
		{
			ImGui::OpenNextNode(1); 
			mOnce = 1;
		}
		
		if (ImGui::CollapsingHeader("HSV Modifier"))
		{
			ret = common();

			ImGui::SliderFloat("Hue       ", &mH, -360, 360); ImGui::SameLine();	
			if (ImGui::Button("Reset hue        ")) mH = 0;
			
			ImGui::SliderFloat("Saturation", &mS, -2, 2);     ImGui::SameLine();	
			if (ImGui::Button("Reset saturation ")) mS = 0;
			
			ImGui::SliderFloat("Value     ", &mV, -2, 2);     ImGui::SameLine();	
			if (ImGui::Button("Reset value      ")) mV = 0;
		}
		
		int i;
		if (mEnabled)
		{
			for (i = 0; i < 256 * 192; i++)
			{
				rgb2hsv(gBitmapProcFloat[i * 3 + 0], gBitmapProcFloat[i * 3 + 1], gBitmapProcFloat[i * 3 + 2], gBitmapProcFloat[i * 3 + 0], gBitmapProcFloat[i * 3 + 1], gBitmapProcFloat[i * 3 + 2]);
				
				gBitmapProcFloat[i * 3 + 0] += mH;
				gBitmapProcFloat[i * 3 + 1] += mS;
				gBitmapProcFloat[i * 3 + 2] += mV;
				
				if (gBitmapProcFloat[i * 3 + 0] < 0) gBitmapProcFloat[i * 3 + 0] += 360;
				if (gBitmapProcFloat[i * 3 + 0] > 360) gBitmapProcFloat[i * 3 + 0] -= 360;
				
				hsv2rgb(gBitmapProcFloat[i * 3 + 0], gBitmapProcFloat[i * 3 + 1], gBitmapProcFloat[i * 3 + 2], gBitmapProcFloat[i * 3 + 0], gBitmapProcFloat[i * 3 + 1], gBitmapProcFloat[i * 3 + 2]);
			}
		}
		ImGui::PopID();
		return ret;
	}
};


void process_image()
{
	int i;
	for (i = 0; i < 256 * 192; i++)
	{
		int c = gBitmapOrig[i];
		int r = (c >> 16) & 0xff;
		int g = (c >> 8) & 0xff;
		int b = (c >> 0) & 0xff;

		gBitmapProcFloat[i * 3 + 0] = r * (1.0f / 255.0f);
		gBitmapProcFloat[i * 3 + 1] = g * (1.0f / 255.0f);
		gBitmapProcFloat[i * 3 + 2] = b * (1.0f / 255.0f);
	}
	
	Modifier *walker = gModifierRoot;
	Modifier *prev = 0;

	while (walker)
	{
		int ret = walker->process();
		if (ret == 1) // move down
		{
			if (walker->mNext)
			{
				Modifier *curr = walker;
				Modifier *next = walker->mNext;
				/*
				prev->curr->next->..
				*/
				curr->mNext = next->mNext;
				next->mNext = curr;
				if (prev == 0)
				{
					gModifierRoot = next;
				}
				else
				{
					prev->mNext = next;
				}
			}
			else
			{
				prev = walker;
				walker = walker->mNext;
			}
		}
		else
		if (ret == -1) // delete
		{
			Modifier *t = walker;
			if (prev == 0)
			{
				gModifierRoot = walker->mNext;
			}
			else
			{
				prev->mNext = walker->mNext;
			}
			walker = walker->mNext;
			delete t;
		}
		else // normal op
		{
			prev = walker;
			walker = walker->mNext;
		}
	}
	
	for (i = 0; i < 256 * 192; i++)
	{
		float r = gBitmapProcFloat[i * 3 + 0];
		float g = gBitmapProcFloat[i * 3 + 1];
		float b = gBitmapProcFloat[i * 3 + 2];

		r = (r < 0) ? 0 : (r > 1) ? 1 : r;
		g = (g < 0) ? 0 : (g > 1) ? 1 : g;
		b = (b < 0) ? 0 : (b > 1) ? 1 : b;

		gBitmapProc[i] = 
			((int)floor(r * 255) << 16) |
			((int)floor(g * 255) << 8) |
			((int)floor(b * 255) << 0) |
			0xff000000;
	}
}


int rgb_to_speccy_pal(int c, int first, int count)
{
	int i;
	int r = (c >> 16) & 0xff;
	int g = (c >> 8) & 0xff;
	int b = (c >> 0) & 0xff;
	int bestdist = 256*256*4;
	int best = 0;

	for (i = first; i < first + count; i++)
	{
		int s_b = (gSpeccyPalette[i] >> 0) & 0xff;
		int s_g = (gSpeccyPalette[i] >> 8) & 0xff;
		int s_r = (gSpeccyPalette[i] >> 16) & 0xff;

		int dist = (r - s_r) * (r - s_r) +
			(g - s_g) * (g - s_g) +
			(b - s_b) * (b - s_b);

		if (dist < bestdist)
		{
			best = i;
			bestdist = dist;
		}
	}
	return best; // gSpeccyPalette[best] | 0xff000000;
}

int pick_from_2_speccy_cols(int c, int col1, int col2)
{
	int r = (c >> 16) & 0xff;
	int g = (c >> 8) & 0xff;
	int b = (c >> 0) & 0xff;

	int s_b = (gSpeccyPalette[col1] >> 0) & 0xff;
	int s_g = (gSpeccyPalette[col1] >> 8) & 0xff;
	int s_r = (gSpeccyPalette[col1] >> 16) & 0xff;

	int dist1 = (r - s_r) * (r - s_r) +
		(g - s_g) * (g - s_g) +
		(b - s_b) * (b - s_b);

	s_b = (gSpeccyPalette[col2] >> 0) & 0xff;
	s_g = (gSpeccyPalette[col2] >> 8) & 0xff;
	s_r = (gSpeccyPalette[col2] >> 16) & 0xff;

	int dist2 = (r - s_r) * (r - s_r) +
		(g - s_g) * (g - s_g) +
		(b - s_b) * (b - s_b);

	if (dist1 < dist2)
		return col1;
	return col2;
}


void spectrumize_image()
{
	int x, y, i, j;

	// Find closest colors in the speccy palette
	for (i = 0; i < 256 * 192; i++)
	{		
		gBitmapSpec[i] = rgb_to_speccy_pal(gBitmapProc[i], 0, 16);
	}

	for (y = 0; y < 24; y++)
	{
		for (x = 0; x < 32; x++)
		{
			// Count bright pixels in cell
			int brights = 0;
			int blacks = 0;
			for (i = 0; i < 8; i++)
			{
				for (j = 0; j < 8; j++)
				{
					int loc = (y * 8 + i) * 256 + x * 8 + j;
					if (gBitmapSpec[loc] > 7)
						brights++;
					if (gBitmapSpec[loc] == 0)
						blacks++;
				}
			}

			switch (gOptBright)
			{
			case 0: // only dark
				brights = 0;
				break;
			case 1: // prefer dark
				brights = (brights >= (8 * 8 - blacks)) * 8;
				break;
			case 2: // default
				brights = (brights >= (8 * 8 - blacks) / 2) * 8;
				break;
			case 3: // prefer bright
				brights = (brights >= (8 * 8 - blacks) / 4) * 8;
				break;
			case 4: // only bright
				brights = 8;
				break;
			}		

			int counts[16];
			for (i = 0; i < 16; i++)
				counts[i] = 0;

			// Remap with just bright OR dark colors, based on whether the whole cell is bright or not
			for (i = 0; i < 8; i++)
			{
				for (j = 0; j < 8; j++)
				{
					int loc = (y * 8 + i) * 256 + x * 8 + j;
					int r = rgb_to_speccy_pal(gBitmapProc[loc], brights, 8);
					gBitmapSpec[loc] = r;
					counts[r]++;
				}
			}

			// Find two most common colors in cell
			int col1 = 0, col2 = 0;
			int best = 0;
			if (gOptPaper == 0)
			{
				for (i = 0; i < 16; i++)
				{
					if (counts[i] > best)
					{
						best = counts[i];
						col1 = i;
					}
				}
			}
			else
			{
				col1 = (gOptPaper - 1) + brights;
			}
			counts[col1] = 0;
			best = 0;
			for (i = 0; i < 16; i++)
			{
				if (counts[i] > best)
				{
					best = counts[i];
					col2 = i;
				}
			}

			// Make sure we're using bright black
			if (col2 == 0 && col1 > 7) col2 = 8;

			// Final pass on cell, select which of two colors we can use
			for (i = 0; i < 8; i++)
			{
				for (j = 0; j < 8; j++)
				{
					int loc = (y * 8 + i) * 256 + x * 8 + j;					
					gBitmapSpec[loc] = pick_from_2_speccy_cols(gBitmapProc[loc], col1, col2);
				}
			}		
			
			if (gOptPaper == 0)
			{
				// Make the "brighter" color the ink to make bitmap pretty (if wanted)
				if (gOptAttribOrder == 0 && col2 < col1)
				{
					int tmp = col2;
					col2 = col1;
					col1 = tmp;
				}
			}

			// Store the cell's attribute
			gSpectrumAttributes[y * 32 + x] = (col2 & 0x7) | ((col1 & 7) << 3) | (((col1 & 8) != 0) << 6);
		}
	}

	// Map color indices to palette
	for (i = 0; i < 256 * 192; i++)
	{
		gBitmapSpec[i] = gSpeccyPalette[gBitmapSpec[i]] | 0xff000000;
	}

}

void calc_histogram(unsigned int *src)
{
	int i;
	for (i = 0; i < 256; i++)
	{
		gHistogramR[i] = 0;
		gHistogramG[i] = 0;
		gHistogramB[i] = 0;
	}
	for (i = 0; i < 192 * 256; i++)
	{
		gHistogramR[(src[i] >> 0) & 0xff]++;
		gHistogramG[(src[i] >> 8) & 0xff]++;
		gHistogramB[(src[i] >> 16) & 0xff]++;
	}
}

// Should probably add to the end of the list instead of beginning..
void addModifier(Modifier *aNewModifier)
{
	aNewModifier->mNext = gModifierRoot;
	gModifierRoot = aNewModifier;
}


void loadimg()
{
	OPENFILENAMEA ofn;
	char szFileName[1024] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter =
		"All supported types\0*.png;*.jpg;*.jpeg;*.tga;*.bmp;*.psd;*.gif;*.hdr;*.pic;*.pnm\0"
		"PNG (*.png)\0*.png\0"
		"JPG (*.jpg)\0*.jpg;*.jpeg\0"
		"TGA (*.tga)\0*.tga\0"
		"BMP (*.bmp)\0*.bmp\0"
		"PSD (*.psd)\0*.psd\0"
		"GIF (*.gif)\0*.gif\0"
		"HDR (*.hdr)\0*.hdr\0"
		"PIC (*.pic)\0*.pic\0"
		"PNM (*.pnm)\0*.pnm\0"
		"All Files (*.*)\0*.*\0\0";

	ofn.nMaxFile = 1024;

	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrFile = szFileName;

	ofn.lpstrTitle = "Load image";


	FILE * f = NULL;

	if (GetOpenFileNameA(&ofn))
	{
		int x, y, n;
		unsigned int *data = (unsigned int*)stbi_load(szFileName, &x, &y, &n, 4);

		int i, j;
		for (i = 0; i < 192; i++)
		{
			for (j = 0; j < 256; j++)
			{
				int pix = 0;
				if (j < x && i < y)
					pix = data[i * x + j] | 0xff000000;
				gBitmapOrig[i * 256 + j] = pix;
			}
		}

		stbi_image_free(data);

		glBindTexture(GL_TEXTURE_2D, gTextureOrig);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)gBitmapOrig);
	}
}


void savepng()
{
	OPENFILENAMEA ofn;
	char szFileName[1024] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter =
		"PNG (*.png)\0*.png\0"
		"All Files (*.*)\0*.*\0\0";

	ofn.nMaxFile = 1024;

	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrFile = szFileName;

	ofn.lpstrTitle = "Save png";

	FILE * f = NULL;

	if (GetSaveFileNameA(&ofn))
	{
		stbi_write_png(szFileName, 256, 192, 4, gBitmapProc, 256*4);
	}
}


// When converting to speccy format we process stuff cell by cell..
// easier to just analyze the result to generate the bitmap than to
// mess up the conversion further.
void grab_speccy_bitmap()
{	
	int i, j;
	for (i = 0; i < 192; i++)
	{
		for (j = 0; j < 256; j++)
		{
			gSpectrumBitmap[SPEC_Y(i) * 32 + j / 8] <<= 1;
			int v = ((gBitmapSpec[i * 256 + j] & 0xffffff) != gSpeccyPalette[((gSpectrumAttributes[(i / 8) * 32 + j / 8] >> 3) & 7) | ((gSpectrumAttributes[(i / 8) * 32 + j / 8] >> 6) << 3)]);
			gSpectrumBitmap[SPEC_Y(i) * 32 + j / 8] |= v;
		}
	}
}


void saveraw()
{
	OPENFILENAMEA ofn;
	char szFileName[1024] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter =
		"RAW (*.raw)\0*.raw\0"
		"All Files (*.*)\0*.*\0\0";

	ofn.nMaxFile = 1024;

	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrFile = szFileName;

	ofn.lpstrTitle = "Save raw";

	FILE * f = NULL;

	if (GetSaveFileNameA(&ofn))
	{
		grab_speccy_bitmap();
		FILE * f = fopen(szFileName, "wb");
		fwrite(gSpectrumBitmap, 32 * 192, 1, f);
		fwrite(gSpectrumAttributes, 32 * 24, 1, f);
		fclose(f);
	}
}


void saveh()
{
	OPENFILENAMEA ofn;
	char szFileName[1024] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter =
		"h (*.h)\0*.h\0"
		"All Files (*.*)\0*.*\0\0";

	ofn.nMaxFile = 1024;

	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrFile = szFileName;

	ofn.lpstrTitle = "Save h";

	FILE * f = NULL;

	if (GetSaveFileNameA(&ofn))
	{
		grab_speccy_bitmap();
		FILE * f = fopen(szFileName, "w");
		int i, w = 0;
		for (i = 0; i < 32 * 192; i++)
		{
			w += fprintf(f, "%3d,", gSpectrumBitmap[i]);
			if (w > 75)
			{
				fprintf(f, "\n");
				w = 0;
			}				
		}
		fprintf(f,"\n\n");
		w = 0;
		for (i = 0; i < 32 * 24; i++)
		{
			w += fprintf(f, "%3d,", gSpectrumAttributes[i]);
			if (w > 75)
			{
				fprintf(f, "\n");
				w = 0;
			}
		}
		fclose(f);
	}
}


void saveinc()
{
	OPENFILENAMEA ofn;
	char szFileName[1024] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = 0;
	ofn.lpstrFilter =
		"inc (*.inc)\0*.inc\0"
		"All Files (*.*)\0*.*\0\0";

	ofn.nMaxFile = 1024;

	ofn.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	ofn.lpstrFile = szFileName;

	ofn.lpstrTitle = "Save inc";

	FILE * f = NULL;

	if (GetSaveFileNameA(&ofn))
	{
		grab_speccy_bitmap();
		FILE * f = fopen(szFileName, "w");
		int i;
		for (i = 0; i < 32 * 192; i++)
		{
			fprintf(f, "\t.db #0x%02x\n", gSpectrumBitmap[i]);
		}
		fprintf(f, "\n\n");
		for (i = 0; i < 32 * 24; i++)
		{
			fprintf(f, "\t.db #0x%02x\n", gSpectrumAttributes[i]);
		}
		fclose(f);
	}
}


void gen_attr_bitm()
{
	grab_speccy_bitmap();
	int i, j;
	for (i = 0; i < 192; i++)
	{
		for (j = 0; j < 256; j++)
		{
			int attr = gSpectrumAttributes[(i / 8) * 32 + j / 8];
			int fg = gSpeccyPalette[((attr >> 0) & 7) | (((attr & 64)) >> 3)] | 0xff000000;
			int bg = gSpeccyPalette[((attr >> 3) & 7) | (((attr & 64)) >> 3)] | 0xff000000;
			gBitmapAttr[i * 256 + j] = (j % 8 < i % 8) ? fg : bg;			
			gBitmapBitm[i * 256 + j] = (gSpectrumBitmap[SPEC_Y(i) * 32 + j / 8] & (1 << (7-(j % 8)))) ? 0xffc0c0c0 : 0xff000000;
		}
	}
	glBindTexture(GL_TEXTURE_2D, gTextureAttr);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)gBitmapAttr);
	glBindTexture(GL_TEXTURE_2D, gTextureBitm);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)gBitmapBitm);

}



int main(int, char**)
{

	// Setup SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
        printf("Error: %s\n", SDL_GetError());
        return -1;
	}

    // Setup window
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_DisplayMode current;
	SDL_GetCurrentDisplayMode(0, &current);
	SDL_Window *window = SDL_CreateWindow("Image Spectrumizer " VERSION " - http://iki.fi/sol", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	SDL_GLContext glcontext = SDL_GL_CreateContext(window);

    // Setup ImGui binding
    ImGui_ImplSdl_Init(window);

    bool show_test_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImColor(114, 144, 154);

	glGenTextures(1, &gTextureOrig);
	glBindTexture(GL_TEXTURE_2D, gTextureOrig);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &gTextureProc);
	glBindTexture(GL_TEXTURE_2D, gTextureProc);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &gTextureSpec);
	glBindTexture(GL_TEXTURE_2D, gTextureSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &gTextureAttr);
	glBindTexture(GL_TEXTURE_2D, gTextureAttr);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGenTextures(1, &gTextureBitm);
	glBindTexture(GL_TEXTURE_2D, gTextureBitm);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)0);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    // Main loop
	bool done = false;
    while (!done)
    {
		ImVec2 picsize(256, 192);
		
		SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdl_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }
        ImGui_ImplSdl_NewFrame(window);
		ImGui::ShowTestWindow();

		if (gWindowConvOptions)
		{
			if (ImGui::Begin("Conversion options", &gWindowConvOptions, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
			{
				ImGui::Combo("Attribute order", &gOptAttribOrder, "Pretty\0Compressable\0");
				ImGui::Combo("Bright attributes", &gOptBright, "Only dark\0Prefer dark\0Normal\0Prefer bright\0Only bright\0");
				ImGui::Combo("Paper attribute", &gOptPaper, "Optimal\0Black\0Blue\0Red\0Purple\0Green\0Cyan\0Yellow\0White\0");
				//ImGui::Combo("Attribute cell size", &gOptCellSize, "8x8 (standard)\08x4 (bicolor)\08x2\08x1\0");
			}
			ImGui::End();
		}

		if (gWindowAttribBitmap)
		{
			if (ImGui::Begin("Attrib/bitmap", &gWindowAttribBitmap, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
			{
				gen_attr_bitm();
				ImGui::Image((ImTextureID)gTextureAttr, picsize); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
				ImGui::Image((ImTextureID)gTextureBitm, picsize); ImGui::SameLine(); ImGui::Text(" ");
			}
			ImGui::End();
		}

		if (gWindowHistograms)
		{
			if (ImGui::Begin("Histograms", &gWindowHistograms, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
			{
				calc_histogram(gBitmapOrig);
				ImGui::PlotHistogram("###hist1", gHistogramR, 256, 0, 0, 0, 1024, ImVec2(256, 32)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
				ImGui::PlotHistogram("###hist2", gHistogramG, 256, 0, 0, 0, 1024, ImVec2(256, 32)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
				ImGui::PlotHistogram("###hist3", gHistogramB, 256, 0, 0, 0, 1024, ImVec2(256, 32));
				calc_histogram(gBitmapProc);
				ImGui::PlotHistogram("###hist4", gHistogramR, 256, 0, 0, 0, 1024, ImVec2(256, 32)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
				ImGui::PlotHistogram("###hist5", gHistogramG, 256, 0, 0, 0, 1024, ImVec2(256, 32)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
				ImGui::PlotHistogram("###hist6", gHistogramB, 256, 0, 0, 0, 1024, ImVec2(256, 32));
				calc_histogram(gBitmapSpec);
				ImGui::PlotHistogram("###hist7", gHistogramR, 256, 0, 0, 0, 1024, ImVec2(256, 32)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
				ImGui::PlotHistogram("###hist8", gHistogramG, 256, 0, 0, 0, 1024, ImVec2(256, 32)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
				ImGui::PlotHistogram("###hist9", gHistogramB, 256, 0, 0, 0, 1024, ImVec2(256, 32));
			}
			ImGui::End();
		}
		
		ImGui::Begin("Main", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

		ImGui::Image((ImTextureID)gTextureSpec, picsize); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
		ImGui::Image((ImTextureID)gTextureProc, picsize); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
		ImGui::Image((ImTextureID)gTextureOrig, picsize);
		
		ImGui::Text("File ops:");
		ImGui::SameLine();
		if (ImGui::Button("Load image"))
		{
			loadimg();
		}
		ImGui::SameLine();
		if (ImGui::Button("Save png"))
		{
			savepng();
		}
		ImGui::SameLine();
		if (ImGui::Button("Save raw"))
		{
			saveraw();
		}
		ImGui::SameLine();
		if (ImGui::Button("Save .h"))
		{
			saveh();
		}
		ImGui::SameLine();
		if (ImGui::Button("Save .inc"))
		{
			saveinc();
		}
		ImGui::Text("Windows:");
		ImGui::SameLine();
		if (ImGui::Button("Histogram"))
		{
			gWindowHistograms = !gWindowHistograms;
		}
		ImGui::SameLine();
		if (ImGui::Button("Attribute/Bitmap"))
		{
			gWindowAttribBitmap = !gWindowAttribBitmap;
		}
		ImGui::SameLine();
		if (ImGui::Button("Conversion options"))
		{
			gWindowConvOptions = !gWindowConvOptions;
		}

		ImGui::Text("Add modifiers:");
		ImGui::SameLine();
		if (ImGui::Button("RGB"))
		{
			addModifier(new RGBModifier);
		}
		ImGui::SameLine();
		if (ImGui::Button("HSV"))
		{
			addModifier(new HSVModifier);
		}
		ImGui::SameLine();
		if (ImGui::Button("Contrast"))
		{
			addModifier(new ContrastModifier);
		}
		ImGui::SameLine();
		if (ImGui::Button("Noise"))
		{
			addModifier(new NoiseModifier);
		}
		

		process_image();
		spectrumize_image();

		glBindTexture(GL_TEXTURE_2D, gTextureProc);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)gBitmapProc);
		glBindTexture(GL_TEXTURE_2D, gTextureSpec);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)gBitmapSpec);

		ImGui::End();

        // Rendering
        glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplSdl_Shutdown();
    SDL_GL_DeleteContext(glcontext);  
	SDL_DestroyWindow(window);
	SDL_Quit();

    return 0;
}
