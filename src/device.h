class Device
{
public:
	virtual ~Device()
	{
		gDirty = 1;
		gDirtyPic = 1;
	}
	int mXRes;
	int mYRes;
	virtual void filter() = 0;
	virtual int estimate_rgb(int c) = 0;
	virtual void savescr(FILE * f) = 0;
	virtual void saveh(FILE * f) = 0;
	virtual void saveinc(FILE * f) = 0;
	virtual void attr_bitm() = 0;
	virtual void options() = 0;
	virtual void zoomed() = 0;
	virtual void writeOptions(FILE *f) = 0;
	virtual void readOptions(FILE *f) = 0;
};