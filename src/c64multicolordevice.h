
class C64MulticolorDevice : public C64HiresDevice
{
public:

	// Spectrum format data
	unsigned char mAttributes[128 * 64 * 8 * 2]; // big enough for 8x1 attribs in 3x64 mode at 1024x512
	unsigned char mBitmap[128 * 512];

	C64MulticolorDevice()
	{
		mOptAttribOrder = 0;
		mOptPaper = 0;
		mOptCellSize = 0;

		mXRes = 320;
		mYRes = 200;

		mOptWidthCells = mXRes / 8;
		mOptHeightCells = mYRes / 8;
	}

	int pick_from_4_c64_cols(int c, int col1, int col2, int col3, int col4)
	{
		int r = (c >> 16) & 0xff;
		int g = (c >> 8) & 0xff;
		int b = (c >> 0) & 0xff;

		int s_b = (gC64Palette[col1] >> 0) & 0xff;
		int s_g = (gC64Palette[col1] >> 8) & 0xff;
		int s_r = (gC64Palette[col1] >> 16) & 0xff;

		int dist1 = (r - s_r) * (r - s_r) +
			(g - s_g) * (g - s_g) +
			(b - s_b) * (b - s_b);

		s_b = (gC64Palette[col2] >> 0) & 0xff;
		s_g = (gC64Palette[col2] >> 8) & 0xff;
		s_r = (gC64Palette[col2] >> 16) & 0xff;

		int dist2 = (r - s_r) * (r - s_r) +
			(g - s_g) * (g - s_g) +
			(b - s_b) * (b - s_b);

		s_b = (gC64Palette[col3] >> 0) & 0xff;
		s_g = (gC64Palette[col3] >> 8) & 0xff;
		s_r = (gC64Palette[col3] >> 16) & 0xff;

		int dist3 = (r - s_r) * (r - s_r) +
			(g - s_g) * (g - s_g) +
			(b - s_b) * (b - s_b);

		s_b = (gC64Palette[col4] >> 0) & 0xff;
		s_g = (gC64Palette[col4] >> 8) & 0xff;
		s_r = (gC64Palette[col4] >> 16) & 0xff;

		int dist4 = (r - s_r) * (r - s_r) +
			(g - s_g) * (g - s_g) +
			(b - s_b) * (b - s_b);

		if (dist1 <= dist2 && dist1 <= dist3 && dist1 <= dist4)
			return col1;
		if (dist2 <= dist1 && dist2 <= dist3 && dist2 <= dist4)
			return col2;
		if (dist3 <= dist1 && dist3 <= dist2 && dist3 <= dist4)
			return col3;
		return col4;
	}


	virtual void filter()
	{
		int x, y, i, j;
		int counts[16];
		for (i = 0; i < 16; i++)
			counts[i] = 0;

		// Find closest colors in the speccy palette
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i+=2) // halve x res
		{
			int r = rgb_to_c64_pal(gBitmapProc[i], 0, 16);
			gBitmapSpec[i] = r;
			counts[r]++;
		}

