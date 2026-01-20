#pragma once

#include <cstdint>
#include <string>

#include <imtui/imtui.h>
#include <imtui/imtui-impl-ncurses.h>

#include "../networking/tcp_client_observer.hpp"
#include "../networking/tcp_server.hpp"
#include "../networking/tcp_logger.hpp"

struct esp_32 {
    uint16_t esp_id = 0;
    std::string name;
};

class gui_window : public tcp_client_observer_base
{
public:
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
        std::remove_if(m_connected_esps.begin(), m_connected_esps.end(), [&](const esp_32& esp_32) {
            return esp_32.esp_id == client_id;
        });
    }

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
        ImVec2 display_size = ImGui::GetIO().DisplaySize;
        ImVec2 dashboard_pos = ImVec2(0.0f, 0.0f);
        ImVec2 dashboard_size = ImVec2(display_size.x, std::floor(0.8f * display_size.y));

        create_dashboard_window(display_size, dashboard_pos, dashboard_size);
        create_console_window(display_size, dashboard_size.y);
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
            if (ImGui::BeginTabItem("Connect"))
            {
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