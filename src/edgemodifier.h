class EdgeModifier : public Modifier
{
public:
	float mThreshold;
	float mV;
	bool mDirectional;
	bool mSeparate;
	bool mAntialias;
	float mFillColor[3];
	float mDirection;
	int mOnce;

	virtual void serialize(FILE * f)
	{
		write(f, mV);
		write(f, mThreshold);
		write(f, mDirectional);
		write(f, mSeparate);
		write(f, mAntialias);
		write(f, mDirection);
		write(f, mFillColor[0]);
		write(f, mFillColor[1]);
		write(f, mFillColor[2]);
	}

	virtual void deserialize(FILE * f)
	{
		read(f, mV);
		read(f, mThreshold);
		read(f, mDirectional);
		read(f, mSeparate);
		read(f, mAntialias);
		read(f, mDirection);
		read(f, mFillColor[0]);
		read(f, mFillColor[1]);
		read(f, mFillColor[2]);
	}

	virtual int gettype()
	{
		return MOD_EDGE;
	}


	EdgeModifier()
	{
		mV = 1;
		mThreshold = 1;
		mOnce = 0;
		mSeparate = false;
		mDirectional = false;
		mAntialias = false;
		mFillColor[0] = 0;
		mFillColor[1] = 0;
		mFillColor[2] = 0;
		mDirection = 0;
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

		if (ImGui::CollapsingHeader("Edge Modifier"))
		{
			ret = common();

			if (ImGui::SliderFloat("##Strength", &mV, 0, 1.0f)) { gDirty = 1; }	ImGui::SameLine();
			if (ImGui::Button("Reset##strength   ")) { gDirty = 1; mV = 1; } ImGui::SameLine();
			ImGui::Text("Strength");

			if (ImGui::SliderFloat("##Threshold  ", &mThreshold, 0, 2)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##Threshold   ")) { gDirty = 1; mThreshold = 1; } ImGui::SameLine();
			ImGui::Text("Threshold");

			if (ImGui::SliderFloat("##Direction  ", &mDirection, 0, 1)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##Direction   ")) { gDirty = 1; mDirection = 0; } ImGui::SameLine();
			ImGui::Text("Direction");

			if (ImGui::ColorEdit3("##fill", mFillColor)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Button("Reset##fill   ")) { gDirty = 1; mFillColor[0] = 0; mFillColor[1] = 0; mFillColor[2] = 0; } ImGui::SameLine();
			ImGui::Text("Fill color");

			if (ImGui::Checkbox("Separate color", &mSeparate)) { gDirty = 1; }ImGui::SameLine();
			if (ImGui::Checkbox("Directional", &mDirectional)) { gDirty = 1; } ImGui::SameLine();
			if (ImGui::Checkbox("Antialiased", &mAntialias)) { gDirty = 1; }
		}
		ImGui::PopID();
		return ret;
	}

	virtual void process()
	{
		float matrix_x[] =
		{
			-1, 0, 1,
			-2, 0, 2,
			-1, 0, 1
		};

		float matrix_y[] =
		{
			-1, -2, -1,
			 0,  0,  0,
			 1,  2,  1
		};

		float *xedge = new float[3 * gDevice->mXRes * gDevice->mYRes];
		float *yedge = new float[3 * gDevice->mXRes * gDevice->mYRes];

		int i, j;
		for (i = 0; i < gDevice->mYRes; i++)
		{
			for (j = 0; j < gDevice->mXRes; j++)
			{
				int c;
				for (c = 0; c < 3; c++)
				{
					float ccx = 0;
					float ccy = 0;
					int x, y, m;
					for (y = -1, m = 0; y < 2; y++)
					{
						for (x = -1; x < 2; x++, m++)
						{
							if (x + j > 0 && x + j < gDevice->mXRes &&
								y + i > 0 && y + i < gDevice->mYRes)
							{
								ccx += matrix_x[m] * gBitmapProcFloat[((i + y) * gDevice->mXRes + j + x) * 3 + c];
								ccy += matrix_y[m] * gBitmapProcFloat[((i + y) * gDevice->mXRes + j + x) * 3 + c];
							}
						}
					}
					xedge[(i * gDevice->mXRes + j) * 3 + c] = ccx;
					yedge[(i * gDevice->mXRes + j) * 3 + c] = ccy;
				}
			}
		}

		if (!mSeparate)
		{
			for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
			{
				float x = (xedge[i * 3 + 0] + xedge[i * 3 + 1] + xedge[i * 3 + 2]) / 3;
				xedge[i * 3 + 0] = x;
				xedge[i * 3 + 1] = x;
				xedge[i * 3 + 2] = x;
				float y = (yedge[i * 3 + 0] + yedge[i * 3 + 1] + yedge[i * 3 + 2]) / 3;
				yedge[i * 3 + 0] = y;
				yedge[i * 3 + 1] = y;
				yedge[i * 3 + 2] = y;
			}

		}

		if (mAntialias)
		{
			if (mDirectional)
			{
				float xdir = (float)sin(mDirection * 2 * M_PI);
				float ydir = (float)cos(mDirection * 2 * M_PI);

				for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
				{
					if (mB_en && (xedge[i * 3 + 0] * xdir + yedge[i * 3 + 0] * ydir) > mThreshold)
					{
						float p = (xedge[i * 3 + 0] * xdir + yedge[i * 3 + 0] * ydir) - mThreshold;
						if (p > 1) p = 1;
						gBitmapProcFloat[i * 3 + 0] += (mFillColor[0] - gBitmapProcFloat[i * 3 + 0]) * mV * p;
					}
					if (mG_en && (xedge[i * 3 + 1] * xdir + yedge[i * 3 + 1] * ydir) > mThreshold)
					{
						float p = (xedge[i * 3 + 1] * xdir + yedge[i * 3 + 1] * ydir) - mThreshold;
						if (p > 1) p = 1;
						gBitmapProcFloat[i * 3 + 1] += (mFillColor[1] - gBitmapProcFloat[i * 3 + 1]) * mV * p;
					}
					if (mR_en && (xedge[i * 3 + 2] * xdir + yedge[i * 3 + 2] * ydir) > mThreshold)
					{
						float p = (xedge[i * 3 + 2] * xdir + yedge[i * 3 + 2] * ydir) - mThreshold;
						if (p > 1) p = 1;
						gBitmapProcFloat[i * 3 + 2] += (mFillColor[2] - gBitmapProcFloat[i * 3 + 2]) * mV * p;
					}
				}
			}
			else
			{
				for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
				{
					if (mB_en && ((abs(xedge[i * 3 + 0]) + abs(yedge[i * 3 + 0])) > mThreshold))
					{
						float p = abs(yedge[i * 3 + 0]) + abs(xedge[i * 3 + 0]) - mThreshold;
						if (p > 1) p = 1;
						gBitmapProcFloat[i * 3 + 0] += (mFillColor[0] - gBitmapProcFloat[i * 3 + 0]) * mV * p;
					}
					if (mG_en && ((abs(xedge[i * 3 + 1]) + abs(yedge[i * 3 + 1])) > mThreshold))
					{
						float p = abs(yedge[i * 3 + 1]) + abs(xedge[i * 3 + 1]) - mThreshold;
						if (p > 1) p = 1;
						gBitmapProcFloat[i * 3 + 1] += (mFillColor[1] - gBitmapProcFloat[i * 3 + 1]) * mV * p;
					}
					if (mR_en && ((abs(xedge[i * 3 + 2]) + abs(yedge[i * 3 + 2])) > mThreshold))
					{
						float p = abs(yedge[i * 3 + 2]) + abs(xedge[i * 3 + 2]) - mThreshold;
						if (p > 1) p = 1;
						gBitmapProcFloat[i * 3 + 2] += (mFillColor[2] - gBitmapProcFloat[i * 3 + 2]) * mV * p;
					}
				}
			}
		}
		else
		{
			if (mDirectional)
			{
				float xdir = (float)sin(mDirection * 2 * M_PI);
				float ydir = (float)cos(mDirection * 2 * M_PI);

				for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
				{
					if (mB_en && (xedge[i * 3 + 0] * xdir + yedge[i * 3 + 0] * ydir) > mThreshold) gBitmapProcFloat[i * 3 + 0] += (mFillColor[0] - gBitmapProcFloat[i * 3 + 0]) * mV;
					if (mG_en && (xedge[i * 3 + 1] * xdir + yedge[i * 3 + 1] * ydir) > mThreshold) gBitmapProcFloat[i * 3 + 1] += (mFillColor[1] - gBitmapProcFloat[i * 3 + 1]) * mV;
					if (mR_en && (xedge[i * 3 + 2] * xdir + yedge[i * 3 + 2] * ydir) > mThreshold) gBitmapProcFloat[i * 3 + 2] += (mFillColor[2] - gBitmapProcFloat[i * 3 + 2]) * mV;
				}
			}
			else
			{
				for (i = 0; i < gDevice->mXRes * gDevice->mYRes; i++)
				{
					if (mB_en && (abs(xedge[i * 3 + 0]) > mThreshold || abs(yedge[i * 3 + 0]) > mThreshold)) gBitmapProcFloat[i * 3 + 0] += (mFillColor[0] - gBitmapProcFloat[i * 3 + 0]) * mV;
					if (mG_en && (abs(xedge[i * 3 + 1]) > mThreshold || abs(yedge[i * 3 + 1]) > mThreshold)) gBitmapProcFloat[i * 3 + 1] += (mFillColor[1] - gBitmapProcFloat[i * 3 + 1]) * mV;
					if (mR_en && (abs(xedge[i * 3 + 2]) > mThreshold || abs(yedge[i * 3 + 2]) > mThreshold)) gBitmapProcFloat[i * 3 + 2] += (mFillColor[2] - gBitmapProcFloat[i * 3 + 2]) * mV;
				}
			}
		}

		delete[] xedge;
		delete[] yedge;
	}

};
