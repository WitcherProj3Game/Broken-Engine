#include "Application.h"
#include "CurveEditor.h"
#include "Imgui/imgui.h"

#include "mmgr/mmgr.h"

using namespace Broken;

CurveEditor::CurveEditor(const char* _name, CurveType _type, int _multiplier)
{
	name.append(_name);
	type = _type;
	current = type;
	multiplier = _multiplier;
}

CurveEditor::~CurveEditor()
{
}

void CurveEditor::Init()
{
	Point p;
	p.prev_tangent = float2(0.f, 0.f);
	p.p = float2(0.f, 0.f);
	p.next_tangent = float2(0.1f, 0.1f);
	pointsCurveTangents.push_back(p);

	p.prev_tangent = float2(-0.1f, -0.1f);
	p.p = float2(1.f, 1.f);
	p.next_tangent = float2(1.f, 1.f);
	pointsCurveTangents.push_back(p);
}

void CurveEditor::DrawCurveEditor()
{
	if (pointsCurveTangents.size() > 0) {
		if (pointsCurveTangents.size() == pointsCurveTangents.capacity())
			pointsCurveTangents.reserve(pointsCurveTangents.size() + 1);

		int num_points = pointsCurveTangents.size();
		int new_count = num_points;
		Point* data = pointsCurveTangents.data();

		int flags = (int)SHOW_GRID;

		if (ImGui::Combo("Type", &current, "Linear\0Curve\0\0"))
		{
			type = (CurveType)current;
		}

		if (type == LINEAR) {
			flags = flags | (int)NO_TANGENTS;
		}

		int changed_idx = ImGui::CurveEditor(name.c_str(), (float*)data, num_points, multiplier, ImVec2(230, 200), flags, &new_count);

		if (new_count != pointsCurveTangents.size()) {
			std::vector<Point> auxCurve = { data, data + new_count };
			pointsCurveTangents.clear();

			pointsCurveTangents = auxCurve;
			bool t = true;
		}

		if (changed_idx >= 0 && type == CurveType::CURVE) {
			for (int i = 1; i < new_count; ++i)
			{
				auto prev_p = pointsCurveTangents[i - 1].p;
				auto next_p = pointsCurveTangents[i].p;
				auto& tangent = pointsCurveTangents[i - 1].next_tangent;
				auto& tangent2 = pointsCurveTangents[i].prev_tangent;
				float half = 0.5f * (next_p.x - prev_p.x);
				tangent = tangent.Normalized() * half;
				tangent2 = tangent2.Normalized() * half;
			}
		}
	}
}

float CurveEditor::GetCurrentValue(float cur_time, float max_time) {
	float ret = 0;
	if (cur_time > 0) {
		float time = cur_time / max_time;
		if (type == CurveType::LINEAR) {
			for (int i = 0; i < pointsCurveTangents.size() - 1; ++i) {
				if (time > pointsCurveTangents[i].p.x&& time < pointsCurveTangents[i + 1].p.x) {
					float p1_time = pointsCurveTangents[i].p.x * max_time;
					float p2_time = pointsCurveTangents[i + 1].p.x * max_time;
					float p1_value = pointsCurveTangents[i].p.y * multiplier;
					float p2_value = pointsCurveTangents[i + 1].p.y * multiplier;
					float scope = (p2_value - p1_value) / (p2_time - p1_time);
					ret = p1_value + scope * (cur_time - p1_time);
					break;
				}
				else if (time > pointsCurveTangents[i + 1].p.x) {
					ret = pointsCurveTangents[i + 1].p.y * multiplier;
				}
				else if (time > pointsCurveTangents[i].p.x) {
					ret = pointsCurveTangents[i].p.y * multiplier;
				}
			}
		}
		if (type == CurveType::CURVE) {
			for (int i = 0; i < pointsCurveTangents.size() - 1; ++i) {
				if (time > pointsCurveTangents[i].p.x&& time < pointsCurveTangents[i + 1].p.x) {
					float2 p1 = pointsCurveTangents[i].p;
					float2 p2 = pointsCurveTangents[i].p + pointsCurveTangents[i].next_tangent;
					float2 p3 = pointsCurveTangents[i + 1].p + pointsCurveTangents[i + 1].prev_tangent;
					float2 p4 = pointsCurveTangents[i + 1].p;
					float t = (time - p1.x) / (p4.x - p1.x);
					float u = 1.0f - t;
					float w1 = u * u * u;
					float w2 = 3 * u * u * t;
					float w3 = 3 * u * t * t;
					float w4 = t * t * t;
					ImVec2 P(ImVec2(w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x, w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y));

					ret = P.y * multiplier;
					break;
				}
				else if (time > pointsCurveTangents[i + 1].p.x) {
					ret = pointsCurveTangents[i + 1].p.y * multiplier;
				}
				else if (time > pointsCurveTangents[i].p.x) {
					ret = pointsCurveTangents[i].p.y * multiplier;
				}
			}
		}
	}
	return ret;
}

void CurveEditor::SetPoints(std::vector<Point>& points)
{
	pointsCurveTangents = points;
}
