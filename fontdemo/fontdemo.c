#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <commctrl.h>

typedef struct PaddedControl
{
    WNDPROC baseWndProc;
    RECT fullClientRect;
} PaddedControl;

static HINSTANCE instance;
static HWND mainWindow;
static HWND buttonSF;
static HWND textBoxSF;
static HWND buttonMF;
static HWND textBoxMF;
static HWND buttonMFC;
static HWND textBoxMFC;
static PaddedControl textBoxMFCPadded;

#define WC_mainWindow L"W32CtlTestDemo"

static NONCLIENTMETRICSW ncm;
static HFONT messageFont;
static TEXTMETRICW messageFontMetrics;
static int controlHeightSF;
static int controlHeightMF;
static int buttonWidthSF;
static int buttonWidthMF;

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
    return (int) ulpActivationCookie;
}

static void init(void)
{
    INITCOMMONCONTROLSEX icx;
    icx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icx.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icx);
    ncm.cbSize = sizeof(ncm);
    SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0);
    messageFont = CreateFontIndirectW(&ncm.lfStatusFont);

    LONG sysDbu = GetDialogBaseUnits();
    HDC dc = GetDC(0);
    SelectObject(dc, (HGDIOBJ) messageFont);
    GetTextMetricsW(dc, &messageFontMetrics);
    SIZE sampleSize;
    GetTextExtentExPointW(dc,
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
            52, 0, 0, 0, &sampleSize);
    ReleaseDC(0, dc);
    controlHeightSF = MulDiv(HIWORD(sysDbu), 14, 8);
    controlHeightMF = MulDiv(messageFontMetrics.tmHeight, 14, 8);
    buttonWidthSF = MulDiv(LOWORD(sysDbu), 50, 4);
    buttonWidthMF = MulDiv(sampleSize.cx, 50, 4 * 52);
    instance = GetModuleHandleW(0);
}

static LRESULT CALLBACK paddedControlProc(
        HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    PaddedControl *self = (PaddedControl *)GetPropW(w, L"paddedControl");
    WNDCLASSEXW wc;

    switch (msg)
    {
    case WM_ERASEBKGND:
        wc.cbSize = sizeof(wc);
        GetClassInfoExW(0, L"Edit", &wc);
        HDC dc = GetDC(w);
        FillRect(dc, &self->fullClientRect, wc.hbrBackground);
        ReleaseDC(w, dc);
        return 1;

    case WM_NCCALCSIZE:
        if (!wp) break;
        LRESULT result = CallWindowProc(self->baseWndProc, w, msg, wp, lp);
        NCCALCSIZE_PARAMS *p = (NCCALCSIZE_PARAMS *)lp;
        int height = p->rgrc[0].bottom - p->rgrc[0].top;
        if (height > messageFontMetrics.tmHeight + 3)
        {
            memcpy(&self->fullClientRect, &(p->rgrc[0]), sizeof(RECT));
            MapWindowPoints(GetParent(w), w,
                    (LPPOINT) &self->fullClientRect, 2);
            int offset = (height - messageFontMetrics.tmHeight - 3) / 2;
            p->rgrc[0].top += offset;
            p->rgrc[0].bottom -= offset;
            self->fullClientRect.top -= offset;
            self->fullClientRect.bottom -= offset;
        }
        return result;
    }

    return CallWindowProc(self->baseWndProc, w, msg, wp, lp);
}

static LRESULT CALLBACK wproc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        buttonSF = CreateWindowExW(0, L"Button", L"sysfont",
                WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                4, 4, buttonWidthSF, controlHeightSF,
                w, 0, instance, 0);

        buttonMF = CreateWindowExW(0, L"Button", L"msgfont",
                WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                4, 8 + controlHeightSF, buttonWidthMF, controlHeightMF,
                w, 0, instance, 0);
        SendMessageW(buttonMF, WM_SETFONT, (WPARAM)messageFont, 0);

        buttonMFC = CreateWindowExW(0, L"Button", L"msgfont adj",
                WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                4, 12 + controlHeightSF + controlHeightMF,
                buttonWidthMF, controlHeightMF,
                w, 0, instance, 0);
        SendMessageW(buttonMFC, WM_SETFONT, (WPARAM)messageFont, 0);

        textBoxSF = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", L"abcdefgh",
                WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
                8 + buttonWidthSF, 4, 100, controlHeightSF,
                w, 0, instance, 0);

        textBoxMF = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", L"abcdefgh",
                WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
                8 + buttonWidthMF, 8 + controlHeightSF,
                100, controlHeightMF,
                w, 0, instance, 0);
        SendMessageW(textBoxMF, WM_SETFONT, (WPARAM)messageFont, 0);

        textBoxMFC = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", L"abcdefgh",
                WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
                8 + buttonWidthMF, 12 + controlHeightSF + controlHeightMF,
                100, controlHeightMF,
                w, 0, instance, 0);
        memset(&textBoxMFCPadded, 0, sizeof(PaddedControl));
        textBoxMFCPadded.baseWndProc = (WNDPROC)SetWindowLongPtr(
                textBoxMFC, GWLP_WNDPROC, (LONG_PTR)paddedControlProc);
        SetPropW(textBoxMFC, L"paddedControl", &textBoxMFCPadded);
        SetWindowPos(textBoxMFC, 0, 0, 0, 0, 0,
                SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_FRAMECHANGED);
        SendMessageW(textBoxMFC, WM_SETFONT, (WPARAM)messageFont, 0);

        break;

    case WM_DESTROY:
        PostQuitMessage(0);
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

    mainWindow = CreateWindowExW(0, WC_mainWindow, L"fontdemo",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 240, 180,
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
