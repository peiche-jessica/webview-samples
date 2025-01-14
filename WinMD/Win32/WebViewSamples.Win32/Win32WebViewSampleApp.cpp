#include "stdafx.h"
#include "Win32WebViewSampleApp.h"
#include <iostream>
#include <Shellapi.h>
#include <string.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Web::UI;
using namespace ABI::Windows::Web::UI::Interop;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    return App::RunNewInstance(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}

int App::Run(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    m_cmdline = lpCmdLine;
    m_nShow = nCmdShow;
    UNREFERENCED_PARAMETER(hPrevInstance);
    std::wstring initialUri(L"https://www.bing.com");

    if (lpCmdLine && lpCmdLine[0])
    {
        bool commandLineError = false;

        LPWSTR nextParam = lpCmdLine;

        if (nextParam[0] == L'-')
        {
            ++nextParam;
            if (nextParam[0] == L'-')
            {
                ++nextParam;
            }
            if (_wcsnicmp(nextParam, L"dpiunaware", ARRAYSIZE(L"dpiunaware") - 1) == 0)
            {
                SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE);
            }
            else if (_wcsnicmp(nextParam, L"dpisystemaware", ARRAYSIZE(L"dpisystemaware") - 1) == 0)
            {
                SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);
            }
            else if (_wcsnicmp(nextParam, L"dpipermonitorawarev2", ARRAYSIZE(L"dpipermonitorawarev2") - 1) == 0)
            {
                SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            }
            else if (_wcsnicmp(nextParam, L"dpipermonitoraware", ARRAYSIZE(L"dpipermonitoraware") - 1) == 0)
            {
                SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
            }
            else if (_wcsnicmp(nextParam, L"initialUri=",
                ARRAYSIZE(L"initialUri=") - 1) == 0)
            {
                PWSTR uriStart = nextParam + ARRAYSIZE(L"initialUri=") - 1;
                size_t len = 0;
                while (uriStart[len] && (uriStart[len] != ' '))
                    ++len;
                initialUri = std::wstring(uriStart, len);
            }
            else
            {
                commandLineError = true;
            }
        }
        else
        {
            commandLineError = true;
        }
        

        if (commandLineError)
        {
            MessageBox(nullptr, L"Valid command line parameters:\n\t-DPIUnaware\n\t-DPISystemAware\n\t-DPIPerMonitorAware\n\t-DPIPerMonitorAwareV2", L"Command Line Parameters", MB_OK);
        }
    }
    
    m_initialUri = initialUri;

    LoadStringW(hInstance, IDC_WIN32WEBVIEWSAMPLEAPP, m_windowClass, s_maxLoadString);

    MyRegisterClasses(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32WEBVIEWSAMPLEAPP));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
