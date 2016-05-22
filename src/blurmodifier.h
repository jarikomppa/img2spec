class BlurModifier : public Modifier
{
public:
	float mV;
	int mAreaX, mAreaY;
	bool mNegate;
	int mOnce;

	virtual char *getname() { return "Blur"; }


	virtual void serialize(JSON_Object * root)
	{
		SERIALIZE(mV);
		SERIALIZE(mAreaX);
		SERIALIZE(mAreaY);
		SERIALIZE(mNegate);
	}

	virtual void deserialize(JSON_Object * root)
	{
#pragma warning(disable:4244; disable:4800)
		DESERIALIZE(mV);
		DESERIALIZE(mAreaX);
		DESERIALIZE(mAreaY);
		DESERIALIZE(mNegate);
#pragma warning(default:4244; default:4800)
	}

	virtual int gettype()
	{
		return MOD_BLUR;
	}

	BlurModifier()
	{
		mNegate = false;
		mV = 1;
		mOnce = 0;
		mAreaX = 3;
		mAreaY = 3;
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

		if (ImGui::CollapsingHeader("Blur Modifier"))
		{
			ret = common();

			complexsliderfloat("Strength", &mV, 0, 1, 1, 0.001f);
			complexsliderint("Kernel width", &mAreaX, 1, 32, 2, 1);
			complexsliderint("Kernel height", &mAreaY, 1, 32, 2, 1);

			if (ImGui::Checkbox("Subtract blurred from source", &mNegate)) { gDirty = 1; }
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		int i, j, c;
		float * buf = new float[gDevice->mYRes * gDevice->mXRes * 3 * 3];
		float * rbuf = buf + (gDevice->mXRes * gDevice->mYRes * 0);
		float * gbuf = buf + (gDevice->mXRes * gDevice->mYRes * 1);
		float * bbuf = buf + (gDevice->mXRes * gDevice->mYRes * 2);

		for (j = 0, c = 0; j < gDevice->mYRes; j++)
		{
			float ra = 0, ga = 0, ba = 0;
			for (i = 0; i < gDevice->mXRes; i++, c++)
			{
				ra += gBitmapProcFloat[c * 3 + 2];
				ga += gBitmapProcFloat[c * 3 + 1];
				ba += gBitmapProcFloat[c * 3 + 0];

				if (j == 0)
				{
					rbuf[c] = ra;
					gbuf[c] = ga;
					bbuf[c] = ba;
				}
				else
				{
					rbuf[c] = ra + rbuf[c - gDevice->mXRes];
					gbuf[c] = ga + gbuf[c - gDevice->mXRes];
					bbuf[c] = ba + bbuf[c - gDevice->mXRes];
				}
			}
		}

		for (j = 0, c = 0; j < gDevice->mYRes; j++)
		{
			for (i = 0; i < gDevice->mXRes; i++, c++)
			{
				int x1 = i - mAreaX / 2;
				int y1 = j - mAreaY / 2;
				int x2 = x1 + mAreaX - 1;
				int y2 = y1 + mAreaY - 1;

				if (mAreaX == 1)
				{
					x1 = x2 = i;
					x2++;
				}
				if (mAreaY == 1)
				{
					y1 = y2 = j;
					y2++;
				}

				int div = (x2 - x1) * (y2 - y1);
				if (x1 < 0) x1 = 0;
				if (x1 >= gDevice->mXRes) x1 = gDevice->mXRes-1;
				if (x2 < 0) x2 = 0;
				if (x2 >= gDevice->mXRes) x2 = gDevice->mXRes-1;
				if (y1 < 0) y1 = 0;
				if (y1 >= gDevice->mYRes) y1 = gDevice->mYRes-1;
				if (y2 < 0) y2 = 0;
				if (y2 >= gDevice->mYRes) y2 = gDevice->mYRes-1;

				y1 *= gDevice->mXRes;
				y2 *= gDevice->mXRes;
				float r = rbuf[y2 + x2] - rbuf[y1 + x2] - rbuf[y2 + x1] + rbuf[y1 + x1];
				float g = gbuf[y2 + x2] - gbuf[y1 + x2] - gbuf[y2 + x1] + gbuf[y1 + x1];
				float b = bbuf[y2 + x2] - bbuf[y1 + x2] - bbuf[y2 + x1] + bbuf[y1 + x1];

				r /= div;
				g /= div;
				b /= div;

				if (mNegate)
				{
					if (mB_en) gBitmapProcFloat[c * 3 + 0] -= b * mV;
					if (mG_en) gBitmapProcFloat[c * 3 + 1] -= g * mV;
					if (mR_en) gBitmapProcFloat[c * 3 + 2] -= r * mV;
				}
				else
				{
					if (mB_en) gBitmapProcFloat[c * 3 + 0] += (b - gBitmapProcFloat[c * 3 + 0]) * mV;
					if (mG_en) gBitmapProcFloat[c * 3 + 1] += (g - gBitmapProcFloat[c * 3 + 1]) * mV;
					if (mR_en) gBitmapProcFloat[c * 3 + 2] += (r - gBitmapProcFloat[c * 3 + 2]) * mV;
				}
			}
		}

	}
};
