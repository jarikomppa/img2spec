class RGBModifier : public Modifier
{
public:
	float mR, mG, mB;
	int mOnce;

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
			ImGui::SliderFloat("##Red  ", &mR, -1, 1); ImGui::SameLine();
			if (ImGui::Button("Reset##red   ")) mR = 0; ImGui::SameLine();
			ImGui::Text("Red");

			ImGui::SliderFloat("##Green", &mG, -1, 1); ImGui::SameLine();
			if (ImGui::Button("Reset##green ")) mG = 0; ImGui::SameLine();
			ImGui::Text("Green");

			ImGui::SliderFloat("##Blue ", &mB, -1, 1); ImGui::SameLine();
			if (ImGui::Button("Reset##blue  ")) mB = 0; ImGui::SameLine();
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
