#define SPEC_Y(y)  (((y) & 0xff00) | ((((y) >> 0) & 7) << 3) | ((((y) >> 3) & 7) << 0) | (((y) >> 6) & 3) << 6)

int gSpeccyPalette[16 + 3 * 64]
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
		0xffffff,

		//#include "pal3x64.h"
#include "pal3x64_measured.h"
};

class ZXSpectrumDevice : public Device
{
public:

	int mOptAttribOrder;
	float mOptBright;
	int mOptPaper;
	int mOptCellSize;
	int mOptScreenOrder;
	int mOptWidthCells;
	int mOptHeightCells;
	int mOptConversionMode;
	float mOptPivotBias;

	// Spectrum format data
	unsigned char mSpectrumAttributes[128 * 64 * 8 * 2]; // big enough for 8x1 attribs in 3x64 mode at 1024x512
	unsigned char mSpectrumBitmap[128 * 512];
	unsigned char mSpectrumBitmapLinear[128 * 512];

	ZXSpectrumDevice()
	{
		mOptAttribOrder = 0;
		mOptBright = 0.5f;
		mOptPaper = 0;
		mOptCellSize = 0;
		mOptScreenOrder = 1;
		mOptConversionMode = 0;
		mOptPivotBias = 0.5f;
 

		mXRes = 256;
		mYRes = 192;

		mOptWidthCells = mXRes / 8;
		mOptHeightCells = mYRes / 8;

		memset(mSpectrumAttributes, 0, 128 * 64 * 8 * 2);
		memset(mSpectrumBitmap, 0, 128 * 512);
		memset(mSpectrumBitmapLinear, 0, 128 * 512);
	}

	static int rgb_to_speccy_pal(int c, int first, int count)
	{
		int i;
		int r = (c >> 16) & 0xff;
		int g = (c >> 8) & 0xff;
		int b = (c >> 0) & 0xff;
		int bestdist = 256 * 256 * 4;
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

	virtual int estimate_rgb(int c)
	{
		return gSpeccyPalette[rgb_to_speccy_pal(c, 0, 16)] | 0xff000000;
	}

	static int pick_from_2_speccy_cols(int c, int col1, int col2)
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

	void find_nearestcolor(int &col1, int &col2, int cellht, int x, int y) const
	{
		int i, j;
		// Count bright pixels in cell
		int brights = 0;
		int blacks = 0;
		for (i = 0; i < cellht; i++)
		{
			for (j = 0; j < 8; j++)
			{
				int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
				if (gBitmapSpec[loc] > 7)
					brights++;
				if (gBitmapSpec[loc] == 0)
					blacks++;
			}
		}

		if (brights >= ((63 - 64 * mOptBright) * cellht / 8 - blacks))
			brights = 8;
		else
			brights = 0;

		if (mOptBright <= 0) brights = 0;
		if (mOptBright >= 1) brights = 8;

		int counts[16];
		for (i = 0; i < 16; i++)
			counts[i] = 0;

		// Remap with just bright OR dark colors, based on whether the whole cell is bright or not
		for (i = 0; i < cellht; i++)
		{
			for (j = 0; j < 8; j++)
			{
				int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
				int r = rgb_to_speccy_pal(gBitmapProc[loc], brights, 8);
				gBitmapSpec[loc] = r;
				counts[r]++;
			}
		}

		// Find two most common colors in cell
		col1 = 0;
		col2 = 0;
		int best = 0;
		if (mOptPaper == 0)
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
			col1 = (mOptPaper - 1) + brights;
		}

		// reset count of selected color to zero so we don't pick it twice
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
	}

