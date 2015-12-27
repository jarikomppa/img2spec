class ZX3x64Device : public ZXSpectrumDevice
{
public:
	virtual int estimate_rgb(int c)
	{
		return gSpeccyPalette[rgb_to_speccy_pal(c, 16, 3 * 64)] | 0xff000000;
	}

	virtual void filter()
	{
		int x, y, i, j;

		// Find closest colors in the 3x64 palette
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			gBitmapSpec[i] = rgb_to_speccy_pal(gBitmapProc[i], 16, 3 * 64);
		}

		int ymax;
		int cellht;
		switch (mOptCellSize)
		{
		default:
			//case 0:
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
			for (x = 0; x < gDevice->mXRes / 8; x++)
			{
				// Count pixel brightnesses in cell
				int darks = 0;
				int brights = 0;
				int midbrights = 0;
				int blacks = 0;
				for (i = 0; i < cellht; i++)
				{
					for (j = 0; j < 8; j++)
					{
						int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
						if (gSpeccyPalette[gBitmapSpec[loc]] == 0)
						{
							blacks++;
						}
						else
							if (gBitmapSpec[loc] < (16 + 64 * 1))
							{
								darks++;
							}
							else
								if (gBitmapSpec[loc] < (16 + 64 * 2))
								{
									midbrights++;
								}
								else
								{
									brights++;
								}
					}
				}

				int palofs = 16 + 64 * 0;

				if (midbrights > darks)
				{
					palofs = 16 + 64 * 1;
				}

				if (brights > darks && brights > midbrights)
				{
					palofs = 16 + 64 * 2;
				}

				if (brights == 0 && midbrights == 0 && darks == 0)
				{
					palofs = 16 + 64 * 0;
				}

				int counts[16 + 3 * 64];
				for (i = 0; i < 16 + 3 * 64; i++)
					counts[i] = 0;

				// Remap with just the set of colors we have in the cell
				for (i = 0; i < cellht; i++)
				{
					for (j = 0; j < 8; j++)
					{
						int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
						int r = rgb_to_speccy_pal(gBitmapProc[loc], palofs, 64);
						gBitmapSpec[loc] = r;
						counts[r]++;
					}
				}

				// Find two most common colors in cell
				int paper = 0, ink = 0;
				int best = 0;
				if (mOptPaper == 0)
				{
					for (i = 16; i < 16 + 3 * 64; i++)
					{
						if (counts[i] > best)
						{
							best = counts[i];
							paper = i;
						}
					}
				}
				else
				{
					paper = (mOptPaper - 1) + palofs;
				}

				// reset count of selected color to zero so we don't pick it twice
				counts[paper] = 0;

				best = 0;
				for (i = 16; i < 16 + 3 * 64; i++)
				{
					if (counts[i] > best)
					{
						best = counts[i];
						ink = i;
					}
				}

				// Final pass on cell, select which of two colors we can use
				for (i = 0; i < cellht; i++)
				{
					for (j = 0; j < 8; j++)
					{
						int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
						gBitmapSpec[loc] = pick_from_2_speccy_cols(gBitmapProc[loc], paper, ink);
						mSpectrumBitmap[SPEC_Y(y * cellht + i) * (gDevice->mXRes / 8) + x] <<= 1;
						mSpectrumBitmap[SPEC_Y(y * cellht + i) * (gDevice->mXRes / 8) + x] |= (gBitmapSpec[loc] == ink ? 1 : 0);
					}
				}

				// Make sure col2 is within range..
				if (paper == 0)
					paper = 16;
				if (ink == 0)
					ink = 16;

				// Store the cell's attributes

				int col1ink = (((ink - 16) & 63) / 1) & 7;
				int col2ink = (((ink - 16) & 63) / 8) & 7;

				int col1pap = (((paper - 16) & 63) / 1) & 7;
				int col2pap = (((paper - 16) & 63) / 8) & 7;

				int col1bri = palofs >= 16 + 64 * 1;
				int col2bri = palofs >= 16 + 64 * 2;

				int v1 = (col1ink << 0) | (col1pap << 3) | (col1bri ? 64 : 0);
				int v2 = (col2ink << 0) | (col2pap << 3) | (col2bri ? 64 : 0);

				/* // This makes things look a bit better on emulators, but worse on actual device.
				if ((x & 1) ^ (y & 1))
				{
				int t = v1;
				v1 = v2;
				v2 = t;
				}
				*/

				mSpectrumAttributes[y * (gDevice->mXRes / 8) + x] = v1;
				mSpectrumAttributes[y * (gDevice->mXRes / 8) + x + ((gDevice->mYRes / 8) << mOptCellSize) * (gDevice->mXRes / 8)] = v2;

			}
		}
		// Map color indices to palette
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			gBitmapSpec[i] = gSpeccyPalette[gBitmapSpec[i]] | 0xff000000;
		}

