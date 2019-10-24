#include "stdafx.h"
#include <Commdlg.h>
#include "IAsyncOperationHelper.h"
#include <iostream>
#include <stdio.h>
#include <string.h>

// high resolution clock code
#include <ctime>
#include <ratio>
#include <chrono>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Storage::Streams;
using namespace ABI::Windows::Web::UI;
using namespace ABI::Windows::Web::UI::Interop;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

using namespace std::chrono;

void CheckFailure(_In_ HRESULT hr)
{
    if (FAILED(hr))
    {
        WCHAR message[512] = L"";
        StringCchPrintf(message, ARRAYSIZE(message), L"Error: 0x%x", hr);
        MessageBoxW(nullptr, message, nullptr, MB_OK);
        ExitProcess(-1);
    }
}

template <typename TInterface>
Microsoft::WRL::ComPtr<TInterface> GetActivationFactoryFailFast(_In_z_ PCWSTR factoryClassName)
{
    ComPtr<TInterface> factoryInstance;
    CheckFailure(RoGetActivationFactory(
        HStringReference(factoryClassName).Get(),
        IID_PPV_ARGS(&factoryInstance)));
    return factoryInstance;
}

template <typename TInterface>
Microsoft::WRL::ComPtr<TInterface> ActivateInstanceFailFast(_In_z_ PCWSTR className)
{
    ComPtr<TInterface> classInstanceAsInspectable;
    ComPtr<TInterface> classInstance;
    CheckFailure(RoActivateInstance(
        HStringReference(className).Get(),
        &classInstanceAsInspectable));
    CheckFailure(classInstanceAsInspectable.As(&classInstance));
    return classInstance;
}

ComPtr<IUriRuntimeClass> CreateWinRtUri(_In_z_ PCWSTR uri, _In_ bool allowInvalidUri = false)
{
    auto uriRuntimeClassFactory = GetActivationFactoryFailFast<IUriRuntimeClassFactory>(RuntimeClass_Windows_Foundation_Uri);
    ComPtr<IUriRuntimeClass> uriRuntimeClass;
    if (!allowInvalidUri)
    {
        CheckFailure(uriRuntimeClassFactory->CreateUri(HStringReference(uri).Get(), &uriRuntimeClass));
    }
    else
    {
        uriRuntimeClassFactory->CreateUri(HStringReference(uri).Get(), &uriRuntimeClass);
    }
    return uriRuntimeClass;
}

Rect HwndWindowRectToBoundsRect(_In_ HWND hwnd)
{
    RECT windowRect = { 0 };
    GetWindowRect(hwnd, &windowRect);

    Rect bounds =
    {
        0,
        0,
        static_cast<float>(windowRect.right - windowRect.left),
        static_cast<float>(windowRect.bottom - windowRect.top)
    };

    return bounds;
}

ComPtr<IWebViewControlProcessOptions> App::CreatePrivateNetworkProcessOptions()
{
    ComPtr<IWebViewControlProcessOptions> processOptions = ActivateInstanceFailFast<IWebViewControlProcessOptions>(RuntimeClass_Windows_Web_UI_Interop_WebViewControlProcessOptions);
    processOptions->put_PrivateNetworkClientServerCapability(WebViewControlProcessCapabilityState_Enabled);

    return processOptions;
}

void App::ConfigureAddressBar()
{
    EventRegistrationToken token = { 0 };
    HRESULT hr = m_webViewControl->add_ContentLoading(Callback<ITypedEventHandler<IWebViewControl*, WebViewControlContentLoadingEventArgs*>>(
        [this](IWebViewControl* webViewControl, IWebViewControlContentLoadingEventArgs* args) -> HRESULT
    {
        ComPtr<IUriRuntimeClass> uri;
        CheckFailure(args->get_Uri(&uri));
        HString uriAsHString;
        CheckFailure(uri->get_AbsoluteUri(uriAsHString.ReleaseAndGetAddressOf()));
        SetWindowText(m_addressbarWindow, uriAsHString.GetRawBuffer(nullptr));
        return S_OK;
    }).Get(), &token);
    CheckFailure(hr);

}

void App::InitializeWin32WebView(bool async)
{
    high_resolution_clock::time_point before_webview_creation = high_resolution_clock::now();

    // Use default options if options weren't set on the App during App::RunNewInstance
    if (!m_processOptions)
    {
        m_processOptions = ActivateInstanceFailFast<IWebViewControlProcessOptions>(RuntimeClass_Windows_Web_UI_Interop_WebViewControlProcessOptions);
    }

    // Use a default new process if one wasn't set on the App during App::RunNewInstance
    if (!m_process)
    {
        ComPtr<IWebViewControlProcessFactory> webViewControlProcessFactory = GetActivationFactoryFailFast<IWebViewControlProcessFactory>(RuntimeClass_Windows_Web_UI_Interop_WebViewControlProcess);
        CheckFailure(webViewControlProcessFactory->CreateWithOptions(m_processOptions.Get(), &m_process));
    }

    ComPtr<IAsyncOperation<WebViewControl*>> createWebViewAsyncOperation;
    CheckFailure(m_process->CreateWebViewControlAsync(
        reinterpret_cast<INT64>(m_hostWindow),
        HwndWindowRectToBoundsRect(m_hostWindow),
        &createWebViewAsyncOperation));

    if (async) {
        HRESULT hr = createWebViewAsyncOperation->put_Completed(Callback<IAsyncOperationCompletedHandler<WebViewControl*>>([this, createWebViewAsyncOperation, before_webview_creation](IAsyncOperation<WebViewControl*>*, AsyncStatus status) -> HRESULT
        {
            high_resolution_clock::time_point after_webview_creation = high_resolution_clock::now();

            duration<double> webview_creation = duration_cast<duration<double>>(after_webview_creation - before_webview_creation);
            std::cout << "\"" << webview_creation.count() << "\"";

            CheckFailure(createWebViewAsyncOperation->GetResults(&m_webViewControl));
            ConfigureAddressBar();
            NavigateToUri(m_initialUri);

            return S_OK;
        }).Get());
        CheckFailure(hr);
    }
    else
    {
        CheckFailure(AsyncOpHelpers::WaitForCompletionAndGetResults(createWebViewAsyncOperation.Get(), m_webViewControl.ReleaseAndGetAddressOf()));
        ConfigureAddressBar();
        NavigateToUri(m_initialUri);
    }
}

