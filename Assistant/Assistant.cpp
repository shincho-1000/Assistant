// Including Libraries
#include <windows.h>
#include <commctrl.h>
#include <d2d1.h>
#include <windowsx.h>
#include <dwrite.h>  
#include <wingdi.h>
#include <sapi.h>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <comutil.h>
#include <thread>
#include <shobjidl.h>
#include <future>
#include "basewin.h"

#pragma comment(lib, "d2d1")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "winmm.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// TypeDefs

using json = nlohmann::json;
using namespace std;

// SafeRelease Function

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

// Converts Strings To WStrings

wstring s2ws(const string& s)
{
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    wstring r(buf);
    delete[] buf;
    return r;
}

// Play Music

bool musicFunc;

int playMusic(LPWSTR file)
{
    wstring filePath;
    wstring w1(L"open \"");
    wstring w2(file);
    wstring w3(L"\" type mpegvideo alias mp3");
    filePath = w1 + w2 + w3;
    const wchar_t* path = filePath.c_str();
    mciSendString(path, NULL, 0, NULL);
    mciSendString(L"play mp3 repeat", NULL, 0, NULL);

    while (musicFunc == true)
    {
        Sleep(1000);
    }

    musicFunc = false;
    mciSendString(L"close mp3", NULL, 0, NULL);

    return true;
}

// Text To Speech

int speakOutput(LPWSTR output)
{
    if (musicFunc == false)
    {
        ISpVoice* pVoice = NULL;

        if (FAILED(::CoInitialize(NULL)))
            return false;

        HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice);
        if (SUCCEEDED(hr))
        {
            hr = pVoice->Speak(output, 0, NULL);

            pVoice->Release();
            pVoice = NULL;
        }
        ::CoUninitialize();
    }

    return true;
}

// Time And Greetings

int checkTime(int data) 
{
    time_t currentTime;
    struct tm localTime;

    time(&currentTime);
    localtime_s(&localTime,&currentTime);

    int hour = localTime.tm_hour;
    int min = localTime.tm_min;
    int sec = localTime.tm_sec;
    int day = localTime.tm_wday;

    if (data == 1) {
        return hour;
    }
    else if (data == 2) {
        return min;
    }
    else if (data == 3) {
        return sec;
    }
    else {
        return day;
    }
}

bool alarmFunc;

int checkAlarms(string no)
{
    ifstream ifs_al("alarms.json");
    json alarms_data = json::parse(ifs_al);

    if (alarms_data[no]["day"].get<string>() == (string)"") { return 0; }

    int hour;
    int min = stoi(alarms_data[no]["min"].get<string>());
    int day;

    if ((alarms_data[no]["day"].get<string>()).find("Sun") != string::npos) { day = 0; }
    else if ((alarms_data[no]["day"].get<string>()).find("Mon") != string::npos) { day = 1; }
    else if ((alarms_data[no]["day"].get<string>()).find("Tue") != string::npos) { day = 2; }
    else if ((alarms_data[no]["day"].get<string>()).find("Wed") != string::npos) { day = 3; }
    else if ((alarms_data[no]["day"].get<string>()).find("Thurs") != string::npos) { day = 4; }
    else if ((alarms_data[no]["day"].get<string>()).find("Fri") != string::npos) { day = 5; }
    else if ((alarms_data[no]["day"].get<string>()).find("Sat") != string::npos) { day = 6; }

    if (alarms_data[no]["AMPM"].get<string>() == (string)"AM")
    {
        hour = stoi(alarms_data[no]["hour"].get<string>());
        if (hour == 12)
        {
            hour = 0;
        }
    }
    else if (alarms_data[no]["AMPM"].get<string>() == (string)"PM")
    {
        hour = stoi(alarms_data[no]["hour"].get<string>()) + 12;
        if (hour == 24)
        {
            hour = 12;
        }
    }
    else
    {

    }

    if ((checkTime(1) == hour) && (checkTime(2) == min) && (checkTime(4) == day))
    {
        string pathMusic = alarms_data[no]["path"].get<string>();

        alarmFunc = true;

        wstring filePath;
        wstring w1(L"open \"");
        wstring w2(s2ws(pathMusic));
        wstring w3(L"\" type mpegvideo alias mp3");
        filePath = w1 + w2 + w3;
        const wchar_t* path = filePath.c_str();

        mciSendString(path, NULL, 0, NULL);
        mciSendString(L"play mp3", NULL, 0, NULL);

        while (alarmFunc == true)
        {
            Sleep(1000);
        }

        alarmFunc = false;
        mciSendString(L"close mp3", NULL, 0, NULL);
    }
    else
    {

    }

    return 0;
}

LPWSTR greeting()
{
    int hour = checkTime(1);
    int min = checkTime(2);

    if (hour >= 0  && hour < 12) {
        return (LPWSTR)L"Good Morning, I am your assistant";
    }
    else if (hour >= 12 && hour <= 16) {
        return (LPWSTR)L"Good Afternoon, I am your assistant";
    }
    else {
        return (LPWSTR)L"Good Evening, I am your assistant";
    }
}

// Creating MainWindow Class

class MainWindow : public BaseWindow<MainWindow>
{
    IDWriteFactory* m_pDWriteFactory;
    IDWriteTextFormat* m_pTextFormat;
    IDWriteTextLayout* m_pTextLayout;
    ID2D1Factory* pFactory;
    ID2D1HwndRenderTarget* pRenderTarget;

    ID2D1SolidColorBrush* pBrush;
    ID2D1SolidColorBrush* pBlackBrush;
    int x;
    int y;

    HWND hwndButton_wiki;
    bool wikiFunc;
    HWND hwndButton_openWeb;
    bool webFunc;
    HWND hwndButton_searchGoogle;
    bool googleFunc;
    HWND hwndButton_playmusic;
    HWND hwndButton_pausemusic;
    thread thread_obj_music;

    HWND hwndInputBox;
    HDC hdc;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;

    HWND hwndEnter;
    int cTextLen;
    PSTR pszMem;

    D2D1_RECT_F result_rect;

    int totalNoOfShortcuts;
    HWND d;
    HWND deleteButtonShortcuts;
    HWND newShortcut;
    bool ifNewShortcuts;

    int totalNoOfAlarms;
    HWND alarmButton;
    HWND dAlarm;
    HWND newAlarm;
    bool ifNewAlarms;

    HWND alarmMin;
    HWND alarmHour;
    HWND alarmAMPM;
    HWND alarmDay;
    HWND alarmMusicPath;
    HWND alarmMusicPathBrowse;
    HWND hwndEnterAlarm;
    HWND hwndCloseAlarm;

    HWND inputName;
    HWND inputPath;
    HWND inputSymbol;
    HWND inputSymbolBrowse;
    HWND hwndEnterShortcut;
    HWND hwndCloseShortcut;
    bool add;

    HCURSOR hCursor_i = LoadCursor(NULL, IDC_IBEAM);
    HCURSOR hCursor_arrow = LoadCursor(NULL, IDC_ARROW);
    HCURSOR hCursor_hand = LoadCursor(NULL, IDC_HAND);
    HFONT buttonFontA = CreateFont(22,0,0,0,FW_NORMAL,false,false,false,DEFAULT_CHARSET,OUT_OUTLINE_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,VARIABLE_PITCH,TEXT("Verdana"));

    json shortcuts_data;
    json alarms_data;

    int     GetShortcutPositions(int axis, int item_no, int totalNoOfShortcuts);
    int     GetAlarmPositions(int axis, int item_no, int totalNoOfAlarms);
    void    CalculateLayout();
    void    DiscardGraphicsResources();
    void    DiscardDeviceIndependentResources();
    void    ShowButtons();
    void    ArrangeShortcuts();
    void    ShowNewShortcut();
    void    ArrangeAlarms();
    void    ShowAlarm();
    void    OnPaint();
    void    Resize();
    void    DestroyBasicButtons();
    void    DestroyShortcuts();
    void    DestroyNewShortcutsButtons();
    string  RemoveJsonFeild(int itemNo, int field);
    string  RemoveJsonFeildAl(string no, string item);
    HRESULT CreateGraphicsResources();
    HRESULT CreateDeviceIndependentResources();

public:

    MainWindow() : pFactory(NULL), pRenderTarget(NULL), pBrush(NULL)
    {
    }

    void checkChangeInTime();
    void startAlarm();
    PCWSTR  ClassName() const { return L"Assistant Window Class"; }
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

// To Calculate Layout Size

void MainWindow::CalculateLayout()
{
    if (pRenderTarget != NULL)
    {
        D2D1_SIZE_F size = pRenderTarget->GetSize();
        x = size.width;
        y = size.height;
        result_rect = D2D1::RectF((x / 10), y / 4, x - (x / 10), y - (y / 5));
    }
}

// Creating And Deleting Resources

HRESULT MainWindow::CreateGraphicsResources()
{
    HRESULT hr = S_OK;
    if (pRenderTarget == NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        hr = pFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &pRenderTarget);

        if (SUCCEEDED(hr))
        {
            const D2D1_COLOR_F color = D2D1::ColorF(0.30, 0.4, 0.44, 1);
            hr = pRenderTarget->CreateSolidColorBrush(color, &pBrush);

            if (SUCCEEDED(hr))
            {
                CalculateLayout();
                ShowButtons();
            }
        }
    }
    return hr;
}

