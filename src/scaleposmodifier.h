class ScalePosModifier : public Modifier
{
public:
	int mX, mY;
	bool mHQ;
	float mScale;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mX);
		write(f, mY);
		write(f, mHQ);
		write(f, mScale);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mX);
		read(f, mY);
		read(f, mHQ);
		read(f, mScale);
	}

	virtual int gettype()
	{
		return MOD_SCALEPOS;
	}

	ScalePosModifier()
	{
		mHQ = false;
		mX = 0;
		mY = 0;
		mScale = 1;
		gDirtyPic = 1;
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

		if (ImGui::CollapsingHeader("Scale / Position Modifier"))
		{
			ret = common(0); // don't show RGB enable controls

			if (complexsliderfloat("Scale", &mScale, 0, 2, 1, 0.001f)) gDirtyPic = 1;

			if (ImGui::Button("Fit horizontally")) { if (gSourceImageX) mScale = (float)gDevice->mXRes / gSourceImageX; gDirtyPic = 1; gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Fit vertically")) { if (gSourceImageX) mScale = (float)gDevice->mYRes / gSourceImageY; gDirtyPic = 1; gDirty = 1; }
			
			if (complexsliderint("X Offset", &mX, -gDevice->mXRes, (int)floor(gSourceImageX * mScale), 0, 1)) gDirtyPic = 1;
			if (complexsliderint("Y Offset", &mY, -gDevice->mYRes, (int)floor(gSourceImageY * mScale), 0, 1)) gDirtyPic = 1;

			if (ImGui::Checkbox("High quality scaling", &mHQ)) { gDirty = 1; gDirtyPic = 1; }

		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		if (gDirtyPic && gSourceImageData)
		{
			int i, j;
			int h = (int)floor(gSourceImageY * mScale);
			int w = (int)floor(gSourceImageX * mScale);
			unsigned int *temp = new unsigned int[h * w];
			if (mHQ)
			{
				stbir_resize_uint8(
					(unsigned char*)gSourceImageData, gSourceImageX, gSourceImageY, 0,
					(unsigned char*)temp, w, h, 0, 4);
			}
			else
			{
				for (i = 0; i < h; i++)
				{
					for (j = 0; j < w; j++)
					{
						temp[i * w + j] = gSourceImageData[(i * gSourceImageY / h) * gSourceImageX + (j * gSourceImageX / w)] | 0xff000000;
					}
				}
			}

			for (i = 0; i < gDevice->mYRes; i++)
			{
				for (j = 0; j < gDevice->mXRes; j++)
				{
					int pix = 0xff000000;
					if (j - mX >= 0 &&
						j - mX < w &&
						i - mY >= 0 &&
						i - mY < h)
					{
						pix = temp[(i - mY) * w + j - mX] | 0xff000000;
					}
					gBitmapOrig[i * gDevice->mXRes + j] = pix;
				}
			}
			delete[] temp;

			update_texture(gTextureOrig, gBitmapOrig);

			bitmap_to_float(gBitmapOrig);
		}
	}

};
