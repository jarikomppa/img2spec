class YIQModifier : public Modifier
{
public:
	float mY, mI, mQ;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mY);
		write(f, mI);
		write(f, mQ);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mY);
		read(f, mI);
		read(f, mQ);
	}


	YIQModifier()
	{
		mY = mI = mQ = 0;
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

		if (ImGui::CollapsingHeader("YIQ Modifier"))
		{
			ret = common();

			if (ImGui::SliderFloat("##Y  ", &mY, -1, 1)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##Y   ")) { gDirty = 1; mY = 0; } ImGui::SameLine();
			ImGui::Text("Y (brightness)");

			if (ImGui::SliderFloat("##I", &mI, -1, 1)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##I ")) { gDirty = 1; mI = 0; } ImGui::SameLine();
			ImGui::Text("I (blue-red)");

			if (ImGui::SliderFloat("##Q ", &mQ, -1, 1)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##Q  ")) { gDirty = 1; mQ = 0; } ImGui::SameLine();
			ImGui::Text("Q (green-purple)");
		}
		ImGui::PopID();

		return ret;
	}

	virtual void process()
	{
		int c;
		for (c = 0; c < 256 * 192; c++)
		{
			float r = gBitmapProcFloat[c * 3 + 2];
			float g = gBitmapProcFloat[c * 3 + 1];
			float b = gBitmapProcFloat[c * 3 + 0];

			float y = 0.299f * r + 0.587f * g + 0.114f * b;
			float i = 0.596f * r - 0.274f * g - 0.322f * b;
			float q = 0.211f * r - 0.523f * g + 0.312f * b;

			y += mY;
			i += mI;
			q += mQ;

			r = y + 0.956f * i + 0.621f * q;
			g = y - 0.272f * i - 0.647f * q;
			b = y - 1.106f * i + 1.703f * q;

			gBitmapProcFloat[c * 3 + 0] = b;
			gBitmapProcFloat[c * 3 + 1] = g;
			gBitmapProcFloat[c * 3 + 2] = r;
		}
	}
};
