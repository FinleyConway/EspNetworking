#pragma once

#include <vector>
#include <string>

#include <imtui/imtui.h>

#include "../networking/tcp_server.hpp"

#include "esp_32.hpp"

class gui_connection_tab 
{
public:
    static void show_connection_table(std::vector<esp_32>& connected_esps) {
        if (ImGui::BeginTable("Connected ESPs", 2, ImGuiTableFlags_Borders)) {
            ImGui::TableSetupColumn("ID");
            ImGui::TableSetupColumn("Name");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < connected_esps.size(); i++) {
                esp_32& esp = connected_esps[i];

                ImGui::TableNextRow();                             
                ImGui::PushID(esp.esp_id);
                
                ImGui::TableSetColumnIndex(0);
                if (ImGui::Selectable(std::to_string(esp.esp_id).c_str(), i == s_selected_row)) {
                    s_selected_row = i;
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::InputText("##name", esp.name, IM_ARRAYSIZE(esp.name));

                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }

    static void handle_button_options(tcp_server* tcp_server, std::vector<esp_32>& connected_esps, ImVec2 dashboard_size) {
        ImGui::SetCursorPosY(dashboard_size.y - 1); 

        handle_esp_disconnect(tcp_server, connected_esps, dashboard_size);
    }

private:
    static void handle_esp_disconnect(tcp_server* tcp_server, std::vector<esp_32>& connected_esps, ImVec2 dashboard_size) {
        if (ImGui::Button("Disconnect")) {
            if (s_selected_row != -1 && !connected_esps.empty()) {
                const esp_32& esp = connected_esps[s_selected_row];

                tcp_server->disconnect_client_by(esp.esp_id);

                if (s_selected_row >= connected_esps.size()) {
                    s_selected_row = connected_esps.size() - 1;
                }
                else {
                    s_selected_row = -1;
                }
            }
        }
    }

private:
    inline static int s_selected_row = -1;
};