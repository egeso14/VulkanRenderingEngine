#pragma once
#include "Colors.h"
namespace VTA_UI
{
    void ShiftCursorX(float distance);
	void ShiftCursorY(float distance);
	void ShiftCursor(float x, float y);

	inline void Underline(bool fullWidth = false, float offsetX = 0.0f, float offsetY = -1.0f)
	{
		if (fullWidth)
		{
			if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)
				ImGui::PushColumnsBackground();
			else if (ImGui::GetCurrentTable() != nullptr)
				ImGui::TablePushBackgroundChannel();
		}

		const float width = fullWidth ? ImGui::GetWindowWidth() : ImGui::GetContentRegionAvail().x;
		const ImVec2 cursor = ImGui::GetCursorScreenPos();
		ImGui::GetWindowDrawList()->AddLine(ImVec2(cursor.x + offsetX, cursor.y + offsetY),
			ImVec2(cursor.x + width, cursor.y + offsetY),
			Colors::Theme::backgroundDark, 1.0f);

		if (fullWidth)
		{
			if (ImGui::GetCurrentWindow()->DC.CurrentColumns != nullptr)
				ImGui::PopColumnsBackground();
			else if (ImGui::GetCurrentTable() != nullptr)
				ImGui::TablePopBackgroundChannel();
		}
	}
}