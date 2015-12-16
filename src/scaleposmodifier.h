class ScalePosModifier : public Modifier
{
public:
	int mDirtyPic;
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

	ScalePosModifier()
	{
		mHQ = false;
		mX = 0;
		mY = 0;
		mScale = 1;
		mDirtyPic = 1;
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
			ret = common();

			if (ImGui::SliderFloat("##s  ", &mScale, 0, 1)) { gDirty = 1; mDirtyPic = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##s   ")) { gDirty = 1; mDirtyPic = 1; mScale = 1; } ImGui::SameLine();
			ImGui::Text("Scale");

			if (ImGui::SliderInt("##x  ", &mX, -255, (int)floor(gSourceImageX * mScale))) { gDirty = 1; mDirtyPic = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##x   ")) { gDirty = 1; mDirtyPic = 1; mX = 0; } ImGui::SameLine();
			ImGui::Text("X Offset");

			if (ImGui::SliderInt("##y  ", &mY, -192, (int)floor(gSourceImageY * mScale))) { gDirty = 1; mDirtyPic = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##y   ")) { gDirty = 1; mDirtyPic = 1; mY = 0; } ImGui::SameLine();
			ImGui::Text("Y Offset");

			if (ImGui::Checkbox("High quality scaling", &mHQ)) { gDirty = 1; mDirtyPic = 1; }

		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		if (mDirtyPic && gSourceImageData)
		{
			mDirtyPic = 0;
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

			for (i = 0; i < 192; i++)
			{
				for (j = 0; j < 256; j++)
				{
					int pix = 0xff000000;
					if (j - mX >= 0 &&
						j - mX < w &&
						i - mY >= 0 &&
						i - mY < h)
					{
						pix = temp[(i - mY) * w + j - mX] | 0xff000000;
					}
					gBitmapOrig[i * 256 + j] = pix;
				}
			}
			delete[] temp;

			glBindTexture(GL_TEXTURE_2D, gTextureOrig);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 192, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)gBitmapOrig);

			bitmap_to_float(gBitmapOrig);
		}
	}

};