	void avg_colors(int brightness[8 * 8], int total, int cellht, int x, int y, int pivot, int &col1, int &col2)
	{
		int i, j, c;
		int r0 = 0, r1 = 0;
		int g0 = 0, g1 = 0;
		int b0 = 0, b1 = 0;

		int locount = 0, hicount = 0;
		for (i = 0, c = 0; i < cellht; i++)
		{
			for (j = 0; j < 8; j++, c++)
			{
				int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;

				if (brightness[c] > pivot)
				{
					r1 += (gBitmapProc[loc] >> 16) & 0xff;
					g1 += (gBitmapProc[loc] >> 8) & 0xff;
					b1 += (gBitmapProc[loc] >> 0) & 0xff;
					hicount++;
				}
				else
				{
					r0 += (gBitmapProc[loc] >> 16) & 0xff;
					g0 += (gBitmapProc[loc] >> 8) & 0xff;
					b0 += (gBitmapProc[loc] >> 0) & 0xff;
					locount++;
				}
			}
		}

		if (locount)
		{
			r0 /= locount;
			g0 /= locount;
			b0 /= locount;
		}
		if (hicount)
		{
			r1 /= hicount;
			g1 /= hicount;
			b1 /= hicount;
		}		

		int c0 = (r0 << 16) | (g0 << 8) | (b0 << 0);
		int c1 = (r1 << 16) | (g1 << 8) | (b1 << 0);

		int brights = 0;
		total /= cellht * 8;
		if (total > 0xff * 9 * (1 - mOptBright))
		{
			brights = 8;
		}

		int res0 = rgb_to_speccy_pal(c0, brights, 8);
		int res1 = rgb_to_speccy_pal(c1, brights, 8);

		if (mOptPaper)
		{
			// Well great, user wants to pick paper color. Let's do the best we can.
			int closest = pick_from_2_speccy_cols(gSpeccyPalette[mOptPaper-1], res0, res1);
			if (closest == res0)
			{
				res0 = mOptPaper - 1;
			}
			else
			{
				res1 = res0;
				res0 = mOptPaper - 1;
			}
		}

		col1 = res0;
		col2 = res1;
	}

	static void calc_brightness(int brightness[8 * 8], int cellht, int x, int y, int &total)
	{
		int i, j, c;
		for (c = 0; c < 8 * 8; c++)
			brightness[c] = 0;

		for (i = 0, c = 0; i < cellht; i++)
		{
			for (j = 0; j < 8; j++, c++)
			{
				int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
				// approximate brightness - b*1, r*3, g*5
				brightness[c] += (((gBitmapProc[loc] >> 0) & 0xff) * 1) + (((gBitmapProc[loc] >> 8) & 0xff) * 5) + (((gBitmapProc[loc] >> 16) & 0xff) * 3);
				total += brightness[c];
			}
		}
	}

	static void sort_brightness(int brightness[8 * 8], int order[8 * 8], int max)
	{
		int i, j;
		for (i = 0; i < max; i++)
			order[i] = i;

		for (i = 0; i < max; i++)
		{
			for (j = i; j < max; j++)
			{
				if (brightness[order[i]] > brightness[order[j]])
				{
					int t = order[i];
					order[i] = order[j];
					order[j] = t;
				}
			}
		}
	}

	void find_pivot_middle(int &col1, int &col2, int cellht, int x, int y)
	{
		int brightness[8 * 8];
		int total = 0;

		calc_brightness(brightness, cellht, x, y, total);

		int pivot = total / (cellht * 8);
		pivot -= (int)floor(pivot - 2 * pivot * mOptPivotBias);

		avg_colors(brightness, total, cellht, x, y, pivot, col1, col2);
	}

	void find_pivot_median(int &col1, int &col2, int cellht, int x, int y)
	{
		int brightness[8 * 8];
		int total = 0;

		calc_brightness(brightness, cellht, x, y, total);

		int order[8 * 8];
		sort_brightness(brightness, order, cellht * 8);

		int pivot = (int)floor((cellht * 8 - 1) * mOptPivotBias);

		pivot = brightness[order[pivot]];

		avg_colors(brightness, total, cellht, x, y, pivot, col1, col2);
	}

