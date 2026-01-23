#pragma once

#include <cstdint>
#include <string>

#include <imtui/imtui.h>
#include <imtui/imtui-impl-ncurses.h>

#include "../networking/tcp_client_observer.hpp"
#include "../networking/tcp_server.hpp"
#include "../networking/tcp_logger.hpp"

#include "esp_32.hpp"
#include "gui_connection_tab.hpp"

class gui_window : public tcp_client_observer_base
{
public:
    ~gui_window() {
        get_tcp_server()->close();
    }

    void on_client_connect(uint16_t client_id) override {
        // send confirmation to esp_32
        get_tcp_server()->send_to_client_by(client_id, {
            .esp_id = client_id,
            .is_led_on = false,
            .is_restarting = false
        });

        m_connected_esps.emplace_back(client_id);
    }

    void on_receive_from(const esp_info_t& esp_info) override {
        if (esp_info.is_restarting) {
            get_tcp_server()->disconnect_client_by(esp_info.esp_id);
        }
    }

    void on_client_disconnect(uint16_t client_id) override {
        // remove disconnected esps
        auto found = std::remove_if(m_connected_esps.begin(), m_connected_esps.end(), [&](const esp_32& esp_32) {
            return esp_32.esp_id == client_id;
        });
        m_connected_esps.erase(found);
    }

    void run() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        auto* screen = ImTui_ImplNcurses_Init(true);
        ImTui_ImplText_Init();

        apply_theme();

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
        ImVec2 display_size = ImGui::GetIO().DisplaySize;
        ImVec2 dashboard_pos = ImVec2(0.0f, 0.0f);
        ImVec2 dashboard_size = ImVec2(display_size.x, std::floor(0.8f * display_size.y));

        create_dashboard_window(display_size, dashboard_pos, dashboard_size);
        create_console_window(display_size, dashboard_size.y);
    }

    void apply_theme() {
        ImGuiStyle& style = ImGui::GetStyle();

        auto col = [](int r, int g, int b) { return ImVec4(r/255.0f, g/255.0f, b/255.0f, 1.0f); };
        auto c = style.Colors;

        // Core colors
        c[ImGuiCol_Text] = col(220, 220, 220);       // light gray
        c[ImGuiCol_TextDisabled] = col(130, 130, 130);

        c[ImGuiCol_WindowBg] = col(18, 18, 18);      // dark background
        c[ImGuiCol_ChildBg] = col(20, 20, 20);
        c[ImGuiCol_PopupBg] = col(25, 25, 25);

        c[ImGuiCol_Border] = col(50, 20, 20);
        c[ImGuiCol_BorderShadow] = col(0, 0, 0);

        ImVec4 black      = col(18, 18, 18);
        ImVec4 red_dark   = col(60, 20, 20);  
        ImVec4 red_active = col(80, 20, 20);  
        ImVec4 red_normal = col(100, 20, 20); 
        
        // Frames / Inputs
        c[ImGuiCol_FrameBg] = red_dark;
        c[ImGuiCol_FrameBgHovered] = red_normal;
        c[ImGuiCol_FrameBgActive] = red_active;

        // Buttons
        c[ImGuiCol_Button] = red_dark;
        c[ImGuiCol_ButtonHovered] = red_normal;
        c[ImGuiCol_ButtonActive] = red_active;

        // Headers
        c[ImGuiCol_Header] = red_dark;
        c[ImGuiCol_HeaderHovered] = red_normal;
        c[ImGuiCol_HeaderActive] = red_active;

        // Tabs
        c[ImGuiCol_Tab] = red_dark;
        c[ImGuiCol_TabHovered] = red_normal;
        c[ImGuiCol_TabActive] = red_active;
        c[ImGuiCol_TabUnfocused] = col(25, 25, 25);
        c[ImGuiCol_TabUnfocusedActive] = red_dark;

        // Tables
        c[ImGuiCol_TableHeaderBg] = red_dark;
        c[ImGuiCol_TableRowBg] = col(20, 20, 20);
        c[ImGuiCol_TableRowBgAlt] = col(25, 25, 25);
        c[ImGuiCol_TableBorderStrong] = black;
        c[ImGuiCol_TableBorderLight] = black;

        // Scrollbar
        c[ImGuiCol_ScrollbarBg] = col(18, 18, 18);
        c[ImGuiCol_ScrollbarGrab] = red_dark;
        c[ImGuiCol_ScrollbarGrabHovered] = red_normal;
        c[ImGuiCol_ScrollbarGrabActive] = red_active;

        // Selection / Navigation
        c[ImGuiCol_TextSelectedBg] = red_normal;
        c[ImGuiCol_NavHighlight] = red_active;

        // ---- Window title bar ----
        c[ImGuiCol_TitleBg] = red_dark;          
        c[ImGuiCol_TitleBgActive] = red_dark;   

    }

    void create_dashboard_window(ImVec2 display_size, ImVec2 dashboard_pos, ImVec2 dashboard_size) {
        ImGui::SetNextWindowPos(dashboard_pos);
        ImGui::SetNextWindowSize(dashboard_size);

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

            if (ImGui::BeginTabItem("Connected"))
            {   
                gui_connection_tab::show_connection_table(m_connected_esps);
                gui_connection_tab::handle_button_options(get_tcp_server(), m_connected_esps, dashboard_size);

                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    void create_console_window(ImVec2 display_size, float dashboard_size_y) {
        ImVec2 consolePos = ImVec2(0, dashboard_size_y);
        ImVec2 consoleSize = ImVec2(display_size.x, std::floor(0.2f * display_size.y) + 1);

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
    std::vector<esp_32> m_connected_esps;
    bool m_running = true;
};