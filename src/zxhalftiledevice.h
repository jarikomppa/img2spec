class ZXHalfTileDevice : public ZXSpectrumDevice
{
public:

	int mOptTileType;

	ZXHalfTileDevice() 
	{
		mOptTileType = 0;
	}

	virtual void options()
	{
		if (ImGui::Combo("Wide or tall tiles", &mOptTileType, "Tall (64x24)\0Wide (32x48)\0")) { gDirty = 1; gDirtyPic = 1; }
		if (ImGui::SliderFloat("Bright attribute bias", &mOptBright, 0, 1)) gDirty = 1;
		if (ImGui::Combo("Attribute cell size", &mOptCellSize, "8x8 (standard)\08x4 (bicolor)\08x2\08x1\0")) { gDirty = 1; mOptHeightCells = mXRes / (8 >> mOptCellSize); mXRes = mOptHeightCells * (8 >> mOptCellSize); gDirtyPic = 1; }
		if (ImGui::SliderInt("Bitmap width in cells", &mOptWidthCells, 1, 1028 / 8)) { gDirty = 1; gDirtyPic = 1; mXRes = mOptWidthCells * 8; }
		if (ImGui::SliderInt("Bitmap height in cells", &mOptHeightCells, 1, 512 / (8 >> mOptCellSize))) { gDirty = 1; gDirtyPic = 1; mYRes = mOptHeightCells * (8 >> mOptCellSize); }
		ImGui::Combo("Bitmap order when saving", &mOptScreenOrder, "Linear order\0Spectrum video RAM order\0");
	}

	virtual void writeOptions(FILE *f)
	{
		Modifier::write(f, mOptTileType);
		Modifier::write(f, mOptBright);
		Modifier::write(f, mOptCellSize);
		Modifier::write(f, mOptScreenOrder);
		Modifier::write(f, mOptWidthCells);
		Modifier::write(f, mOptHeightCells);
	}

	virtual void readOptions(FILE *f)
	{
		Modifier::read(f, mOptTileType);
		Modifier::read(f, mOptBright);
		Modifier::read(f, mOptCellSize);
		Modifier::read(f, mOptScreenOrder);
		Modifier::read(f, mOptWidthCells);
		Modifier::read(f, mOptHeightCells);

		mXRes = mOptWidthCells * 8;
		mYRes = mOptHeightCells * (8 >> mOptCellSize);

		gDirty = 1;
		gDirtyPic = 1;
	}


	virtual int estimate_rgb(int c)
	{
		return gSpeccyPalette[rgb_to_speccy_pal(c, 0, 16)] | 0xff000000;
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

				int counts_left[16];
				int counts_right[16];
				for (i = 0; i < 16; i++)
				{
					counts_left[i] = 0;
					counts_right[i] = 0;
				}

				// Remap with just bright OR dark colors, based on whether the whole cell is bright or not
				for (i = 0; i < cellht; i++)
				{
					for (j = 0; j < 8; j++)
					{
						int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
						int r = rgb_to_speccy_pal(gBitmapProc[loc], brights, 8);
						gBitmapSpec[loc] = r;
						if (mOptTileType == 0)
						{
							if (j < 4)
							{
								counts_left[r]++;
							}
							else
							{
								counts_right[r]++;
							}
						}
						else
						{
							if (i < cellht/2)
							{
								counts_left[r]++;
							}
							else
							{
								counts_right[r]++;
							}
						}
					}
				}

				// Find most common colors in left/right regions
				int col1 = 0, col2 = 0;
				int best = 0;
				for (i = 0; i < 16; i++)
				{
					if (counts_left[i] > best)
					{
						best = counts_left[i];
						col1 = i;
					}
				}

				best = 0;
				for (i = 0; i < 16; i++)
				{
					if (counts_right[i] > best)
					{
						best = counts_right[i];
						col2 = i;
					}
				}

				// Make sure we're using bright black
				if (col2 == 0 && col1 > 7) col2 = 8;

				// Final pass on cell, select which of two colors we can use
				for (i = 0; i < cellht; i++)
				{
					for (j = 0; j < 8; j++)
					{
						int loc = (y * cellht + i) * gDevice->mXRes + x * 8 + j;
						int bit = 0;
						if (mOptTileType == 0)
						{
							bit = j < 4;
						}
						else
						{
							bit = i < (cellht / 2);
						}
						gBitmapSpec[loc] = bit ? col1 : col2;
						mSpectrumBitmap[SPEC_Y(y * cellht + i) * (gDevice->mXRes / 8) + x] <<= 1;
						mSpectrumBitmap[SPEC_Y(y * cellht + i) * (gDevice->mXRes / 8) + x] |= bit;
						mSpectrumBitmapLinear[(y * cellht + i) * (gDevice->mXRes / 8) + x] <<= 1;
						mSpectrumBitmapLinear[(y * cellht + i) * (gDevice->mXRes / 8) + x] |= bit;
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
};