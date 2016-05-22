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

	static int complexsliderfloat(char *aName, float *aValue, float aX0, float aX1, float aReset, float aNudge)
	{
		char buttonstring[256];
		char resetstring[256];
		char plusstring[256];
		char minusstring[256];
		sprintf(buttonstring, "##%s", aName);
		sprintf(resetstring, "Reset##%s", aName);
		sprintf(minusstring, "-##%s", aName);
		sprintf(plusstring, "+##%s", aName);
		int modified = 0;

		if (ImGui::SliderFloat(buttonstring, aValue, aX0, aX1)) { modified = 1; }	ImGui::SameLine();
		if (ImGui::Button(minusstring) && *aValue - aNudge >= aX0) { modified = 1; *aValue -= aNudge; } ImGui::SameLine();
		if (ImGui::Button(plusstring) && *aValue + aNudge <= aX1) { modified = 1; *aValue += aNudge; } ImGui::SameLine();
		if (ImGui::Button(resetstring)) { modified = 1; *aValue = aReset; } ImGui::SameLine();
		ImGui::Text(aName);

		gDirty = gDirty | modified;
		
		return modified;
	}

	static int complexsliderint(char *aName, int *aValue, int aX0, int aX1, int aReset, int aNudge)
	{
		char buttonstring[256];
		char resetstring[256];
		char plusstring[256];
		char minusstring[256];
		sprintf(buttonstring, "##%s", aName);
		sprintf(resetstring, "Reset##%s", aName);
		sprintf(minusstring, "-##%s", aName);
		sprintf(plusstring, "+##%s", aName);
		int modified = 0;

		if (ImGui::SliderInt(buttonstring, aValue, aX0, aX1)) { modified = 1; }	ImGui::SameLine();
		if (ImGui::Button(minusstring) && *aValue - aNudge >= aX0) { modified = 1; *aValue -= aNudge; } ImGui::SameLine();
		if (ImGui::Button(plusstring) && *aValue + aNudge <= aX1) { modified = 1; *aValue += aNudge; } ImGui::SameLine();
		if (ImGui::Button(resetstring)) { modified = 1; *aValue = aReset; } ImGui::SameLine();

		gDirty = gDirty | modified;

		ImGui::Text(aName);
		return modified;
	}

	virtual int ui() = 0;
	virtual void process() = 0;
	virtual void serialize(JSON_Object *root) = 0;
	virtual void deserialize(JSON_Object *root) = 0;

	void serialize_common(JSON_Object * root)
	{
		SERIALIZE(mEnabled);
		SERIALIZE(mR_en);
		SERIALIZE(mG_en);
		SERIALIZE(mB_en);
	}

	void deserialize_common(JSON_Object *root) 
	{ 
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mEnabled); 
		DESERIALIZE(mR_en);
		DESERIALIZE(mG_en);
		DESERIALIZE(mB_en);
#pragma warning(default:4244; default:4800)
	}
	virtual int gettype() = 0;
	virtual char *getname() = 0;
};
