namespace ImGui
{
	int Curve(const char *label, const ImVec2& size, int maxpoints, ImVec2 *points);
	float CurveValue(float p, int maxpoints, const ImVec2 *points);
};

class CurveModifier : public Modifier
{
public:
	ImVec2 mPoints[16];
	int mOnce;

	virtual char *getname() { return "Curve"; }


	virtual void serialize(JSON_Object * root)
	{
		int i;
		char temp[256];
		for (i = 0; i < 16; i++)
		{ 
			sprintf(temp, "mPoints[%d].x", i);
			json_object_dotset_number(root, temp, mPoints[i].x);
			sprintf(temp, "mPoints[%d].y", i);
			json_object_dotset_number(root, temp, mPoints[i].y);
		}
	}

	virtual void deserialize(JSON_Object * root)
	{
		int i;
		char temp[256];
		for (i = 0; i < 16; i++)
		{
			sprintf(temp, "mPoints[%d].x", i);
			if (json_object_dotget_value(root, temp)) mPoints[i].x = (float)json_object_dotget_number(root, temp);
			sprintf(temp, "mPoints[%d].y", i);
			if (json_object_dotget_value(root, temp)) mPoints[i].y = (float)json_object_dotget_number(root, temp);
		}
	}

	virtual int gettype()
	{
		return MOD_CURVE;
	}


	CurveModifier()
	{
		mPoints[0].x = 0;
		mPoints[0].y = 0;
		mPoints[1].x = 1;
		mPoints[1].y = 1;
		mPoints[2].x = -1;
		mPoints[2].y = 0;
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

		if (ImGui::CollapsingHeader("Curve Modifier"))
		{
			ret = common();
			if (ImGui::Curve("", ImVec2(600, 200), 16, mPoints)) gDirty = 1;
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		int i;
		for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
		{
			if (mB_en) gBitmapProcFloat[i * 3 + 0] = ImGui::CurveValue(gBitmapProcFloat[i * 3 + 0], 16, mPoints);
			if (mG_en) gBitmapProcFloat[i * 3 + 1] = ImGui::CurveValue(gBitmapProcFloat[i * 3 + 1], 16, mPoints);
			if (mR_en) gBitmapProcFloat[i * 3 + 2] = ImGui::CurveValue(gBitmapProcFloat[i * 3 + 2], 16, mPoints);
		}
	}

};
