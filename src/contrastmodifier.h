
class ContrastModifier : public Modifier
{
public:
	float mC, mB, mP;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mC);
		write(f, mB);
		write(f, mP);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mC);
		read(f, mB);
		read(f, mP);
	}

	virtual int gettype()
	{
		return MOD_CONTRAST;
	}

	ContrastModifier()
	{
		mB = 0;
		mC = 1;
		mP = 0.5f;
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

		if (ImGui::CollapsingHeader("Contrast Modifier"))
		{
			ret = common();

			complexsliderfloat("Contrast", &mC, 0, 5, 1, 0.001f);
			complexsliderfloat("Brightness", &mB, -2, 2, 0, 0.001f);
			complexsliderfloat("Pivot", &mP, -2, 2, 0.5f, 0.001f);
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{

		int i;
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			if (mB_en) gBitmapProcFloat[i * 3 + 0] = (gBitmapProcFloat[i * 3 + 0] - mP) * mC + mP + mB;
			if (mG_en) gBitmapProcFloat[i * 3 + 1] = (gBitmapProcFloat[i * 3 + 1] - mP) * mC + mP + mB;
			if (mR_en) gBitmapProcFloat[i * 3 + 2] = (gBitmapProcFloat[i * 3 + 2] - mP) * mC + mP + mB;
		}
	}
};