void App::SetScale(_In_ double scale)
{
    ComPtr<IWebViewControlSite> site;
    CheckFailure(m_webViewControl.As(&site));
    CheckFailure(site->put_Scale(scale));
}

void App::ResizeWebView()
{
    Rect bounds = HwndWindowRectToBoundsRect(m_hostWindow);
    ComPtr<IWebViewControlSite> site;
    CheckFailure(m_webViewControl.As(&site));
    CheckFailure(site->put_Bounds(bounds));
}

void App::SaveScreenshot()
{
    ComPtr<IAsyncAction> capturePreviewToStreamAsyncAction;
    ComPtr<IRandomAccessStream> stream = ActivateInstanceFailFast<IRandomAccessStream>(RuntimeClass_Windows_Storage_Streams_InMemoryRandomAccessStream);
    CheckFailure(m_webViewControl->CapturePreviewToStreamAsync(stream.Get(), &capturePreviewToStreamAsyncAction));
    HRESULT hr = capturePreviewToStreamAsyncAction->put_Completed((Callback<IAsyncActionCompletedHandler>([this, stream](IAsyncAction*, AsyncStatus status) -> HRESULT
    {
        OPENFILENAME openFileName = {};
        openFileName.lStructSize = sizeof(openFileName);
        openFileName.hwndOwner = m_hostWindow;
        openFileName.hInstance = m_hInst;
        WCHAR fileName[MAX_PATH] = L"screenshot.png";
        openFileName.lpstrFile = fileName;
        openFileName.nMaxFile = ARRAYSIZE(fileName);

        if (GetSaveFileName(&openFileName))
        {
            ComPtr<IDataReaderFactory> dataReaderFactory = GetActivationFactoryFailFast<IDataReaderFactory>(RuntimeClass_Windows_Storage_Streams_DataReader);
            ComPtr<IDataReader> dataReader;
            ComPtr<IInputStream> streamAsInputStream;
            CheckFailure(stream->GetInputStreamAt(0, &streamAsInputStream));
            CheckFailure(dataReaderFactory->CreateDataReader(streamAsInputStream.Get(), &dataReader));

            UINT64 size = 0;
            CheckFailure(stream->get_Size(&size));
            ComPtr<IAsyncOperation<UINT32>> loadAsyncOperation;
            CheckFailure(dataReader->LoadAsync(size, &loadAsyncOperation));
            loadAsyncOperation->put_Completed((Callback<IAsyncOperationCompletedHandler<UINT32>>([this, dataReader, size, fileName](IAsyncOperation<UINT32>*, AsyncStatus status) -> HRESULT
            {
                BYTE* bytes = new BYTE[size];
                for (UINT32 position = 0; position < size; ++position)
                {
                    CheckFailure(dataReader->ReadByte(bytes + position));
                }

                HANDLE file = CreateFile(fileName, GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
                CheckFailure(file == INVALID_HANDLE_VALUE ? E_FAIL : S_OK);
                CheckFailure(WriteFile(file, bytes, size, nullptr, nullptr) ? S_OK : E_FAIL);
                CloseHandle(file);
                file = INVALID_HANDLE_VALUE;

                return S_OK;
            })).Get());
        }
        return S_OK;
    })).Get());
    CheckFailure(hr);
}

void App::ToggleVisibility()
{
    boolean visible = false;
    ComPtr<IWebViewControlSite> site;
    CheckFailure(m_webViewControl.As(&site));
    CheckFailure(site->get_IsVisible(&visible));
    CheckFailure(site->put_IsVisible(!visible));
}

void App::NavigateToUri(_In_ LPCWSTR uriAsString)
{
    ComPtr<IUriRuntimeClass> uri = CreateWinRtUri(uriAsString, true);
    if (uri != nullptr)
    {
        high_resolution_clock::time_point navigation_starting = high_resolution_clock::now();
        CheckFailure(m_webViewControl->Navigate(uri.Get()));
        
        EventRegistrationToken token = { 0 };
        HRESULT hr = m_webViewControl->add_NavigationCompleted(Callback<ITypedEventHandler<IWebViewControl*, WebViewControlNavigationCompletedEventArgs*>>(
            [this, navigation_starting](IWebViewControl* webViewControl, IWebViewControlNavigationCompletedEventArgs* args) -> HRESULT
        {
            high_resolution_clock::time_point navigation_completed = high_resolution_clock::now();
            duration<double> navigation = duration_cast<duration<double>>(navigation_completed - navigation_starting);
            std::cout << ",\"" << navigation.count() << "\"" << std::endl;

            return S_OK;
        }).Get(), &token);
        CheckFailure(hr);
    }
}