		int papercolor = 0;
		int best = 0;
		if (mOptPaper == 0)
		{
			for (i = 0; i < 16; i++)
			{
				if (counts[i] > best)
				{
					best = counts[i];
					papercolor = i;
				}
			}
		}
		else
		{
			papercolor = (mOptPaper - 1);
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
				for (i = 0; i < 16; i++)
					counts[i] = 0;

				for (i = 0; i < cellht; i++)
				{
					for (j = 0; j < 8; j += 2) // halve x res
					{
						int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
						int r = rgb_to_c64_pal(gBitmapProc[loc], 0, 16);
						gBitmapSpec[loc] = r;
						counts[r]++;
					}
				}

				// don't count the paper color..
				counts[papercolor] = 0;

				// Find tree most common colors in cell
				int col1 = 0, col2 = 0, col3 = 0;
				int best = 0;
				for (i = 0; i < 16; i++)
				{
					if (counts[i] > best)
					{
						best = counts[i];
						col1 = i;
					}
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

				// Once more, don't count the last color..
				counts[col2] = 0;

				best = 0;
				for (i = 0; i < 16; i++)
				{
					if (counts[i] > best)
					{
						best = counts[i];
						col3 = i;
					}
				}

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
					for (j = 0; j < 4; j++) // halve x res
					{
						int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j * 2;
						int col = pick_from_4_c64_cols(gBitmapProc[loc], papercolor, col1, col2, col3);
						gBitmapSpec[loc] = col;
						gBitmapSpec[loc + 1] = gBitmapSpec[loc];
						int bitmask = 0;
						if (col == papercolor) bitmask = 0;
						if (col == col1) bitmask = 1;
						if (col == col2) bitmask = 2;
						if (col == col3) bitmask = 3;
						
						mBitmap[(y * cellht + i) * (gDevice->mXRes / 8) + x] <<= 1;
						mBitmap[(y * cellht + i) * (gDevice->mXRes / 8) + x] |= (bitmask & 1) != 0;
						mBitmap[(y * cellht + i) * (gDevice->mXRes / 8) + x] <<= 1;
						mBitmap[(y * cellht + i) * (gDevice->mXRes / 8) + x] |= (bitmask & 2) != 0;
					}
				}

				// Store the cell's attribute				
				mAttributes[y * (gDevice->mXRes / 8) + x] = (col2 & 0xf) | ((col1 & 0xf) << 4);
				mAttributes[(y + ymax) * (gDevice->mXRes / 8) + x] = col3;
			}
		}
		mAttributes[ymax * 2 * (gDevice->mXRes / 8)] = papercolor;

		// Map color indices to palette
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			gBitmapSpec[i] = gC64Palette[gBitmapSpec[i]] | 0xff000000;
		}
	}

	virtual void savescr(FILE * f)
	{
		int attrib_size_multiplier = 1 << (mOptCellSize);
		fwrite(mBitmap, (gDevice->mXRes / 8) * gDevice->mYRes, 1, f);
		fwrite(mAttributes, (gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier * 2 + 1, 1, f);
	}

	virtual void saveh(FILE * f)
	{
		int attrib_size_multiplier = 1 << (mOptCellSize);
		int i, c = 0;
		for (i = 0; i < (gDevice->mXRes / 8) * gDevice->mYRes; i++)
		{
			fprintf(f, "%3u,", mBitmap[i]);
			c++;
			if (c >= 32)
			{
				fprintf(f, "\n");
				c = 0;
			}
		}
		fprintf(f, "\n\n");
		c = 0;
		for (i = 0; i < (gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier * 2 + 1; i++)
		{
			fprintf(f, "%3u%s", mAttributes[i], i != ((gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier) - 1 ? "," : "");
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
		int attrib_size_multiplier = 1 << (mOptCellSize);
		int i;
		for (i = 0; i < (gDevice->mYRes / 8) * gDevice->mYRes; i++)
		{
			fprintf(f, "\t.db #0x%02x\n", mBitmap[i]);
		}
		fprintf(f, "\n\n");
		for (i = 0; i < (gDevice->mXRes / 8) * (gDevice->mYRes / 8) * attrib_size_multiplier * 2 + 1; i++)
		{
			fprintf(f, "\t.db #0x%02x\n", mAttributes[i]);
		}
	}

	virtual void attr_bitm()
	{
		ImGui::Text("Not suppored for this device");
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

					if (j != (gDevice->mXRes / 8) - 1)
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
		Modifier::write(f, mOptPaper);
		Modifier::write(f, mOptCellSize);
		Modifier::write(f, mOptWidthCells);
		Modifier::write(f, mOptHeightCells);
	}

	virtual void readOptions(FILE *f)
	{
		Modifier::read(f, mOptAttribOrder);
		Modifier::read(f, mOptPaper);
		Modifier::read(f, mOptCellSize);
		Modifier::read(f, mOptWidthCells);
		Modifier::read(f, mOptHeightCells);

		mXRes = mOptWidthCells * 8;
		mYRes = mOptHeightCells * (8 >> mOptCellSize);

		gDirty = 1;
		gDirtyPic = 1;
	}
};