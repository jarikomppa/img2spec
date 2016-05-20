class RGBModifier : public Modifier
{
public:
	float mR, mG, mB;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mR);
		write(f, mG);
		write(f, mB);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mR);
		read(f, mG);
		read(f, mB);
	}

	virtual int gettype()
	{
		return MOD_RGB;
	}


	RGBModifier()
	{
		mR = mG = mB = 0;
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

		if (ImGui::CollapsingHeader("RGB Modifier"))
		{
			ret = common();

			complexsliderfloat("Red", &mR, -1, 1, 0, 0.001f);
			complexsliderfloat("Green", &mG, -1, 1, 0, 0.001f);
			complexsliderfloat("Blue", &mB, -1, 1, 0, 0.001f);
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		int i;
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			if (mB_en) gBitmapProcFloat[i * 3 + 0] += mB;
			if (mG_en) gBitmapProcFloat[i * 3 + 1] += mG;
			if (mR_en) gBitmapProcFloat[i * 3 + 2] += mR;
		}
	}

};
