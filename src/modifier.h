class Modifier
{
public:
	int mUnique;
	bool mEnabled;
	Modifier * mNext;
	Modifier * mApplyNext;

	Modifier()
	{
		gUniqueValueCounter++;
		mUnique = gUniqueValueCounter;
		mEnabled = true;
		mNext = 0;
		mApplyNext = 0;
	}

	int common()
	{
		int ret = 0;
		if (ImGui::Checkbox("Enabled", &mEnabled)) { gDirty = 1; }
		ImGui::SameLine();
		if (ImGui::Button("Remove")) { gDirty = 1; ret = -1; }
		ImGui::SameLine();
		if (ImGui::Button("Move down")) { gDirty = 1; ret = 1; }
		return ret;
	}

	template <typename T> void write(FILE * f, T v) { fwrite(&v, sizeof(T), 1, f); }
	template <typename T> void read(FILE * f, T &v) { fread(&v, sizeof(T), 1, f); }

	virtual int ui() = 0;
	virtual void process() = 0;
	virtual void serialize(FILE * f) = 0;
	virtual void deserialize(FILE * f) = 0;
	void serialize_common(FILE * f) { write(f, mEnabled); }
	void deserialize_common(FILE * f) { read(f, mEnabled); }
};
