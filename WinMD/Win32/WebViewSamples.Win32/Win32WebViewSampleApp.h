#pragma once

#include <string>
#include "resource.h"

class App
{
public:
    static int RunNewInstance(_In_ HINSTANCE hInstance,
        _In_opt_ HINSTANCE hPrevInstance,
        _In_ LPWSTR    lpCmdLine,
        _In_ int       nCmdShow,
        _In_opt_ ABI::Windows::Web::UI::Interop::IWebViewControlProcess* process = nullptr,
        _In_opt_ ABI::Windows::Web::UI::Interop::IWebViewControlProcessOptions* processOptions = nullptr)
    {
        App* app = new App();
        app->m_process = process;
        app->m_processOptions = processOptions;
        int result = app->Run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
        delete app;
        app = nullptr;
        return result;
    }

protected:
    static LRESULT CALLBACK WndProcStatic(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);
    static INT_PTR CALLBACK About(_In_ HWND hDlg, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam);

    static const size_t s_maxLoadString = 100;
    static const DWORD s_initializeWindowMessage = WM_USER;

    int Run(_In_ HINSTANCE hInstance,
        _In_opt_ HINSTANCE hPrevInstance,
        _In_ LPWSTR    lpCmdLine,
        _In_ int       nCmdShow);
    void MyRegisterClasses(_In_ HINSTANCE hInstance);
    BOOL InitInstance(_In_ HINSTANCE hInstance, _In_ int nCmdShow);
    LRESULT CALLBACK WndProc(_In_ HWND hWnd, _In_ UINT message, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ bool* handled);
    void InitializeWin32WebView(_In_opt_ bool async = true);
    void SetScale(_In_ double scale);
    void ResizeWebView();
    void SaveScreenshot();
    void ToggleVisibility();
    void NavigateToUri(_In_ LPCWSTR uri);
    void ConfigureAddressBar();

    Microsoft::WRL::ComPtr<ABI::Windows::Web::UI::Interop::IWebViewControlProcessOptions> CreatePrivateNetworkProcessOptions();

    HINSTANCE m_hInst = nullptr;                                // current instance
    LPWSTR m_cmdline = nullptr;
    std::wstring m_initialUri = L"https://www.bing.com";
    INT m_nShow = 0;
    WCHAR m_windowClass[s_maxLoadString];            // the main window class name
    WCHAR m_hostClass[s_maxLoadString] = L"Win32WebViewSampleAppHost"; // the host window class name
    HWND m_mainWindow = nullptr;
    HWND m_hostWindow = nullptr; // HWND of the window containing the webview.
    HWND m_backWindow = nullptr;
    HWND m_addressbarWindow = nullptr;
    HWND m_addressbarGoWindow = nullptr;
    static const DWORD s_addressbarHeight = 32;
    static const DWORD s_addressbarGoWidth = 64;
    static const DWORD s_backWidth = 64;

    Microsoft::WRL::ComPtr<ABI::Windows::Web::UI::IWebViewControl> m_webViewControl;
    Microsoft::WRL::ComPtr<ABI::Windows::Web::UI::Interop::IWebViewControlProcess> m_process;
    Microsoft::WRL::ComPtr<ABI::Windows::Web::UI::Interop::IWebViewControlProcessOptions> m_processOptions;
};
