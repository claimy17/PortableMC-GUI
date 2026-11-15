#include <windows.h>
#include <shlobj.h>
#include "modrinth.h"
#include <commdlg.h>
#include <dwmapi.h>
#include <uxtheme.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#pragma comment(lib, "msimg32.lib")

#define ID_COMBO_AUTH      100
#define ID_EDIT_USER       101
#define ID_BTN_LOGIN       102
#define ID_COMBO_LOADER    103
#define ID_COMBO_VERSION   104
#define ID_BTN_TOGGLE_SNAP 105
#define ID_BTN_TOGGLE_BETA 106
#define ID_BTN_IMPORT_MOD  107
#define ID_EDIT_DIR        108
#define ID_BTN_BROWSE_DIR  109
#define ID_BTN_LAUNCH      110
#define ID_EDIT_LOG        111
#define ID_COMBO_THEME     112
#define ID_BTN_IMPORT_SKIN 113
#define ID_BTN_IMPORT_CAPE 114
#define ID_BTN_SEARCH_MODS 115
#define ID_BTN_BOOST_FPS   116
#define ID_BTN_ISOLATED    117
#define ID_BTN_AUTOCLOSE   118

#define ID_MENU_BASE       2000

#define WM_APPEND_LOG      (WM_USER + 1)
#define WM_CMD_FINISHED    (WM_USER + 2)

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

HWND hBtnAuth, hEditUser, hBtnLogin, hBtnLoader, hBtnVersion, hBtnSnap, hBtnBeta, hEditDir, hEditLog, hBtnLaunch, hBtnTheme;
HWND hTitle, lblLoader, hVer, lblVer, lblDir, btnMod, btnBrowse, btnSkin, btnCape;
HWND btnSearchMods, btnBoostFps, hBtnIsolated, hBtnAutoClose;
int isIsolated = 0;
int isAutoClose = 1;
void ShowCustomAlert(HWND parent, const char* title, const char* message, const char* folderPath);
void ImportSkinOrCape(HWND hwnd, BOOL isCape);
HFONT hFontMain, hFontTitle, hFontLog, hFontBtn, hFontSmall;

int isDarkTheme = 0; 
int authMode = 0;    
int loaderIdx = 0;   
int showSnapshots = 0;
int showBetas = 0;
int appStatus = 0;       
int downloadPercent = 0;
BOOL isRunningCmd = FALSE;
BOOL isDownloading = FALSE;
HANDLE hActiveProcess = NULL;

char selectedVersion[64] = "26.3";

HWND hHoveredBtn = NULL;
WNDPROC origBtnProc = NULL;
WNDPROC origEditProc = NULL;

HBRUSH hBrushInput = NULL;

const char* LOADERS[] = { "Vanilla", "fabric", "forge", "neoforge", "quilt" };

const char* VERSIONS_RELEASE[] = {
    "26.3", "26.2", "26.1",
    "1.21.5", "1.21.4", "1.21.3", "1.21.2", "1.21.1", "1.21",
    "1.20.6", "1.20.5", "1.20.4", "1.20.3", "1.20.2", "1.20.1", "1.20",
    "1.19.4", "1.19.3", "1.19.2", "1.19.1", "1.19",
    "1.18.2", "1.18.1", "1.18",
    "1.17.1", "1.17",
    "1.16.5", "1.16.4", "1.16.3", "1.16.2", "1.16.1", "1.16",
    "1.15.2", "1.15.1", "1.15",
    "1.14.4", "1.14.3", "1.14.2", "1.14.1", "1.14",
    "1.13.2", "1.13.1", "1.13",
    "1.12.2", "1.12.1", "1.12",
    "1.11.2", "1.11.1", "1.11",
    "1.10.2", "1.10.1", "1.10",
    "1.9.4", "1.9.3", "1.9.2", "1.9.1", "1.9",
    "1.8.9", "1.8.8", "1.8.7", "1.8.6", "1.8.5", "1.8.4", "1.8.3", "1.8.2", "1.8.1", "1.8",
    "1.7.10", "1.7.9", "1.7.8", "1.7.7", "1.7.6", "1.7.5", "1.7.4", "1.7.2",
    "1.6.4", "1.6.2", "1.6.1",
    "1.5.2", "1.5.1",
    "1.4.7", "1.4.6", "1.4.5", "1.4.4", "1.4.2",
    "1.3.2", "1.3.1",
    "1.2.5", "1.2.4", "1.2.3", "1.2.2", "1.2.1",
    "1.1", "1.0"
};

const char* VERSIONS_SNAPSHOT[] = {
    "26.4-rc1", "26.4-pre2", "26.4-pre1", "26w18a", "26w16a", "26w14a", "26w10a", "26w06a", "26w02a",
    "25w46a", "25w44a", "25w41a", "25w35a", "25w20a", "25w14craftmine", "25w08a", "25w04a",
    "1.21.4-rc3", "1.21.4-pre1", "24w46a", "24w45a", "24w44a", "24w40a", "24w33a",
    "24w14potato", "24w12a", "24w09a", "24w03a",
    "23w51b", "23w45a", "23w31a", "23w13a_or_b", "23w07a",
    "22w46a", "22w24a", "22w13oneblockatatime", "22w11a",
    "21w44a", "21w37a", "21w14a", "21w03a",
    "20w51a", "20w45a", "20w14infinite", "20w06a",
    "19w46b", "19w34a", "19w13b", "19w02a",
    "18w50a", "18w43c", "18w30b", "18w16a", "18w01a",
    "17w50a", "17w43b", "17w18b", "17w06a",
    "16w50a", "16w39c", "16w20a", "16w02a",
    "15w51b", "15w41b", "15w31c", "15w14a",
    "14w34d", "14w25b", "14w11b", "14w02c",
    "13w48b", "13w36b", "13w24b", "13w16b", "13w01b",
    "12w50b", "12w34b", "12w21b", "12w07b", "11w49a"
};

const char* VERSIONS_BETA[] = {
    "b1.8.1", "b1.8", "b1.7.3", "b1.7.2", "b1.7",
    "b1.6.6", "b1.6.5", "b1.6.4", "b1.6",
    "b1.5_01", "b1.5", "b1.4_01", "b1.4",
    "b1.3_01", "b1.3b", "b1.2_02", "b1.2",
    "b1.1_02", "b1.0_01", "b1.0",
    "a1.2.6", "a1.2.5", "a1.2.3_04", "a1.2.2b", "a1.2.0_02",
    "a1.1.2_01", "a1.1.0", "a1.0.17_04", "a1.0.15", "a1.0.11", "a1.0.4",
    "inf-20100618",
    "c0.30_01c", "c0.0.13a", "c0.0.11a",
    "rd-161348", "rd-160052", "rd-132328", "rd-132211"
};

#define COUNT_RELEASE  (sizeof(VERSIONS_RELEASE) / sizeof(VERSIONS_RELEASE[0]))
#define COUNT_SNAPSHOT (sizeof(VERSIONS_SNAPSHOT) / sizeof(VERSIONS_SNAPSHOT[0]))
#define COUNT_BETA     (sizeof(VERSIONS_BETA) / sizeof(VERSIONS_BETA[0]))

char activeVersions[256][64];
int activeVersionCount = 0;

typedef struct {
    HWND hwnd;
    char* args;
} ThreadParams;

COLORREF GetGradTopColor()     { return isDarkTheme ? RGB(38, 38, 42)   : RGB(252, 252, 255); }
COLORREF GetGradBottomColor()  { return isDarkTheme ? RGB(18, 18, 20)   : RGB(228, 232, 240); }
COLORREF GetCardBgColor()      { return isDarkTheme ? RGB(44, 44, 46)   : RGB(255, 255, 255); }
COLORREF GetCardHoverColor()   { return isDarkTheme ? RGB(58, 58, 62)   : RGB(242, 244, 248); }
COLORREF GetTextColorCustom()  { return isDarkTheme ? RGB(245, 245, 247) : RGB(28, 28, 30); }
COLORREF GetSubTextColor()     { return isDarkTheme ? RGB(152, 152, 157): RGB(110, 110, 115); }
COLORREF GetAccentColor()      { return isDarkTheme ? RGB(10, 132, 255) : RGB(0, 122, 255); }
COLORREF GetAccentHoverColor() { return isDarkTheme ? RGB(64, 156, 255) : RGB(0, 102, 214); }
COLORREF GetSecBtnColor()      { return isDarkTheme ? RGB(58, 58, 60)   : RGB(225, 228, 235); }
COLORREF GetSecBtnHoverColor() { return isDarkTheme ? RGB(74, 74, 78)   : RGB(208, 212, 222); }
COLORREF GetBorderColor()      { return isDarkTheme ? RGB(72, 72, 76)   : RGB(205, 208, 216); }
COLORREF GetArrowColor()       { return isDarkTheme ? RGB(85, 85, 90)   : RGB(110, 110, 115); }

void RebuildVersionList() {
    int fetched = FetchMojangVersions(activeVersions, 256, showSnapshots, showBetas);
    if (fetched > 0) {
        activeVersionCount = fetched;
        // Se la versione selezionata non è nella nuova lista, imposta la più recente
        BOOL found = FALSE;
        for (int i = 0; i < activeVersionCount; i++) {
            if (strcmp(activeVersions[i], selectedVersion) == 0) {
                found = TRUE;
                break;
            }
        }
        if (!found) {
            strcpy(selectedVersion, activeVersions[0]);
            if (hBtnVersion) SetWindowTextA(hBtnVersion, selectedVersion);
        }
        return;
    }

    // Fallback offline se non c'è connessione internet
    activeVersionCount = 0;
    for (size_t i = 0; i < COUNT_RELEASE && activeVersionCount < 256; i++) {
        strcpy(activeVersions[activeVersionCount++], VERSIONS_RELEASE[i]);
    }
}

