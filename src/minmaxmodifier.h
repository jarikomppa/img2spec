class MinmaxModifier : public Modifier
{
public:
	float mV;
	int mArea;
	bool mR_en, mG_en, mB_en;
	bool mMin;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mV);
		write(f, mArea);
		write(f, mR_en);
		write(f, mG_en);
		write(f, mB_en);
		write(f, mMin);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mV);
		read(f, mArea);
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
		mArea = 3;
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

			if (ImGui::SliderInt("##kernel area", &mArea, 2, 16)) { gDirty = 1; }	ImGui::SameLine();
			if (ImGui::Button("Reset##kernel area   ")) { gDirty = 1; mArea = 2; } ImGui::SameLine();
			ImGui::Text("Kernel area");
			
			if (ImGui::Checkbox("Red enable", &mR_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Green enable", &mG_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Blue enable", &mB_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Min (vs max)", &mMin)) { gDirty = 1; }
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		int i, j;
		float * buf = new float[192 * 256 * 3];
		memcpy(buf, gBitmapProcFloat, 192 * 256 * 3 * sizeof(float));
		
		if (mMin)
		{
			for (i = 0; i < 192; i++)
			{
				for (j = 0; j < 256; j++)
				{
					int x, y;
					float rmin = 1;
					float gmin = 1;
					float bmin = 1;
					for (x = 0; x < mArea; x++)
					{
						for (y = 0; y < mArea; y++)
						{
							if ((j + x - mArea / 2) > 0 &&
								(j + x - mArea / 2) < 256 &&
								(i + y - mArea / 2) > 0 &&
								(i + y - mArea / 2) < 192)
							{
								float b = buf[((i + y - mArea / 2) * 256 + j + x - mArea / 2) * 3 + 0];
								float g = buf[((i + y - mArea / 2) * 256 + j + x - mArea / 2) * 3 + 1];
								float r = buf[((i + y - mArea / 2) * 256 + j + x - mArea / 2) * 3 + 2];
								if (b < bmin) bmin = b;
								if (g < gmin) gmin = g;
								if (r < rmin) rmin = r;
							}
						}
						if (mB_en) gBitmapProcFloat[(i * 256 + j) * 3 + 0] += (bmin - gBitmapProcFloat[(i * 256 + j) * 3 + 0]) * mV;
						if (mG_en) gBitmapProcFloat[(i * 256 + j) * 3 + 1] += (gmin - gBitmapProcFloat[(i * 256 + j) * 3 + 1]) * mV;
						if (mR_en) gBitmapProcFloat[(i * 256 + j) * 3 + 2] += (rmin - gBitmapProcFloat[(i * 256 + j) * 3 + 2]) * mV;
					}
				}
			}
		}
		else
		{
			for (i = 0; i < 192; i++)
			{
				for (j = 0; j < 256; j++)
				{
					int x, y;
					float rmax = 0;
					float gmax = 0;
					float bmax = 0;
					for (x = 0; x < mArea; x++)
					{
						for (y = 0; y < mArea; y++)
						{
							if ((j + x - mArea / 2) > 0 &&
								(j + x - mArea / 2) < 256 &&
								(i + y - mArea / 2) > 0 &&
								(i + y - mArea / 2) < 192)
							{
								float b = buf[((i + y - mArea / 2) * 256 + j + x - mArea / 2) * 3 + 0];
								float g = buf[((i + y - mArea / 2) * 256 + j + x - mArea / 2) * 3 + 1];
								float r = buf[((i + y - mArea / 2) * 256 + j + x - mArea / 2) * 3 + 2];
								if (b > bmax) bmax = b;
								if (g > gmax) gmax = g;
								if (r > rmax) rmax = r;
							}
						}
						if (mB_en) gBitmapProcFloat[(i * 256 + j) * 3 + 0] += (bmax - gBitmapProcFloat[(i * 256 + j) * 3 + 0]) * mV;
						if (mG_en) gBitmapProcFloat[(i * 256 + j) * 3 + 1] += (gmax - gBitmapProcFloat[(i * 256 + j) * 3 + 1]) * mV;
						if (mR_en) gBitmapProcFloat[(i * 256 + j) * 3 + 2] += (rmax - gBitmapProcFloat[(i * 256 + j) * 3 + 2]) * mV;
					}
				}
			}
		}

		delete[] buf;
	}
};
