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

	virtual int ui() = 0;
	virtual void process() = 0;
};
