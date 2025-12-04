#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <imgui_internal.h>
#include "ImGui/Colors.h"
#include "ImGui/ImGuiModule.h"
#include <format>
#include "ImGui/ImGuiUtilities.h"

void VTA_UI::SceneHierarchyPanel::OnImGuiRender(bool& isOpen)
{
	ImRect windowRect = { ImGui::GetWindowContentRegionMin(), ImGui::GetWindowContentRegionMax() };
	{
		const float edgeOffset = 4.0f;
		VTA_UI::ShiftCursorX(edgeOffset * 3.0f);
		VTA_UI::ShiftCursorY(edgeOffset * 2.0f);

		//VTA_UI::ShiftCursorX(edgeOffset * 3.0f); not sure what these do yet
		//VTA_UI::ShiftCursorY(edgeOffset * 2.0f);

		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
		ImGui::PushStyleColor(ImGuiCol_TableRowBgAlt, ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader));
		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::Theme::backgroundDark);
			ImGuiTableFlags tableFlags = ImGuiTableFlags_NoPadInnerX
				| ImGuiTableFlags_Resizable
				| ImGuiTableFlags_Reorderable
				| ImGuiTableFlags_ScrollY;
			const int numColumns = 3;
			if (ImGui::BeginTable("##SceneHierarchy-Table", numColumns, tableFlags, ImVec2(ImGui::GetContentRegionAvail())))
			{
				ImGui::TableSetupColumn("Label");
				ImGui::TableSetupColumn("Type");
				ImGui::TableSetupColumn("Visibility");

				// Headers
				{
					const ImU32 colActive = Colors::Theme::groupHeader; // here Hazel actually multiplies this value with 1.2
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, colActive);
					ImGui::PushStyleColor(ImGuiCol_HeaderActive, colActive);

					ImGui::TableSetupScrollFreeze(ImGui::TableGetColumnCount(), 1);
					ImGui::TableNextRow(ImGuiTableRowFlags_Headers, 22.0f); // creates a header row with a specific height


					for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
					{
						ImGui::TableSetColumnIndex(column);
						const char* column_name = ImGui::TableGetColumnName(column);
						ImGui::PushID(column);

						VTA_UI::ShiftCursor(edgeOffset * 3.0f, edgeOffset * 2.0f);
						ImGui::TableHeader(column_name);
						VTA_UI::ShiftCursor(-edgeOffset * 3.0f, -edgeOffset * 2.0f);

						ImGui::PopID();
					}
					ImGui::SetCursorPosX(ImGui::GetCurrentTable()->OuterRect.Min.x);
					VTA_UI::Underline(true, 0.0f, 5.0f);


					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
				}

				// List
				{
					ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32_DISABLE);
					ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32_DISABLE);
					ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32_DISABLE);

					for (auto entity : context->GetAllEntitiesWith<IDComponent, RelationshipComponent>())
					{
						Entity e(entity, m_Context.Raw());
						if (e.GetParentUUID() == 0)
							DrawGameObjectNode({ entity, m_Context.Raw() }, searchedString);
					}

					ImGui::PopStyleColor();
					ImGui::PopStyleColor();
					ImGui::PopStyleColor();

				}

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Spacing();
				ImGui::Dummy(ImVec2(0, 50.0f));

				ImGui::EndTable();
			}


		}
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}
}

void VTA_UI::SceneHierarchyPanel::DrawGameObjectNode(VTA::VTAGameObject gameObj)
{
	const char* name = "Unnamed Entity";
	if (gameObj.HasComponent<TagComponent>()) // still need to complete this component system
		name = gameObj.GetComponent<TagComponent>().Tag.c_str();

	const float edgeOffset = 4.0f;
	const float rowHeight = 21.0f;

	// ImGui item height tweaks
	auto* window = ImGui::GetCurrentWindow();
	window->DC.CurrLineSize.y = rowHeight;
	//---------------------------------------------
	ImGui::TableNextRow(0, rowHeight);

	// Label column
	//-------------

	ImGui::TableNextColumn();
	window->DC.CurrLineTextBaseOffset = 3.0f;

	const ImVec2 rowAreaMin = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0).Min;
	const ImVec2 rowAreaMax = { ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), ImGui::TableGetColumnCount() - 1).Max.x - 20,
								rowAreaMin.y + rowHeight };

	const bool isSelected = false; // add a selection manager class here?
	ImGuiTreeNodeFlags flags = (isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

	if (gameObj.Children().empty())
		flags |= ImGuiTreeNodeFlags_Leaf;

	// maybe flags here?

	const std::string strID = std::format("{0}{1}", name, (uint64_t)gameObj.getId());
	ImGui::PushClipRect(rowAreaMin, rowAreaMax, false);
	bool isRowHovered, held;
	bool isRowClicked = ImGui::ButtonBehavior(ImRect(rowAreaMin, rowAreaMax), ImGui::GetID(strID.c_str()),
		&isRowHovered, &held,  ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
	// removed this flag ImGuiButtonFlags_AllowItemOverlap
	bool wasRowRightClicked = ImGui::IsMouseReleased(ImGuiMouseButton_Right);
	// ImGui::SetItemAllowOverlap(); not sure if we need this line

	ImGui::PopClipRect();
	const bool isWindowFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

	// Fill with light selection colour if any of the child entities selected
	//auto isAnyDescendantSelected = [&](Entity ent, auto isAnyDescendantSelected) -> bool
	//	{
	//		if (SelectionManager::IsSelected(s_ActiveSelectionContext, ent.GetUUID()))
	//			return true;

	//		if (!ent.Children().empty())
	//		{
	//			for (auto& childEntityID : ent.Children())
	//			{
	//				Entity childEntity = m_Context->GetEntityWithUUID(childEntityID);
	//				if (isAnyDescendantSelected(childEntity, isAnyDescendantSelected))
	//					return true;
	//			}
	//		}

	//		return false;
	//	};

	auto fillRowWithColour = [](const ImColor& colour)
		{
			for (int column = 0; column < ImGui::TableGetColumnCount(); column++)
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, colour, column);
		};

	if (isSelected)
	{
		if (isWindowFocused)
			fillRowWithColour(Colors::Theme::selection);
		else
		{
			const ImColor col = Colors::Theme::selection; // multiplied by 0.9 here
			fillRowWithColour(col); // multiplied saturation here
		}
	}
	else if (isRowHovered)
	{
		fillRowWithColour(Colors::Theme::groupHeader);
	}
	//else if (isAnyDescendantSelected(entity, isAnyDescendantSelected))
	//{
	//	fillRowWithColour(Colors::Theme::selectionMuted);
	//}

	// text coloring

	if (isSelected)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark));

	}


	// Tree node
//----------
// TODO: clean up this mess
	ImGuiContext& g = *GImGui;
	auto& style = ImGui::GetStyle();
	const ImVec2 label_size = ImGui::CalcTextSize(strID.c_str(), nullptr, false);
	const ImVec2 padding = ((flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));
	const float text_offset_x = g.FontSize + padding.x * 2;           // Collapser arrow width + Spacing
	const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it
	const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);  // Include collapser
	ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
	const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
	const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
	const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);

	bool opened = ImGui::TreeNodeEx((void*)(uintptr_t)gameObj.getId(), flags, name);
	bool entityDeleted = false;

	// we can add popup logic here later, like on right click and whatnot

	if (opened)
	{
		for (auto child : gameObj.Children())
			DrawGameObjectNode(*context->GetObjectWithUUID(child));

		ImGui::TreePop();
	}

	if (isSelected)
		ImGui::PopStyleColor();

}
