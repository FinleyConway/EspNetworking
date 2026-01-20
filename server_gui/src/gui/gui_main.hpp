#pragma once

#include <imtui/imtui.h>
#include <imtui/imtui-impl-ncurses.h>

#include "../networking/tcp_logger.hpp"

class gui_main 
{
public:
    void run() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto* screen = ImTui_ImplNcurses_Init(true);
        ImTui_ImplText_Init();

        while (m_running) {
            ImTui_ImplNcurses_NewFrame();
            ImTui_ImplText_NewFrame();
            ImGui::NewFrame();

            create_gui();

            ImGui::Render();
            ImTui_ImplText_RenderDrawData(ImGui::GetDrawData(), screen);
            ImTui_ImplNcurses_DrawScreen();
        }

        ImTui_ImplText_Shutdown();
        ImTui_ImplNcurses_Shutdown();
    }

private:
    void create_gui() {
        ImVec2 displaySize = ImGui::GetIO().DisplaySize;

        ImVec2 examplePos = ImVec2(0.0f, 0.0f);
        ImVec2 exampleSize = ImVec2(displaySize.x, std::floor(0.8f * displaySize.y));
        ImGui::SetNextWindowPos(examplePos);
        ImGui::SetNextWindowSize(exampleSize);

        ImGui::Begin("ESP Network Dashboard", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize
        );

        if (ImGui::BeginTabBar("Tabs"))
        {
            if (ImGui::BeginTabItem("Home"))
            {
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Connect"))
            {
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();

        ImVec2 consolePos = ImVec2(0, exampleSize.y);
        ImVec2 consoleSize = ImVec2(displaySize.x, std::floor(0.2f * displaySize.y) + 1);

        ImGui::SetNextWindowPos(consolePos);
        ImGui::SetNextWindowSize(consoleSize);

        ImGui::Begin("Console", nullptr,
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize
        ); 

        auto buffer = logger::get_sink()->get_buffer();
        for (const auto& message : buffer) {
            ImGui::TextUnformatted(message.c_str());
        }

        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::End();
    }

private:
    bool m_running = true;
};