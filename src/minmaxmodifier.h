class MinmaxModifier : public Modifier
{
public:
	float mV;
	int mAreaX, mAreaY;
	bool mMin;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mV);
		write(f, mAreaX);
		write(f, mAreaY);
		write(f, mMin);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mV);
		read(f, mAreaX);
		read(f, mAreaY);
		read(f, mMin);
	}

	virtual int gettype()
	{
		return MOD_MINMAX;
	}

	MinmaxModifier()
	{
		mMin = true;
		mV = 1;
		mOnce = 0;
		mAreaX = 2;
		mAreaY = 2;
	}

	virtual int ui()
	{
		int ret = 0;
		ImGui::PushID(mUnique);

		if (!mOnce)
		{
			ImGui::OpenNextNode(1);
			mOnce = 1;
		}

		if (ImGui::CollapsingHeader("Min/max Modifier"))
		{
			ret = common();

			complexsliderfloat("Strength", &mV, 0, 1, 1, 0.001f);
			complexsliderint("Kernel width", &mAreaX, 1, 20, 2, 1);
			complexsliderint("Kernel height", &mAreaY, 1, 20, 2, 1);

			if (ImGui::Checkbox(mMin?"Min###minmax":"Max###minmax", &mMin)) { gDirty = 1; }
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		int i, j;
		float * buf = new float[gDevice->mYRes * gDevice->mXRes * 3];
		memcpy(buf, gBitmapProcFloat, gDevice->mYRes * gDevice->mXRes * 3 * sizeof(float));
		
		if (mMin)
		{
			for (i = 0; i < gDevice->mYRes; i++)
			{
				for (j = 0; j < gDevice->mXRes; j++)
				{
					int x, y;
					float rmin = 1;
					float gmin = 1;
					float bmin = 1;
					for (x = 0; x < mAreaX; x++)
					{
						for (y = 0; y < mAreaY; y++)
						{
							if ((j + x - mAreaX / 2) > 0 &&
								(j + x - mAreaX / 2) < gDevice->mXRes &&
								(i + y - mAreaY / 2) > 0 &&
								(i + y - mAreaY / 2) < gDevice->mYRes)
							{
								float b = buf[((i + y - mAreaY / 2) * gDevice->mXRes + j + x - mAreaX / 2) * 3 + 0];
								float g = buf[((i + y - mAreaY / 2) * gDevice->mXRes + j + x - mAreaX / 2) * 3 + 1];
								float r = buf[((i + y - mAreaY / 2) * gDevice->mXRes + j + x - mAreaX / 2) * 3 + 2];
								if (b < bmin) bmin = b;
								if (g < gmin) gmin = g;
								if (r < rmin) rmin = r;
							}
						}
						if (mB_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 0] += (bmin - gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 0]) * mV;
						if (mG_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 1] += (gmin - gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 1]) * mV;
						if (mR_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 2] += (rmin - gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 2]) * mV;
					}
				}
			}
		}
		else
		{
			for (i = 0; i < gDevice->mYRes; i++)
			{
				for (j = 0; j < gDevice->mXRes; j++)
				{
					int x, y;
					float rmax = 0;
					float gmax = 0;
					float bmax = 0;
					for (x = 0; x < mAreaX; x++)
					{
						for (y = 0; y < mAreaY; y++)
						{
							if ((j + x - mAreaX / 2) > 0 &&
								(j + x - mAreaX / 2) < gDevice->mXRes &&
								(i + y - mAreaY / 2) > 0 &&
								(i + y - mAreaY / 2) < gDevice->mYRes)
							{
								float b = buf[((i + y - mAreaY / 2) * gDevice->mXRes + j + x - mAreaX / 2) * 3 + 0];
								float g = buf[((i + y - mAreaY / 2) * gDevice->mXRes + j + x - mAreaX / 2) * 3 + 1];
								float r = buf[((i + y - mAreaY / 2) * gDevice->mXRes + j + x - mAreaX / 2) * 3 + 2];
								if (b > bmax) bmax = b;
								if (g > gmax) gmax = g;
								if (r > rmax) rmax = r;
							}
						}
						if (mB_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 0] += (bmax - gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 0]) * mV;
						if (mG_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 1] += (gmax - gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 1]) * mV;
						if (mR_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 2] += (rmax - gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 2]) * mV;
					}
				}
			}
		}

		delete[] buf;
	}
};
