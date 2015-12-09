
class ContrastModifier : public Modifier
{
public:
	float mC, mB, mP;
	int mOnce;

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

			if (ImGui::SliderFloat("##Contrast  ", &mC, 0, 5)) { gDirty = 1; }  ImGui::SameLine();
			if (ImGui::Button("Reset##contrast   ")) { gDirty = 1; mC = 1; }ImGui::SameLine();
			ImGui::Text("Contrast");

			if (ImGui::SliderFloat("##Brightness", &mB, -2, 2)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##brightness ")) { gDirty = 1; mB = 0; }ImGui::SameLine();
			ImGui::Text("Brightness");

			if (ImGui::SliderFloat("##Pivot     ", &mP, -2, 2)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##pivot      ")) { gDirty = 1; mP = 0.5f; }ImGui::SameLine();
			ImGui::Text("Pivot");
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{

		int i;
		for (i = 0; i < 256 * 192 * 3; i++)
		{
			gBitmapProcFloat[i] = (gBitmapProcFloat[i] - mP) * mC + mP + mB;
		}
	}
};