	void find_pivot_dualmedian(int &col1, int &col2, int cellht, int x, int y)
	{
		int brightness[8 * 8];
		int total = 0;

		calc_brightness(brightness, cellht, x, y, total);

		int order[8 * 8];
		sort_brightness(brightness, order, cellht * 8);

		int pivot = (int)floor(cellht * 8 * mOptPivotBias / 2); // 0..0.5

		pivot = (brightness[order[pivot]] + brightness[order[cellht * 8 - pivot - 1]]) / 2;

		avg_colors(brightness, total, cellht, x, y, pivot, col1, col2);
	}

	virtual void filter()
	{
		int x, y, i, j;

		// Find closest colors in the speccy palette
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			gBitmapSpec[i] = rgb_to_speccy_pal(gBitmapProc[i], 0, 16);
		}

		int ymax;
		int cellht;
		switch (mOptCellSize)
		{
			//case 0:
		default:
			cellht = 8;
			break;
		case 1:
			cellht = 4;
			break;
		case 2:
			cellht = 2;
			break;
		case 3:
			cellht = 1;
			break;
		}
		ymax = gDevice->mYRes / cellht;

		for (y = 0; y < ymax; y++)
		{
			for (x = 0; x < (gDevice->mXRes / 8); x++)
			{
				int col1 = 0, col2 = 0;
				if (mOptConversionMode == 0)
				{
					find_nearestcolor(col1, col2, cellht, x, y);
				}
				else
				if (mOptConversionMode == 1)
				{
					find_pivot_middle(col1, col2, cellht, x, y);
				}
				else
				if (mOptConversionMode == 2)
				{
					find_pivot_median(col1, col2, cellht, x, y);
				}
				else
				if (mOptConversionMode == 3)
				{
					find_pivot_dualmedian(col1, col2, cellht, x, y);
				}


				// Make sure we're using bright black
				if (col2 == 0 && col1 > 7) col2 = 8;

				if (mOptPaper == 0)
				{
					// Make the "brighter" color the ink to make bitmap pretty (if wanted)
					if (mOptAttribOrder == 0 && col2 < col1)
					{
						int tmp = col2;
						col2 = col1;
						col1 = tmp;
					}
				}

				// Final pass on cell, select which of two colors we can use
				for (i = 0; i < cellht; i++)
				{
					for (j = 0; j < 8; j++)
					{
						int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
						gBitmapSpec[loc] = pick_from_2_speccy_cols(gBitmapProc[loc], col1, col2);
						mSpectrumBitmap[SPEC_Y(y * cellht + i) * (gDevice->mXRes / 8) + x] <<= 1;
						mSpectrumBitmap[SPEC_Y(y * cellht + i) * (gDevice->mXRes / 8) + x] |= (gBitmapSpec[loc] == col1 ? 0 : 1);
						mSpectrumBitmapLinear[(y * cellht + i) * (gDevice->mXRes / 8) + x] <<= 1;
						mSpectrumBitmapLinear[(y * cellht + i) * (gDevice->mXRes / 8) + x] |= (gBitmapSpec[loc] == col1 ? 0 : 1);
					}
				}

				// Store the cell's attribute
				mSpectrumAttributes[y * (gDevice->mXRes / 8) + x] = (col2 & 0x7) | ((col1 & 7) << 3) | (((col1 & 8) != 0) << 6);
			}
		}

