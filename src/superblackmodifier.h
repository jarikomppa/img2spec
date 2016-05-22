
class SuperblackModifier : public Modifier
{
public:
	float mThreshold;
	int mOnce;

	virtual char *getname() { return "Superblack"; }


	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mThreshold);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mThreshold);
#pragma warning(default:4244; default:4800)
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
