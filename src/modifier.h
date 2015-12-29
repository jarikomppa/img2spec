class Modifier
{
public:
	int mUnique;
	bool mEnabled;
	bool mR_en, mG_en, mB_en;
	Modifier * mNext;
	Modifier * mApplyNext;

	Modifier()
	{
		gUniqueValueCounter++;
		mUnique = gUniqueValueCounter;
		mEnabled = true;
		mNext = 0;
		mApplyNext = 0;
		mR_en = true;
		mG_en = true;
		mB_en = true;
	}

	int common(int aRGBControls = 1)
	{
		int ret = 0;
		if (ImGui::Checkbox("Enabled", &mEnabled)) { gDirty = 1; }
		ImGui::SameLine();
		if (ImGui::Button("Remove")) { gDirty = 1; ret = -1; }
		ImGui::SameLine();
		if (ImGui::Button("Move down")) { gDirty = 1; ret = 1; }
		if (aRGBControls)
		{
			ImGui::SameLine();
			if (ImGui::Checkbox("Red enable", &mR_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Green enable", &mG_en)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Blue enable", &mB_en)) { gDirty = 1; }
		}

		return ret;
	}

	template <typename T> static void write(FILE * f, T v) { fwrite(&v, sizeof(T), 1, f); }
	template <typename T> static void read(FILE * f, T &v) { fread(&v, sizeof(T), 1, f); }

	virtual int ui() = 0;
	virtual void process() = 0;
	virtual void serialize(FILE * f) = 0;
	virtual void deserialize(FILE * f) = 0;
	void serialize_common(FILE * f) 
	{ 
		write(f, mEnabled); 
		write(f, mR_en);
		write(f, mG_en);
		write(f, mB_en);
	}
	void deserialize_common(FILE * f) 
	{ 
		read(f, mEnabled); 
		read(f, mR_en);
		read(f, mG_en);
		read(f, mB_en);
	}
	virtual int gettype() = 0;
};
