class OrderedDitherModifier : public Modifier
{
public:
	float mV;
	int mOnce;
	int mXOfs, mYOfs;
	int mMatrix;

	virtual char *getname() { return "OrderedDither"; }


	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mV);
		SERIALIZE(mXOfs);
		SERIALIZE(mYOfs);
		SERIALIZE(mMatrix);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mV);
		DESERIALIZE(mXOfs);
		DESERIALIZE(mYOfs);
		DESERIALIZE(mMatrix);
#pragma warning(default:4244; default:4800)
	}

	virtual int gettype()
	{
		return MOD_ORDEREDDITHER;
	}


	OrderedDitherModifier()
	{
		mV = 1.0f;
		mOnce = 0;
		mXOfs = 0;
		mYOfs = 0;
		mMatrix = 4;
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

		if (ImGui::CollapsingHeader("Ordered Dither Modifier"))
		{
			ret = common();
			complexsliderfloat("Strength", &mV, 0, 2, 1, 0.001f);

			if (ImGui::Combo("##Matrix  ", &mMatrix, "2x2\0" "3x3\0" "3x3 (alt)\0" "4x4\0" "8x8\0")) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("-##matrix")) { gDirty = 1;  mMatrix = (mMatrix + 5 - 1) % 5; } ImGui::SameLine();
			if (ImGui::Button("+##matrix")) { gDirty = 1;  mMatrix = (mMatrix + 5 + 1) % 5; } ImGui::SameLine();
			if (ImGui::Button("Reset##matrix     ")) { gDirty = 1; mMatrix = 4; } ImGui::SameLine();
			ImGui::Text("Matrix");

			complexsliderint("X Offset", &mXOfs, 0, 8, 0, 1);
			complexsliderint("Y Offset", &mYOfs, 0, 8, 0, 1);

		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{

		float matrix2x2[] =
		{
			1.0f, 3.0f,
			4.0f, 2.0f
		};

		float matrix3x3[] =
		{
			8.0f, 3.0f, 4.0f,
			6.0f, 1.0f, 2.0f,
			7.0f, 5.0f, 9.0f
		};

		float matrix3x3alt[] =
		{
			1.0f, 7.0f, 4.0f,
			5.0f, 8.0f, 3.0f,
			6.0f, 2.0f, 9.0f
		};

		float matrix4x4[] =
		{
			1.0f, 9.0f, 3.0f, 11.0f,
			13.0f, 5.0f, 15.0f, 7.0f,
			4.0f, 12.0f, 2.0f, 10.0f,
			16.0f, 8.0f, 14.0f, 6.0f
		};

		float matrix8x8[] =
		{
			0.0f, 32.0f, 8.0f, 40.0f, 2.0f, 34.0f, 10.0f, 42.0f,
			48.0f, 16.0f, 56.0f, 24.0f, 50.0f, 18.0f, 58.0f, 26.0f,
			12.0f, 44.0f, 4.0f, 36.0f, 14.0f, 46.0f, 6.0f, 38.0f,
			60.0f, 28.0f, 52.0f, 20.0f, 62.0f, 30.0f, 54.0f, 22.0f,
			3.0f, 35.0f, 11.0f, 43.0f, 1.0f, 33.0f, 9.0f, 41.0f,
			51.0f, 19.0f, 59.0f, 27.0f, 49.0f, 17.0f, 57.0f, 25.0f,
			15.0f, 47.0f, 7.0f, 39.0f, 13.0f, 45.0f, 5.0f, 37.0f,
			63.0f, 31.0f, 55.0f, 23.0f, 61.0f, 29.0f, 53.0f, 21.0f
		};

		float *matrix;
		int matsize;
		float matdiv;

		switch (mMatrix)
		{
		case 0:
			matrix = matrix2x2;
			matsize = 2;
			matdiv = 4.0f;
			break;
		case 1:
			matrix = matrix3x3;
			matsize = 3;
			matdiv = 9.0f;
			break;
		case 2:
			matrix = matrix3x3alt;
			matsize = 3;
			matdiv = 9.0f;
			break;
		case 3:
			matrix = matrix4x4;
			matsize = 4;
			matdiv = 16.0f;
			break;
//		case 4:
		default:
			matrix = matrix8x8;
			matsize = 8;
			matdiv = 63.0f;
			break;
		}
		int i, j;
		for (i = 0; i < gDevice->mYRes; i++)
		{
			for (j = 0; j < gDevice->mXRes; j++)
			{
				if (mB_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 0] += (matrix[((i + mYOfs) % matsize) * matsize + ((j + mXOfs) % matsize)] / matdiv - 0.5f) * gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 0] * mV;
				if (mG_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 1] += (matrix[((i + mYOfs) % matsize) * matsize + ((j + mXOfs) % matsize)] / matdiv - 0.5f) * gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 1] * mV;
				if (mR_en) gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 2] += (matrix[((i + mYOfs) % matsize) * matsize + ((j + mXOfs) % matsize)] / matdiv - 0.5f) * gBitmapProcFloat[(i * gDevice->mXRes + j) * 3 + 2] * mV;
			}
		}
	}
};
