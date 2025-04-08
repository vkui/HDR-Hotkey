#include <windows.h>

#pragma comment(lib, "user32.lib")

bool ToggleHDR() {
    UINT32 pathCount, modeCount;
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &pathCount, &modeCount) != ERROR_SUCCESS) return false;

    DISPLAYCONFIG_PATH_INFO* paths = new DISPLAYCONFIG_PATH_INFO[pathCount];
    DISPLAYCONFIG_MODE_INFO* modes = new DISPLAYCONFIG_MODE_INFO[modeCount];

    if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &pathCount, paths, &modeCount, modes, nullptr) != ERROR_SUCCESS) {
        delete[] paths;
        delete[] modes;
        return false;
    }

    for (UINT32 i = 0; i < pathCount; i++) {
        DISPLAYCONFIG_GET_ADVANCED_COLOR_INFO getColorInfo = {};
        getColorInfo.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADVANCED_COLOR_INFO;
        getColorInfo.header.size = sizeof(getColorInfo);
        getColorInfo.header.adapterId = paths[i].targetInfo.adapterId;
        getColorInfo.header.id = paths[i].targetInfo.id;

        if (DisplayConfigGetDeviceInfo(&getColorInfo.header) == ERROR_SUCCESS && getColorInfo.advancedColorSupported) {
            DISPLAYCONFIG_SET_ADVANCED_COLOR_STATE setColorState = {};
            setColorState.header.type = DISPLAYCONFIG_DEVICE_INFO_SET_ADVANCED_COLOR_STATE;
            setColorState.header.size = sizeof(setColorState);
            setColorState.header.adapterId = paths[i].targetInfo.adapterId;
            setColorState.header.id = paths[i].targetInfo.id;
            setColorState.enableAdvancedColor = !getColorInfo.advancedColorEnabled;

            if (DisplayConfigSetDeviceInfo(&setColorState.header) == ERROR_SUCCESS) {
                delete[] paths;
                delete[] modes;
                return true;
            }
        }
    }

    delete[] paths;
    delete[] modes;
    return false;
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_SYSKEYDOWN) {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
        bool winPressed = GetAsyncKeyState(VK_LWIN) & 0x8000 || GetAsyncKeyState(VK_RWIN) & 0x8000;
        bool altPressed = GetAsyncKeyState(VK_MENU) & 0x8000;

        if (winPressed && altPressed && p->vkCode == 'B') {
            ToggleHDR();
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, hInstance, 0);
    if (keyboardHook == NULL) {
        return 1;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(keyboardHook);
    return 0;
}