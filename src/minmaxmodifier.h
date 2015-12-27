class MinmaxModifier : public Modifier
{
public:
	float mV;
	int mAreaX, mAreaY;
	bool mR_en, mG_en, mB_en;
	bool mMin;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mV);
		write(f, mAreaX);
		write(f, mAreaY);
		write(f, mR_en);
		write(f, mG_en);
		write(f, mB_en);
		write(f, mMin);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mV);
		read(f, mAreaX);
		read(f, mAreaY);
		read(f, mR_en);
		read(f, mG_en);
		read(f, mB_en);
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
		mR_en = true;
		mG_en = true;
		mB_en = true;
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
			if (ImGui::SliderFloat("##Strength", &mV, 0, 1.0f)) { gDirty = 1; }	ImGui::SameLine();
			if (ImGui::Button("Reset##strength   ")) { gDirty = 1; mV = 1; } ImGui::SameLine();
			ImGui::Text("Strength");

			if (ImGui::SliderInt("##kernel areaX", &mAreaX, 1, 20)) { gDirty = 1; }	ImGui::SameLine();
			if (ImGui::Button("Reset##kernel areaX   ")) { gDirty = 1; mAreaX = 2; } ImGui::SameLine();
			ImGui::Text("Kernel width");

			if (ImGui::SliderInt("##kernel areaY", &mAreaY, 1, 20)) { gDirty = 1; }	ImGui::SameLine();
			if (ImGui::Button("Reset##kernel areaY   ")) { gDirty = 1; mAreaY = 2; } ImGui::SameLine();
			ImGui::Text("Kernel height");

			if (ImGui::Checkbox("Red enable", &mR_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Green enable", &mG_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Blue enable", &mB_en)) { gDirty = 1; } ImGui::SameLine();
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