HRESULT MainWindow::CreateDeviceIndependentResources()
{
    static const WCHAR msc_fontName[] = L"Verdana";
    static const FLOAT msc_fontSize = 15;
    HRESULT hr;

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory);

    if (SUCCEEDED(hr))
    {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown**>(&m_pDWriteFactory)
        );
    }
    if (SUCCEEDED(hr))
    {
        const D2D1_COLOR_F color = D2D1::ColorF(0.75, 0.75, 0.75, 1);
        hr = pRenderTarget->CreateSolidColorBrush(color, &pBlackBrush);
        hr = m_pDWriteFactory->CreateTextFormat(
            msc_fontName,
            NULL,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            msc_fontSize,
            L"",
            &m_pTextFormat
        );
    }
    if (SUCCEEDED(hr))
    {
        m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);

        m_pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }

    return hr;
}

void MainWindow::DiscardGraphicsResources()
{
    SafeRelease(&pRenderTarget);
    SafeRelease(&pBrush);
}

void MainWindow::DiscardDeviceIndependentResources()
{
    SafeRelease(&pFactory);
    SafeRelease(&m_pDWriteFactory);
    SafeRelease(&pRenderTarget);
    SafeRelease(&m_pTextFormat);
}

// To Show Buttons

void MainWindow::ShowButtons() 
{
    hwndButton_wiki = CreateWindowEx(
        0,
        L"BUTTON",
        L"Search Wikipedia",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 
        x / 30, 
        y / 40,
        x / 5,
        30,
        m_hwnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE), 
        NULL
    );
    SetClassLongPtr(hwndButton_wiki, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
    SendMessage(hwndButton_wiki, WM_SETFONT, (WPARAM)buttonFontA, true);

    hwndButton_openWeb = CreateWindowEx(
        0,
        L"BUTTON",
        L"Open A Link",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 
        2 * (x / 30) + (2 - 1) * (x / 5),
        y / 40,
        x / 5,
        30,
        m_hwnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE), 
        NULL
    );
    SetClassLongPtr(hwndButton_openWeb, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
    SendMessage(hwndButton_openWeb, WM_SETFONT, (WPARAM)buttonFontA, true);

    hwndButton_searchGoogle = CreateWindowEx(
        0,
        L"BUTTON",
        L"Search Google",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT, 
        3 * (x / 30) + (3 - 1) * (x / 5),
        y / 40,
        x / 5,
        30,
        m_hwnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
        NULL
    );
    SetClassLongPtr(hwndButton_searchGoogle, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
    SendMessage(hwndButton_searchGoogle, WM_SETFONT, (WPARAM)buttonFontA, true);

    if (musicFunc == false)
    {
        hwndButton_playmusic = CreateWindowEx(
            0,
            L"BUTTON",
            L"Play Music",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
            4 * (x / 30) + (4 - 1) * (x / 5),
            y / 40,
            x / 5,
            30,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SetClassLongPtr(hwndButton_playmusic, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
        SendMessage(hwndButton_playmusic, WM_SETFONT, (WPARAM)buttonFontA, true);
    }
    else
    {
        hwndButton_pausemusic = CreateWindowEx(
            0,
            L"BUTTON",
            L"Stop Music",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
            4 * (x / 30) + (4 - 1) * (x / 5),
            y / 40,
            x / 5,
            30,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SetClassLongPtr(hwndButton_pausemusic, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
        SendMessage(hwndButton_pausemusic, WM_SETFONT, (WPARAM)buttonFontA, true);
    }

    hwndInputBox = CreateWindowEx(
        0,
        L"EDIT",
        L"",
        WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        x / 10,
        y - (y / 10),
        x - 2 * (x / 10) - x / 8,
        28,
        m_hwnd,
        NULL, 
        (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
        NULL
    );
    SendMessage(hwndInputBox, WM_SETFONT, (WPARAM)buttonFontA, true);

    hwndEnter = CreateWindowEx(
        0,
        L"BUTTON",
        L"Enter",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
        x / 10 + x - (2 * (x / 10)) - x / 8 + 2.5,
        y - (y / 10),
        (x - (2 * result_rect.left) - (x - 2 * (x / 10) - x / 8) - 2.5),
        28,
        m_hwnd,
        NULL,
        (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
        NULL
    );
    SetClassLongPtr(hwndEnter, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
    SendMessage(hwndEnter, WM_SETFONT, (WPARAM)buttonFontA, true);

    if (ifNewShortcuts == true)
    {
        inputName = CreateWindowEx(
            0,
            L"EDIT",
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            result_rect.left + 30,
            result_rect.top + ((y - (y / 5) - (y / 4)) / 5 * 1) - 14,
            x - (result_rect.left * 2) - 60,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(inputName, WM_SETFONT, (WPARAM)buttonFontA, true);
        Edit_SetCueBannerText(inputName, L"Name Of The Shortcut");

        inputPath = CreateWindowEx(
            0,
            L"EDIT",
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            result_rect.left + 30,
            result_rect.top + ((y - (y / 5) - (y / 4)) / 5 * 2) - 14,
            x - (result_rect.left * 2) - 60,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(inputPath, WM_SETFONT, (WPARAM)buttonFontA, true);
        Edit_SetCueBannerText(inputPath, L"Path/Url");

        inputSymbol = CreateWindowEx(
            0,
            L"EDIT",
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            result_rect.left + 30,
            result_rect.top + ((y - (y / 5) - (y / 4)) / 5 * 3) - 14,
            x - (result_rect.left * 2) - 60 - 100,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(inputSymbol, WM_SETFONT, (WPARAM)buttonFontA, true);
        Edit_SetCueBannerText(inputSymbol, L"Path Of The Symbol");

        inputSymbolBrowse = CreateWindowEx(
            0,
            L"BUTTON",
            L"Browse",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            result_rect.left + 30 + x - (result_rect.left * 2) - 60 - 100,
            result_rect.top + ((y - (y / 5) - (y / 4)) / 5 * 3) - 14,
            100,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(inputSymbolBrowse, WM_SETFONT, (WPARAM)buttonFontA, true);

        hwndEnterShortcut = CreateWindowEx(
            0,
            L"BUTTON",
            L"Submit",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            result_rect.left + 30 + (x - (result_rect.left * 2) - 60) / 3,
            result_rect.top + ((y - (y / 5) - (y / 4)) / 5 * 4) - 14,
            (( x - (result_rect.left * 2) - 60 ) / 3)/2,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SetClassLongPtr(hwndEnterShortcut, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
        SendMessage(hwndEnterShortcut, WM_SETFONT, (WPARAM)buttonFontA, true);

        hwndCloseShortcut = CreateWindowEx(
            0,
            L"BUTTON",
            L"Close",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            result_rect.left + 30 + (x - (result_rect.left * 2) - 60) / 3 + ((x - (result_rect.left * 2) - 60) / 3) / 2,
            result_rect.top + ((y - (y / 5) - (y / 4)) / 5 * 4) - 14,
            ((x - (result_rect.left * 2) - 60) / 3) / 2,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SetClassLongPtr(hwndCloseShortcut, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
        SendMessage(hwndCloseShortcut, WM_SETFONT, (WPARAM)buttonFontA, true);
    }
    else if (ifNewAlarms==true) 
    {
        hwndEnterAlarm = CreateWindowEx(
            0,
            L"BUTTON",
            L"Submit",
            WS_BORDER | WS_CHILD | WS_VISIBLE,
            result_rect.right - 10 - (((x - (result_rect.left * 2) - 60) / 3) / 4) - (((x - (result_rect.left * 2) - 60) / 3) / 4),
            result_rect.bottom - 38,
            ((x - (result_rect.left * 2) - 60) / 3) / 2,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SetClassLongPtr(hwndEnterAlarm, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
        SendMessage(hwndEnterAlarm, WM_SETFONT, (WPARAM)buttonFontA, true);

        alarmHour = CreateWindowEx(
            0,
            L"EDIT",
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_NUMBER,
            result_rect.right - 10 - 80 - 10 - 80,
            result_rect.top + y / 28,
            80,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(alarmHour, WM_SETFONT, (WPARAM)buttonFontA, true);
        Edit_SetCueBannerText(alarmHour, L"Hour");

        alarmMin = CreateWindowEx(
            0,
            L"EDIT",
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_NUMBER,
            result_rect.right - 10 - 80,
            result_rect.top + y / 28,
            80,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(alarmMin, WM_SETFONT, (WPARAM)buttonFontA, true);
        Edit_SetCueBannerText(alarmMin, L"Min");

        alarmDay = CreateWindowEx(
            0,
            WC_COMBOBOX,
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS,
            result_rect.right - 15 - 140,
            result_rect.top + (y / 28) * 2 + 28,
            120,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(alarmDay, WM_SETFONT, (WPARAM)buttonFontA, true);
        ComboBox_SetCueBannerText(alarmDay, L"Day");
        TCHAR A_days[16];
        int k_days;
        TCHAR Days[7][10] =
        {
            TEXT("Sunday"),
            TEXT("Monday"),
            TEXT("Tuesday"),
            TEXT("Wednesday"),
            TEXT("Thursday"),
            TEXT("Friday"),
            TEXT("Saturday")
        };
        memset(&A_days, 0, sizeof(A_days));
        for (k_days = 0; k_days <= 6; k_days += 1)
        {
            wcscpy_s(A_days, sizeof(A_days) / sizeof(TCHAR), (TCHAR*)Days[k_days]);
            SendMessage(alarmDay, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A_days);
        }

        alarmAMPM = CreateWindowEx(
            0,
            WC_COMBOBOX,
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_HASSTRINGS,
            result_rect.right - 15 - 140,
            result_rect.top + (y / 28) * 3 + 28 + 28,
            120,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(alarmAMPM, WM_SETFONT, (WPARAM)buttonFontA, true);
        ComboBox_SetCueBannerText(alarmAMPM, L"AM/PM");
        TCHAR A_ap[16];
        int k_ap;
        TCHAR AMPM[2][3] =
        {
            TEXT("AM"),
            TEXT("PM")
        };
        memset(&A_ap, 0, sizeof(A_ap));
        for (k_ap = 0; k_ap <= 1; k_ap += 1)
        {
            wcscpy_s(A_ap, sizeof(A_ap) / sizeof(TCHAR), (TCHAR*)AMPM[k_ap]);
            SendMessage(alarmAMPM, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A_ap);
        }

        alarmMusicPath = CreateWindowEx(
            0,
            L"EDIT",
            L"",
            WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
            result_rect.right - 15 - 280 - 100,
            result_rect.top + (y / 28) * 4 + 28 + 28 + 28,
            280,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(alarmMusicPath, WM_SETFONT, (WPARAM)buttonFontA, true);
        Edit_SetCueBannerText(alarmMusicPath, L"Path Of The Music");

        alarmMusicPathBrowse = CreateWindowEx(
            0,
            L"BUTTON",
            L"Browse",
            WS_BORDER | WS_CHILD | WS_VISIBLE,
            result_rect.right - 15 - 180 + 80,
            result_rect.top + (y / 28) * 4 + 28 + 28 + 28,
            80,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SetClassLongPtr(alarmMusicPathBrowse, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
        SendMessage(alarmMusicPathBrowse, WM_SETFONT, (WPARAM)buttonFontA, true);

        hwndCloseAlarm = CreateWindowEx(
            0,
            L"BUTTON",
            L"Close",
            WS_BORDER | WS_CHILD | WS_VISIBLE,
            result_rect.left + 10 - (((x - (result_rect.left * 2) - 60) / 3) / 4) + (((x - (result_rect.left * 2) - 60) / 3) / 4),
            result_rect.bottom - 38,
            ((x - (result_rect.left * 2) - 60) / 3) / 2,
            28,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SetClassLongPtr(hwndCloseAlarm, GCL_HCURSOR, (LONG_PTR)hCursor_hand);
        SendMessage(hwndCloseAlarm, WM_SETFONT, (WPARAM)buttonFontA, true);

        ArrangeAlarms();
    }
    else
    {
        alarmButton = CreateWindowEx(
            0,
            L"BUTTON",
            L"",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT | BS_BITMAP,
            result_rect.left + 10,
            result_rect.top + 10,
            40,
            40,
            m_hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL);

        HBITMAP hAlarm = (HBITMAP)LoadImage(NULL, L"clock.bmp", IMAGE_BITMAP, 40, 40, LR_LOADFROMFILE);
        SendMessage(alarmButton, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hAlarm);

        ArrangeShortcuts();
    }
}

// This Fucntion Arranges Shorctuts On The ResultRect

void MainWindow::ArrangeShortcuts()
{
    string checkNull = "";
    if (shortcuts_data["name1"].get<string>().compare(checkNull) == 0) {
        totalNoOfShortcuts = 1;
        add = true;
    }
    else if (shortcuts_data["name2"].get<string>().compare(checkNull) == 0) {
        totalNoOfShortcuts = 2;
        add = true;
    }
    else if (shortcuts_data["name3"].get<string>().compare(checkNull) == 0) {
        totalNoOfShortcuts = 3;
        add = true;
    }
    else if (shortcuts_data["name4"].get<string>().compare(checkNull) == 0) {
        totalNoOfShortcuts = 4;
        add = true;
    }
    else if (shortcuts_data["name5"].get<string>().compare(checkNull) == 0) {
        totalNoOfShortcuts = 5;
        add = true;
    }
    else if (shortcuts_data["name6"].get<string>().compare(checkNull) == 0) {
        totalNoOfShortcuts = 6;
        add = true;
    }
    else {
        add = false;
        totalNoOfShortcuts = 7;
    }
   
    int item_no = 1;

    while (item_no <= totalNoOfShortcuts)
    {
        if (totalNoOfShortcuts >= 7)
        {
            totalNoOfShortcuts = 6;
        }

        d = CreateWindowEx(
            0,
            L"BUTTON",
            L"",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT | BS_BITMAP,
            GetShortcutPositions(1, item_no, totalNoOfShortcuts),
            GetShortcutPositions(2, item_no, totalNoOfShortcuts),
            80,
            80,
            m_hwnd,
            (HMENU)item_no,     // To Check Which Button Is Pushed
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SetClassLongPtr(hwndEnter, GCL_HCURSOR, (LONG_PTR)hCursor_hand);

        deleteButtonShortcuts = CreateWindowEx(
            0,
            L"BUTTON",
            L"",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT | BS_BITMAP,
            GetShortcutPositions(1, item_no, totalNoOfShortcuts) + 40 - 10,
            GetShortcutPositions(2, item_no, totalNoOfShortcuts) + 5 + 80,
            20,
            20,
            m_hwnd,
            (HMENU)(item_no + 10),     // To Check Which Button Is Pushed
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL);

        HBITMAP hDel = (HBITMAP)LoadImage(NULL, L"delete.bmp", IMAGE_BITMAP, 20, 20, LR_LOADFROMFILE);
        SendMessage(deleteButtonShortcuts, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hDel);

        if (item_no == totalNoOfShortcuts && add != false)
        {
            HBITMAP add = (HBITMAP)LoadImage(NULL, L"Add.bmp", IMAGE_BITMAP, 80, 80, LR_LOADFROMFILE);
            SendMessage(d, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)add);
            DestroyWindow(deleteButtonShortcuts);
        }
        else {
            if (item_no == 1) {
                string symbol_path = shortcuts_data["symbol1"].get<string>();
                wstring stemp = s2ws(symbol_path);
                LPCWSTR result = stemp.c_str();
                HBITMAP image1 = (HBITMAP)LoadImage(NULL, result, IMAGE_BITMAP, 80, 80, LR_LOADFROMFILE);
                SendMessage(d, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)image1);
            }
            if (item_no == 2) {
                string symbol_path = shortcuts_data["symbol2"].get<string>();
                wstring stemp = s2ws(symbol_path);
                LPCWSTR result = stemp.c_str();
                HBITMAP image1 = (HBITMAP)LoadImage(NULL, result, IMAGE_BITMAP, 80, 80, LR_LOADFROMFILE);
                SendMessage(d, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)image1);
            }
            if (item_no == 3) {
                string symbol_path = shortcuts_data["symbol3"].get<string>();
                wstring stemp = s2ws(symbol_path);
                LPCWSTR result = stemp.c_str();
                HBITMAP image1 = (HBITMAP)LoadImage(NULL, result, IMAGE_BITMAP, 80, 80, LR_LOADFROMFILE);
                SendMessage(d, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)image1);
            }
            if (item_no == 4) {
                string symbol_path = shortcuts_data["symbol4"].get<string>();
                wstring stemp = s2ws(symbol_path);
                LPCWSTR result = stemp.c_str();
                HBITMAP image1 = (HBITMAP)LoadImage(NULL, result, IMAGE_BITMAP, 80, 80, LR_LOADFROMFILE);
                SendMessage(d, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)image1);
            }
            if (item_no == 5) {
                string symbol_path = shortcuts_data["symbol5"].get<string>();
                wstring stemp = s2ws(symbol_path);
                LPCWSTR result = stemp.c_str();
                HBITMAP image1 = (HBITMAP)LoadImage(NULL, result, IMAGE_BITMAP, 80, 80, LR_LOADFROMFILE);
                SendMessage(d, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)image1);
            }
            if (item_no == 6) {
                string symbol_path = shortcuts_data["symbol6"].get<string>();
                wstring stemp = s2ws(symbol_path);
                LPCWSTR result = stemp.c_str();
                HBITMAP image1 = (HBITMAP)LoadImage(NULL, result, IMAGE_BITMAP, 80, 80, LR_LOADFROMFILE);
                SendMessage(d, BM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)image1);
            }
        }

        item_no = item_no + 1;
    }
}

// This Fucntion Arranges Alarms On The ResultRect

void MainWindow::ArrangeAlarms()
{
    string checkNull = "";
    if (alarms_data["alarm1"]["day"].get<string>().compare(checkNull) == 0) {
        totalNoOfAlarms = 0;
    }
    else if (alarms_data["alarm2"]["day"].get<string>().compare(checkNull) == 0) {
        totalNoOfAlarms = 1;
    }
    else if (alarms_data["alarm3"]["day"].get<string>().compare(checkNull) == 0) {
        totalNoOfAlarms = 2;
    }
    else if (alarms_data["alarm4"]["day"].get<string>().compare(checkNull) == 0) {
        totalNoOfAlarms = 3;
    }
    else {
        totalNoOfAlarms = 4;
    }

    int item_no = 1;

    while (item_no <= totalNoOfAlarms)
    {
        wstring buttonName;

        if (item_no == 1) {
            wstring day = s2ws(alarms_data["alarm1"]["day"].get<string>());
            wstring hour = s2ws(alarms_data["alarm1"]["hour"].get<string>());
            wstring min = s2ws(alarms_data["alarm1"]["min"].get<string>());
            wstring ampm = s2ws(alarms_data["alarm1"]["AMPM"].get<string>()); 
            buttonName = day + (wstring)L"," + hour + (wstring)L":" + min + (wstring)L" " + ampm;
        }
        if (item_no == 2) {
            wstring day = s2ws(alarms_data["alarm2"]["day"].get<string>());
            wstring hour = s2ws(alarms_data["alarm2"]["hour"].get<string>());
            wstring min = s2ws(alarms_data["alarm2"]["min"].get<string>());
            wstring ampm = s2ws(alarms_data["alarm2"]["AMPM"].get<string>());
            buttonName = day + (wstring)L"," + hour + (wstring)L":" + min + (wstring)L" " + ampm;
        }
        if (item_no == 3) {
            wstring day = s2ws(alarms_data["alarm3"]["day"].get<string>());
            wstring hour = s2ws(alarms_data["alarm3"]["hour"].get<string>());
            wstring min = s2ws(alarms_data["alarm3"]["min"].get<string>());
            wstring ampm = s2ws(alarms_data["alarm3"]["AMPM"].get<string>());
            buttonName = day + (wstring)L"," + hour + (wstring)L":" + min + (wstring)L" " + ampm;
        }
        if (item_no == 4) {
            wstring day = s2ws(alarms_data["alarm4"]["day"].get<string>());
            wstring hour = s2ws(alarms_data["alarm4"]["hour"].get<string>());
            wstring min = s2ws(alarms_data["alarm4"]["min"].get<string>());
            wstring ampm = s2ws(alarms_data["alarm4"]["AMPM"].get<string>());
            buttonName = day + (wstring)L"," + hour + (wstring)L":" + min + (wstring)L" " + ampm;
        }

        dAlarm = CreateWindowEx(
            0,
            L"BUTTON",
            (LPWSTR)buttonName.c_str(),
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_FLAT,
            GetAlarmPositions(1, item_no, totalNoOfAlarms),
            GetAlarmPositions(2, item_no, totalNoOfAlarms),
            200,
            28,
            m_hwnd,
            (HMENU)(item_no + 20),     // To Check Which Button Is Pushed
            (HINSTANCE)GetWindowLongPtr(m_hwnd, GWLP_HINSTANCE),
            NULL
        );
        SendMessage(dAlarm, WM_SETFONT, (WPARAM)buttonFontA, true);
        SetClassLongPtr(dAlarm, GCL_HCURSOR, (LONG_PTR)hCursor_hand);

        item_no = item_no + 1;
    }
}

// Function To Show Add Shortcut Details

void MainWindow::ShowNewShortcut()
{
    DestroyBasicButtons();
    DestroyShortcuts();
    DestroyNewShortcutsButtons();
    ifNewShortcuts = true;
    ShowButtons();
}

// Function To Show Alarm Details

void MainWindow::ShowAlarm()
{
    DestroyBasicButtons();
    DestroyShortcuts();
    DestroyNewShortcutsButtons();
    ifNewAlarms = true;
    ShowButtons();
}

// This Function Finds Positions Of Different Shortcuts

int MainWindow::GetShortcutPositions(int axis, int item_no, int totalNoOfShortcuts)
{
    // Using a formula to get the value
    // Margin between buttons = Total empty space (Excluding buttons) / Total no. of buttons + 1

    int heightOfRect = y - (y / 5) - (y / 4);
    int widthOfRect = x - (x / 10) - (x / 10);

    int a = (heightOfRect - (80)) / (2);   // For Y Axis
    int b = (widthOfRect - (80 * totalNoOfShortcuts)) / (totalNoOfShortcuts + 1);   // For X Axis

    if (axis == 1)
    {
        int d = (x / 10) + (b * item_no) + (80 * (item_no - 1));
        return d;
    }
    else
    {
        int f = (y / 4) + (a);
        return f;
    }
}

// This Function Finds Positions Of Different Alarms

int MainWindow::GetAlarmPositions(int axis, int item_no, int totalNoOfAlarms)
{
    // Using a formula to get the value
    // Margin between buttons = Total empty space (Excluding buttons) / Total no. of buttons + 1

    int heightOfRect = y - (y / 5) - (y / 4);
    int widthOfRect = x - (x / 10) - (x / 10);

    if (axis == 1)
    {
        return result_rect.left + (widthOfRect/30);
    }
    else
    {
        int f = (heightOfRect - (28 * totalNoOfAlarms) - 38 ) / (totalNoOfAlarms + 1);
        return result_rect.top + (28 * (item_no - 1)) + (f * (item_no));
    }
}

// On Paint

void MainWindow::OnPaint()
{
    HRESULT hr = CreateGraphicsResources();
    
    if (SUCCEEDED(hr))
    {
        PAINTSTRUCT ps;
        BeginPaint(m_hwnd, &ps);

        pRenderTarget->BeginDraw();

        pRenderTarget->Clear(D2D1::ColorF(0.15, 0.2, 0.22, 1));
        pRenderTarget->FillRectangle(result_rect, pBrush);

        hr = pRenderTarget->EndDraw();
        if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET)
        {
            DiscardGraphicsResources();
        }

        EndPaint(m_hwnd, &ps);
    }

    HRESULT hr_text = CreateDeviceIndependentResources();

    if (SUCCEEDED(hr_text))
    {
        wstring timeNow;
        int hour = checkTime(1);
        int min = checkTime(2);
        wstring hourStr;
        wstring minStr;
        wstring suffix;
        
        if (hour > 12) {
            hourStr = to_wstring(hour - 12);
            suffix = L" PM";
        }
        else {
            if (hour == 0) {
                hourStr = to_wstring(12);
            }
            else {
                hourStr = to_wstring(hour);
            }
            suffix = L" AM";
        }

        if (checkTime(2) < 10) {
            minStr = L"0" + to_wstring(checkTime(2));   
        }
        else {
            minStr = to_wstring(checkTime(2));
        }

        timeNow = hourStr + L":" + minStr + suffix;
        const WCHAR* timeWchar = timeNow.c_str();

        pRenderTarget->BeginDraw();

        pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

        D2D1_RECT_F text_rect = D2D1::RectF(10, y - 55, 80, y - 15);

        pRenderTarget->DrawText(
            timeWchar,
            9,
            m_pTextFormat,
            text_rect,
            pBlackBrush
        );

        hr_text = pRenderTarget->EndDraw();

        if (hr_text == D2DERR_RECREATE_TARGET)
        {
            hr_text = S_OK;
            DiscardDeviceIndependentResources();
        }
    }
}

// Change Positions When Resize

void MainWindow::Resize()
{
    if (pRenderTarget != NULL)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pRenderTarget->Resize(size);
        CalculateLayout();
        DestroyBasicButtons();
        DestroyShortcuts();
        DestroyNewShortcutsButtons();
        ShowButtons();
        InvalidateRect(m_hwnd, NULL, false);
    }
}

// To Remove A Json Field From Shortcuts

string MainWindow::RemoveJsonFeild(int itemNo, int field)
{
    string returnVal;

    if (totalNoOfShortcuts == 1)
    {
        
    }
    else if (totalNoOfShortcuts == 2)
    {
        if (field == 1)
        {
            returnVal = shortcuts_data["name1"].get<string>();
            shortcuts_data["name1"] = "";
        }
        if (field == 2)
        {
            returnVal = shortcuts_data["path_url1"].get<string>();
            shortcuts_data["path_url1"] = "";
        }
        if (field == 3)
        {
            returnVal = shortcuts_data["symbol1"].get<string>();
            shortcuts_data["symbol1"] = "";
        }
    }
    else if (totalNoOfShortcuts == 3)
    {
        if (field == 1)
        {
            returnVal = shortcuts_data["name2"].get<string>();
            shortcuts_data["name2"] = "";
        }
        if (field == 2)
        {
            returnVal = shortcuts_data["path_url2"].get<string>();
            shortcuts_data["path_url2"] = "";
        }
        if (field == 3)
        {
            returnVal = shortcuts_data["symbol2"].get<string>();
            shortcuts_data["symbol2"] = "";
        }
    }
    else if (totalNoOfShortcuts == 4)
    {
        if (field == 1)
        {
            returnVal = shortcuts_data["name3"].get<string>();
            shortcuts_data["name3"] = "";
        }
        if (field == 2)
        {
            returnVal = shortcuts_data["path_url3"].get<string>();
            shortcuts_data["path_url3"] = "";
        }
        if (field == 3)
        {
            returnVal = shortcuts_data["symbol3"].get<string>();
            shortcuts_data["symbol3"] = "";
        }
    }
    else if (totalNoOfShortcuts == 5)
    {
        if (field == 1)
        {
            returnVal = shortcuts_data["name4"].get<string>();
            shortcuts_data["name4"] = "";
        }
        if (field == 2)
        {
            returnVal = shortcuts_data["path_url4"].get<string>();
            shortcuts_data["path_url4"] = "";
        }
        if (field == 3)
        {
            returnVal = shortcuts_data["symbol4"].get<string>();
            shortcuts_data["symbol4"] = "";
        }
    }
    else if (totalNoOfShortcuts == 6)
    {
        if (field == 1)
        {
            returnVal = shortcuts_data["name5"].get<string>();
            shortcuts_data["name1"] = "";
        }
        if (field == 2)
        {
            returnVal = shortcuts_data["path_url5"].get<string>();
            shortcuts_data["path_url5"] = "";
        }
        if (field == 3)
        {
            returnVal = shortcuts_data["symbol5"].get<string>();
            shortcuts_data["symbol5"] = "";
        }
    }
    else if (totalNoOfShortcuts == 7)
    {
        if (field == 1)
        {
            returnVal = shortcuts_data["name6"].get<string>();
            shortcuts_data["name6"] = "";
        }
        if (field == 2)
        {
            returnVal = shortcuts_data["path_url6"].get<string>();
            shortcuts_data["path_url6"] = "";
        }
        if (field == 3)
        {
            returnVal = shortcuts_data["symbol6"].get<string>();
            shortcuts_data["symbol6"] = "";
        }
    }
    else
    {

    }

    return returnVal;
}


// To Remove A Json Field From Alarms

string MainWindow::RemoveJsonFeildAl(string no, string item)
{
    string returnVal;

    if (totalNoOfAlarms == 1 && no != "alarm1")
    {
        returnVal = "";
    }
    else if (totalNoOfAlarms == 2 && no != "alarm2")
    {
        returnVal = alarms_data["alarm2"][item].get<string>();
        alarms_data["alarm2"][item] = "";
    }
    else if (totalNoOfAlarms == 3 && no != "alarm3")
    {
        returnVal = alarms_data["alarm3"][item].get<string>();
        alarms_data["alarm3"][item] = "";
    }
    else if (totalNoOfAlarms == 4 && no != "alarm4")
    {
        returnVal = alarms_data["alarm4"][item].get<string>();
        alarms_data["alarm4"][item] = "";
    }
    else
    {
        returnVal = "";
    }

    return returnVal;
}

// Destroying Buttons

void MainWindow::DestroyBasicButtons() 
{
    DestroyWindow(hwndButton_wiki);
    DestroyWindow(hwndButton_openWeb);
    DestroyWindow(hwndButton_searchGoogle);
    DestroyWindow(hwndButton_playmusic);
    DestroyWindow(hwndButton_pausemusic);
    DestroyWindow(hwndInputBox);
    DestroyWindow(hwndEnter);
}

void MainWindow::DestroyShortcuts() 
{
    DestroyWindow(GetDlgItem(m_hwnd, 1));
    DestroyWindow(GetDlgItem(m_hwnd, 2));
    DestroyWindow(GetDlgItem(m_hwnd, 3));
    DestroyWindow(GetDlgItem(m_hwnd, 4));
    DestroyWindow(GetDlgItem(m_hwnd, 5));
    DestroyWindow(GetDlgItem(m_hwnd, 6));
    DestroyWindow(GetDlgItem(m_hwnd, 7));

    DestroyWindow(GetDlgItem(m_hwnd, 11));
    DestroyWindow(GetDlgItem(m_hwnd, 12));
    DestroyWindow(GetDlgItem(m_hwnd, 13));
    DestroyWindow(GetDlgItem(m_hwnd, 14));
    DestroyWindow(GetDlgItem(m_hwnd, 15));
    DestroyWindow(GetDlgItem(m_hwnd, 16));
    DestroyWindow(GetDlgItem(m_hwnd, 17));

    DestroyWindow(GetDlgItem(m_hwnd, 21));
    DestroyWindow(GetDlgItem(m_hwnd, 22));
    DestroyWindow(GetDlgItem(m_hwnd, 23));
    DestroyWindow(GetDlgItem(m_hwnd, 24));
    DestroyWindow(GetDlgItem(m_hwnd, 25));

    DestroyWindow(alarmButton);
    DestroyWindow(hwndEnterAlarm);
    DestroyWindow(hwndCloseAlarm);
    DestroyWindow(alarmMin);
    DestroyWindow(alarmHour);
    DestroyWindow(alarmAMPM);
    DestroyWindow(alarmDay);
    DestroyWindow(alarmMusicPath);
    DestroyWindow(alarmMusicPathBrowse);
}

void MainWindow::DestroyNewShortcutsButtons()
{

    DestroyWindow(inputName);
    DestroyWindow(inputPath);
    DestroyWindow(inputSymbol);
    DestroyWindow(inputSymbolBrowse);
    DestroyWindow(hwndEnterShortcut);
    DestroyWindow(hwndCloseShortcut);
}

void MainWindow::checkChangeInTime()
{
    int hour = checkTime(1);
    int min = checkTime(2);

    while (true) {
        int new_min = checkTime(2);
        if (min != new_min) {
            hour = checkTime(1);
            min = checkTime(2);
            thread thread_obj_al1(checkAlarms, "alarm1");
            thread_obj_al1.detach();
            thread thread_obj_al2(checkAlarms, "alarm2");
            thread_obj_al2.detach();
            thread thread_obj_al3(checkAlarms, "alarm3");
            thread_obj_al3.detach();
            thread thread_obj_al4(checkAlarms, "alarm4");
            thread_obj_al4.detach();
            RedrawWindow(m_hwnd, NULL, NULL, RDW_INVALIDATE);
            Sleep(3000);
        }

        if (alarmFunc == true)
        {
            startAlarm();
        }
    }
}

void MainWindow::startAlarm() 
{
    ShowWindow(m_hwnd, SW_SHOWNORMAL);
    if (MessageBox(m_hwnd, L"Alarm", L"Alarm", MB_OK) == IDOK)
    {
        alarmFunc = false;
    }
}

// Handles Messages

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    // On Creation 
    case WM_CREATE:
    {
        if (FAILED(D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory)))
        {
            return -1;
        }

        ifstream ifs_sh("shortcuts.json");
        shortcuts_data = json::parse(ifs_sh);

        ifstream ifs_al("alarms.json");
        alarms_data = json::parse(ifs_al);

        HICON hIcon = (HICON)LoadImage(NULL, L"Assistant.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
        SendMessage(m_hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(m_hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

        wikiFunc = false;
        webFunc = false;
        googleFunc = false;
        musicFunc = false;
        alarmFunc = false;

        thread thread_obj_time_check(&MainWindow::checkChangeInTime, this);
        thread_obj_time_check.detach();

        SystemParametersInfo(SPI_SETBEEP, FALSE, NULL, 0);

        thread thread_obj_greeting(speakOutput, greeting());
        thread_obj_greeting.detach();

        thread thread_obj_al1(checkAlarms, "alarm1");
        thread_obj_al1.detach();
        thread thread_obj_al2(checkAlarms, "alarm2");
        thread_obj_al2.detach();
        thread thread_obj_al3(checkAlarms, "alarm3");
        thread_obj_al3.detach();
        thread thread_obj_al4(checkAlarms, "alarm4");
        thread_obj_al4.detach();

        return 0;
    }

    // On Destruction
    case WM_DESTROY:
        DiscardGraphicsResources();
        DiscardDeviceIndependentResources();
        SafeRelease(&pFactory);
        PostQuitMessage(0);
        return 0;

    // Run The Paint Function
    case WM_PAINT:
        OnPaint();
        return 0;

    // When Close Button Is Clicked
    case WM_CLOSE:
    {
        int ifClose = MessageBox(m_hwnd, L"Do You Want To Hide Assistant ( If Cancelled App Will Close But Alarms Won't Work ) ?", L"Assistant", MB_OKCANCEL);

        if (ifClose == IDCANCEL)
        {
            DestroyWindow(m_hwnd);
        }
        else if (ifClose == IDOK)
        {
            ShowWindow(m_hwnd, SW_HIDE);
        }
        else
        {

        }

        return 0;
    }

    // Sets The Cursor When Mouse Moves
    case WM_MOUSEMOVE:
    {
        int xPos = GET_X_LPARAM(lParam);
        int yPos = GET_Y_LPARAM(lParam);
        if (pRenderTarget != NULL)
        {
            SetCursor(hCursor_arrow);
        }
        return 0;
    }

    // Mostly Using For Button Clicks
    case WM_COMMAND:
    {
        if (LOWORD(wParam) != totalNoOfShortcuts) {
            if (LOWORD(wParam) == 1)
            {
                string path = shortcuts_data["path_url1"].get<string>();
                wstring stemp_path = s2ws(path);
                LPCWSTR result = stemp_path.c_str();
                thread thread_obj(speakOutput, (LPWSTR)L"Opening");
                thread_obj.detach();
                ShellExecute(NULL, L"open", result, NULL, NULL, SW_SHOWNORMAL);
            }
            else if (LOWORD(wParam) == 2)
            {
                string path = shortcuts_data["path_url2"].get<string>();
                wstring stemp_path = s2ws(path);
                LPCWSTR result = stemp_path.c_str();
                thread thread_obj(speakOutput, (LPWSTR)L"Opening");
                thread_obj.detach();
                ShellExecute(NULL, L"open", result, NULL, NULL, SW_SHOWNORMAL);
            }
            else if (LOWORD(wParam) == 3)
            {
                string path = shortcuts_data["path_url3"].get<string>();
                wstring stemp_path = s2ws(path);
                LPCWSTR result = stemp_path.c_str();
                thread thread_obj(speakOutput, (LPWSTR)L"Opening");
                thread_obj.detach();
                ShellExecute(NULL, L"open", result, NULL, NULL, SW_SHOWNORMAL);
            }
            else if (LOWORD(wParam) == 4)
            {
                string path = shortcuts_data["path_url4"].get<string>();
                wstring stemp_path = s2ws(path);
                LPCWSTR result = stemp_path.c_str();
                thread thread_obj(speakOutput, (LPWSTR)L"Opening");
                thread_obj.detach();
                ShellExecute(NULL, L"open", result, NULL, NULL, SW_SHOWNORMAL);
            }
            else if (LOWORD(wParam) == 5)
            {
                string path = shortcuts_data["path_url5"].get<string>();
                wstring stemp_path = s2ws(path);
                LPCWSTR result = stemp_path.c_str();
                thread thread_obj(speakOutput, (LPWSTR)L"Opening");
                thread_obj.detach();
                ShellExecute(NULL, L"open", result, NULL, NULL, SW_SHOWNORMAL);
            }
            else if (LOWORD(wParam) == 6)
            {
                string path = shortcuts_data["path_url6"].get<string>();
                wstring stemp_path = s2ws(path);
                LPCWSTR result = stemp_path.c_str();
                thread thread_obj(speakOutput, (LPWSTR)L"Opening");
                thread_obj.detach();
                ShellExecute(NULL, L"open", result, NULL, NULL, SW_SHOWNORMAL);
            }
            else
            {

            }
        }
        else
        {
            thread thread_obj(speakOutput, (LPWSTR)L"Please provide the specifications");
            thread_obj.detach();

            ShowNewShortcut();
        }
        if (LOWORD(wParam) >= 11 && LOWORD(wParam) < 20)
        {
            thread thread_obj(speakOutput, (LPWSTR)L"Do You Want To Delete The Selected Shortcut ?");
            thread_obj.detach();
            if (MessageBox(m_hwnd, L"Do You Want To Delete The Selected Shortcut ?", L"Assistant", MB_OKCANCEL) == IDOK)
            {
                thread thread_obj(speakOutput, (LPWSTR)L"Deleting the selected shortcut");
                thread_obj.detach();
                if ((LOWORD(wParam) - 10) != totalNoOfShortcuts - 1) {
                    if (LOWORD(wParam) == 11)
                    {
                        shortcuts_data["name1"] = RemoveJsonFeild(1, 1);
                        shortcuts_data["path_url1"] = RemoveJsonFeild(1, 2);
                        shortcuts_data["symbol1"] = RemoveJsonFeild(1, 3);
                    }
                    if (LOWORD(wParam) == 12)
                    {
                        shortcuts_data["name2"] = RemoveJsonFeild(2, 1);
                        shortcuts_data["path_url2"] = RemoveJsonFeild(2, 2);
                        shortcuts_data["symbol2"] = RemoveJsonFeild(2, 3);
                    }
                    if (LOWORD(wParam) == 13)
                    {
                        shortcuts_data["name3"] = RemoveJsonFeild(3, 1);
                        shortcuts_data["path_url3"] = RemoveJsonFeild(3, 2);
                        shortcuts_data["symbol3"] = RemoveJsonFeild(3, 3);
                    }
                    if (LOWORD(wParam) == 14)
                    {
                        shortcuts_data["name4"] = RemoveJsonFeild(4, 1);
                        shortcuts_data["path_url4"] = RemoveJsonFeild(4, 2);
                        shortcuts_data["symbol4"] = RemoveJsonFeild(4, 3);
                    }
                    if (LOWORD(wParam) == 15)
                    {
                        shortcuts_data["name5"] = RemoveJsonFeild(5, 1);
                        shortcuts_data["path_url5"] = RemoveJsonFeild(5, 2);
                        shortcuts_data["symbol5"] = RemoveJsonFeild(5, 3);
                    }
                    if (LOWORD(wParam) == 16)
                    {
                        shortcuts_data["name6"] = RemoveJsonFeild(6, 1);
                        shortcuts_data["path_url6"] = RemoveJsonFeild(6, 2);
                        shortcuts_data["symbol6"] = RemoveJsonFeild(6, 3);
                    }
                }
                else
                {
                    if (LOWORD(wParam) == 11)
                    {
                        shortcuts_data["name1"] = "";
                        shortcuts_data["path_url1"] = "";
                        shortcuts_data["symbol1"] = "";
                    }
                    if (LOWORD(wParam) == 12)
                    {
                        shortcuts_data["name2"] = "";
                        shortcuts_data["path_url2"] = "";
                        shortcuts_data["symbol2"] = "";
                    }
                    if (LOWORD(wParam) == 13)
                    {
                        shortcuts_data["name3"] = "";
                        shortcuts_data["path_url3"] = "";
                        shortcuts_data["symbol3"] = "";
                    }
                    if (LOWORD(wParam) == 14)
                    {
                        shortcuts_data["name4"] = "";
                        shortcuts_data["path_url4"] = "";
                        shortcuts_data["symbol4"] = "";
                    }
                    if (LOWORD(wParam) == 15)
                    {
                        shortcuts_data["name5"] = "";
                        shortcuts_data["path_url5"] = "";
                        shortcuts_data["symbol5"] = "";
                    }
                    if (LOWORD(wParam) == 16)
                    {
                        shortcuts_data["name6"] = "";
                        shortcuts_data["path_url6"] = "";
                        shortcuts_data["symbol6"] = "";
                    }
                }

                ofstream ob("shortcuts.json");
                ob << shortcuts_data;

                DestroyBasicButtons();
                DestroyShortcuts();
                DestroyNewShortcutsButtons();
                ShowButtons();
            }
            else
            {
                thread thread_obj(speakOutput, (LPWSTR)L"Didn't delete anything");
                thread_obj.detach();
            }
        }
        if (LOWORD(wParam) > 20)
        {
            if (LOWORD(wParam) == 21)
            {
                if (MessageBox(m_hwnd, L"Do You Want To Delete The Selected Alarm ?", L"Assistant", MB_OKCANCEL) == IDOK)
                {
                    alarms_data["alarm1"]["day"] = RemoveJsonFeildAl("alarm1", "day");
                    alarms_data["alarm1"]["hour"] = RemoveJsonFeildAl("alarm1", "hour");
                    alarms_data["alarm1"]["min"] = RemoveJsonFeildAl("alarm1", "min");
                    alarms_data["alarm1"]["AMPM"] = RemoveJsonFeildAl("alarm1", "AMPM");
                    alarms_data["alarm1"]["path"] = RemoveJsonFeildAl("alarm1", "path");
                }
            }
            if (LOWORD(wParam) == 22)
            {
                if (MessageBox(m_hwnd, L"Do You Want To Delete The Selected Alarm ?", L"Assistant", MB_OKCANCEL) == IDOK)
                {
                    alarms_data["alarm2"]["day"] = RemoveJsonFeildAl("alarm2", "day");
                    alarms_data["alarm2"]["hour"] = RemoveJsonFeildAl("alarm2", "hour");
                    alarms_data["alarm2"]["min"] = RemoveJsonFeildAl("alarm2", "min");
                    alarms_data["alarm2"]["AMPM"] = RemoveJsonFeildAl("alarm2", "AMPM");
                    alarms_data["alarm2"]["path"] = RemoveJsonFeildAl("alarm2", "path");
                }
            }
            if (LOWORD(wParam) == 23)
            {
                if (MessageBox(m_hwnd, L"Do You Want To Delete The Selected Alarm ?", L"Assistant", MB_OKCANCEL) == IDOK)
                {
                    alarms_data["alarm3"]["day"] = RemoveJsonFeildAl("alarm3", "day");
                    alarms_data["alarm3"]["hour"] = RemoveJsonFeildAl("alarm3", "hour");
                    alarms_data["alarm3"]["min"] = RemoveJsonFeildAl("alarm3", "min");
                    alarms_data["alarm3"]["AMPM"] = RemoveJsonFeildAl("alarm3", "AMPM");
                    alarms_data["alarm3"]["path"] = RemoveJsonFeildAl("alarm3", "path");
                }
            }
            if (LOWORD(wParam) == 24)
            {
                if (MessageBox(m_hwnd, L"Do You Want To Delete The Selected Alarm ?", L"Assistant", MB_OKCANCEL) == IDOK)
                {
                    alarms_data["alarm4"]["day"] = "";
                    alarms_data["alarm4"]["hour"] = "";
                    alarms_data["alarm4"]["min"] = "";
                    alarms_data["alarm4"]["AMPM"] = "";
                    alarms_data["alarm4"]["path"] = "";
                }
            }

            ofstream ob("alarms.json");
            ob << alarms_data;

            DestroyBasicButtons();
            DestroyShortcuts();
            DestroyNewShortcutsButtons();
            ShowButtons();
        }

        switch (wParam)
        {
        case BN_CLICKED:
            if ((HWND)lParam == hwndButton_wiki)
            {
                wikiFunc = true;
                webFunc = false;
                googleFunc = false;
                thread thread_obj(speakOutput, (LPWSTR)L"PLease enter the wikipedia search query");
                thread_obj.detach();
            }

            if ((HWND)lParam == hwndButton_openWeb)
            {
                webFunc = true;
                wikiFunc = false;
                googleFunc = false;
                thread thread_obj(speakOutput, (LPWSTR)L"PLease enter the url");
                thread_obj.detach();
            }

            if ((HWND)lParam == hwndButton_searchGoogle)
            {
                googleFunc = true;
                wikiFunc = false;
                webFunc = false;
                thread thread_obj(speakOutput, (LPWSTR)L"PLease enter the search query");
                thread_obj.detach();
            }

            if ((HWND)lParam == hwndButton_playmusic)
            {
                thread thread_obj(speakOutput, (LPWSTR)L"PLease select the mp3 file");
                thread_obj.detach();

                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                    COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr))
                {
                    IFileOpenDialog* pFileOpen;

                    // Create the FileOpenDialog object.
                    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                    if (SUCCEEDED(hr))
                    {
                        // Show the Open dialog box.
                        hr = pFileOpen->Show(NULL);

                        // Get the file name from the dialog box.
                        if (SUCCEEDED(hr))
                        {
                            IShellItem* pItem;
                            hr = pFileOpen->GetResult(&pItem);
                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszMusicFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszMusicFilePath);

                                // Display the file name to the user.
                                if (SUCCEEDED(hr))
                                {
                                    thread_obj_music = thread(playMusic, pszMusicFilePath);
                                    thread_obj_music.detach();
                                    musicFunc = true;
                                    DestroyBasicButtons();
                                    DestroyShortcuts();
                                    DestroyNewShortcutsButtons();
                                    ShowButtons();
                                    CoTaskMemFree(pszMusicFilePath);
                                }
                                pItem->Release();
                            }
                        }
                        pFileOpen->Release();
                    }
                }
            }

            if ((HWND)lParam == hwndButton_pausemusic)
            {
                musicFunc = false;
                DestroyBasicButtons();
                DestroyShortcuts();
                DestroyNewShortcutsButtons();
                ShowButtons();
            }

            if ((HWND)lParam == alarmButton) 
            {
                DestroyBasicButtons();
                DestroyShortcuts();
                DestroyNewShortcutsButtons();
                ifNewAlarms = true;
                ShowButtons();
            }

            if ((HWND)lParam == alarmMusicPathBrowse)
            {
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                    COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr))
                {
                    IFileOpenDialog* pFileOpen;

                    // Create the FileOpenDialog object.
                    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                    if (SUCCEEDED(hr))
                    {
                        // Show the Open dialog box.
                        hr = pFileOpen->Show(NULL);

                        // Get the file name from the dialog box.
                        if (SUCCEEDED(hr))
                        {
                            IShellItem* pItem;
                            hr = pFileOpen->GetResult(&pItem);
                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                                // Display the file name to the user.
                                if (SUCCEEDED(hr))
                                {
                                    SendMessage(alarmMusicPath, WM_SETTEXT, 0, (LPARAM)pszFilePath);
                                    MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
                                    CoTaskMemFree(pszFilePath);
                                }
                                pItem->Release();
                            }
                        }
                        pFileOpen->Release();
                    }
                }
            }

            if ((HWND)lParam == hwndEnterAlarm)
            {
                int cTextLenHour = GetWindowTextLength(alarmHour);
                PSTR pszMemHour = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLenHour + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(alarmHour, (LPWSTR)pszMemHour, cTextLenHour + 1);
                wstring hourW((LPWSTR)pszMemHour);
                string hourS(hourW.begin(), hourW.end());

                int cTextLenMin = GetWindowTextLength(alarmMin);
                PSTR pszMemMin = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLenMin + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(alarmMin, (LPWSTR)pszMemMin, cTextLenMin + 1);
                wstring minW((LPWSTR)pszMemMin);
                string minS(minW.begin(), minW.end());

                int cTextLenDay = GetWindowTextLength(alarmDay);
                PSTR pszMemDay = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLenDay + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(alarmDay, (LPWSTR)pszMemDay, cTextLenDay + 1);
                wstring dayW((LPWSTR)pszMemDay);
                string dayS(dayW.begin(), dayW.end());

                int cTextLenAMPM = GetWindowTextLength(alarmAMPM);
                PSTR pszMemAMPM = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLenAMPM + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(alarmAMPM, (LPWSTR)pszMemAMPM, cTextLenAMPM + 1);
                wstring ampmW((LPWSTR)pszMemAMPM);
                string ampmS(ampmW.begin(), ampmW.end());

                int cTextLenMusicPath = GetWindowTextLength(alarmMusicPath);
                PSTR pszMemMusicPath = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLenMusicPath + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(alarmMusicPath, (LPWSTR)pszMemMusicPath, cTextLenMusicPath + 1);
                wstring musicPathW((LPWSTR)pszMemMusicPath);
                string musicPathS(musicPathW.begin(), musicPathW.end());

                ofstream ob("alarms.json");

                if (minS.length() == 1)
                {
                    minS = "0" + minS;
                }

                if (totalNoOfAlarms == 0) {
                    alarms_data["alarm1"]["day"] = dayS;
                    alarms_data["alarm1"]["hour"] = hourS;
                    alarms_data["alarm1"]["min"] = minS;
                    alarms_data["alarm1"]["AMPM"] = ampmS;
                    alarms_data["alarm1"]["path"] = musicPathS;
                }
                else if (totalNoOfAlarms == 1) {
                    alarms_data["alarm2"]["day"] = dayS;
                    alarms_data["alarm2"]["hour"] = hourS;
                    alarms_data["alarm2"]["min"] = minS;
                    alarms_data["alarm2"]["AMPM"] = ampmS;
                    alarms_data["alarm2"]["path"] = musicPathS;
                }
                else if (totalNoOfAlarms == 2) {
                    alarms_data["alarm3"]["day"] = dayS;
                    alarms_data["alarm3"]["hour"] = hourS;
                    alarms_data["alarm3"]["min"] = minS;
                    alarms_data["alarm3"]["AMPM"] = ampmS;
                    alarms_data["alarm3"]["path"] = musicPathS;
                }
                else if (totalNoOfAlarms == 3) {
                    alarms_data["alarm4"]["day"] = dayS;
                    alarms_data["alarm4"]["hour"] = hourS;
                    alarms_data["alarm4"]["min"] = minS;
                    alarms_data["alarm4"]["AMPM"] = ampmS;
                    alarms_data["alarm4"]["path"] = musicPathS;
                }
                else {

                }
                ob << alarms_data;

                ifNewAlarms = false;
                DestroyBasicButtons();
                DestroyShortcuts();
                DestroyNewShortcutsButtons();
                ShowButtons();
            }

            if ((HWND)lParam == hwndCloseAlarm)
            {
                DestroyBasicButtons();
                DestroyShortcuts();
                DestroyNewShortcutsButtons();
                ifNewAlarms = false;
                ShowButtons();
            }

            if ((HWND)lParam == hwndEnter)
            {

                cTextLen = GetWindowTextLength(hwndInputBox);
                pszMem = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLen + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(hwndInputBox, (LPWSTR)pszMem, cTextLen + 1);

                if (wikiFunc == true)
                {
                    wstring link;
                    wstring w1(L"https://en.wikipedia.org/wiki/");
                    wstring w2((LPWSTR)pszMem);
                    link = w1 + w2;
                    const wchar_t* searchLink = link.c_str();
                    ShellExecute(NULL, L"open", (LPCWSTR)searchLink, NULL, NULL, SW_SHOWNORMAL);
                    wikiFunc = false;
                    webFunc = false;
                    googleFunc = false;
                }
                else if (webFunc == true)
                {
                    ShellExecute(NULL, L"open", (LPCWSTR)pszMem, NULL, NULL, SW_SHOWNORMAL);
                    wikiFunc = false;
                    webFunc = false;
                    googleFunc = false;
                }
                else if (googleFunc == true)
                {
                    wstring link;
                    wstring w1(L"https://www.google.com/search?q=");
                    wstring w2((LPWSTR)pszMem);
                    link = w1 + w2;
                    const wchar_t* searchLink = link.c_str();
                    ShellExecute(NULL, L"open", (LPCWSTR)searchLink, NULL, NULL, SW_SHOWNORMAL);
                    wikiFunc = false;
                    webFunc = false;
                    googleFunc = false;
                }
                else
                {
                    wstring inputWstr((LPWSTR)pszMem);
                    string inputStr(inputWstr.begin(), inputWstr.end());
                    transform(inputStr.begin(), inputStr.end(), inputStr.begin(), ::tolower);
                    string searchStr("search");
                    string wikiStr("wiki");
                    string googleStr("google");
                    string openStr("open");
                    string linkStr("link");
                    string playStr("play music");

                    string name1Str = shortcuts_data["name1"].get<string>();
                    transform(name1Str.begin(), name1Str.end(), name1Str.begin(), ::tolower);
                    string name2Str = shortcuts_data["name2"].get<string>();
                    transform(name2Str.begin(), name2Str.end(), name2Str.begin(), ::tolower);
                    string name3Str = shortcuts_data["name3"].get<string>();
                    transform(name3Str.begin(), name3Str.end(), name3Str.begin(), ::tolower);
                    string name4Str = shortcuts_data["name4"].get<string>();
                    transform(name4Str.begin(), name4Str.end(), name4Str.begin(), ::tolower);
                    string name5Str = shortcuts_data["name5"].get<string>();
                    transform(name5Str.begin(), name5Str.end(), name5Str.begin(), ::tolower);
                    string name6Str = shortcuts_data["name6"].get<string>();
                    transform(name6Str.begin(), name6Str.end(), name6Str.begin(), ::tolower);

                    if (inputStr.find(searchStr) != string::npos) 
                    {
                        if (inputStr.find(googleStr) != string::npos)
                        {
                            SendMessage(hwndButton_searchGoogle, BM_CLICK, 0, 0);
                        }
                        else if (inputStr.find(wikiStr) != string::npos)
                        {
                            SendMessage(hwndButton_wiki, BM_CLICK, 0, 0);
                        }
                        else
                        {

                        }
                    }
                    else if (inputStr.find(playStr) != string::npos)
                    {
                        SendMessage(hwndButton_playmusic, BM_CLICK, 0, 0);
                    }
                    else if (inputStr.find(openStr) != string::npos)
                    {
                        if (inputStr.find(linkStr) != string::npos)
                        {
                            SendMessage(hwndButton_openWeb, BM_CLICK, 0, 0);
                        }
                        else
                        {
                            if (inputStr.find(name1Str) != string::npos)
                            {
                                SendMessage(GetDlgItem(m_hwnd, 1), BM_CLICK, 0, 0);
                            }
                            else if (inputStr.find(name2Str) != string::npos)
                            {
                                SendMessage(GetDlgItem(m_hwnd, 2), BM_CLICK, 0, 0);
                            }
                            else if (inputStr.find(name3Str) != string::npos)
                            {
                                SendMessage(GetDlgItem(m_hwnd, 3), BM_CLICK, 0, 0);
                            }
                            else if (inputStr.find(name4Str) != string::npos)
                            {
                                SendMessage(GetDlgItem(m_hwnd, 4), BM_CLICK, 0, 0);
                            }
                            else if (inputStr.find(name5Str) != string::npos)
                            {
                                SendMessage(GetDlgItem(m_hwnd, 5), BM_CLICK, 0, 0);
                            }
                            else if (inputStr.find(name6Str) != string::npos)
                            {
                                SendMessage(GetDlgItem(m_hwnd, 6), BM_CLICK, 0, 0);
                            }
                            else
                            {

                            }
                        }
                    }
                    else
                    {
                        
                    }
                }
            }

            if ((HWND)lParam == inputSymbolBrowse)
            {
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
                    COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr))
                {
                    IFileOpenDialog* pFileOpen;

                    // Create the FileOpenDialog object.
                    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                    if (SUCCEEDED(hr))
                    {
                        // Show the Open dialog box.
                        hr = pFileOpen->Show(NULL);

                        // Get the file name from the dialog box.
                        if (SUCCEEDED(hr))
                        {
                            IShellItem* pItem;
                            hr = pFileOpen->GetResult(&pItem);
                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                                // Display the file name to the user.
                                if (SUCCEEDED(hr))
                                {
                                    SendMessage(inputSymbol, WM_SETTEXT, 0, (LPARAM)pszFilePath);
                                    MessageBoxW(NULL, pszFilePath, L"File Path", MB_OK);
                                    CoTaskMemFree(pszFilePath);
                                }
                                pItem->Release();
                            }
                        }
                        pFileOpen->Release();
                    }
                }
            }

            if ((HWND)lParam == hwndEnterShortcut)
            {
                int cTextLenName = GetWindowTextLength(inputName);
                PSTR pszMemName = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLenName + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(inputName, (LPWSTR)pszMemName, cTextLenName + 1);
                wstring nameW((LPWSTR)pszMemName);
                string nameS(nameW.begin(), nameW.end());

                int cTextLenPath = GetWindowTextLength(inputPath);
                PSTR pszMemPath = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLenPath + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(inputPath, (LPWSTR)pszMemPath, cTextLenPath + 1);
                wstring pathW((LPWSTR)pszMemPath);
                string pathS(pathW.begin(), pathW.end());

                int cTextLenSymbol = GetWindowTextLength(inputSymbol);
                PSTR pszMemSymbol = (PSTR)VirtualAlloc((LPVOID)NULL,
                    (DWORD)(cTextLenSymbol + 1), MEM_COMMIT,
                    PAGE_READWRITE);
                GetWindowText(inputSymbol, (LPWSTR)pszMemSymbol, cTextLenSymbol + 1);
                wstring symbolW((LPWSTR)pszMemSymbol);
                string symbolS(symbolW.begin(), symbolW.end());

                ofstream ob("shortcuts.json");

                if (totalNoOfShortcuts == 1) {
                    shortcuts_data["name1"] = nameS;
                    shortcuts_data["path_url1"] = pathS;
                    shortcuts_data["symbol1"] = symbolS;
                }
                else if (totalNoOfShortcuts == 2) {
                    shortcuts_data["name2"] = nameS;
                    shortcuts_data["path_url2"] = pathS;
                    shortcuts_data["symbol2"] = symbolS;
                }
                else if (totalNoOfShortcuts == 3) {
                    shortcuts_data["name3"] = nameS;
                    shortcuts_data["path_url3"] = pathS;
                    shortcuts_data["symbol3"] = symbolS;
                }
                else if (totalNoOfShortcuts == 4) {
                    shortcuts_data["name4"] = nameS;
                    shortcuts_data["path_url4"] = pathS;
                    shortcuts_data["symbol4"] = symbolS;
                }
                else if (totalNoOfShortcuts == 5) {
                    shortcuts_data["name5"] = nameS;
                    shortcuts_data["path_url5"] = pathS;
                    shortcuts_data["symbol5"] = symbolS;
                }
                else if (totalNoOfShortcuts == 6) {
                    shortcuts_data["name6"] = nameS;
                    shortcuts_data["path_url6"] = pathS;
                    shortcuts_data["symbol6"] = symbolS;
                }
                else {

                }
                ob << shortcuts_data;

                ifNewShortcuts = false;
                DestroyBasicButtons();
                DestroyNewShortcutsButtons();
                ShowButtons();
            }
            if ((HWND)lParam == hwndCloseShortcut)
            {
                ifNewShortcuts = false;
                DestroyBasicButtons();
                DestroyNewShortcutsButtons();
                ShowButtons();
            }
        }
        return 0;
    }

    case WM_SIZE:
        Resize();
        return 0;

    // Sets A Minimum Resize Value
    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 900;
        lpMMI->ptMinTrackSize.y = 500;
        return 0;
    }

    // Sets Edit Box Text Color
    case WM_CTLCOLOREDIT:
        SetTextColor((HDC)wParam, RGB(10, 10, 10));
        return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

    // Using It To Color Buttons
    case WM_NOTIFY:
        switch (((LPNMHDR)lParam)->code)
        {
        case NM_CUSTOMDRAW:
            LPNMCUSTOMDRAW lpnmCD = (LPNMCUSTOMDRAW)lParam;

            switch (lpnmCD->dwDrawStage)
            {
            case CDDS_PREPAINT:
                
                SetDCBrushColor(lpnmCD->hdc, RGB(9, 155, 189));
                SetDCPenColor(lpnmCD->hdc, RGB(9, 155, 189));
                SelectObject(lpnmCD->hdc, GetStockObject(DC_BRUSH));
                SelectObject(lpnmCD->hdc, GetStockObject(DC_PEN));
                Rectangle(
                    lpnmCD->hdc,
                    lpnmCD->rc.left,
                    lpnmCD->rc.top,
                    lpnmCD->rc.right,
                    lpnmCD->rc.bottom
                );
                return true;
            }
            return 0;
        }
        return 0;
    }
    return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
}


// WinMain Function
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
    MainWindow win;
    CreateMutex(NULL, TRUE, L"AssistantMutex038");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND existingApp = FindWindow(L"Assistant Window Class", L"Assistant");
        ShowWindow(existingApp, SW_SHOWMAXIMIZED);
    }
    else 
    {
        if (!win.Create(L"Assistant", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN))
        {
            return 0;
        }

        ShowWindow(win.Window(), nCmdShow);

        MSG msg = { };
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return TRUE;
}