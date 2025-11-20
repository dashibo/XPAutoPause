#include "XPLMPlugin.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMMenus.h"
#include "XPLMProcessing.h"
#include "imgui.h"

// Apple braucht einen anderen Header Pfad für OpenGL
#if APL
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <string>
#include <vector>
#include <cmath>
#include <cstdarg>
#include <fstream>
#include <sstream>

// --- GLOBALE DEFINITIONEN ---

// Pfade und Referenzen
static XPLMDataRef gLatRef = NULL;
static XPLMDataRef gLonRef = NULL;
static XPLMDataRef gGsRef = NULL;
static XPLMMenuID  gMenuId = NULL;

// Status des Plugins
struct PluginState {
    bool window_visible = true;
    bool active = false;        // Ist die Überwachung scharf geschaltet?
    bool paused_triggered = false;

    // Ziel-Daten
    double target_lat = 0.0;
    double target_lon = 0.0;
    float  radius_nm = 150.0f;

    // GUI Eingabe-Puffer
    char input_lat[32] = "0.0";
    char input_lon[32] = "0.0";
};

PluginState gState;
std::string gConfigPath;

// --- LOGGING HELPER ---
// Schreibt formatierten Text in die X-Plane Log.txt
void LogMsg(const char* fmt, ...) {
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    std::string finalMsg = "[XP12AutoTOD] " + std::string(buf) + "\n";
    XPLMDebugString(finalMsg.c_str());
}

// --- KONFIGURATION LADEN/SPEICHERN ---

void DetermineConfigPath() {
    char path[512];
    XPLMGetPluginInfo(XPLMGetMyID(), NULL, path, NULL, NULL);
    std::string p(path);
    // Dateiname abschneiden und settings.ini anhängen
    size_t last_slash = p.find_last_of("\\/");
    if (last_slash != std::string::npos) {
        gConfigPath = p.substr(0, last_slash + 1) + "XP12AutoTOD_settings.ini";
    }
}

void SaveConfig() {
    std::ofstream file(gConfigPath);
    if (file.is_open()) {
        file << "active=" << gState.active << "\n";
        file << "radius=" << gState.radius_nm << "\n";
        file << "lat=" << gState.target_lat << "\n";
        file << "lon=" << gState.target_lon << "\n";
        file << "window_visible=" << gState.window_visible << "\n";
        LogMsg("Konfiguration gespeichert: %s", gConfigPath.c_str());
    }
}

void LoadConfig() {
    std::ifstream file(gConfigPath);
    if (!file.is_open()) return;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream is_line(line);
        std::string key;
        if (std::getline(is_line, key, '=')) {
            std::string value;
            if (std::getline(is_line, value)) {
                if (key == "active") gState.active = (value == "1");
                else if (key == "radius") gState.radius_nm = std::stof(value);
                else if (key == "lat") {
                    gState.target_lat = std::stod(value);
                    sprintf(gState.input_lat, "%.6f", gState.target_lat);
                }
                else if (key == "lon") {
                    gState.target_lon = std::stod(value);
                    sprintf(gState.input_lon, "%.6f", gState.target_lon);
                }
                else if (key == "window_visible") gState.window_visible = (value == "1");
            }
        }
    }
    LogMsg("Konfiguration geladen.");
}

// --- LOGIK (Haversine) ---