void UpdateThemeBrushes(HWND hwnd) {
    if (hBrushInput) DeleteObject(hBrushInput);
    hBrushInput = CreateSolidBrush(GetCardBgColor());

    BOOL dark = isDarkTheme ? TRUE : FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

    if (hEditLog) {
        SetWindowTheme(hEditLog, isDarkTheme ? L"DarkMode_Explorer" : L"Explorer", NULL);

        RECT rc;
        GetClientRect(hEditLog, &rc);
        HRGN hRgn = CreateRoundRectRgn(0, 0, rc.right + 1, rc.bottom + 1, 14, 14);
        SetWindowRgn(hEditLog, hRgn, TRUE);
    }

    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
}

void DrawDownArrow(HDC hdc, int cx, int cy, COLORREF col) {
    HPEN pen = CreatePen(PS_SOLID, 2, col);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    MoveToEx(hdc, cx - 4, cy - 2, NULL);
    LineTo(hdc, cx, cy + 2);
    LineTo(hdc, cx + 5, cy - 3);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

LRESULT CALLBACK RoundedCtrlProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_MOUSEMOVE) {
        if (hHoveredBtn != hwnd) {
            hHoveredBtn = hwnd;
            TRACKMOUSEEVENT tme = { sizeof(tme), TME_LEAVE, hwnd, 0 };
            TrackMouseEvent(&tme);
            InvalidateRect(hwnd, NULL, FALSE);
        }
    } else if (uMsg == WM_MOUSELEAVE) {
        if (hHoveredBtn == hwnd) {
            hHoveredBtn = NULL;
            InvalidateRect(hwnd, NULL, FALSE);
        }
    }
    return CallWindowProcA(origBtnProc, hwnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK RoundedEditProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    LRESULT res = CallWindowProcA(origEditProc, hwnd, uMsg, wParam, lParam);
    if (uMsg == WM_NCPAINT || uMsg == WM_PAINT) {
        HDC hdc = GetWindowDC(hwnd);
        RECT rc;
        GetWindowRect(hwnd, &rc);
        OffsetRect(&rc, -rc.left, -rc.top);

        HPEN pen = CreatePen(PS_SOLID, 1, GetBorderColor());
        HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
        HGDIOBJ oldPen = SelectObject(hdc, pen);
        HGDIOBJ oldBrush = SelectObject(hdc, nullBrush);

        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 12, 12);

        SelectObject(hdc, oldPen);
        SelectObject(hdc, oldBrush);
        DeleteObject(pen);
        ReleaseDC(hwnd, hdc);
    }
    return res;
}

void CreateDirectories(const char* path) {
    char* tmp = _strdup(path);
    if (!tmp) return;
    size_t len = strlen(tmp);
    if (len > 0 && (tmp[len - 1] == '\\' || tmp[len - 1] == '/')) {
        tmp[len - 1] = 0;
    }
    for (char* p = tmp + 1; *p; p++) {
        if (*p == '\\' || *p == '/') {
            *p = 0;
            CreateDirectoryA(tmp, NULL);
            *p = '\\';
        }
    }
    CreateDirectoryA(tmp, NULL);
    free(tmp);
}

void BrowseFolder(HWND hwnd) {
    BROWSEINFOA bi = { 0 };
    bi.hwndOwner = hwnd;
    bi.lpszTitle = "Select the folder where Minecraft files will be saved:";
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl != NULL) {
        char* path = (char*)calloc(MAX_PATH, 1);
        if (path && SHGetPathFromIDListA(pidl, path)) {
            SetWindowTextA(hEditDir, path);
        }
        free(path);
        CoTaskMemFree(pidl);
    }
}

#define IDT_INTRO_ANIM 901
int introFrame = 0;
const int INTRO_MAX_FRAMES = 95;
BOOL showIntro = TRUE;

void GetEffectiveGameDir(char* outDir, size_t maxLen) {
    char base[MAX_PATH] = { 0 };
    GetWindowTextA(hEditDir, base, MAX_PATH);
    size_t len = strlen(base);
    while (len > 0 && (base[len - 1] == '\\' || base[len - 1] == '/')) base[--len] = '\0';

    if (isIsolated && len > 0) {
        snprintf(outDir, maxLen, "%s\\instances\\%s", base, selectedVersion);
    } else {
        snprintf(outDir, maxLen, "%s", base);
    }
}

void SaveConfigWin() {
    char cfgPath[MAX_PATH], user[128], dir[MAX_PATH];
    GetModuleFileNameA(NULL, cfgPath, MAX_PATH);
    char* slash = strrchr(cfgPath, '\\');
    if (slash) strcpy(slash + 1, "portablemc_gui.ini");
    else strcpy(cfgPath, "portablemc_gui.ini");

    GetWindowTextA(hEditUser, user, sizeof(user));
    GetWindowTextA(hEditDir, dir, sizeof(dir));

    FILE* f = fopen(cfgPath, "w");
    if (!f) return;
    fprintf(f, "dark=%d\nauth=%d\nuser=%s\nloader=%d\nver=%s\ndir=%s\nisolated=%d\nautoclose=%d\n",
            isDarkTheme, authMode, user, loaderIdx, selectedVersion, dir, isIsolated, isAutoClose);
    fclose(f);
}

void LoadConfigWin() {
    char cfgPath[MAX_PATH];
    GetModuleFileNameA(NULL, cfgPath, MAX_PATH);
    char* slash = strrchr(cfgPath, '\\');
    if (slash) strcpy(slash + 1, "portablemc_gui.ini");
    else strcpy(cfgPath, "portablemc_gui.ini");

    FILE* f = fopen(cfgPath, "r");
    if (!f) return;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strncmp(line, "dark=", 5) == 0) isDarkTheme = atoi(line + 5);
        else if (strncmp(line, "auth=", 5) == 0) authMode = atoi(line + 5);
        else if (strncmp(line, "user=", 5) == 0) SetWindowTextA(hEditUser, line + 5);
        else if (strncmp(line, "loader=", 7) == 0) loaderIdx = atoi(line + 7);
        else if (strncmp(line, "ver=", 4) == 0) strncpy(selectedVersion, line + 4, sizeof(selectedVersion) - 1);
        else if (strncmp(line, "dir=", 4) == 0 && strlen(line + 4) > 0) SetWindowTextA(hEditDir, line + 4);
        else if (strncmp(line, "isolated=", 9) == 0) isIsolated = atoi(line + 9);
        else if (strncmp(line, "autoclose=", 10) == 0) isAutoClose = atoi(line + 10);
    }
    fclose(f);
    SetWindowTextA(hBtnTheme, isDarkTheme ? "Dark Theme" : "Light Theme");
    SetWindowTextA(hBtnLoader, LOADERS[loaderIdx]);
    SetWindowTextA(hBtnVersion, selectedVersion);
}

void RunBoostFps(HWND hwnd) {
    if (loaderIdx == 0) {
        ShowCustomAlert(hwnd, "Select a Mod Loader", "Boost FPS requires a Mod Loader (select 'fabric' recommended).", NULL);
        return;
    }
    char gameDir[MAX_PATH];
    GetEffectiveGameDir(gameDir, sizeof(gameDir));
    CreateDirectories(gameDir);

    int count = InstallBoostFPSPack(gameDir, LOADERS[loaderIdx], selectedVersion);
    char modsDir[MAX_PATH], msg[256];
    snprintf(modsDir, sizeof(modsDir), "%s\\mods", gameDir);
    snprintf(msg, sizeof(msg), "Installed %d optimization mods (Sodium, Lithium, FerriteCore, EntityCulling) for %s!", count, selectedVersion);
    ShowCustomAlert(hwnd, "Boost FPS Ready", msg, modsDir);
}

typedef struct {
    char query[128];
    HWND hEdit;
    BOOL submitted;
    BOOL done;
} SearchInputData;

