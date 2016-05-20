#define MAX_LEVELS 8
class QuantizeModifier : public Modifier
{
public:
	float mV;
	int mLevels;
	float mScale;
	float mNudge;
	float mRange;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mV);
		write(f, mLevels);
		write(f, mScale);
		write(f, mNudge);
		write(f, mRange);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mV);
		read(f, mLevels);
		read(f, mScale);
		read(f, mNudge);
		read(f, mRange);
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

			complexsliderfloat("Strength", &mV, 0, 1, 1, 0.001f);
			complexsliderint("Levels", &mLevels, 2, MAX_LEVELS, 4, 1);
			complexsliderfloat("Exp/Log Scale", &mScale, -2, 2, 0, 0.001f);
			complexsliderfloat("Nudge", &mNudge, -0.5f, 0.5f, 0, 0.001f);
			complexsliderfloat("Range", &mRange, 0, 2, 1, 0.001f);
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