#if 0
		for (i = 0; i < gDevice->mYRes; i++)
		{
			for (j = 0; j < gDevice->mXRes; j++)
			{
				int bm = ((gSpectrumBitmap[SPEC_Y(i) * (gDevice->mXRes / 8) + j / 8] >> (7 - (j & 7))) & 1);

				int ax = j / 8;
				int ay = i / 8;

				int a1 = gSpectrumAttributes[ay * (gDevice->mXRes / 8) + ax];
				int a2 = gSpectrumAttributes[ay * (gDevice->mXRes / 8) + ax + (gDevice->mXRes / 8) * (gDevice->mYRes / 8)];

				int c1ink = gSpeccyPalette[((a1 >> 0) & 7) | ((a1 & 64) ? 8 : 0)];
				int c1pap = gSpeccyPalette[((a1 >> 3) & 7) | ((a1 & 64) ? 8 : 0)];

				int c2ink = gSpeccyPalette[((a2 >> 0) & 7) | ((a2 & 64) ? 8 : 0)];
				int c2pap = gSpeccyPalette[((a2 >> 3) & 7) | ((a2 & 64) ? 8 : 0)];

				int c1 = bm ? c1ink : c1pap;
				int c2 = bm ? c2ink : c2pap;

				int c = mix(c1, c2) | 0xff000000;

				int ref = gBitmapSpec[i * gDevice->mXRes + j];

				if (a1 & 128 || a2 & 128)
				{
					c = c;
				}

				if (ref != c)
				{
					c = c;
				}

				gBitmapSpec[i * gDevice->mXRes + j] = c | 0xff000000;
			}
		}
#endif
	}

	virtual void savescr(FILE * f)
	{
		int attrib_size_multiplier = 1 << (mOptCellSize + 1);
		fwrite(mSpectrumBitmap, (gDevice->mXRes / 8) * gDevice->mYRes, 1, f);
		fwrite(mSpectrumAttributes, (gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier, 1, f);
	}

	virtual void saveh(FILE * f)
	{
		int attrib_size_multiplier = 1 << (mOptCellSize + 1);
		int i, c = 0;
		for (i = 0; i < (gDevice->mXRes / 8) * gDevice->mYRes; i++)
		{
			fprintf(f, "%3u,", mSpectrumBitmap[i]);
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
		int attrib_size_multiplier = 1 << (mOptCellSize + 1);
		int i;
		for (i = 0; i < (gDevice->mXRes / 8) * gDevice->mYRes; i++)
		{
			fprintf(f, "\t.db #0x%02x\n", mSpectrumBitmap[i]);
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

				attr = mSpectrumAttributes[(i / cellht) * (gDevice->mXRes / 8) + j / 8 + (gDevice->mXRes / 8) * ((gDevice->mYRes / 8) << mOptCellSize)];
				fg = gSpeccyPalette[((attr >> 0) & 7) | (((attr & 64)) >> 3)] | 0xff000000;
				bg = gSpeccyPalette[((attr >> 3) & 7) | (((attr & 64)) >> 3)] | 0xff000000;
				gBitmapAttr2[i * gDevice->mXRes + j] = (j % 8 < (((8 - cellht) / 2) + i % cellht)) ? fg : bg;

				gBitmapBitm[i * gDevice->mXRes + j] = (mSpectrumBitmap[SPEC_Y(i) * (gDevice->mXRes / 8) + j / 8] & (1 << (7 - (j % 8)))) ? 0xffc0c0c0 : 0xff000000;
			}
		}
		update_texture(gTextureAttr, gBitmapAttr);
		update_texture(gTextureAttr2, gBitmapAttr2);
		update_texture(gTextureBitm, gBitmapBitm);

		ImVec2 picsize((float)gDevice->mXRes, (float)gDevice->mYRes);
		ImGui::Image((ImTextureID)gTextureAttr, picsize, ImVec2(0, 0), ImVec2(gDevice->mXRes / 1024.0f, gDevice->mYRes / 512.0f)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
		ImGui::Image((ImTextureID)gTextureAttr2, picsize, ImVec2(0, 0), ImVec2(gDevice->mXRes / 1024.0f, gDevice->mYRes / 512.0f)); ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
		ImGui::Image((ImTextureID)gTextureBitm, picsize, ImVec2(0, 0), ImVec2(gDevice->mXRes / 1024.0f, gDevice->mYRes / 512.0f)); ImGui::SameLine(); ImGui::Text(" ");
	}
};