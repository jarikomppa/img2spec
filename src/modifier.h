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
	}

	int common()
	{
		int ret = 0;
		ImGui::Checkbox("Enabled", &mEnabled);
		ImGui::SameLine();
		if (ImGui::Button("Remove")) ret = -1;
		ImGui::SameLine();
		if (ImGui::Button("Move down")) ret = 1;
		return ret;
	}

	virtual int ui() = 0;
	virtual void process() = 0;
};
