class NoiseModifier : public Modifier
{
public:
	float mV;
	int mSeed;
	bool mColornoise;
	bool mR_en, mG_en, mB_en;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mV);
		write(f, mSeed);
		write(f, mColornoise);
		write(f, mR_en);
		write(f, mG_en);
		write(f, mB_en);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mV);
		read(f, mSeed);
		read(f, mColornoise);
		read(f, mR_en);
		read(f, mG_en);
		read(f, mB_en);
	}

	virtual int gettype()
	{
		return MOD_NOISE;
	}

	NoiseModifier()
	{
		mV = 0.1f;
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
			if (ImGui::SliderFloat("##Strength", &mV, 0, 0.25f)) { gDirty = 1; }	ImGui::SameLine();
			if (ImGui::Button("Reset##strength   ")) { gDirty = 1; mV = 0.1f; } ImGui::SameLine();
			if (ImGui::Button("Randomize##strength")) { gDirty = 1; mV = 0.25f * ((SDL_GetTicks() * 74531) % 65535) / 65535.0f; } ImGui::SameLine();
			ImGui::Text("Strength");

			if (ImGui::SliderInt("##Seed    ", &mSeed, 0, 65535)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##seed       ")) { gDirty = 1; mSeed = 0; } ImGui::SameLine();
			if (ImGui::Button("Randomize")) { gDirty = 1; mSeed = (SDL_GetTicks() * 74531) % 65535; } ImGui::SameLine();
			ImGui::Text("Seed");

			if (ImGui::Checkbox("Color noise", &mColornoise)) { gDirty = 1; };   ImGui::SameLine();
			if (ImGui::Checkbox("Red enable", &mR_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Green enable", &mG_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Blue enable", &mB_en)) { gDirty = 1; }
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