void App::MyRegisterClasses(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProcStatic;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32WEBVIEWSAMPLEAPP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WIN32WEBVIEWSAMPLEAPP);
    wcex.lpszClassName  = m_windowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProcStatic;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = m_hostClass;
    wcex.hIconSm = nullptr;

    RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL App::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   m_hInst = hInstance; // Store instance handle in our global variable

   WCHAR szTitle[s_maxLoadString];                  // The title bar text
   LoadStringW(hInstance, IDS_APP_TITLE, szTitle, s_maxLoadString);
   m_mainWindow = CreateWindowW(m_windowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   SetWindowLongPtr(m_mainWindow, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

   RECT hwndBounds = { 0 };
   GetClientRect(m_mainWindow, &hwndBounds);

   m_backWindow = CreateWindow(L"button", L"Back", WS_CHILD | WS_VISIBLE | WS_BORDER, 0, 0, s_backWidth, s_addressbarHeight, m_mainWindow, (HMENU)IDE_BACK, nullptr, 0);
   m_addressbarWindow = CreateWindow(L"edit", nullptr, WS_CHILD | WS_VISIBLE | WS_BORDER, s_backWidth, 0, (hwndBounds.right - hwndBounds.left) - (s_addressbarGoWidth + s_backWidth), s_addressbarHeight, m_mainWindow, (HMENU)IDE_ADDRESSBAR, nullptr, 0);
   m_addressbarGoWindow = CreateWindow(L"button", L"Go", WS_CHILD | WS_VISIBLE | WS_BORDER, (hwndBounds.right - hwndBounds.left) - s_addressbarGoWidth, 0, s_addressbarGoWidth, s_addressbarHeight, m_mainWindow, (HMENU)IDE_ADDRESSBAR_GO, nullptr, 0);
   m_hostWindow = CreateWindowW(
       m_hostClass, L"", (WS_CHILD | WS_VISIBLE),
       0, s_addressbarHeight, hwndBounds.right - hwndBounds.left, (hwndBounds.bottom - hwndBounds.top) - s_addressbarHeight,
       m_mainWindow, (HMENU)45, nullptr, 0);

   ShowWindow(m_mainWindow, nCmdShow);
   UpdateWindow(m_mainWindow);

   ShowWindow(m_hostWindow, SW_SHOWNA);
   UpdateWindow(m_hostWindow);

   PostMessage(m_mainWindow, s_initializeWindowMessage, 0, 0);

   return TRUE;
}

LRESULT CALLBACK App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, _Out_ bool* handled)
{
    if (m_hostWindow != nullptr && m_webViewControl != nullptr)
    {
        switch (message)
        {
        case WM_DESTROY:
        {
            ComPtr<IWebViewControlSite> webViewControlSite;

            m_webViewControl->QueryInterface(IID_PPV_ARGS(&webViewControlSite));
            webViewControlSite->Close();
        }
        break;

        case WM_SIZE:
        {
            RECT hwndBounds = { 0 };
            GetClientRect(hWnd, &hwndBounds);
            const int clientWidth = hwndBounds.right - hwndBounds.left;
            const int clientHeight = hwndBounds.bottom - hwndBounds.top;

            SetWindowPos(m_backWindow, nullptr, 0, 0, s_backWidth, s_addressbarHeight, SWP_NOZORDER);
            SetWindowPos(m_addressbarWindow, nullptr, s_backWidth, 0, clientWidth - s_addressbarGoWidth - s_backWidth, s_addressbarHeight, SWP_NOZORDER);
            SetWindowPos(m_addressbarGoWindow, nullptr, clientWidth - s_addressbarGoWidth, 0, s_addressbarGoWidth, s_addressbarHeight, SWP_NOZORDER);
            SetWindowPos(m_hostWindow, nullptr, 0, s_addressbarHeight, clientWidth, clientHeight - s_addressbarHeight, SWP_NOZORDER);

            ResizeWebView();
            *handled = true;
        }
        break;

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDM_ZOOM_05:
                SetScale(0.5);
                *handled = true;
                break;
            case IDM_ZOOM_10:
                SetScale(1.0);
                *handled = true;
                break;
            case IDM_ZOOM_20:
                SetScale(2.0);
                *handled = true;
                break;
            case IDM_SAVE_SCREENSHOT:
                SaveScreenshot();
                *handled = true;
                break;

            case IDM_NEW_WINDOW:
            {
                App::RunNewInstance(m_hInst, nullptr, m_cmdline, m_nShow, m_process.Get(), m_processOptions.Get());
            }
            *handled = true;
            break;

            case IDM_PROCESS_INFO:
            {
                UINT32 processId = 0;
                boolean privateNetwork = false;
                HString enterpriseId;
                m_process->get_ProcessId(&processId);
                m_process->get_IsPrivateNetworkClientServerCapabilityEnabled(&privateNetwork);
                m_process->get_EnterpriseId(enterpriseId.GetAddressOf());

                WCHAR buffer[4096] = L"";
                StringCchPrintf(buffer, ARRAYSIZE(buffer), L"Process ID: %u\nPrivateNetwork enabled: %s\nEnterprise ID: %s",
                    processId, privateNetwork ? L"yes" : L"no", enterpriseId.GetRawBuffer(nullptr));
                MessageBox(m_mainWindow, buffer, L"Process Info", MB_OK);
                *handled = true;
            }
                break;

            case IDM_NEW_PROCESS:
            {
                App::RunNewInstance(m_hInst, nullptr, m_cmdline, m_nShow);
                *handled = true;
            }
                break;

            case IDM_NEW_PROCESS_PRIVATE_NETWORK:
            {
                App::RunNewInstance(m_hInst, nullptr, m_cmdline, m_nShow, nullptr, CreatePrivateNetworkProcessOptions().Get());
                *handled = true;
            }
                break;

            case IDM_CRASH_PROCESS:
            {
                UINT32 processId = 0;
                m_process->get_ProcessId(&processId);
                HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
                // Creating a remote thread that starts executing at address 0 will crash the process.
                if (!CreateRemoteThread(process, nullptr, 0, nullptr, nullptr, 0, nullptr))
                {
                    WCHAR message[1024] = L"";
                    StringCchPrintfW(message, ARRAYSIZE(message), L"Unable to crash process: %u", GetLastError());
                    MessageBox(m_mainWindow, message, L"Error", MB_OK);
                }

                CloseHandle(process);
                *handled = true;
            }
                break;

            case IDM_REINIT:
                InitializeWin32WebView(true);
                *handled = true;
                break;

            case IDM_REINIT_SYNC:
                InitializeWin32WebView(false);
                *handled = true;
                break;

            case IDM_TOGGLE_VISIBILITY:
                ToggleVisibility();
                *handled = true;
                break;

            case IDE_ADDRESSBAR_GO:
                {
                    WCHAR addressbarText[2048] = L"";
                    switch (HIWORD(wParam))
                    {
                    case BN_CLICKED:
                        GetWindowText(m_addressbarWindow, addressbarText, ARRAYSIZE(addressbarText));
                        NavigateToUri(addressbarText);
                        break;
                    }
                }
                break;

            case IDE_BACK:
                {
                    m_webViewControl->GoBack();
                }
                break;
            }
        }
        break;

        }
    }

    if (!*handled)
    {
        switch (message)
        {
        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(m_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                *handled = true;
                break;
            case IDM_OPEN_WEBSITE:
                ShellExecute(m_hostWindow, L"open", L"https://github.com/rjmurillo/webview-samples", nullptr, nullptr, SW_SHOW);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                *handled = true;
                break;
            }
        }
        break;

        case s_initializeWindowMessage:
            InvalidateRect(m_hostWindow, nullptr, TRUE);
            InitializeWin32WebView();
            *handled = true;
            break;
        }
    }

    return 0;
}

LRESULT CALLBACK App::WndProcStatic(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    bool handled = false;

    App* app = reinterpret_cast<App*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    if (app != nullptr)
    {
        result = app->WndProc(hWnd, message, wParam, lParam, &handled);
    }

    if (!handled)
    {
        result = 0;
        switch (message)
        {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
            handled = true;
        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
            handled = true;
            break;

        }
    }

    if (!handled)
    {
        result = DefWindowProc(hWnd, message, wParam, lParam);
    }

    return result;
}


// Message handler for about box.
INT_PTR CALLBACK App::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
