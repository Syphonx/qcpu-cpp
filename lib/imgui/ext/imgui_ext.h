//-----------------------------------------------------------------------------
// Copyright 2021 Jak Barnes. All rights reserved.
//-----------------------------------------------------------------------------

#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui
{
    namespace Ext
    {
        // Attrib: JSandusky @ https://github.com/ocornut/imgui/issues/1831
        // Dupe of DragFloatN with a tweak to add colored lines
        static bool DragFloat3_Colored(const char* label,
                                float v[3],
                                float v_speed = 1.f,
                                float v_min = 0.f,
                                float v_max = 0.f,
                                const char* display_format = "%.3f"
        )
        {
            ImGuiWindow* window = GetCurrentWindow();
            if (window->SkipItems)
            {
                return false;
            }

            ImGuiContext& g = *GImGui;
            bool value_changed = false;
            BeginGroup();
            PushID(label);
            float w = GetContentRegionAvailWidth();
            PushMultiItemsWidths(3, w);

            for (int i = 0; i < 3; i++)
            {
                static const ImU32 colors[] = {
                    0xBB0000FF, // red
                    0xBB00FF00, // green
                    0xBBFF0000, // blue
                    0xBBFFFFFF, // white for alpha?
                };

                PushID(i);
                value_changed |= DragFloat("##v", &v[i], v_speed, v_min, v_max, display_format);

                const ImVec2 min = GetItemRectMin();
                const ImVec2 max = GetItemRectMax();
                const float spacing = g.Style.FrameRounding;
                const float halfSpacing = spacing / 2;

                // This is the main change
                window->DrawList->AddLine({ min.x + spacing, max.y - halfSpacing }, { max.x - spacing, max.y - halfSpacing }, colors[i], 4);

                SameLine(0, g.Style.ItemInnerSpacing.x);
                PopID();
                PopItemWidth();
            }

            PopID();
            TextUnformatted(label, FindRenderedTextEnd(label));
            EndGroup();

            return value_changed;
        }
    }
}
