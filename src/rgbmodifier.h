class RGBModifier : public Modifier
{
public:
	float mR, mG, mB;
	int mOnce;

	virtual char *getname() { return "RGB"; }


	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mR);
		SERIALIZE(mG);
		SERIALIZE(mB);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mR);
		DESERIALIZE(mG);
		DESERIALIZE(mB);
#pragma warning(default:4244; default:4800)
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
