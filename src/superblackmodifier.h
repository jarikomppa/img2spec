
class SuperblackModifier : public Modifier
{
public:
	float mThreshold;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mThreshold);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mThreshold);
	}

	virtual int gettype()
	{
		return MOD_SUPERBLACK;
	}

	SuperblackModifier()
	{
		mThreshold = 0;
		mOnce = 0;
	}

	virtual int ui()
	{
		int ret = 0;
		ImGui::PushID(mUnique);

		if (!mOnce) {
			ImGui::OpenNextNode(1);
			mOnce = 1;
		}

		if (ImGui::CollapsingHeader("Superblack Modifier"))
		{
			ret = common();

			complexsliderfloat("Threshold", &mThreshold, -1, 1, 0, 0.001f);
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		int i;
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			if (mB_en && gBitmapProcFloat[i * 3 + 0] <= mThreshold) gBitmapProcFloat[i * 3 + 0] = -1;
			if (mG_en && gBitmapProcFloat[i * 3 + 1] <= mThreshold) gBitmapProcFloat[i * 3 + 1] = -1;
			if (mR_en && gBitmapProcFloat[i * 3 + 2] <= mThreshold) gBitmapProcFloat[i * 3 + 2] = -1;
		}
	}
};
