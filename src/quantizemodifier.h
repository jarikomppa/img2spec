#define MAX_LEVELS 8
class QuantizeModifier : public Modifier
{
public:
	float mV;
	int mLevels;
	float mScale;
	float mNudge;
	float mRange;
	bool mR_en, mG_en, mB_en;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mV);
		write(f, mLevels);
		write(f, mScale);
		write(f, mNudge);
		write(f, mRange);
		write(f, mR_en);
		write(f, mG_en);
		write(f, mB_en);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mV);
		read(f, mLevels);
		read(f, mScale);
		read(f, mNudge);
		read(f, mRange);
		read(f, mR_en);
		read(f, mG_en);
		read(f, mB_en);
	}

	virtual int gettype()
	{
		return MOD_QUANTIZE;
	}


	QuantizeModifier()
	{
		mV = 1;
		mLevels = 4;
		mScale = 0;
		mNudge = 0;
		mRange = 1;
		mR_en = mG_en = mB_en = true;
		mOnce = 0;
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

		if (ImGui::CollapsingHeader("Quantize Modifier"))
		{
			ret = common();

			if (ImGui::SliderFloat("##Strength", &mV, 0, 1.0f)) { gDirty = 1; }	ImGui::SameLine();
			if (ImGui::Button("Reset##strength   ")) { gDirty = 1; mV = 1; } ImGui::SameLine();
			ImGui::Text("Strength");

			if (ImGui::SliderInt("##levels  ", &mLevels, 2, MAX_LEVELS)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##levels   ")) { gDirty = 1; mLevels = 4; } ImGui::SameLine();
			ImGui::Text("Levels");

			if (ImGui::SliderFloat("##scale", &mScale, -2, 2)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##scale ")) { gDirty = 1; mScale = 0; } ImGui::SameLine();
			ImGui::Text("Exp/Log Scale");

			if (ImGui::SliderFloat("##nudge ", &mNudge, -0.5f, 0.5f)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##nudge  ")) { gDirty = 1; mNudge = 0; } ImGui::SameLine();
			ImGui::Text("Nudge");

			if (ImGui::SliderFloat("##range ", &mRange, 0, 2)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##range  ")) { gDirty = 1; mRange = 1; } ImGui::SameLine();
			ImGui::Text("Range");

			if (ImGui::Checkbox("Red enable", &mR_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Green enable", &mG_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Blue enable", &mB_en)) { gDirty = 1; } 
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		float point[MAX_LEVELS];
		int i;
		float p = 1;
		if (mScale > 0)
		{
			p = mScale + 1;
		}
		else
		{
			p = 1 / (1 + -mScale);
		}

		for (i = 0; i < mLevels; i++)
		{
			point[i] = pow(i / ((float)mLevels - 1), p) * mRange - (mRange - 1)/2 + mNudge;
		}

		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			int c;
			for (c = 0; c < 3; c++)
			{
				if ((c == 0 && mB_en) ||
					(c == 1 && mG_en) ||
					(c == 2 && mR_en))
				{
					int j;
					float dist = 1000;
					float best = 0;
					float p = gBitmapProcFloat[i * 3 + c];
					for (j = 0; j < mLevels; j++)
					{
						float d = abs(p - point[j]);
						if (d < dist)
						{
							best = point[j];
							dist = d;
						}
					}
					gBitmapProcFloat[i * 3 + c] += (best - p) * mV;
				}
			}
		}
	}

};
#undef MAX_LEVELS