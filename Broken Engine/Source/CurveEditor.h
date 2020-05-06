#ifndef __COMPONENT_CURVE_EDITOR__
#define __COMPONENT_CURVE_EDITOR__

#include "BrokenCore.h"
#include "Math.h"


BE_BEGIN_NAMESPACE

struct BROKEN_API Point
{
	float2 prev_tangent;
	float2 p;
	float2 next_tangent;
};

enum BROKEN_API CurveType {
	LINEAR,
	CURVE
};

class BROKEN_API CurveEditor
{
public:
	CurveEditor(const char* name, CurveType type, int _multiplier = 1.0);
	~CurveEditor();
	void Init();

	void DrawCurveEditor();
	float GetCurrentValue(float cur_time, float max_time);
	void SetPoints(std::vector<Point>& pointsCurveTangents);
	inline std::vector<Point>* GetPoints() { return &pointsCurveTangents; }

public:
	std::vector<Point> pointsCurveTangents;
	CurveType type = LINEAR;
	std::string name;
	float multiplier = 1.0f;

private:
	int current = 0;
};
BE_END_NAMESPACE

#endif //__COMPONENT_PARTICLE_EMITTER__
