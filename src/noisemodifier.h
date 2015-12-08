class NoiseModifier : public Modifier
{
public:
	float mV;
	int mSeed;
	bool mColornoise;
	bool mR_en, mG_en, mB_en;
	int mOnce;

	NoiseModifier()
	{
		mV = 0;
		mSeed = 0;
		mColornoise = false;
		mOnce = 0;
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

		if (ImGui::CollapsingHeader("Noise Modifier"))
		{
			ret = common();
			ImGui::SliderFloat("##Strength", &mV, 0, 0.25f);	ImGui::SameLine();
			if (ImGui::Button("Reset##strength   ")) mV = 0; ImGui::SameLine();
			if (ImGui::Button("Randomize##strength")) mV = 0.25f * ((SDL_GetTicks() * 74531) % 65535) / 65535.0f; ImGui::SameLine();
			ImGui::Text("Strength");

			ImGui::SliderInt("##Seed    ", &mSeed, 0, 65535); ImGui::SameLine();
			if (ImGui::Button("Reset##seed       ")) mSeed = 0; ImGui::SameLine();
			if (ImGui::Button("Randomize")) mSeed = (SDL_GetTicks() * 74531) % 65535; ImGui::SameLine();
			ImGui::Text("Seed");

			ImGui::Checkbox("Color noise", &mColornoise);   ImGui::SameLine();
			ImGui::Checkbox("Red enable", &mR_en); ImGui::SameLine();
			ImGui::Checkbox("Green enable", &mG_en); ImGui::SameLine();
			ImGui::Checkbox("Blue enable", &mB_en);
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{

		int i;
		srand(mSeed);
		for (i = 0; i < 256 * 192; i++)
		{

			float r = (((rand() % 1024) - 512) / 512.0f) * mV;
			float g = (((rand() % 1024) - 512) / 512.0f) * mV;
			float b = (((rand() % 1024) - 512) / 512.0f) * mV;
			if (!mColornoise)
			{
				r = g = b;
			}
			if (mB_en) gBitmapProcFloat[i * 3 + 0] += b;
			if (mG_en) gBitmapProcFloat[i * 3 + 1] += g;
			if (mR_en) gBitmapProcFloat[i * 3 + 2] += r;
		}
	}
};