		// Map color indices to palette
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			gBitmapSpec[i] = gSpeccyPalette[gBitmapSpec[i]] | 0xff000000;
		}
	}

	virtual void savescr(FILE * f)
	{
		int attrib_size_multiplier = 1 << (mOptCellSize);
		unsigned char *bm = mSpectrumBitmap;
		if (mOptScreenOrder == 0)
		{
			bm = mSpectrumBitmapLinear;
		}
		fwrite(bm, (gDevice->mXRes / 8) * gDevice->mYRes, 1, f);
		fwrite(mSpectrumAttributes, (gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier, 1, f);
	}

	virtual void saveh(FILE * f)
	{
		unsigned char *bm = mSpectrumBitmap;
		if (mOptScreenOrder == 0)
		{
			bm = mSpectrumBitmapLinear;
		}
		int attrib_size_multiplier = 1 << (mOptCellSize);
		int i, c = 0;
		for (i = 0; i < (gDevice->mXRes / 8) * gDevice->mYRes; i++)
		{
			fprintf(f, "%3u,", bm[i]);
			c++;
			if (c >= 32)
			{
				fprintf(f, "\n");
				c = 0;
			}
		}
		fprintf(f, "\n\n");
		c = 0;
		for (i = 0; i < (gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier; i++)
		{
			fprintf(f, "%3u%s", mSpectrumAttributes[i], i != ((gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier) - 1 ? "," : "");
			c++;
			if (c >= 32)
			{
				fprintf(f, "\n");
				c = 0;
			}
		}
	}

	virtual void saveinc(FILE * f)
	{
		unsigned char *bm = mSpectrumBitmap;
		if (mOptScreenOrder == 0)
		{
			bm = mSpectrumBitmapLinear;
		}
		int attrib_size_multiplier = 1 << (mOptCellSize);
		int i;
		for (i = 0; i < (gDevice->mYRes / 8) * gDevice->mYRes; i++)
		{
			fprintf(f, "\t.db #0x%02x\n", bm[i]);
		}
		fprintf(f, "\n\n");
		for (i = 0; i < (gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier; i++)
		{
			fprintf(f, "\t.db #0x%02x\n", mSpectrumAttributes[i]);
		}
	}

	virtual void attr_bitm()
	{
		int cellht;
		switch (mOptCellSize)
		{
			//case 0:
		default:
			cellht = 8;
			break;
		case 1:
			cellht = 4;
			break;
		case 2:
			cellht = 2;
			break;
		case 3:
			cellht = 1;
			break;
		}

		int i, j;
		for (i = 0; i < gDevice->mYRes; i++)
		{
			for (j = 0; j < gDevice->mXRes; j++)
			{
				int fg, bg;

				int attr = mSpectrumAttributes[(i / cellht) * (gDevice->mXRes / 8) + j / 8];
				fg = gSpeccyPalette[((attr >> 0) & 7) | (((attr & 64)) >> 3)] | 0xff000000;
				bg = gSpeccyPalette[((attr >> 3) & 7) | (((attr & 64)) >> 3)] | 0xff000000;

				gBitmapAttr[i * gDevice->mXRes + j] = (j % 8 < (((8 - cellht) / 2) + i % cellht)) ? fg : bg;
				gBitmapBitm[i * gDevice->mXRes + j] = (mSpectrumBitmapLinear[(i) * (gDevice->mXRes / 8) + j / 8] & (1 << (7 - (j % 8)))) ? 0xffc0c0c0 : 0xff000000;
			}
		}
		update_texture(gTextureAttr, gBitmapAttr);
		update_texture(gTextureBitm, gBitmapBitm);

		ImVec2 picsize((float)gDevice->mXRes, (float)gDevice->mYRes);
		ImGui::Image((ImTextureID)gTextureAttr, picsize, ImVec2(0, 0), ImVec2(gDevice->mXRes / 1024.0f, gDevice->mYRes / 512.0f)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
		ImGui::Image((ImTextureID)gTextureBitm, picsize, ImVec2(0, 0), ImVec2(gDevice->mXRes / 1024.0f, gDevice->mYRes / 512.0f)); ImGui::SameLine(); ImGui::Text(" ");
	}

	virtual void options()
	{
		if (ImGui::Combo("Attribute cell size", &mOptCellSize, "8x8 (standard)\08x4 (bicolor)\08x2\08x1\0")) { gDirty = 1; mOptHeightCells = mXRes / (8 >> mOptCellSize); mXRes = mOptHeightCells * (8 >> mOptCellSize); gDirtyPic = 1; }
		if (ImGui::SliderInt("Bitmap width in cells", &mOptWidthCells, 1, 1028 / 8)) { gDirty = 1; gDirtyPic = 1; mXRes = mOptWidthCells * 8; }
		if (ImGui::SliderInt("Bitmap height in cells", &mOptHeightCells, 1, 512 / (8 >> mOptCellSize))) { gDirty = 1; gDirtyPic = 1; mYRes = mOptHeightCells * (8 >> mOptCellSize); }
		if (ImGui::Combo("Attribute order", &mOptAttribOrder, "Make bitmap pretty\0Make bitmap compressable\0")) gDirty = 1;
		ImGui::Combo("Bitmap order when saving", &mOptScreenOrder, "Linear order\0Spectrum video RAM order\0");
		ImGui::Separator();
		if (ImGui::SliderFloat("Bright attribute bias", &mOptBright, 0, 1)) gDirty = 1;
		if (ImGui::Combo("Paper attribute", &mOptPaper, "Optimal\0Black\0Blue\0Red\0Purple\0Green\0Cyan\0Yellow\0White\0")) gDirty = 1;
		if (ImGui::Combo("Coversion mode", &mOptConversionMode, "Popular color\0Average - middle\0Average - median\0Average - dual median\0")) { gDirty = 1; }
		if (mOptConversionMode != 0)
		if (ImGui::SliderFloat("Pivot bias", &mOptPivotBias, 0, 1)) { gDirty = 1; }
	}

	virtual void zoomed(int aWhich)
	{
		int tex;
		switch (aWhich)
		{
		default: //case 0:
			tex = gTextureSpec;
			break;
		case 1:
			tex = gTextureProc;
			break;
		case 2:
			tex = gTextureOrig;
			break;
		}

		if (gOptZoomStyle == 1)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1, 1));
			int i, j;
			int cellht = 8 >> mOptCellSize;
			int ymax = (gDevice->mYRes / 8) << mOptCellSize;
			for (i = 0; i < ymax; i++)
			{
				for (j = 0; j < (gDevice->mXRes / 8); j++)
				{
					ImGui::Image(
						(ImTextureID)tex,
						ImVec2(8.0f * gOptZoom, (float)cellht * gOptZoom),
						ImVec2((8 / 1024.0f) * (j + 0), (cellht / (float)gDevice->mYRes) * (i + 0) * (gDevice->mYRes / 512.0f)),
						ImVec2((8 / 1024.0f) * (j + 1), (cellht / (float)gDevice->mYRes) * (i + 1) * (gDevice->mYRes / 512.0f)));

					if (j != (gDevice->mXRes / 8)-1)
					{
						ImGui::SameLine();
					}
				}
			}
			ImGui::PopStyleVar();
		}
		else
		{
			ImGui::Image(
				(ImTextureID)tex, 
				ImVec2((float)gDevice->mXRes * gOptZoom, 
				(float)gDevice->mYRes * gOptZoom), 
				ImVec2(0, 0),
				ImVec2(gDevice->mXRes / 1024.0f, gDevice->mYRes / 512.0f));
		}
	}

	virtual void writeOptions(FILE *f)
	{
		Modifier::write(f, mOptAttribOrder);
		Modifier::write(f, mOptBright);
		Modifier::write(f, mOptPaper);
		Modifier::write(f, mOptCellSize);
		Modifier::write(f, mOptScreenOrder);
		Modifier::write(f, mOptWidthCells);
		Modifier::write(f, mOptHeightCells);
		Modifier::write(f, mOptConversionMode);
		Modifier::write(f, mOptPivotBias);
	}

	virtual void readOptions(FILE *f)
	{
		Modifier::read(f, mOptAttribOrder);
		Modifier::read(f, mOptBright);
		Modifier::read(f, mOptPaper);
		Modifier::read(f, mOptCellSize);
		Modifier::read(f, mOptScreenOrder);
		Modifier::read(f, mOptWidthCells);
		Modifier::read(f, mOptHeightCells);
		Modifier::read(f, mOptConversionMode);
		Modifier::read(f, mOptPivotBias);

		mXRes = mOptWidthCells * 8;
		mYRes = mOptHeightCells * (8 >> mOptCellSize);

		gDirty = 1;
		gDirtyPic = 1;
	}
};