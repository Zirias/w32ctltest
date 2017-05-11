#include <stdio.h>
#include <stdlib.h>
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
static TEXTMETRICW messageFontMetrics;
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
    return (int) ulpActivationCookie;
}

static LRESULT CALLBACK textBoxProc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    WNDPROC defaultProc = (WNDPROC)GetPropW(w, L"defaultProc");
    RECT *fullClientRect;

    switch (msg)
    {
    case WM_ERASEBKGND:
        fullClientRect = (RECT *)GetPropW(w, L"fullClientRect");
        if (!fullClientRect) break;
        puts("custom erase background");
        fflush(stdout);
        WNDCLASSEXW wc;
        wc.cbSize = sizeof(wc);
        GetClassInfoExW(0, L"Edit", &wc);
        HDC dc = GetDC(w);
        FillRect(dc, fullClientRect, wc.hbrBackground);
        ReleaseDC(w, dc);
        return 1;

    case WM_NCCALCSIZE:
        if (!wp) break;
        LRESULT result = CallWindowProc(defaultProc, w, msg, wp, lp);
        NCCALCSIZE_PARAMS *p = (NCCALCSIZE_PARAMS *)lp;
        int height = p->rgrc[0].bottom - p->rgrc[0].top;
        if (height > messageFontMetrics.tmHeight + 2)
        {
            fullClientRect = (RECT *)GetPropW(w, L"fullClientRect");
            if (!fullClientRect)
            {
                puts("creating full client rect");
                fflush(stdout);
                fullClientRect = malloc(sizeof(RECT));
                SetPropW(w, L"fullClientRect", (HANDLE)fullClientRect);
            }
            memcpy(fullClientRect, &(p->rgrc[0]), sizeof(RECT));
            MapWindowPoints(GetParent(w), w, (LPPOINT) fullClientRect, 2);
            int offset = (height - messageFontMetrics.tmHeight - 2) / 2;
            p->rgrc[0].top += offset;
            p->rgrc[0].bottom -= offset;
            fullClientRect->top -= offset;
            fullClientRect->bottom -= offset;
            printf("full client rect: {%d,%d,%d,%d}\n"
                    "adjusted client screen rect: {%d,%d,%d,%d}\n",
                    fullClientRect->left, fullClientRect->top,
                    fullClientRect->right, fullClientRect->bottom,
                    p->rgrc[0].left, p->rgrc[0].top,
                    p->rgrc[0].right, p->rgrc[0].bottom);
            fflush(stdout);
        }
        printf("WM_NCCALCSIZE result: 0x%08x\n", result);
        fflush(stdout);
        return result;
    }

    CallWindowProc(defaultProc, w, msg, wp, lp);
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
    //messageFont = GetStockObject(DEFAULT_GUI_FONT);
    HDC dc = GetDC(0);
    SelectObject(dc, (HGDIOBJ) messageFont);
    GetTextMetricsW(dc, &messageFontMetrics);
    SIZE sampleSize;
    GetTextExtentExPointW(dc,
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
            52, 0, 0, 0, &sampleSize);
    ReleaseDC(0, dc);
    buttonWidth = MulDiv(sampleSize.cx, 50, 4 * 52);
    buttonHeight = MulDiv(messageFontMetrics.tmHeight, 14, 8);
    textBoxWidth = 100;
    textBoxHeight = MulDiv(messageFontMetrics.tmHeight, 14, 8);
    instance = GetModuleHandleW(0);
}

static LRESULT CALLBACK wproc(HWND w, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CREATE:
        button = CreateWindowExW(0, L"Button", L"test",
                WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,
                2, 2, buttonWidth, buttonHeight,
                w, (HMENU)CID_button, instance, 0);
        SendMessageW(button, WM_SETFONT, (WPARAM)messageFont, 0);

        textBox = CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", L"",
                WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,
                6 + buttonWidth, 2, textBoxWidth, textBoxHeight,
                w, 0, instance, 0);
        SetPropW(textBox, L"defaultProc",
                (HANDLE)SetWindowLongPtr(textBox, GWLP_WNDPROC,
                    (LONG_PTR)textBoxProc));
        SetWindowPos(textBox, 0, 0, 0, 0, 0,
                SWP_NOOWNERZORDER|SWP_NOSIZE|SWP_NOMOVE|SWP_FRAMECHANGED);
        SendMessageW(textBox, WM_SETFONT, (WPARAM)messageFont, 0);

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
