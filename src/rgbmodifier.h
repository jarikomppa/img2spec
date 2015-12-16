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
			if (ImGui::SliderFloat("##Red  ", &mR, -1, 1)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##red   ")) { gDirty = 1; mR = 0; } ImGui::SameLine();
			ImGui::Text("Red");

			if (ImGui::SliderFloat("##Green", &mG, -1, 1)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##green ")) { gDirty = 1; mG = 0; } ImGui::SameLine();
			ImGui::Text("Green");

			if (ImGui::SliderFloat("##Blue ", &mB, -1, 1)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##blue  ")) { gDirty = 1; mB = 0; } ImGui::SameLine();
			ImGui::Text("Blue");
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		int i;
		for (i = 0; i < 256 * 192; i++)
		{
			gBitmapProcFloat[i * 3 + 0] += mB;
			gBitmapProcFloat[i * 3 + 1] += mG;
			gBitmapProcFloat[i * 3 + 2] += mR;
		}
	}

};