double GetDistanceNM(double lat1, double lon1, double lat2, double lon2) {
    double R = 3440.065;
    double dLat = (lat2 - lat1) * 3.1415926535 / 180.0;
    double dLon = (lon2 - lon1) * 3.1415926535 / 180.0;
    double a = sin(dLat / 2) * sin(dLat / 2) +
        cos(lat1 * 3.1415926535 / 180.0) * cos(lat2 * 3.1415926535 / 180.0) *
        sin(dLon / 2) * sin(dLon / 2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

// Dieser Loop läuft IMMER, auch wenn das Fenster zu ist
float FlightLoopCallback(float, float, int, void*) {
    if (!gState.active || gLatRef == NULL) return 1.0f; // 1 Sekunde warten

    double myLat = XPLMGetDatad(gLatRef);
    double myLon = XPLMGetDatad(gLonRef);
    double dist = GetDistanceNM(myLat, myLon, gState.target_lat, gState.target_lon);

    if (!gState.paused_triggered && dist <= gState.radius_nm) {
        XPLMCommandOnce(XPLMFindCommand("sim/operation/pause_on"));
        gState.paused_triggered = true;
        LogMsg("PAUSE AUSGELOEST! Distanz: %.1f NM", dist);
    }

    // Reset Trigger wenn wir uns entfernen
    if (dist > (gState.radius_nm + 5.0)) {
        gState.paused_triggered = false;
    }

    return 1.0f; // Check jede Sekunde reicht
}


// --- GUI RENDERING (ImGui) ---

void RenderImGuiDrawData(ImDrawData* draw_data) {
    // (Hier der gleiche OpenGL Code wie vorher, aber gekürzt für Übersichtlichkeit)
    // Kopiere den Renderer-Code aus dem vorherigen Schritt hier rein, 
    // oder nutze den Block unten, falls du ihn noch hast.
    // --- START RENDERER ---
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE); glDisable(GL_DEPTH_TEST); glDisable(GL_SCISSOR_TEST);
    glEnable(GL_TEXTURE_2D); glEnableClientState(GL_VERTEX_ARRAY); glEnableClientState(GL_TEXTURE_COORD_ARRAY); glEnableClientState(GL_COLOR_ARRAY);

    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
    glOrtho(0.0f, last_viewport[2], last_viewport[3], 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    ImVec2 clip_off = draw_data->DisplayPos;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) pcmd->UserCallback(cmd_list, pcmd);
            else {
                // HIER IST DER FIX FÜR DEINEN FEHLER:
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->GetTexID());
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
    glDisableClientState(GL_COLOR_ARRAY); glDisableClientState(GL_TEXTURE_COORD_ARRAY); glDisableClientState(GL_VERTEX_ARRAY);
    glMatrixMode(GL_MODELVIEW); glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
    glPopAttrib(); glBindTexture(GL_TEXTURE_2D, last_texture);
    // --- END RENDERER ---
}

int DrawCallback(XPLMDrawingPhase inPhase, int inIsBefore, void* inRefcon) {
    if (!gState.window_visible) return 1;

    // Setup IO
    int w, h; XPLMGetScreenSize(&w, &h);
    int mx, my; XPLMGetMouseLocationGlobal(&mx, &my);
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.MousePos = ImVec2((float)mx, (float)(h - my));
    static XPLMDataRef mouseBtn = XPLMFindDataRef("sim/input/global/mouse_button_down");
    io.MouseDown[0] = (XPLMGetDatai(mouseBtn) != 0);

    ImGui::NewFrame();

    ImGui::SetNextWindowSize(ImVec2(350, 250), ImGuiCond_FirstUseEver);
    // Wenn User auf das X klickt, wird gState.window_visible false
    if (ImGui::Begin("XP12AutoTOD C++", &gState.window_visible)) {

        ImGui::Text("Status: %s", gState.active ? "AKTIV" : "Standby");
        ImGui::Separator();

        ImGui::InputText("Lat", gState.input_lat, 32);
        ImGui::InputText("Lon", gState.input_lon, 32);
        ImGui::SliderFloat("Radius (NM)", &gState.radius_nm, 10.0f, 500.0f);

        if (ImGui::Button("Setzen & Aktivieren")) {
            gState.target_lat = atof(gState.input_lat);
            gState.target_lon = atof(gState.input_lon);
            gState.active = true;
            gState.paused_triggered = false;
            SaveConfig();
            LogMsg("Aktiviert fuer %.4f, %.4f", gState.target_lat, gState.target_lon);
        }

        ImGui::SameLine();
        if (ImGui::Button("Deaktivieren")) {
            gState.active = false;
            SaveConfig();
            LogMsg("Deaktiviert.");
        }

        // Info Anzeige
        if (gLatRef) {
            double curLat = XPLMGetDatad(gLatRef);
            double curLon = XPLMGetDatad(gLonRef);
            double dist = GetDistanceNM(curLat, curLon, gState.target_lat, gState.target_lon);
            ImGui::Separator();
            ImGui::Text("Aktuell: %.4f, %.4f", curLat, curLon);
            if (gState.active) {
                ImGui::TextColored(ImVec4(0, 1, 0, 1), "Distanz: %.1f NM", dist);
            }
            else {
                ImGui::TextDisabled("Distanz: %.1f NM", dist);
            }
        }
    }
    ImGui::End();

    ImGui::Render();
    RenderImGuiDrawData(ImGui::GetDrawData());
    return 1;
}

// --- MENÜ HANDLER ---
void MenuHandler(void* inMenuRef, void* inItemRef) {
    // Toggle Fenster
    gState.window_visible = !gState.window_visible;
    SaveConfig(); // Zustand speichern
}

// --- PLUGIN START / STOP ---

PLUGIN_API int XPluginStart(char* outName, char* outSig, char* outDesc) {
    strcpy(outName, "XP12AutoTOD");
    strcpy(outSig, "com.dannik.xp12autotod");
    strcpy(outDesc, "Native AutoTOD Plugin");

    DetermineConfigPath();
    LoadConfig(); // Einstellungen laden!

    gLatRef = XPLMFindDataRef("sim/flightmodel/position/latitude");
    gLonRef = XPLMFindDataRef("sim/flightmodel/position/longitude");

    // ImGui Init (Standard)
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels; int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    GLuint font_tex; glGenTextures(1, &font_tex);
    glBindTexture(GL_TEXTURE_2D, font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    io.Fonts->SetTexID((ImTextureID)(intptr_t)font_tex);

    // Callbacks
    XPLMRegisterDrawCallback(DrawCallback, xplm_Phase_Window, 0, NULL);
    XPLMRegisterFlightLoopCallback(FlightLoopCallback, 1.0f, NULL);

    // Menü erstellen (Damit man das Fenster wiederfindet!)
    int mySlot = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "XP12AutoTOD", NULL, 0);
    gMenuId = XPLMCreateMenu("XP12AutoTOD", XPLMFindPluginsMenu(), mySlot, MenuHandler, NULL);
    XPLMAppendMenuItem(gMenuId, "Toggle Window", (void*)0, 1);

    LogMsg("Plugin gestartet.");
    return 1;
}

PLUGIN_API void XPluginStop(void) {
    XPLMUnregisterDrawCallback(DrawCallback, xplm_Phase_Window, 0, NULL);
    XPLMUnregisterFlightLoopCallback(FlightLoopCallback, NULL);
    XPLMDestroyMenu(gMenuId);
    ImGui::DestroyContext();
    LogMsg("Plugin gestoppt.");
}

PLUGIN_API void XPluginDisable(void) {}
PLUGIN_API int XPluginEnable(void) { return 1; }
PLUGIN_API void XPluginReceiveMessage(XPLMPluginID, int, void*) {}