#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <clocale>

#include "parser.h"
#include "integrator.h"

// ---------------------------------------------------------------------------
//  UTF-8 <-> UTF-16  conversion helpers
// ---------------------------------------------------------------------------
static std::wstring UTF8ToWide(std::string_view s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring ws(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), &ws[0], len);
    return ws;
}

static std::string WideToUTF8(std::wstring_view ws) {
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), nullptr, 0, nullptr, nullptr);
    std::string s(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.data(), (int)ws.size(), &s[0], len, nullptr, nullptr);
    return s;
}

// ---------------------------------------------------------------------------
//  Control IDs
// ---------------------------------------------------------------------------
enum Ctrl : int {
    ID_EXPR       = 101,
    ID_LOWER      = 102,
    ID_UPPER      = 103,
    ID_METHOD     = 104,
    ID_TOL        = 105,
    ID_COMPUTE    = 106,
    ID_RESULT     = 107,
    ID_EVALS      = 108,
};

// ---------------------------------------------------------------------------
//  Window layout constants  (client area = 520 x 420)
// ---------------------------------------------------------------------------
static constexpr int CX      = 520;
static constexpr int CY      = 420;
static constexpr int MX      = 16;
static constexpr int MY      = 12;
static constexpr int LH      = 22;
static constexpr int EH      = 26;
static constexpr int BH      = 34;
static constexpr int GAP     = 6;
static constexpr int LW      = 120;
static constexpr int EW      = CX - 2 * MX - LW - GAP;

static constexpr int R1 = MY;
static constexpr int R2 = R1 + LH + GAP;
static constexpr int R3 = R2 + LH + GAP;
static constexpr int R4 = R3 + LH + GAP;
static constexpr int R5 = R4 + LH + GAP;
static constexpr int R6 = R5 + LH + GAP + 4;
static constexpr int R7 = R6 + BH + GAP + 8;
static constexpr int R8 = R7 + 10;
static constexpr int R9 = R8 + LH + 2;

// ---------------------------------------------------------------------------
//  Forward declarations
// ---------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void     DoCompute(HWND hWnd);
static void     CentreWindow(HWND hWnd, int cx, int cy);
static void     CreateLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id);
static HWND     CreateEdit(HWND parent, int x, int y, int w, int h, int id);
static HWND     CreateCombo(HWND parent, int x, int y, int w, int h, int id);

// ---------------------------------------------------------------------------
//  Entry point
// ---------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    // Ensure console (if any) and GUI use UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEX wc = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInst;
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"IntegralCalcClass";
    if (!RegisterClassEx(&wc)) return 1;

    HWND hWnd = CreateWindowEx(
        0, L"IntegralCalcClass", L"定积分计算器",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, CX + 16, CY + 39,
        nullptr, nullptr, hInst, nullptr
    );
    if (!hWnd) return 1;

    CentreWindow(hWnd, CX + 16, CY + 39);
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}

