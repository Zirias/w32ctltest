#include <string.h>

#include <windows.h>
#include <commctrl.h>

static HINSTANCE instance;
static HWND mainWindow;
static HWND button;
static HWND textBox;

#define WC_mainWindow L"W32CtlTestDemo"
#define CID_button 0x101

static NONCLIENTMETRICSW ncm;
static HFONT messageFont;
static int buttonWidth;
static int buttonHeight;
static int textBoxWidth;
static int textBoxHeight;

/* hack to enable visual styles without relying on manifest
 * found at http://stackoverflow.com/a/10444161
 * modified for unicode-only code */
static int enableVisualStyles(void)
{
    wchar_t dir[MAX_PATH];
    ULONG_PTR ulpActivationCookie = 0;
    ACTCTXW actCtx =
    {
        sizeof(actCtx),
        ACTCTX_FLAG_RESOURCE_NAME_VALID
            | ACTCTX_FLAG_SET_PROCESS_DEFAULT
            | ACTCTX_FLAG_ASSEMBLY_DIRECTORY_VALID,
        L"shell32.dll", 0, 0, dir, (LPWSTR)124,
        0, 0
    };
    UINT cch = GetSystemDirectoryW(dir, sizeof(dir) / sizeof(*dir));
    if (cch >= sizeof(dir) / sizeof(*dir)) { return 0; }
    dir[cch] = L'\0';
    ActivateActCtx(CreateActCtxW(&actCtx), &ulpActivationCookie);
    INITCOMMONCONTROLSEX icx;
    icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icx.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icx);
    return (int) ulpActivationCookie;
}

static void init(void)
{
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
    messageFont = CreateFontIndirectW(&ncm.lfMessageFont);
    HDC dc = GetDC(0);
    SelectObject(dc, (HGDIOBJ) messageFont);
    SIZE sampleSize;
    GetTextExtentExPointW(dc,
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
            52, 0, 0, 0, &sampleSize);
    ReleaseDC(0, dc);
    buttonWidth = MulDiv(sampleSize.cx, 50, 4 * 52);
    buttonHeight = MulDiv(sampleSize.cy, 14, 8);
    textBoxWidth = 100;
    textBoxHeight = MulDiv(sampleSize.cy, 14, 8);
    instance = GetModuleHandleW(0);
}

static LRESULT CALLBACK wproc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        button = CreateWindowExW(0, L"Button", L"test",
                WS_CHILD|WS_VISIBLE|SS_CENTER,
                2, 2, buttonWidth, buttonHeight,
                w, (HMENU)CID_button, instance, 0);

        textBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", L"",
                WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
                6 + buttonWidth, 2, textBoxWidth, textBoxHeight,
                w, 0, instance, 0);
	break;

    case WM_DESTROY:
	PostQuitMessage(0);
	break;

    case WM_COMMAND:
	switch (LOWORD(wp))
	{
	case CID_button:
	    DestroyWindow(w);
	    break;
	}
	break;

    }

    return DefWindowProcW(w, msg, wp, lp);
}

int main(int argc, char **argv)
{
    if (argc < 2 || strcmp(argv[1], "-s"))
    {
        enableVisualStyles();
    }
    
    init();

    WNDCLASSEXW wc;
    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(wc);
    wc.hInstance = instance;
    wc.lpszClassName = WC_mainWindow;
    wc.lpfnWndProc = wproc;
    wc.hbrBackground = (HBRUSH) COLOR_WINDOW;
    wc.hCursor = LoadCursorA(0, IDC_ARROW);
    RegisterClassExW(&wc);

    mainWindow = CreateWindowExW(0, WC_mainWindow, L"winapi",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 100,
            0, 0, instance, 0);
    ShowWindow(mainWindow, SW_SHOWNORMAL);

    MSG msg;
    while (GetMessageW(&msg, 0, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