LRESULT CALLBACK SearchInputProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SearchInputData* data = (SearchInputData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (uMsg) {
        case WM_CREATE: {
            CREATESTRUCTA* cs = (CREATESTRUCTA*)lParam;
            data = (SearchInputData*)cs->lpCreateParams;
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)data);

            RECT rc;
            GetClientRect(hwnd, &rc);
            HRGN hRgn = CreateRoundRectRgn(0, 0, rc.right + 1, rc.bottom + 1, 20, 20);
            SetWindowRgn(hwnd, hRgn, TRUE);

            data->hEdit = CreateWindowA("EDIT", "",
                WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | WS_BORDER,
                24, 62, 302, 28, hwnd, (HMENU)10, NULL, NULL);
            SendMessage(data->hEdit, WM_SETFONT, (WPARAM)hFontMain, TRUE);
            SendMessageW(data->hEdit, EM_SETCUEBANNER, TRUE, (LPARAM)L"e.g. sodium, iris, jei, xaero...");
            SetFocus(data->hEdit);

            HWND hBtnCancel = CreateWindowA("BUTTON", "Cancel",
                WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 24, 108, 145, 34, hwnd, (HMENU)2, NULL, NULL);
            HWND hBtnSearch = CreateWindowA("BUTTON", "Search",
                WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 181, 108, 145, 34, hwnd, (HMENU)1, NULL, NULL);
            SendMessage(hBtnCancel, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            SendMessage(hBtnSearch, WM_SETFONT, (WPARAM)hFontBtn, TRUE);
            return 0;
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);
            COLORREF cTop = GetGradTopColor();
            COLORREF cBot = GetGradBottomColor();
            TRIVERTEX v[2] = {
                { 0, 0, (COLOR16)(GetRValue(cTop) << 8), (COLOR16)(GetGValue(cTop) << 8), (COLOR16)(GetBValue(cTop) << 8), 0 },
                { rc.right, rc.bottom, (COLOR16)(GetRValue(cBot) << 8), (COLOR16)(GetGValue(cBot) << 8), (COLOR16)(GetBValue(cBot) << 8), 0 }
            };
            GRADIENT_RECT gRect = { 0, 1 };
            GradientFill(hdc, v, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

            HPEN pen = CreatePen(PS_SOLID, 1, GetBorderColor());
            HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
            HGDIOBJ oldP = SelectObject(hdc, pen);
            HGDIOBJ oldB = SelectObject(hdc, nullBr);
            RoundRect(hdc, 0, 0, rc.right - 1, rc.bottom - 1, 20, 20);
            SelectObject(hdc, oldP); SelectObject(hdc, oldB); DeleteObject(pen);
            return 1;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc, TRANSPARENT);
            RECT rcTitle = { 24, 20, 326, 48 };
            SelectObject(hdc, hFontTitle);
            SetTextColor(hdc, GetTextColorCustom());
            DrawTextA(hdc, "Search Modrinth", -1, &rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (dis->CtlType == ODT_BUTTON) {
                HDC hdc = dis->hDC;
                RECT rc = dis->rcItem;
                BOOL isSearch = (dis->CtlID == 1);
                COLORREF bg = isSearch ? GetAccentColor() : GetSecBtnColor();
                COLORREF fg = isSearch ? RGB(255, 255, 255) : GetTextColorCustom();

                HBRUSH hBr = CreateSolidBrush(bg);
                HPEN hPn = CreatePen(PS_SOLID, 1, bg);
                HGDIOBJ oldB = SelectObject(hdc, hBr);
                HGDIOBJ oldP = SelectObject(hdc, hPn);
                RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 14, 14);
                SelectObject(hdc, oldB); SelectObject(hdc, oldP);
                DeleteObject(hBr); DeleteObject(hPn);

                char txt[64];
                GetWindowTextA(dis->hwndItem, txt, sizeof(txt));
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, fg);
                SelectObject(hdc, hFontBtn);
                DrawTextA(hdc, txt, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            break;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 1 && data) {
                GetWindowTextA(data->hEdit, data->query, sizeof(data->query));
                data->submitted = TRUE;
                data->done = TRUE;
                DestroyWindow(hwnd);
            } else if (id == 2 && data) {
                data->done = TRUE;
                DestroyWindow(hwnd);
            }
            return 0;
        }

        case WM_CLOSE:
            if (data) data->done = TRUE;
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void RunSearchMods(HWND hwnd) {
    if (loaderIdx == 0) {
        ShowCustomAlert(hwnd, "Select a Mod Loader", "Please select Fabric, Forge, NeoForge, or Quilt before searching mods.", NULL);
        return;
    }

    static BOOL reg = FALSE;
    if (!reg) {
        WNDCLASSA wc = { 0 };
        wc.lpfnWndProc = SearchInputProc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "PortableMCSearchInput";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassA(&wc);
        reg = TRUE;
    }

    SearchInputData sData = { {0}, NULL, FALSE, FALSE };
    RECT rcParent;
    GetWindowRect(hwnd, &rcParent);
    int w = 350, h = 162;
    int x = rcParent.left + ((rcParent.right - rcParent.left) - w) / 2;
    int y = rcParent.top + ((rcParent.bottom - rcParent.top) - h) / 2;

    EnableWindow(hwnd, FALSE);
    CreateWindowExA(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, "PortableMCSearchInput", "Search",
        WS_POPUP | WS_VISIBLE, x, y, w, h, hwnd, NULL, GetModuleHandleA(NULL), &sData);

    MSG msg;
    while (!sData.done && GetMessage(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) {
            SendMessageA(GetParent(msg.hwnd), WM_COMMAND, 1, 0);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    EnableWindow(hwnd, TRUE);
    SetForegroundWindow(hwnd);

    if (!sData.submitted || strlen(sData.query) == 0) return;

    char slugs[8][64] = {{0}};
    char titles[8][128] = {{0}};
    int found = SearchModrinthProjects(sData.query, LOADERS[loaderIdx], selectedVersion, slugs, titles, 8);
    if (found <= 0) {
        ShowCustomAlert(hwnd, "No Results", "No compatible mods found on Modrinth for this search.", NULL);
        return;
    }

    RECT rc;
    GetWindowRect(btnSearchMods, &rc);
    HMENU hMenu = CreatePopupMenu();
    for (int i = 0; i < found; i++) {
        AppendMenuA(hMenu, MF_STRING, 5000 + i, titles[i]);
    }
    int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, rc.left, rc.bottom + 2, 0, hwnd, NULL);
    DestroyMenu(hMenu);

    if (cmd >= 5000 && cmd < 5000 + found) {
        int idx = cmd - 5000;
        char gameDir[MAX_PATH], modsDir[MAX_PATH], alertMsg[256];
        GetEffectiveGameDir(gameDir, sizeof(gameDir));
        CreateDirectories(gameDir);
        snprintf(modsDir, sizeof(modsDir), "%s\\mods", gameDir);

        if (DownloadModFromModrinth(slugs[idx], gameDir, LOADERS[loaderIdx], selectedVersion)) {
            snprintf(alertMsg, sizeof(alertMsg), "'%s' was downloaded and installed!", titles[idx]);
            ShowCustomAlert(hwnd, "Mod Installed", alertMsg, modsDir);
        } else {
            ShowCustomAlert(hwnd, "Download Failed", "Could not download the selected mod.", NULL);
        }
    }
}

void ImportModJar(HWND hwnd) {
    char* baseDir = (char*)calloc(MAX_PATH, 1);
    GetWindowTextA(hEditDir, baseDir, MAX_PATH);
    if (strlen(baseDir) == 0) {
        MessageBoxA(hwnd, "Please select a destination game folder first.", "Warning", MB_ICONWARNING);
        free(baseDir);
        return;
    }

    char* modFile = (char*)calloc(MAX_PATH, 1);
    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "Minecraft Mod Files (*.jar)\0*.jar\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = modFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = "Select the mod (.jar) to import";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn)) {
        char* modsFolder = (char*)calloc(MAX_PATH, 1);
        snprintf(modsFolder, MAX_PATH, "%s\\mods", baseDir);
        CreateDirectories(modsFolder);

        char* fileName = strrchr(modFile, '\\');
        fileName = fileName ? fileName + 1 : modFile;

        char* destPath = (char*)calloc(MAX_PATH, 1);
        snprintf(destPath, MAX_PATH, "%s\\%s", modsFolder, fileName);

        if (CopyFileA(modFile, destPath, FALSE)) {
            char* msg = (char*)calloc(MAX_PATH + 128, 1);
            snprintf(msg, MAX_PATH + 128, "Mod successfully imported to:\n%s", destPath);
            MessageBoxA(hwnd, msg, "Mod Imported", MB_ICONINFORMATION);
            free(msg);
        } else {
            MessageBoxA(hwnd, "Failed to copy the .jar file into the mods folder.", "Error", MB_ICONERROR);
        }
        free(modsFolder);
        free(destPath);
    }
    free(baseDir);
    free(modFile);
}

BOOL HasCustomSkinLoaderInMods(const char* modsDir) {
    char searchPath[MAX_PATH];
    snprintf(searchPath, sizeof(searchPath), "%s\\CustomSkinLoader*.jar", modsDir);
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchPath, &fd);
    if (hFind != INVALID_HANDLE_VALUE) {
        FindClose(hFind);
        return TRUE;
    }
    return FALSE;
}

void ImportSkinOrCape(HWND hwnd, BOOL isCape) {
    // 1. Se è in modalità Microsoft, apre il profilo ufficiale sul browser
    if (authMode == 1) {
        ShellExecuteA(NULL, "open", "https://minecraft.net", NULL, NULL, SW_SHOWNORMAL);
        return;
    }

    // 2. Se è in modalità Offline e il Mod Loader selezionato è "Vanilla" (loaderIdx == 0), avvisa subito
    if (loaderIdx == 0) {
        ShowCustomAlert(hwnd,
            "Vanilla Not Supported",
            "Offline skins and capes cannot be applied on Vanilla.\n\nPlease select Fabric, Forge, NeoForge, or Quilt and install the CustomSkinLoader mod.",
            NULL);
        return;
    }

    char baseDir[MAX_PATH] = { 0 };
    GetWindowTextA(hEditDir, baseDir, MAX_PATH);
    size_t dLen = strlen(baseDir);
    while (dLen > 0 && (baseDir[dLen - 1] == '\\' || baseDir[dLen - 1] == '/')) {
        baseDir[--dLen] = '\0';
    }
    if (dLen == 0) {
        ShowCustomAlert(hwnd, "Missing Game Folder", "Please select a destination game folder first.", NULL);
        return;
    }

    char username[128] = { 0 };
    GetWindowTextA(hEditUser, username, sizeof(username));
    if (strlen(username) == 0) {
        strcpy(username, "Steve");
    }

    char pngFile[MAX_PATH] = { 0 };
    OPENFILENAMEA ofn = { 0 };
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = "PNG Texture (*.png)\0*.png\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = pngFile;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrTitle = isCape ? "Select Cape texture (.png)" : "Select Skin texture (.png)";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&ofn)) {
        char targetFolder[MAX_PATH];
        snprintf(targetFolder, MAX_PATH, "%s\\CustomSkinLoader\\LocalSkin\\%s",
                 baseDir, isCape ? "capes" : "skins");
        CreateDirectories(targetFolder);

        char destPath[MAX_PATH];
        snprintf(destPath, MAX_PATH, "%s\\%s.png", targetFolder, username);

        if (CopyFileA(pngFile, destPath, FALSE)) {
            // Crea automaticamente CustomSkinLoader.json con LocalSkin prioritario e checkPNG: false
            char cslRoot[MAX_PATH], jsonPath[MAX_PATH];
            snprintf(cslRoot, MAX_PATH, "%s\\CustomSkinLoader", baseDir);
            snprintf(jsonPath, MAX_PATH, "%s\\CustomSkinLoader.json", cslRoot);

            if (GetFileAttributesA(jsonPath) == INVALID_FILE_ATTRIBUTES) {
                FILE* fjson = fopen(jsonPath, "w");
                if (fjson) {
                    fprintf(fjson,
                        "{\n"
                        "  \"enable\": true,\n"
                        "  \"enableDynamicSkull\": true,\n"
                        "  \"enableTransparentSkin\": true,\n"
                        "  \"forceLoadAllTextures\": true,\n"
                        "  \"enableLocalProfile\": true,\n"
                        "  \"loadlist\": [\n"
                        "    {\n"
                        "      \"name\": \"LocalSkin\",\n"
                        "      \"type\": \"Legacy\",\n"
                        "      \"checkPNG\": false,\n"
                        "      \"skin\": \"LocalSkin/skins/{USERNAME}.png\",\n"
                        "      \"cape\": \"LocalSkin/capes/{USERNAME}.png\",\n"
                        "      \"elytra\": \"LocalSkin/elytras/{USERNAME}.png\"\n"
                        "    }\n"
                        "  ]\n"
                        "}\n"
                    );
                    fclose(fjson);
                }
            }

        char modsDir[MAX_PATH];
        snprintf(modsDir, sizeof(modsDir), "%s\\mods", baseDir);
        CreateDirectories(modsDir);

        const char* statusNote = "CustomSkinLoader is already installed in your mods folder.";
        if (!HasCustomSkinLoaderInMods(modsDir)) {
            int loaderIdx = (int)SendMessageA(lblLoader, CB_GETCURSEL, 0, 0);
            int verIdx = (int)SendMessageA(hVer, CB_GETCURSEL, 0, 0);
            char verStr[64] = "26.3";
            if (verIdx != CB_ERR) {
                SendMessageA(hVer, CB_GETLBTEXT, verIdx, (LPARAM)verStr);
            }
            const char* loaderStr = (loaderIdx >= 1 && loaderIdx <= 4) ? LOADERS[loaderIdx] : "fabric";

            if (DownloadModFromModrinth("customskinloader", baseDir, loaderStr, selectedVersion)) {
                statusNote = "CustomSkinLoader was automatically downloaded to your mods folder!";
            }
            else {
                statusNote = "Could not auto-download CustomSkinLoader. Please import the .jar manually via '+ Mod'.";
            }
        }

        char finalMsg[512];
        snprintf(finalMsg, sizeof(finalMsg), "Texture saved for '%s'!\n\n%s", username, statusNote);
        MessageBoxA(hwnd, finalMsg, isCape ? "Cape Imported" : "Skin Imported", MB_OK | MB_ICONINFORMATION);
        }
    }
}