// ---------------------------------------------------------------------------
//  Window procedure
// ---------------------------------------------------------------------------
static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
    {
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                 DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                 DEFAULT_PITCH, L"Microsoft YaHei");

        // Row 1: expression
        CreateLabel(hWnd, L"被积函数  f(x) =", MX, R1, LW, LH, 0);
        HWND hExpr = CreateEdit(hWnd, MX + LW + GAP, R1 - 2, EW, EH, ID_EXPR);
        (void)hExpr;

        // Row 2: lower bound
        CreateLabel(hWnd, L"积分下限  a =", MX, R2, LW, LH, 0);
        CreateEdit(hWnd, MX + LW + GAP, R2 - 2, EW, EH, ID_LOWER);

        // Row 3: upper bound
        CreateLabel(hWnd, L"积分上限  b =", MX, R3, LW, LH, 0);
        CreateEdit(hWnd, MX + LW + GAP, R3 - 2, EW, EH, ID_UPPER);

        // Row 4: method combo
        CreateLabel(hWnd, L"计算方法", MX, R4, LW, LH, 0);
        HWND hMethod = CreateCombo(hWnd, MX + LW + GAP, R4 - 2, EW, EH, ID_METHOD);
        ComboBox_AddString(hMethod, L"Adaptive Simpson  (自适应)");
        ComboBox_AddString(hMethod, L"Simpson 1/3  (辛普森)");
        ComboBox_AddString(hMethod, L"Trapezoidal  (梯形法)");
        ComboBox_SetCurSel(hMethod, 0);

        // Row 5: tolerance
        CreateLabel(hWnd, L"精度控制 (容差)", MX, R5, LW, LH, 0);
        CreateEdit(hWnd, MX + LW + GAP, R5 - 2, EW, EH, ID_TOL);
        SetWindowText(GetDlgItem(hWnd, ID_TOL), L"1e-8");

        // Row 6: compute button
        CreateWindowEx(
            0, L"BUTTON", L"计算定积分",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            MX, R6 - 2, CX - 2 * MX, BH,
            hWnd, (HMENU)(INT_PTR)ID_COMPUTE, GetModuleHandle(nullptr), nullptr
        );

        // Row 7: separator line
        CreateWindowEx(
            0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
            MX, R7, CX - 2 * MX, 2,
            hWnd, nullptr, GetModuleHandle(nullptr), nullptr
        );

        // Row 8: result value (bigger bold font)
        CreateLabel(hWnd, L"结果:  (等待计算)", MX, R8, CX - 2 * MX, LH + 4, ID_RESULT);

        // Row 9: eval / method info
        CreateLabel(hWnd, L"", MX, R9, CX - 2 * MX, LH, ID_EVALS);

        // Apply default font to all children
        if (hFont) {
            HWND child = nullptr;
            while ((child = FindWindowEx(hWnd, child, nullptr, nullptr))) {
                SendMessage(child, WM_SETFONT, (WPARAM)hFont, TRUE);
            }
        }

        // Larger bold font for result
        HFONT hResultFont = CreateFont(22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                                       DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                                       DEFAULT_PITCH, L"Microsoft YaHei");
        if (hResultFont) {
            SendMessage(GetDlgItem(hWnd, ID_RESULT), WM_SETFONT, (WPARAM)hResultFont, TRUE);
        }

        SetFocus(hExpr);
        return 0;
    }

    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == ID_COMPUTE) {
            DoCompute(hWnd);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// ---------------------------------------------------------------------------
//  Core: compute the integral and display results
// ---------------------------------------------------------------------------
static void DoCompute(HWND hWnd) {
    wchar_t exprBuf[512] = {};
    wchar_t lowerBuf[64] = {};
    wchar_t upperBuf[64] = {};
    wchar_t tolBuf[64]   = {};

    GetWindowText(GetDlgItem(hWnd, ID_EXPR),  exprBuf, 512);
    GetWindowText(GetDlgItem(hWnd, ID_LOWER), lowerBuf, 64);
    GetWindowText(GetDlgItem(hWnd, ID_UPPER), upperBuf, 64);
    GetWindowText(GetDlgItem(hWnd, ID_TOL),   tolBuf,   64);

    int methodSel = ComboBox_GetCurSel(GetDlgItem(hWnd, ID_METHOD));

    // Convert inputs
    std::string expr  = WideToUTF8(exprBuf);
    double a = wcstod(lowerBuf, nullptr);
    double b = wcstod(upperBuf, nullptr);
    double tol = wcstod(tolBuf, nullptr);
    if (tol <= 0.0) tol = 1e-8;

    IntegrationMethod method;
    switch (methodSel) {
    case 1: method = IntegrationMethod::Simpson;     break;
    case 2: method = IntegrationMethod::Trapezoidal; break;
    default:method = IntegrationMethod::Adaptive;    break;
    }

    // Validate
    if (expr.empty()) {
        SetWindowText(GetDlgItem(hWnd, ID_RESULT), L"错误: 请输入被积函数");
        SetWindowText(GetDlgItem(hWnd, ID_EVALS),  L"");
        return;
    }

    // Compile expression
    MathParser parser;
    if (!parser.compile(expr)) {
        SetWindowText(GetDlgItem(hWnd, ID_RESULT), L"错误: 表达式语法错误");
        SetWindowText(GetDlgItem(hWnd, ID_EVALS),  L"");
        return;
    }

    // Compute
    auto f = [&](double x) { return parser.evaluate(x); };
    IntegrationResult result = integrate(f, a, b, method, 1000, tol);

    // Format results
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(12) << result.result;
    std::string resultStr = "结果:  " + oss.str();

    std::ostringstream oss2;
    oss2 << "计算方法: " << result.method_name
         << "    |    函数求值次数: " << result.evaluations;
    std::string evalStr = oss2.str();

    SetWindowText(GetDlgItem(hWnd, ID_RESULT), UTF8ToWide(resultStr).c_str());
    SetWindowText(GetDlgItem(hWnd, ID_EVALS),  UTF8ToWide(evalStr).c_str());
}

// ---------------------------------------------------------------------------
//  Helpers
// ---------------------------------------------------------------------------
static void CentreWindow(HWND hWnd, int cx, int cy) {
    RECT rc;
    GetWindowRect(GetDesktopWindow(), &rc);
    int x = (rc.right - rc.left - cx) / 2;
    int y = (rc.bottom - rc.top - cy) / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    SetWindowPos(hWnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

static void CreateLabel(HWND parent, const wchar_t* text, int x, int y, int w, int h, int id) {
    CreateWindowEx(0, L"STATIC", text,
                   WS_CHILD | WS_VISIBLE | SS_LEFT,
                   x, y, w, h,
                   parent, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
}

static HWND CreateEdit(HWND parent, int x, int y, int w, int h, int id) {
    return CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        x, y, w, h,
        parent, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
}

static HWND CreateCombo(HWND parent, int x, int y, int w, int h, int id) {
    return CreateWindowEx(
        0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
        x, y, w, h + 120,
        parent, (HMENU)(INT_PTR)id, GetModuleHandle(nullptr), nullptr);
}