void CleanAndSendLog(HWND hwnd, const char* raw) {
    size_t len = strlen(raw);
    char* clean = (char*)calloc(len * 2 + 1, 1);
    if (!clean) return;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (raw[i] == '\x1b' && raw[i + 1] == '[') {
            i += 2;
            while (i < len && !((raw[i] >= 'A' && raw[i] <= 'Z') || (raw[i] >= 'a' && raw[i] <= 'z'))) {
                i++;
            }
            continue;
        }
        if (raw[i] == '\r') {
            if (i + 1 < len && raw[i + 1] == '\n') continue;
            clean[j++] = '\r';
            clean[j++] = '\n';
            continue;
        }
        if (raw[i] == '\n') {
            clean[j++] = '\r';
            clean[j++] = '\n';
        } else {
            clean[j++] = raw[i];
        }
    }
    clean[j] = '\0';

    if (j > 0) {
        if (strstr(clean, "[ERROR]") || strstr(clean, "error:") || strstr(clean, "Error:")) {
            appStatus = 2;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        if (strstr(clean, "Downloading") || strstr(clean, "downloading")) {
            isDownloading = TRUE;
        }
        if (strstr(clean, "Starting ") || strstr(clean, "Launching ")) {
            isDownloading = FALSE;
        }
        if (isAutoClose && isRunningCmd && (strstr(clean, "Launching ") || strstr(clean, "Setting user:"))) {
            SaveConfigWin();
            PostMessageA(hwnd, WM_CLOSE, 0, 0);
        }

        int bestPct = -1;

        for (size_t k = 0; k < j; k++) {
            if (clean[k] == '(' && k + 3 < j && clean[k + 1] >= '0' && clean[k + 1] <= '9') {
                char* slash = NULL;
                long done = strtol(&clean[k + 1], &slash, 10);
                if (slash && *slash == '/' && *(slash + 1) >= '0' && *(slash + 1) <= '9') {
                    char* closeParen = NULL;
                    long total = strtol(slash + 1, &closeParen, 10);
                    if (closeParen && *closeParen == ')' && total > 0) {
                        int globalPct = (int)((done * 100) / total);
                        if (globalPct >= 0 && globalPct <= 100) {
                            bestPct = globalPct;
                        }
                    }
                }
            }
        }

        if (bestPct <= 0) {
            for (size_t k = 0; k < j; k++) {
                if (clean[k] == '%' && k > 0 && clean[k - 1] >= '0' && clean[k - 1] <= '9') {
                    size_t start = k - 1;
                    while (start > 0 && ((clean[start - 1] >= '0' && clean[start - 1] <= '9') || clean[start - 1] == '.')) {
                        start--;
                    }
                    double val = atof(&clean[start]);
                    int pct = (int)val;
                    if (pct >= 0 && pct <= 100) {
                        bestPct = pct;
                    }
                }
            }
        }

        if (bestPct >= 0 && isRunningCmd) {
            downloadPercent = bestPct;
            InvalidateRect(hBtnLaunch, NULL, FALSE);
        }

        PostMessageA(hwnd, WM_APPEND_LOG, 0, (LPARAM)clean);
    } else {
        free(clean);
    }
}

DWORD WINAPI ThreadRunBat(LPVOID lpParam) {
    ThreadParams* p = (ThreadParams*)lpParam;
    HWND hwnd = p->hwnd;

    char* exeDir = (char*)calloc(MAX_PATH, 1);
    GetModuleFileNameA(NULL, exeDir, MAX_PATH);
    char* lastSlash = strrchr(exeDir, '\\');
    if (lastSlash) *lastSlash = '\0';

    CleanAndSendLog(hwnd, "Starting process...\n");
    Sleep(1000);

    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        CleanAndSendLog(hwnd, "[ERROR] Unable to initialize output pipe.\n");
        PostMessageA(hwnd, WM_CMD_FINISHED, 0, 0);
        free(exeDir); free(p->args); free(p);
        return 1;
    }
    SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    ZeroMemory(&pi, sizeof(pi));

    // Controlla automaticamente sia portablemc-cli.exe sia portablemc.exe
    char* selfPath = (char*)calloc(MAX_PATH, 1);
    GetModuleFileNameA(NULL, selfPath, MAX_PATH);
    char* selfName = strrchr(selfPath, '\\');
    selfName = selfName ? selfName + 1 : selfPath;

    char* cliPath = (char*)calloc(MAX_PATH, 1);
    snprintf(cliPath, MAX_PATH, "%s\\portablemc-cli.exe", exeDir);

    if (GetFileAttributesA(cliPath) == INVALID_FILE_ATTRIBUTES) {
        if (_stricmp(selfName, "portablemc.exe") != 0) {
            snprintf(cliPath, MAX_PATH, "%s\\portablemc.exe", exeDir);
        }
    }

    char* cmdLine = (char*)calloc(MAX_PATH + 4096, 1);
    snprintf(cmdLine, MAX_PATH + 4096, "\"%s\" %s", cliPath, p->args);

    if (CreateProcessA(NULL, cmdLine, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, exeDir, &si, &pi)) {
        hActiveProcess = pi.hProcess;
        CloseHandle(hWritePipe);

        char buffer[1024];
        DWORD bytesRead;
        while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            CleanAndSendLog(hwnd, buffer);
        }

        WaitForSingleObject(pi.hProcess, INFINITE);
        hActiveProcess = NULL;
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        CloseHandle(hWritePipe);
        CleanAndSendLog(hwnd, "[ERROR] Make sure portablemc.exe or portablemc-cli.exe is in the same folder.\n");
    }

    CloseHandle(hReadPipe);
    CleanAndSendLog(hwnd, "--- Process finished ---\n");
    PostMessageA(hwnd, WM_CMD_FINISHED, 0, 0);

    free(selfPath);
    free(cliPath);
    free(exeDir);
    free(cmdLine);
    free(p->args);
    free(p);
    return 0;
}

void RunCommandInBackground(HWND hwnd, const char* args) {
    SetWindowTextA(hEditLog, "");
    isRunningCmd = TRUE;
    isDownloading = FALSE;
    appStatus = 1;
    downloadPercent = 0;

    EnableWindow(hBtnLogin, FALSE);
    InvalidateRect(hBtnLaunch, NULL, FALSE);
    InvalidateRect(hBtnLogin, NULL, FALSE);
    InvalidateRect(hwnd, NULL, FALSE);

    ThreadParams* p = (ThreadParams*)calloc(1, sizeof(ThreadParams));
    p->hwnd = hwnd;
    p->args = _strdup(args);

    HANDLE hThread = CreateThread(NULL, 0, ThreadRunBat, p, 0, NULL);
    if (hThread) {
        CloseHandle(hThread);
    }
}

void UpdateAuthUI() {
    SetWindowTextA(hBtnAuth, authMode == 1 ? "Microsoft" : "Username");
    if (authMode == 1) {
        ShowWindow(hBtnLogin, SW_SHOW);
        MoveWindow(hEditUser, 175, 62, 230, 26, TRUE);
        SendMessageW(hEditUser, EM_SETCUEBANNER, TRUE, (LPARAM)L"Optional Microsoft nickname...");
    } else {
        ShowWindow(hBtnLogin, SW_HIDE);
        MoveWindow(hEditUser, 175, 62, 315, 26, TRUE);
        SendMessageW(hEditUser, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter offline username...");
    }
    InvalidateRect(hBtnAuth, NULL, FALSE);
}

void ShowPopupDropdown(HWND hwnd, HWND hCtrl, const char** items, int count, int baseCmdId) {
    RECT rc;
    GetWindowRect(hCtrl, &rc);
    HMENU hMenu = CreatePopupMenu();
    for (int i = 0; i < count; i++) {
        AppendMenuA(hMenu, MF_STRING, baseCmdId + i, items[i]);
    }
    TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, rc.left, rc.bottom + 2, 0, hwnd, NULL);
    DestroyMenu(hMenu);
}

typedef struct {
    const char* title;
    const char* message;
    const char* folderPath;
    HWND hBtnOk;
    HWND hBtnOpen;
    HWND hHovered;
    BOOL done;
} CustomAlertData;

LRESULT CALLBACK CustomAlertProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    CustomAlertData* data = (CustomAlertData*)GetWindowLongPtrA(hwnd, GWLP_USERDATA);

    switch (uMsg) {
        case WM_CREATE: {
            CREATESTRUCTA* cs = (CREATESTRUCTA*)lParam;
            data = (CustomAlertData*)cs->lpCreateParams;
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, (LONG_PTR)data);

            RECT rc;
            GetClientRect(hwnd, &rc);
            HRGN hRgn = CreateRoundRectRgn(0, 0, rc.right + 1, rc.bottom + 1, 20, 20);
            SetWindowRgn(hwnd, hRgn, TRUE);

            if (data->folderPath && strlen(data->folderPath) > 0) {
                data->hBtnOpen = CreateWindowA("BUTTON", "Open Folder",
                    WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 24, 158, 145, 34, hwnd, (HMENU)2, NULL, NULL);
                data->hBtnOk = CreateWindowA("BUTTON", "OK",
                    WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 181, 158, 145, 34, hwnd, (HMENU)1, NULL, NULL);
            } else {
                data->hBtnOk = CreateWindowA("BUTTON", "OK",
                    WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 95, 158, 160, 34, hwnd, (HMENU)1, NULL, NULL);
            }
            return 0;
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);

            COLORREF cTop = GetGradTopColor();
            COLORREF cBot = GetGradBottomColor();

            TRIVERTEX vertex[2];
            vertex[0].x = 0; vertex[0].y = 0;
            vertex[0].Red = GetRValue(cTop) << 8; vertex[0].Green = GetGValue(cTop) << 8; vertex[0].Blue = GetBValue(cTop) << 8; vertex[0].Alpha = 0;
            vertex[1].x = rc.right; vertex[1].y = rc.bottom;
            vertex[1].Red = GetRValue(cBot) << 8; vertex[1].Green = GetGValue(cBot) << 8; vertex[1].Blue = GetBValue(cBot) << 8; vertex[1].Alpha = 0;

            GRADIENT_RECT gRect = { 0, 1 };
            GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);

            HPEN pen = CreatePen(PS_SOLID, 1, GetBorderColor());
            HBRUSH nullBr = (HBRUSH)GetStockObject(NULL_BRUSH);
            HGDIOBJ oldP = SelectObject(hdc, pen);
            HGDIOBJ oldB = SelectObject(hdc, nullBr);
            RoundRect(hdc, 0, 0, rc.right - 1, rc.bottom - 1, 20, 20);
            SelectObject(hdc, oldP);
            SelectObject(hdc, oldB);
            DeleteObject(pen);
            return 1;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            SetBkMode(hdc, TRANSPARENT);

            HBRUSH hBrIcon = CreateSolidBrush(GetAccentColor());
            HPEN hPnIcon = CreatePen(PS_SOLID, 1, GetAccentColor());
            HGDIOBJ oldB = SelectObject(hdc, hBrIcon);
            HGDIOBJ oldP = SelectObject(hdc, hPnIcon);
            Ellipse(hdc, 24, 22, 48, 46);
            SelectObject(hdc, oldB);
            SelectObject(hdc, oldP);
            DeleteObject(hBrIcon);
            DeleteObject(hPnIcon);

            RECT rcI = { 24, 22, 48, 46 };
            SelectObject(hdc, hFontBtn);
            SetTextColor(hdc, RGB(255, 255, 255));
            DrawTextA(hdc, "i", -1, &rcI, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            RECT rcTitle = { 58, 22, 326, 48 };
            SelectObject(hdc, hFontTitle);
            SetTextColor(hdc, GetTextColorCustom());
            DrawTextA(hdc, data ? data->title : "", -1, &rcTitle, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

            RECT rcMsg = { 24, 58, 326, 148 };
            SelectObject(hdc, hFontMain);
            SetTextColor(hdc, GetSubTextColor());
            DrawTextA(hdc, data ? data->message : "", -1, &rcMsg, DT_LEFT | DT_WORDBREAK);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (dis->CtlType == ODT_BUTTON) {
                HDC hdc = dis->hDC;
                RECT rc = dis->rcItem;

                POINT pt = { 0, 0 };
                MapWindowPoints(dis->hwndItem, hwnd, &pt, 1);
                SetViewportOrgEx(hdc, -pt.x, -pt.y, NULL);
                SendMessage(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0);
                SetViewportOrgEx(hdc, 0, 0, NULL);

                BOOL isOk = (dis->CtlID == 1);
                BOOL isPressed = (dis->itemState & ODS_SELECTED);

                COLORREF bg = isOk ? (isPressed ? GetAccentHoverColor() : GetAccentColor())
                                   : (isPressed ? GetSecBtnHoverColor() : GetSecBtnColor());
                COLORREF fg = isOk ? RGB(255, 255, 255) : GetTextColorCustom();

                HBRUSH hBr = CreateSolidBrush(bg);
                HPEN hPn = CreatePen(PS_SOLID, 1, bg);
                HGDIOBJ oldB = SelectObject(hdc, hBr);
                HGDIOBJ oldP = SelectObject(hdc, hPn);
                RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 14, 14);
                SelectObject(hdc, oldB);
                SelectObject(hdc, oldP);
                DeleteObject(hBr);
                DeleteObject(hPn);

                char txt[64];
                GetWindowTextA(dis->hwndItem, txt, sizeof(txt));
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, fg);
                SelectObject(hdc, hFontBtn);
                DrawTextA(hdc, txt, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                return TRUE;
            }
            break;
        }

        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProcA(hwnd, uMsg, wParam, lParam);
            if (hit == HTCLIENT) return HTCAPTION;
            return hit;
        }

        case WM_COMMAND: {
            int id = LOWORD(wParam);
            if (id == 2 && data && data->folderPath) {
                ShellExecuteA(NULL, "open", data->folderPath, NULL, NULL, SW_SHOWNORMAL);
            }
            if ((id == 1 || id == 2) && data) {
                data->done = TRUE;
                DestroyWindow(hwnd);
            }
            return 0;
        }

        case WM_CLOSE:
            if (data) data->done = TRUE;
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void ShowCustomAlert(HWND parent, const char* title, const char* message, const char* folderPath) {
    static BOOL registered = FALSE;
    if (!registered) {
        WNDCLASSA wc = { 0 };
        wc.lpfnWndProc = CustomAlertProc;
        wc.hInstance = GetModuleHandleA(NULL);
        wc.lpszClassName = "PortableMCCustomAlert";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        RegisterClassA(&wc);
        registered = TRUE;
    }

    CustomAlertData data = { title, message, folderPath, NULL, NULL, NULL, FALSE };

    RECT rcParent;
    GetWindowRect(parent, &rcParent);
    int w = 350, h = 212;
    int x = rcParent.left + ((rcParent.right - rcParent.left) - w) / 2;
    int y = rcParent.top + ((rcParent.bottom - rcParent.top) - h) / 2;

    EnableWindow(parent, FALSE);

    HWND hDlg = CreateWindowExA(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        "PortableMCCustomAlert", title,
        WS_POPUP | WS_VISIBLE,
        x, y, w, h,
        parent, NULL, GetModuleHandleA(NULL), &data
    );

    MSG msg;
    while (!data.done && GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    EnableWindow(parent, TRUE);
    SetForegroundWindow(parent);
}

BOOL CALLBACK SetChildVisibleProc(HWND hChild, LPARAM lParam) {
    ShowWindow(hChild, (int)lParam);
    return TRUE;
}

void DrawGradientQuad(HDC hdc, POINT p0, COLORREF c0, POINT p1, COLORREF c1, POINT p2, COLORREF c2, POINT p3, COLORREF c3) {
    TRIVERTEX v[4];
    POINT pts[4] = { p0, p1, p2, p3 };
    COLORREF cols[4] = { c0, c1, c2, c3 };

    for (int i = 0; i < 4; i++) {
        v[i].x = pts[i].x;
        v[i].y = pts[i].y;
        v[i].Red   = (COLOR16)(GetRValue(cols[i]) << 8);
        v[i].Green = (COLOR16)(GetGValue(cols[i]) << 8);
        v[i].Blue  = (COLOR16)(GetBValue(cols[i])  << 8);
        v[i].Alpha = 0x0000;
    }

    GRADIENT_TRIANGLE tris[2] = {
        { 0, 1, 2 },
        { 0, 2, 3 }
    };

    GradientFill(hdc, v, 4, tris, 2, GRADIENT_FILL_TRIANGLE);
}

void DrawIntroAnimation(HDC hdc, RECT rc) {
    int cx = (rc.right - rc.left) / 2;
    int cy = (rc.bottom - rc.top) / 2 - 10;

    float t = (float)introFrame / (float)INTRO_MAX_FRAMES;
    COLORREF bg = isDarkTheme ? RGB(24, 24, 28) : RGB(242, 242, 247);

    // Resta al 100% di opacità fino a t = 0.82, poi sfuma nell'ultimo 0.18
    float globalAlpha = (t < 0.82f) ? 1.0f : (1.0f - (t - 0.82f) / 0.18f);
    if (globalAlpha < 0.0f) globalAlpha = 0.0f;

    #define BLEND_BG(r, g, b) RGB( \
        (int)(GetRValue(bg) + ((r) - GetRValue(bg)) * globalAlpha), \
        (int)(GetGValue(bg) + ((g) - GetGValue(bg)) * globalAlpha), \
        (int)(GetBValue(bg) + ((b) - GetBValue(bg)) * globalAlpha))

    // FASE 1: Pallini che esplodono e si riuniscono entro t = 0.52
    if (t < 0.52f) {
        float pT = t / 0.52f;
        float spread = sinf(pT * 3.14159265f);
        float easeSpread = 1.0f - (1.0f - spread) * (1.0f - spread);

        int numDots = 12;
        HGDIOBJ oldPen = SelectObject(hdc, GetStockObject(NULL_PEN));

        for (int i = 0; i < numDots; i++) {
            float angle = (6.2831853f / numDots) * i + pT * 2.2f;
            float dist = easeSpread * (55.0f + (i % 3) * 18.0f);

            int dx = cx + (int)(cosf(angle) * dist);
            int dy = cy + (int)(sinf(angle) * dist);
            int dotR = (int)(5.0f + 4.0f * (1.0f - easeSpread));

            COLORREF col = (i % 2 == 0) ? BLEND_BG(0, 140, 255) : BLEND_BG(52, 199, 110);
            HBRUSH br = CreateSolidBrush(col);
            HGDIOBJ oldB = SelectObject(hdc, br);
            Ellipse(hdc, dx - dotR, dy - dotR, dx + dotR, dy + dotR);
            SelectObject(hdc, oldB);
            DeleteObject(br);
        }
        SelectObject(hdc, oldPen);
    }

    // FASE 2 & 3: Il cubo si forma entro t = 0.58 e resta FISSO fino a t = 0.82 (~0.75 secondi)
    if (t > 0.28f) {
        float cT = (t - 0.28f) / 0.30f; // Raggiunge 1.0 esattamente a t = 0.58
        if (cT > 1.0f) cT = 1.0f;

        float easeScale = (cT < 0.5f) ? (2.0f * cT * cT) : (1.0f - powf(-2.0f * cT + 2.0f, 2.0f) / 2.0f);
        if (easeScale > 1.0f) easeScale = 1.0f;

        float S = 48.0f * easeScale;
        float dx = S * 0.8660254f;
        float dy = S * 0.5f;

        POINT pCenter = { cx, cy };
        POINT pTop    = { cx, cy - (int)S };
        POINT pTopR   = { cx + (int)dx, cy - (int)dy };
        POINT pBotR   = { cx + (int)dx, cy + (int)dy };
        POINT pBot    = { cx, cy + (int)S };
        POINT pBotL   = { cx - (int)dx, cy + (int)dy };
        POINT pTopL   = { cx - (int)dx, cy - (int)dy };

        // Faccia superiore: sfumatura verde smeraldo -> ciano
        DrawGradientQuad(hdc,
            pTop,    BLEND_BG(80, 230, 120),
            pTopR,   BLEND_BG(40, 205, 165),
            pCenter, BLEND_BG(30, 185, 185),
            pTopL,   BLEND_BG(52, 215, 130)
        );

        // Faccia sinistra: sfumatura verde-acqua in alto -> blu elettrico in basso
        DrawGradientQuad(hdc,
            pTopL,   BLEND_BG(45, 200, 150),
            pCenter, BLEND_BG(25, 175, 210),
            pBot,    BLEND_BG(0, 110, 255),
            pBotL,   BLEND_BG(10, 135, 240)
        );

        // Faccia destra: sfumatura ciano-verde in alto -> blu profondo in basso
        DrawGradientQuad(hdc,
            pCenter, BLEND_BG(25, 175, 210),
            pTopR,   BLEND_BG(35, 190, 175),
            pBotR,   BLEND_BG(15, 95, 205),
            pBot,    BLEND_BG(0, 110, 255)
        );
    }

    #undef BLEND_BG

    SetBkMode(hdc, TRANSPARENT);
    COLORREF txtBase = isDarkTheme ? RGB(255, 255, 255) : RGB(28, 28, 30);
    int tr = (int)(GetRValue(bg) + (GetRValue(txtBase) - GetRValue(bg)) * globalAlpha);
    int tg = (int)(GetGValue(bg) + (GetGValue(txtBase) - GetGValue(bg)) * globalAlpha);
    int tb = (int)(GetBValue(bg) + (GetBValue(txtBase) - GetBValue(bg)) * globalAlpha);
    SetTextColor(hdc, RGB(tr, tg, tb));

    RECT rcTxt = { 0, cy + 62, rc.right, cy + 92 };
    DrawTextA(hdc, "PortableMC", -1, &rcTxt, DT_CENTER | DT_SINGLELINE);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            RebuildVersionList();

            hFontTitle = CreateFontA(22, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hFontMain = CreateFontA(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hFontBtn = CreateFontA(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hFontSmall = CreateFontA(13, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
            hFontLog = CreateFontA(13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, FIXED_PITCH, "Consolas");

            hTitle = CreateWindowA("STATIC", "PortableMC",
                WS_VISIBLE | WS_CHILD | SS_LEFT, 25, 18, 125, 28, hwnd, NULL, NULL, NULL);
            SendMessage(hTitle, WM_SETFONT, (WPARAM)hFontTitle, TRUE);

          hBtnTheme = CreateWindowA("BUTTON", "Light Theme",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 365, 18, 125, 28, hwnd, (HMENU)ID_COMBO_THEME, NULL, NULL);
          SendMessage(hBtnTheme, WM_SETFONT, (WPARAM)hFontMain, TRUE);

          hBtnAuth = CreateWindowA("BUTTON", "Username",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 25, 62, 140, 26, hwnd, (HMENU)ID_COMBO_AUTH, NULL, NULL);
          SendMessage(hBtnAuth, WM_SETFONT, (WPARAM)hFontMain, TRUE);

          hEditUser = CreateWindowA("EDIT", "",
              WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 175, 62, 315, 26, hwnd, (HMENU)ID_EDIT_USER, NULL, NULL);
          SendMessage(hEditUser, WM_SETFONT, (WPARAM)hFontMain, TRUE);
          SendMessageW(hEditUser, EM_SETCUEBANNER, TRUE, (LPARAM)L"Enter offline username...");

          hBtnLogin = CreateWindowA("BUTTON", "Login",
              WS_CHILD | BS_OWNERDRAW, 415, 62, 75, 26, hwnd, (HMENU)ID_BTN_LOGIN, NULL, NULL);
          SendMessage(hBtnLogin, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

          lblLoader = CreateWindowA("STATIC", "Mod Loader",
              WS_VISIBLE | WS_CHILD, 25, 104, 140, 24, hwnd, NULL, NULL, NULL);
          SendMessage(lblLoader, WM_SETFONT, (WPARAM)hFontMain, TRUE);

          hBtnLoader = CreateWindowA("BUTTON", "Vanilla",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 175, 102, 315, 26, hwnd, (HMENU)ID_COMBO_LOADER, NULL, NULL);
          SendMessage(hBtnLoader, WM_SETFONT, (WPARAM)hFontMain, TRUE);

          lblVer = CreateWindowA("STATIC", "Version",
              WS_VISIBLE | WS_CHILD, 25, 144, 140, 24, hwnd, NULL, NULL, NULL);
          SendMessage(lblVer, WM_SETFONT, (WPARAM)hFontMain, TRUE);

          hBtnVersion = CreateWindowA("BUTTON", selectedVersion,
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 175, 142, 165, 26, hwnd, (HMENU)ID_COMBO_VERSION, NULL, NULL);
          SendMessage(hBtnVersion, WM_SETFONT, (WPARAM)hFontMain, TRUE);

          hBtnSnap = CreateWindowA("BUTTON", "Snapshot",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 346, 142, 76, 26, hwnd, (HMENU)ID_BTN_TOGGLE_SNAP, NULL, NULL);
          SendMessage(hBtnSnap, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

          hBtnBeta = CreateWindowA("BUTTON", "Beta",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 428, 142, 62, 26, hwnd, (HMENU)ID_BTN_TOGGLE_BETA, NULL, NULL);
          SendMessage(hBtnBeta, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

          lblDir = CreateWindowA("STATIC", "Game folder",
              WS_VISIBLE | WS_CHILD, 25, 184, 140, 24, hwnd, NULL, NULL, NULL);
          SendMessage(lblDir, WM_SETFONT, (WPARAM)hFontMain, TRUE);

          char* defaultDir = (char*)calloc(MAX_PATH, 1);
          if (FAILED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, defaultDir))) {
              GetCurrentDirectoryA(MAX_PATH, defaultDir);
          }
          strcat(defaultDir, "\\.minecraft");

          hEditDir = CreateWindowA("EDIT", defaultDir,
              WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL, 175, 182, 147, 26, hwnd, (HMENU)ID_EDIT_DIR, NULL, NULL);
          SendMessage(hEditDir, WM_SETFONT, (WPARAM)hFontMain, TRUE);
          free(defaultDir);

          btnBrowse = CreateWindowA("BUTTON", "Browse",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 327, 182, 52, 26, hwnd, (HMENU)ID_BTN_BROWSE_DIR, NULL, NULL);
          SendMessage(btnBrowse, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

          btnSkin = CreateWindowA("BUTTON", "+ Skin",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 384, 182, 50, 26, hwnd, (HMENU)ID_BTN_IMPORT_SKIN, NULL, NULL);
          SendMessage(btnSkin, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

          btnCape = CreateWindowA("BUTTON", "+ Cape",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 439, 182, 51, 26, hwnd, (HMENU)ID_BTN_IMPORT_CAPE, NULL, NULL);
          SendMessage(btnCape, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

	  btnMod = CreateWindowA("BUTTON", "+ Mod",
      	      WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 25, 220, 62, 26, hwnd, (HMENU)ID_BTN_IMPORT_MOD, NULL, NULL);
	  SendMessage(btnMod, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

	  btnSearchMods = CreateWindowA("BUTTON", "Search Mods",
   	      WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 92, 220, 96, 26, hwnd, (HMENU)ID_BTN_SEARCH_MODS, NULL, NULL);
	  SendMessage(btnSearchMods, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

	  btnBoostFps = CreateWindowA("BUTTON", "Boost FPS",
	      WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 193, 220, 82, 26, hwnd, (HMENU)ID_BTN_BOOST_FPS, NULL, NULL);
	  SendMessage(btnBoostFps, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

	  hBtnIsolated = CreateWindowA("BUTTON", "Isolated",
 	      WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 280, 220, 68, 26, hwnd, (HMENU)ID_BTN_ISOLATED, NULL, NULL);
	  SendMessage(hBtnIsolated, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

	  hBtnAutoClose = CreateWindowA("BUTTON", "Auto-Close",
  	      WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 353, 220, 137, 26, hwnd, (HMENU)ID_BTN_AUTOCLOSE, NULL, NULL);
	  SendMessage(hBtnAutoClose, WM_SETFONT, (WPARAM)hFontSmall, TRUE);

          hEditLog = CreateWindowA("EDIT", "Ready.\r\n",
              WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
              25, 258, 465, 118, hwnd, (HMENU)ID_EDIT_LOG, NULL, NULL);
          SendMessage(hEditLog, WM_SETFONT, (WPARAM)hFontLog, TRUE);

          hBtnLaunch = CreateWindowA("BUTTON", "LAUNCH MINECRAFT",
              WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, 25, 388, 465, 44, hwnd, (HMENU)ID_BTN_LAUNCH, NULL, NULL);
          SendMessage(hBtnLaunch, WM_SETFONT, (WPARAM)hFontBtn, TRUE);

            origBtnProc = (WNDPROC)SetWindowLongPtrA(hBtnLaunch, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(btnSearchMods, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(btnBoostFps, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(hBtnIsolated, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(hBtnAutoClose, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
	    LoadConfigWin();
            SetWindowLongPtrA(hBtnLogin, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(btnMod, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(btnBrowse, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(hBtnTheme, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(btnSkin, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(btnCape, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(hBtnAuth, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(hBtnLoader, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(hBtnVersion, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(hBtnSnap, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);
            SetWindowLongPtrA(hBtnBeta, GWLP_WNDPROC, (LONG_PTR)RoundedCtrlProc);

            origEditProc = (WNDPROC)SetWindowLongPtrA(hEditUser, GWLP_WNDPROC, (LONG_PTR)RoundedEditProc);
            SetWindowLongPtrA(hEditDir, GWLP_WNDPROC, (LONG_PTR)RoundedEditProc);
            SetWindowLongPtrA(hEditLog, GWLP_WNDPROC, (LONG_PTR)RoundedEditProc);

            UpdateThemeBrushes(hwnd);
            EnumChildWindows(hwnd, SetChildVisibleProc, SW_HIDE);
            SetTimer(hwnd, IDT_INTRO_ANIM, 33, NULL);
            break;
        }

        case WM_TIMER: {
            if (wParam == IDT_INTRO_ANIM) {
                introFrame++;
                if (introFrame >= INTRO_MAX_FRAMES) {
                    KillTimer(hwnd, IDT_INTRO_ANIM);
                    showIntro = FALSE;
                    EnumChildWindows(hwnd, SetChildVisibleProc, SW_SHOW);
                    UpdateAuthUI();
                    InvalidateRect(hwnd, NULL, TRUE);
                } else {
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            break;
        }

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            if (showIntro) {
                RECT rc;
                GetClientRect(hwnd, &rc);
                HDC memDC = CreateCompatibleDC(hdc);
                HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
                HGDIOBJ oldBmp = SelectObject(memDC, memBmp);

                SendMessage(hwnd, WM_ERASEBKGND, (WPARAM)memDC, 0);
                DrawIntroAnimation(memDC, rc);

                BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);
                SelectObject(memDC, oldBmp);
                DeleteObject(memBmp);
                DeleteDC(memDC);
            } else {
                RECT rc;
                GetClientRect(hwnd, &rc);
                SendMessage(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0);

                COLORREF dotCol = RGB(52, 199, 89);
                if (appStatus == 1) dotCol = RGB(255, 149, 0);
                else if (appStatus == 2) dotCol = RGB(255, 59, 48);

                HBRUSH br = CreateSolidBrush(dotCol);
                HPEN pn = CreatePen(PS_SOLID, 1, dotCol);
                HGDIOBJ oldB = SelectObject(hdc, br);
                HGDIOBJ oldP = SelectObject(hdc, pn);

                Ellipse(hdc, 152, 26, 162, 36);

                SelectObject(hdc, oldB);
                SelectObject(hdc, oldP);
                DeleteObject(br);
                DeleteObject(pn);
            }
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DRAWITEM: {
            LPDRAWITEMSTRUCT dis = (LPDRAWITEMSTRUCT)lParam;
            if (dis->CtlType == ODT_BUTTON) {
                HDC hdc = dis->hDC;
                RECT rc = dis->rcItem;

                POINT pt = { 0, 0 };
                MapWindowPoints(dis->hwndItem, hwnd, &pt, 1);
                SetViewportOrgEx(hdc, -pt.x, -pt.y, NULL);
                SendMessage(hwnd, WM_ERASEBKGND, (WPARAM)hdc, 0);
                SetViewportOrgEx(hdc, 0, 0, NULL);

                UINT id = dis->CtlID;
                BOOL isPrimary = (id == ID_BTN_LAUNCH);
                BOOL isDropdown = (id == ID_COMBO_THEME || id == ID_COMBO_AUTH || id == ID_COMBO_LOADER || id == ID_COMBO_VERSION);
                BOOL isToggleActive = (id == ID_BTN_TOGGLE_SNAP && showSnapshots) ||
                      (id == ID_BTN_TOGGLE_BETA && showBetas) ||
                      (id == ID_BTN_ISOLATED && isIsolated) ||
                      (id == ID_BTN_AUTOCLOSE && isAutoClose);
                BOOL isDisabled = (dis->itemState & ODS_DISABLED);
                BOOL isPressed = (dis->itemState & ODS_SELECTED);
                BOOL isHovered = (dis->hwndItem == hHoveredBtn);

                int radius = isPrimary ? 20 : 13;

                if (isPrimary && isRunningCmd) {
                    COLORREF grayTrack = isDarkTheme ? RGB(72, 72, 76) : RGB(195, 198, 205);
                    COLORREF appleGreen = RGB(52, 199, 89);

                    HRGN hRgn = CreateRoundRectRgn(rc.left, rc.top, rc.right + 1, rc.bottom + 1, radius, radius);
                    SelectClipRgn(hdc, hRgn);

                    HBRUSH hBrushGray = CreateSolidBrush(grayTrack);
                    FillRect(hdc, &rc, hBrushGray);
                    DeleteObject(hBrushGray);

                    if (isDownloading) {
                        int fillWidth = ((rc.right - rc.left) * downloadPercent) / 100;
                        if (fillWidth > 0) {
                            RECT rcFill = rc;
                            rcFill.right = rc.left + fillWidth;
                            HBRUSH hBrushGreen = CreateSolidBrush(appleGreen);
                            FillRect(hdc, &rcFill, hBrushGreen);
                            DeleteObject(hBrushGreen);
                        }
                    }

                    SelectClipRgn(hdc, NULL);
                    DeleteObject(hRgn);

                    char progText[64];
                    if (isDownloading) {
                        snprintf(progText, sizeof(progText), "DOWNLOADING... %d%%  (Cancel)", downloadPercent);
                    } else {
                        snprintf(progText, sizeof(progText), "LAUNCHING...  (Cancel)");
                    }

                    SetBkMode(hdc, TRANSPARENT);
                    SetTextColor(hdc, RGB(255, 255, 255));
                    DrawTextA(hdc, progText, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                    return TRUE;
                }

                COLORREF bgBtn, fgText, borderCol;

                if (isPrimary || isToggleActive) {
                    if (isDisabled) bgBtn = GetSubTextColor();
                    else if (isPressed || isHovered) bgBtn = GetAccentHoverColor();
                    else bgBtn = GetAccentColor();
                    fgText = RGB(255, 255, 255);
                    borderCol = bgBtn;
                } else if (isDropdown) {
                    bgBtn = (isPressed || isHovered) ? GetCardHoverColor() : GetCardBgColor();
                    fgText = GetTextColorCustom();
                    borderCol = GetBorderColor();
                } else {
                    if (isDisabled) bgBtn = GetBorderColor();
                    else if (isPressed || isHovered) bgBtn = GetSecBtnHoverColor();
                    else bgBtn = GetSecBtnColor();
                    fgText = GetTextColorCustom();
                    borderCol = bgBtn;
                }

                HBRUSH hBrushBtn = CreateSolidBrush(bgBtn);
                HPEN hPenBtn = CreatePen(PS_SOLID, 1, borderCol);
                HGDIOBJ oldBrush = SelectObject(hdc, hBrushBtn);
                HGDIOBJ oldPen = SelectObject(hdc, hPenBtn);

                RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, radius, radius);

                SelectObject(hdc, oldBrush);
                SelectObject(hdc, oldPen);
                DeleteObject(hBrushBtn);
                DeleteObject(hPenBtn);

                char text[128];
                GetWindowTextA(dis->hwndItem, text, sizeof(text));
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, fgText);

                if (isDropdown) {
                    RECT rcText = rc;
                    rcText.left += 10;
                    rcText.right -= 22;
                    DrawTextA(hdc, text, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                    DrawDownArrow(hdc, rc.right - 13, (rc.top + rc.bottom) / 2, GetArrowColor());
                } else {
                    DrawTextA(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                }

                return TRUE;
            }
            break;
        }

        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hwnd, &rc);

            COLORREF cTop = GetGradTopColor();
            COLORREF cBot = GetGradBottomColor();

            TRIVERTEX vertex[2];
            vertex[0].x     = 0;
            vertex[0].y     = 0;
            vertex[0].Red   = GetRValue(cTop) << 8;
            vertex[0].Green = GetGValue(cTop) << 8;
            vertex[0].Blue  = GetBValue(cTop) << 8;
            vertex[0].Alpha = 0x0000;

            vertex[1].x     = rc.right;
            vertex[1].y     = rc.bottom;
            vertex[1].Red   = GetRValue(cBot) << 8;
            vertex[1].Green = GetGValue(cBot) << 8;
            vertex[1].Blue  = GetBValue(cBot) << 8;
            vertex[1].Alpha = 0x0000;

            GRADIENT_RECT gRect = { 0, 1 };
            GradientFill(hdc, vertex, 2, &gRect, 1, GRADIENT_FILL_RECT_V);
            return 1;
        }

        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            HWND hCtrl = (HWND)lParam;
            if (hCtrl == hEditLog) {
                SetTextColor(hdc, GetTextColorCustom());
                SetBkColor(hdc, GetCardBgColor());
                return (LRESULT)hBrushInput;
            }
            SetTextColor(hdc, (hCtrl == hTitle) ? GetTextColorCustom() : GetSubTextColor());
            SetBkMode(hdc, TRANSPARENT);
            return (LRESULT)GetStockObject(NULL_BRUSH);
        }

        case WM_CTLCOLOREDIT: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, GetTextColorCustom());
            SetBkColor(hdc, GetCardBgColor());
            return (LRESULT)hBrushInput;
        }

        case WM_APPEND_LOG: {
            char* text = (char*)lParam;
            if (text) {
                int len = GetWindowTextLengthA(hEditLog);
                SendMessageA(hEditLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
                SendMessageA(hEditLog, EM_REPLACESEL, FALSE, (LPARAM)text);
                free(text);
            }
            break;
        }

        case WM_CMD_FINISHED: {
            isRunningCmd = FALSE;
            isDownloading = FALSE;
            downloadPercent = 0;
            if (appStatus != 2) appStatus = 0;
            EnableWindow(hBtnLaunch, TRUE);
            EnableWindow(hBtnLogin, TRUE);
            InvalidateRect(hBtnLaunch, NULL, FALSE);
            InvalidateRect(hBtnLogin, NULL, FALSE);
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);

            if (wmId >= ID_MENU_BASE && wmId < ID_MENU_BASE + 10) {
                int choice = wmId - ID_MENU_BASE;
                if (choice == 0 || choice == 1) {
                    isDarkTheme = choice;
                    SetWindowTextA(hBtnTheme, isDarkTheme ? "Dark Theme" : "Light Theme");
                    UpdateThemeBrushes(hwnd);
                }
            }
            else if (wmId >= ID_MENU_BASE + 10 && wmId < ID_MENU_BASE + 20) {
                authMode = wmId - (ID_MENU_BASE + 10);
                UpdateAuthUI();
            }
            else if (wmId >= ID_MENU_BASE + 20 && wmId < ID_MENU_BASE + 30) {
                loaderIdx = wmId - (ID_MENU_BASE + 20);
                SetWindowTextA(hBtnLoader, LOADERS[loaderIdx]);
                InvalidateRect(hBtnLoader, NULL, FALSE);
            }
            else if (wmId >= ID_MENU_BASE + 100 && wmId < ID_MENU_BASE + 100 + activeVersionCount) {
                int vIdx = wmId - (ID_MENU_BASE + 100);
                strcpy(selectedVersion, activeVersions[vIdx]);
                SetWindowTextA(hBtnVersion, selectedVersion);
                InvalidateRect(hBtnVersion, NULL, FALSE);
            }
            else if (wmId == ID_COMBO_THEME) {
                const char* themes[] = { "Light Theme", "Dark Theme" };
                ShowPopupDropdown(hwnd, hBtnTheme, themes, 2, ID_MENU_BASE);
            }
            else if (wmId == ID_BTN_IMPORT_SKIN) {
                ImportSkinOrCape(hwnd, FALSE);
            }
            else if (wmId == ID_BTN_IMPORT_CAPE) {
                ImportSkinOrCape(hwnd, TRUE);
            }
            else if (wmId == ID_BTN_SEARCH_MODS) {
                RunSearchMods(hwnd);
            }
            else if (wmId == ID_BTN_BOOST_FPS) {
                RunBoostFps(hwnd);
            }
            else if (wmId == ID_BTN_ISOLATED) {
                isIsolated = !isIsolated;
                InvalidateRect(hBtnIsolated, NULL, FALSE);
            }
            else if (wmId == ID_BTN_AUTOCLOSE) {
                isAutoClose = !isAutoClose;
                InvalidateRect(hBtnAutoClose, NULL, FALSE);
            }

            else if (wmId == ID_COMBO_AUTH) {
                const char* auths[] = { "Username", "Microsoft" };
                ShowPopupDropdown(hwnd, hBtnAuth, auths, 2, ID_MENU_BASE + 10);
            }
            else if (wmId == ID_COMBO_LOADER) {
                ShowPopupDropdown(hwnd, hBtnLoader, LOADERS, 5, ID_MENU_BASE + 20);
            }
            else if (wmId == ID_COMBO_VERSION) {
                RebuildVersionList();
                RECT rc;
                GetWindowRect(hBtnVersion, &rc);
                HMENU hMenu = CreatePopupMenu();
                for (int i = 0; i < activeVersionCount; i++) {
                    UINT flags = MF_STRING | ((i > 0 && i % 28 == 0) ? MF_MENUBARBREAK : 0);
                    AppendMenuA(hMenu, flags, ID_MENU_BASE + 100 + i, activeVersions[i]);
                }
                TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_TOPALIGN, rc.left, rc.bottom + 2, 0, hwnd, NULL);
                DestroyMenu(hMenu);
            }
            else if (wmId == ID_BTN_TOGGLE_SNAP) {
                showSnapshots = !showSnapshots;
                RebuildVersionList();
                InvalidateRect(hBtnSnap, NULL, FALSE);
            }
            else if (wmId == ID_BTN_TOGGLE_BETA) {
                showBetas = !showBetas;
                RebuildVersionList();
                InvalidateRect(hBtnBeta, NULL, FALSE);
            }
            else if (wmId == ID_BTN_BROWSE_DIR) {
                BrowseFolder(hwnd);
            }
            else if (wmId == ID_BTN_IMPORT_MOD) {
                ImportModJar(hwnd);
            }
            else if (wmId == ID_BTN_LOGIN) {
                char* gameDir = (char*)calloc(MAX_PATH, 1);
                char* cmdArgs = (char*)calloc(1024, 1);
 
                GetWindowTextA(hEditDir, gameDir, MAX_PATH);
                size_t dirLen = strlen(gameDir);
                while (dirLen > 0 && (gameDir[dirLen - 1] == '\\' || gameDir[dirLen - 1] == '/')) {
                    gameDir[--dirLen] = '\0';
                }

                if (dirLen > 0) {
                    CreateDirectories(gameDir);
                    snprintf(cmdArgs, 1024, "--main-dir \"%s\" auth login --no-browser", gameDir);
                } else {
                    snprintf(cmdArgs, 1024, "auth login --no-browser");
                }

                ShellExecuteA(NULL, "open", "https://www.microsoft.com/link", NULL, NULL, SW_SHOWNORMAL);
                RunCommandInBackground(hwnd, cmdArgs);

                free(gameDir);
                free(cmdArgs);
            }

            else if (wmId == ID_BTN_LAUNCH) {
                if (isRunningCmd) {
                    BOOL wasDownloading = isDownloading;

                    if (hActiveProcess != NULL) {
                        TerminateProcess(hActiveProcess, 1);
                        WaitForSingleObject(hActiveProcess, 2000);
                        hActiveProcess = NULL;
                    }
                    appStatus = 2;
                    CleanAndSendLog(hwnd, "\n[CANCELLED] Operation aborted by user.\n");

                    if (wasDownloading) {
                        char gameDir[MAX_PATH];
                        GetWindowTextA(hEditDir, gameDir, MAX_PATH);
                        size_t dLen = strlen(gameDir);
                        while (dLen > 0 && (gameDir[dLen - 1] == '\\' || gameDir[dLen - 1] == '/')) {
                            gameDir[--dLen] = '\0';
                        }

                        char msg[MAX_PATH + 256];
                        snprintf(msg, sizeof(msg),
                            "If you don't want to keep partial files, you can manually delete the generated folders (assets, libraries, versions, runtime) inside:\n%s",
                            dLen > 0 ? gameDir : "your game folder");

                        ShowCustomAlert(hwnd, "Download Cancelled", msg, dLen > 0 ? gameDir : NULL);
                    }
                    return 0;
                }

                char* user = (char*)calloc(256, 1);
                char* gameDir = (char*)calloc(MAX_PATH, 1);
                char* target = (char*)calloc(256, 1);
                char* fullArgs = (char*)calloc(4096, 1);

                const char* loader = LOADERS[loaderIdx];

                GetWindowTextA(hEditUser, user, 256);
                GetEffectiveGameDir(gameDir, MAX_PATH);
                size_t dirLen = strlen(gameDir);
                while (dirLen > 0 && (gameDir[dirLen - 1] == '\\' || gameDir[dirLen - 1] == '/')) {
                    gameDir[--dirLen] = '\0';
                }

                if (strcmp(loader, "Vanilla") == 0) {
                    snprintf(target, 256, "%s", selectedVersion);
                } else {
                    snprintf(target, 256, "%s:%s", loader, selectedVersion);
                }

                if (dirLen > 0) {
                    CreateDirectories(gameDir);
                    snprintf(fullArgs, 4096, "--main-dir \"%s\" start %s", gameDir, target);
                } else {
                    snprintf(fullArgs, 4096, "start %s", target);
                }

                if (authMode == 1) {
                    strcat(fullArgs, " -a");
                    if (strlen(user) > 0) {
                        char* userFlag = (char*)calloc(320, 1);
                        snprintf(userFlag, 320, " -u \"%s\"", user);
                        strcat(fullArgs, userFlag);
                        free(userFlag);
                    }
                } else {
                    if (strlen(user) > 0) {
                        char* userFlag = (char*)calloc(320, 1);
                        snprintf(userFlag, 320, " --username \"%s\"", user);
                        strcat(fullArgs, userFlag);
                        free(userFlag);
                    }
                }

                SaveConfigWin();
                RunCommandInBackground(hwnd, fullArgs);

                free(user);
                free(gameDir);
                free(target);
                free(fullArgs);
            }
            break;
        }
        case WM_DESTROY:
            if (hBrushInput) DeleteObject(hBrushInput);
            DeleteObject(hFontMain);
            DeleteObject(hFontTitle);
            DeleteObject(hFontLog);
            DeleteObject(hFontBtn);
            DeleteObject(hFontSmall);
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    const char* CLASS_NAME = "PortableMCAppleGradientGUI";

    HICON hIconBig = (HICON)LoadImageA(hInstance, MAKEINTRESOURCEA(1), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
    HICON hIconSmall = (HICON)LoadImageA(hInstance, MAKEINTRESOURCEA(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    WNDCLASSA wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = hIconBig;

    RegisterClassA(&wc);

    HWND hwnd = CreateWindowExA(
        WS_EX_APPWINDOW, CLASS_NAME, "PortableMC",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 525, 495,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    if (hIconBig)   SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
    if (hIconSmall) SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CoUninitialize();
    return 0;
}
