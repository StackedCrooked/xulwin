#include "XULWin/WinUtils.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/Unicode.h"
#include "Poco/Path.h"
#include <boost/lexical_cast.hpp>
#include <map>
#include <commctrl.h>


namespace XULWin
{

namespace WinAPI
{

    CommonControlsInitializer::CommonControlsInitializer()
    {
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC  = ICC_LISTVIEW_CLASSES | ICC_BAR_CLASSES | ICC_COOL_CLASSES | ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES;
        if (TRUE != InitCommonControlsEx(&icex))
        {
            throw std::runtime_error("Failed to initialized the Common Controls library. Maybe you forgot to add a manifest file to your project?");
        }
    }


    CurrentDirectoryChanger::CurrentDirectoryChanger(const std::string & inTargetDir)
    {
        ::GetCurrentDirectory(MAX_PATH, mOldDir);
        std::wstring newDir = ToUTF16(inTargetDir);
        if (0 == ::SetCurrentDirectory(newDir.c_str()))
        {
            std::string msg = "Failed to change the directory to '" + inTargetDir + "'. Reason: " + WinAPI::System_GetLastError(::GetLastError());
            throw std::runtime_error(msg.c_str());
        }
    }


    CurrentDirectoryChanger::~CurrentDirectoryChanger()
    {
        ::SetCurrentDirectoryW(mOldDir);
    }


    HWND Window_GetDefaultWindow()
    {
        static HWND hwnd(0);
        if (!hwnd)
        {
            hwnd = ::CreateWindowEx(0, L"STATIC", L"", 0, 0, 0, 0, 0, 0, 0, ::GetModuleHandle(0), 0);
        }
        return hwnd;
    }


    std::string Window_GetClassName(HWND inHWND)
    {
        std::vector<TCHAR> buffer(512);
        ::GetClassName(inHWND, &buffer[0], static_cast<int>(buffer.size()));
        return ToUTF8(&buffer[0]);
    }

    
    Rect Window_GetClientRect(HWND inHWND)
    {
        RECT rc;
        ::GetClientRect(inHWND, &rc);
        return Rect(rc.left, rc.top, rc.right -  rc.left, rc.bottom - rc.top);
    }


    Rect Window_GetWindowRect(HWND inHWND)
    {
        RECT rw;
        ::GetWindowRect(inHWND, &rw);
        return Rect(rw.left, rw.top, rw.right -  rw.left, rw.bottom - rw.top);
    }


    std::string System_GetCurrentDirectory()
    {
        TCHAR buffer[MAX_PATH+1];
        ::GetCurrentDirectory(sizeof(buffer), &buffer[0]);
        return std::string(ToUTF8(buffer)) + "\\";
    }


    std::string System_GetEnvironmentVariable(HINSTANCE hInstance)
    {
        TCHAR fileName[MAX_PATH+1] = L"";
        ::GetModuleFileName(hInstance, fileName, MAX_PATH);
        Poco::Path path(ToUTF8(&fileName[0]));
        return path.parent().toString();
    }


    std::string System_GetEnvironmentVariable(const std::string & inVariableName)
    {
        const int cBufferSize = 1024;
        TCHAR output[cBufferSize];
        std::wstring variableNameW = ToUTF16(inVariableName);
        ExpandEnvironmentStrings(variableNameW.c_str(), &output[0], cBufferSize);
        return ToUTF8(&output[0]);
    }


    std::string System_GetProgramFilesDirectory()
    {
        return System_GetEnvironmentVariable("%ProgramFiles%");
    }


    void System_SetThreadName(WORD inThreadId, const std::string & inThreadName)
    {
        #pragma pack(push,8)
        typedef struct tagTHREADNAME_INFO
        {
            DWORD dwType; // Must be 0x1000.
            LPCSTR szName; // Pointer to name (in user addr space).
            DWORD inThreadId; // Thread ID (-1=caller thread).
            DWORD dwFlags; // Reserved for future use, must be zero.
        } THREADNAME_INFO;
        #pragma pack(pop)

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = inThreadName.c_str();
        info.inThreadId = inThreadId;
        info.dwFlags = 0;

        __try
        {
            const DWORD MS_VC_EXCEPTION=0x406D1388;
            RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
        }
    }


    SIZE Window_GetSizeDifferenceBetweenWindowRectAndClientRect(HWND inHandle)
    {
        RECT rc;
        GetClientRect(inHandle, &rc);

        RECT rw;
        GetWindowRect(inHandle, &rw);

        int rc_w = rc.right-rc.left;
        int rc_h = rc.bottom-rc.top;
        int rw_w = rw.right-rw.left;
        int rw_h = rw.bottom-rw.top;

        SIZE theDifference;
        theDifference.cx = rw_w-rc_w;
        theDifference.cy = rw_h-rc_h;

        return theDifference;
    }


    void System_NavigateURL(const std::string & inURL)
    {
        std::wstring utf16URL = ToUTF16(inURL);
        ::ShellExecute(NULL, TEXT("open"), utf16URL.c_str(), NULL, NULL, SW_SHOWNORMAL);
    }


    void ComboBox_Add(HWND inHandle, const std::string & inString)
    {
        std::wstring utf16String = ToUTF16(inString);
        if (CB_ERR == ::SendMessage(inHandle, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)utf16String.c_str()))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void ComboBox_RemoveByIndex(HWND inHandle, int inIndex)
    {
        if (CB_ERR == ::SendMessage(inHandle, CB_DELETESTRING, (WPARAM)inIndex, (LPARAM)0))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    HWND SpinButton_GetBuddy(HWND inHandle)
    {
        return HWND((HWND)::SendMessage(inHandle, UDM_GETBUDDY, 0, 0L));
    }


    HWND SpinButton_SetBuddy(HWND inHandle, HWND inBuddy)
    {
        return (HWND)::SendMessage(inHandle, UDM_SETBUDDY, (WPARAM)inBuddy, 0L);
    }


    int SpinButton_GetPos(HWND inHandle)
    {
        DWORD result = (DWORD)::SendMessage(inHandle, UDM_GETPOS, 0, 0L);
        return (int)(short)LOWORD(result);
    }


    int SpinButton_SetPos(HWND inHandle, int inPos)
    {
        return (int)(short)LOWORD(::SendMessage(inHandle, UDM_SETPOS, 0, MAKELPARAM(inPos, 0)));
    }


    DWORD SpinButton_GetRange(HWND inHandle)
    {
        return (DWORD)::SendMessage(inHandle, UDM_GETRANGE, 0, 0L);
    }


    void SpinButton_GetRange(HWND inHandle, int & outLower, int & outUpper)
    {
        DWORD res = (DWORD)::SendMessage(inHandle, UDM_GETRANGE, 0, 0L);
        outLower = (int)(short)HIWORD(res);
        outUpper = (int)(short)LOWORD(res);
    }


    void SpinButton_SetRange(HWND inHandle, int inLower, int inUpper)
    {
        ::SendMessage(inHandle, UDM_SETRANGE, 0, MAKELPARAM(inUpper, inLower));
    }

    
    void SpinButton_SetRange32(HWND inHandle, int inLower, int inUpper)
    {
        ::SendMessage(inHandle, UDM_SETRANGE32, inLower, inUpper);
    }


    void SpinButton_GetRange32(HWND inHandle, int& outLower, int& outUpper)
    {
        ::SendMessage(inHandle, UDM_GETRANGE32, (WPARAM)&outLower, (LPARAM)&outUpper);
    }

    
    std::string ListBox_getByIndex(HWND inHandle, int inIndex)
    {
        int len = ::SendMessage(inHandle, LB_GETTEXTLEN, (WPARAM)inIndex, (LPARAM)0);
        if (len == LB_ERR)
        {
            ReportError("LB_GETTEXTLEN returned LB_ERR. Last error: " + System_GetLastError(::GetLastError()));
            return "";
        }

        std::vector<TCHAR> buffer(len, 0);
        ::SendMessage(inHandle, LB_GETTEXT, (WPARAM)inIndex, (LPARAM)&buffer[0]);
        return ToUTF8(&buffer[0]);
    }


    void ListBox_Add(HWND inHandle, const std::string & inString)
    {
        std::wstring utf16String = ToUTF16(inString);
        if (LB_ERR == ::SendMessage(inHandle, (UINT)LB_ADDSTRING, (WPARAM)0, (LPARAM)utf16String.c_str()))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void ListBox_Remove(HWND inHandle, int inIndex)
    {
        if (LB_ERR == ::SendMessage(inHandle, LB_DELETESTRING, (WPARAM)inIndex, (LPARAM)0))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    int ListBox_GetSize(HWND inHandle)
    {
        int result = ::SendMessage(inHandle, LB_GETCOUNT, 0, 0);
        if (LB_ERR == result)
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
        return result;
    }


    int ListBox_GetIndexOf(HWND inHandle, const std::string & inStringValue)
    {
        std::wstring utf16String = ToUTF16(inStringValue);
        return (int)::SendMessage(inHandle, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)utf16String.c_str());
    }


    void ListBox_GetItemRect(HWND inHandle, int inIndex, RECT & outRect)
    {
        if (LB_ERR == ::SendMessage(inHandle, LB_GETITEMRECT, (WPARAM)inIndex, (LPARAM)&outRect))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    int ListBox_GetSelectedIndex(HWND inHandle)
    {
        return ::SendMessage(inHandle, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
    }


    void ListBox_SetSelectedIndex(HWND inHandle, int inIndex)
    {
        if (LB_ERR == ::SendMessage(inHandle, LB_SETCURSEL, (WPARAM)inIndex, (LPARAM)0))
        {
            // If an error occurs, the return value is LB_ERR. If the wParam
            // parameter is �1, the return value is LB_ERR even though no error
            // occurred.
            if (inIndex != -1)
            {
                ReportError(System_GetLastError(::GetLastError()));
            }
        }
    }


    int ListView_AddColumn(HWND inHandle, int inIndex, const std::string & inText)
    {
        static const int cColumnTextPadding = 15;
        LV_COLUMN colInfo;
        colInfo.mask = LVCF_TEXT | LVCF_WIDTH;
        std::wstring utf16Text = ToUTF16(inText);
        colInfo.pszText = const_cast<TCHAR *>(&utf16Text[0]);
        colInfo.cchTextMax = utf16Text.size();
        colInfo.cx = WinAPI::Window_GetTextSize(inHandle, inText).cx + cColumnTextPadding;
        int res = ListView_InsertColumn(inHandle, inIndex, &colInfo);

        if (-1 == res)
        {
            ReportError("Adding a column to the list view failed. Last error: " + System_GetLastError(::GetLastError()));
        }
        return res;
    }


    void ComboBox_Clear(HWND inHandle)
    {
        // This message always returns CB_OKAY.
        ::SendMessage(inHandle, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
    }


	bool ComboBox_IsOpen(HWND inHandle)
	{
		return TRUE == ::SendMessage(inHandle, CB_GETDROPPEDSTATE, (WPARAM)0, (LPARAM)0);
	}


    int ComboBox_FindString(HWND inHandle, const std::string & inString, int inOffset)
    {
        std::wstring utf16String = ToUTF16(inString);
        return (int)::SendMessage(inHandle, CB_FINDSTRING, (WPARAM)inOffset, (LPARAM)(LPTSTR)utf16String.c_str());
    }


    int ComboBox_Size(HWND inHandle)
    {
        int result = ::SendMessage(inHandle, CB_GETCOUNT, 0, 0);
        if (CB_ERR == result)
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
        return result;
    }


    int Window_GetWidth(HWND inHandle)
    {
        RECT rw;
        ::GetWindowRect(inHandle, &rw);
        return rw.right - rw.left;
    }


    int Window_GetHeight(HWND inHandle)
    {
        RECT rw;
        ::GetWindowRect(inHandle, &rw);
        return rw.bottom - rw.top;
    }


    int ComboBox_GetSelectedIndex(HWND inHandle)
    {
        int result = ::SendMessage(inHandle, (UINT)CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
        if (CB_ERR == result)
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
        return result;
    }


    void ComboBox_SetSelectedIndex(HWND inHandle, int inItemIndex)
    {
        if (CB_ERR == ::SendMessage(inHandle, (UINT)CB_SETCURSEL, (WPARAM)inItemIndex, (LPARAM)0))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    int ComboBox_ItemHeight(HWND inHandle, int inItemIndex)
    {
        return (int)::SendMessage(inHandle, (UINT)CB_GETITEMHEIGHT, (WPARAM)inItemIndex, (LPARAM)0);
    }


    void Window_SetWidth(HWND inHandle, int inWidth)
    {
        RECT rw;
        ::GetWindowRect(inHandle, &rw);

        int oldWidth = rw.right - rw.left;
        int x = rw.left - (inWidth - oldWidth)/2;
        if (inWidth < oldWidth)
        {
            x = rw.left + (inWidth - oldWidth)/2;
        }
        if (0 == ::MoveWindow(inHandle, x, rw.top, inWidth, rw.bottom - rw.top, FALSE))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void Window_SetHeight(HWND inHandle, int inHeight)
    {
        RECT rw;
        ::GetWindowRect(inHandle, &rw);

        int oldHeight = rw.bottom - rw.top;
        int y = rw.top - (inHeight - oldHeight)/2;
        if (inHeight < oldHeight)
        {
            y = rw.top + (inHeight - oldHeight)/2;
        }
        if (0 == ::MoveWindow(inHandle, rw.left, y, rw.right - rw.left, inHeight, FALSE))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    HFONT Window_GetFont(HWND inHandle)
    {
        return (HFONT)SendMessage(inHandle, WM_GETFONT, 0, 0);
    }


    SIZE Window_GetTextSize(HWND inHandle, const std::string & inText)
    {
        // get the size in pixels for the given text and font
        SIZE result = {0, 0};
        HDC hDC = GetDC(inHandle);
        SelectObject(hDC, Window_GetFont(inHandle));
        std::wstring utf16Text(ToUTF16(inText));
        ::GetTextExtentPoint32(hDC, utf16Text.c_str(), (int)utf16Text.size(), &result);
        ReleaseDC(inHandle, hDC);
        return result;
    }

    
    SIZE Window_GetTextSize(const std::string & inText)
    {
        return Window_GetTextSize(Window_GetDefaultWindow(), inText);
    }


    std::string Window_GetText(HWND inHandle)
    {
        std::string result;
        int length = ::GetWindowTextLength(inHandle);
        if (length > 0)
        {
            TCHAR * buffer = new TCHAR[length+1];
            ::GetWindowText(inHandle, buffer, length+1);
            result = ToUTF8(buffer);
            delete [] buffer;
        }
        return result;
    }


    void Window_SetText(HWND inHandle, const std::string & inText)
    {
        std::wstring utf16String = ToUTF16(inText);
        if (0 == ::SetWindowText(inHandle, utf16String.c_str()))
        {
            ReportError("Setting the text on component failed. Last error: " + System_GetLastError(::GetLastError()));
        }
    }


    std::string System_GetLastError(DWORD lastError)
    {
        LPVOID lpMsgBuf;
        ::FormatMessage
        (
            FORMAT_MESSAGE_ALLOCATE_BUFFER
            | FORMAT_MESSAGE_FROM_SYSTEM
            | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            lastError,
            MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
            (LPTSTR)&lpMsgBuf,
            0,
            NULL
        );
        if (lpMsgBuf)
        {
            std::wstring errorString = (LPTSTR)lpMsgBuf;
            LocalFree(lpMsgBuf);
            if (errorString.empty())
            {
                try
                {
                    errorString = boost::lexical_cast<std::wstring>(lastError);
                }
                catch (...)
                {
                    ReportError("Bad cast!");
                }
            }
            return ToUTF8(errorString);
        }
        else
        {
            return "";
        }
    }


    bool Window_IsDisabled(HWND inHandle)
    {
        return !::IsWindowEnabled(inHandle);
    }


    void Window_SetDisabled(HWND inHandle, bool inDisable)
    {
        ::EnableWindow(inHandle, inDisable ? FALSE : TRUE);
    }


    int Window_GetMultilineTextHeight(HWND inHandle)
    {
        int result = 0;

        HDC deviceContext(::GetDC(inHandle));

        HFONT font = (HFONT)::SendMessage(inHandle, WM_GETFONT, 0, 0);
        if (font)
        {
            ::SelectObject(deviceContext, font);
        }

        RECT rc;
        ::GetClientRect(inHandle, &rc);

        RECT bounds;
        bounds.left = 0;
        bounds.top = 0;
        bounds.right = rc.right - rc.left;
        bounds.bottom = INT_MAX;

        std::wstring textUTF16 = ToUTF16(Window_GetText(inHandle));

        result = ::DrawText(deviceContext, textUTF16.c_str(), (int)textUTF16.size(), &bounds, DT_CALCRECT | DT_WORDBREAK | DT_EDITCONTROL);

        ::ReleaseDC(inHandle, deviceContext);

        return result;
    }


    void CheckBox_SetState(HWND inHandle, CheckState inState)
    {
        // This message always returns zero.
        ::SendMessage(inHandle, BM_SETCHECK, (WPARAM)inState, 0);
    }


    CheckState CheckBox_GetState(HWND inHandle)
    {
        return static_cast<CheckState>(::SendMessage(inHandle, BM_GETCHECK, 0, 0));
    }


    bool CheckBox_IsChecked(HWND inHandle)
    {
        return CheckBox_GetState(inHandle) != UNCHECKED;
    }


    void CheckBox_SetChecked(HWND inHandle, bool inChecked)
    {
        CheckBox_SetState(inHandle, inChecked ? CHECKED : UNCHECKED);
    }

    
    void RadioButton_SetState(HWND inHandle, CheckState inState)
    {
        CheckBox_SetState(inHandle, inState);
    }

    
    CheckState RadioButton_GetState(HWND inHandle)
    {
        return CheckBox_GetState(inHandle);
    }

    
    bool RadioButton_IsChecked(HWND inHandle)
    {
        return CheckBox_IsChecked(inHandle);
    }


    void RadioButton_SetChecked(HWND inHandle, bool inChecked)
    {
        CheckBox_SetChecked(inHandle, inChecked);
    }


    void ProgressMeter_Init(HWND inHandle, int inLimit)
    {
        ::SendMessage(inHandle, PBM_SETRANGE, 0, MAKELPARAM(0, inLimit));
        ::SendMessage(inHandle, PBM_SETSTEP, 1, 0);
        ::SendMessage(inHandle, PBM_SETPOS, 0, 0);
    }


    void ProgressMeter_Advance(HWND inHandle)
    {
        // When the position exceeds the maximum range value, this message
        // resets the current position so that the progress indicator starts
        // over again from the beginning.
        ::SendMessage(inHandle, PBM_STEPIT, 0, 0);
    }


    void ProgressMeter_SetProgress(HWND inHandle, int inProgress)
    {
        // Returns the previous position.
        ::SendMessage(inHandle, PBM_SETPOS, (WPARAM)inProgress, (LPARAM)0);
    }


    int ProgressMeter_GetProgress(HWND inHandle)
    {
        // Returns a UINT value that represents the current position of the
        // progress bar.
        return (int)::SendMessage(inHandle, PBM_GETPOS, (WPARAM)0, (LPARAM)0);
    }


    void Window_AddStyle(HWND inHandle, LONG inStyle)
    {
        if (0 == ::SetWindowLong(inHandle, GWL_STYLE, ::GetWindowLong(inHandle, GWL_STYLE) | inStyle))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    LONG Window_GetStyles(HWND inHandle)
    {
        return ::GetWindowLong(inHandle, GWL_STYLE);
    }


    void Window_SetStyle(HWND inHandle, LONG inStyle)
    {
        if (0 == ::SetWindowLong(inHandle, GWL_STYLE, inStyle))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void Window_RemoveStyle(HWND inHandle, LONG inStyle)
    {
        if (0 == ::SetWindowLong(inHandle, GWL_STYLE, ::GetWindowLong(inHandle, GWL_STYLE) & ~inStyle))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    bool Window_HasStyle(HWND inHandle, LONG inStyle)
    {
        return (::GetWindowLong(inHandle, GWL_STYLE) & inStyle) != 0;
    }


    void Window_SetVisible(HWND inHandle, bool inVisible)
    {
        // If the window was previously visible, the return value is nonzero.
        // If the window was previously hidden, the return value is zero.
        ::ShowWindow(inHandle, inVisible ? SW_SHOW : SW_HIDE);
    }


    bool Window_IsVisible(HWND inHandle)
    {
        return TRUE == ::IsWindowVisible(inHandle);
    }


    void TextBox_SetReadOnly(HWND inHandle, bool inReadOnly)
    {
        if (0 == ::SendMessage(inHandle, EM_SETREADONLY, inReadOnly ? TRUE : FALSE, 0))
        {
            ReportError("Failed to make a textbox readonly. Last error: " + System_GetLastError(::GetLastError()));
        }
    }


    bool TextBox_IsReadOnly(HWND inHandle)
    {
        return Window_HasStyle(inHandle, ES_READONLY);
    }


    void Scrollbar_SetInfo(HWND inHandle, int inTotalHeight, int inPageHeight, int inCurrentPosition)
    {
        SCROLLINFO si;
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = 0;
        si.nMax = inTotalHeight;
        si.nPage = inPageHeight;
        si.nPos = inCurrentPosition;
        si.nTrackPos = 0; // is ignored by SetScrollInfo

        // The return value is the current position of the scroll box.
        ::SetScrollInfo(inHandle, SB_CTL, &si, TRUE);
    }


    void Scrollbar_GetInfo(HWND inHandle, int & outTotalHeight, int & outPageHeight, int & outCurrentPosition)
    {
        SCROLLINFO si;
        ::ZeroMemory(&si, sizeof(si));
        si.cbSize = sizeof(SCROLLINFO);
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;

        // If the function does not retrieve any values, the return value is
        // zero. To get extended error information, call GetLastError.
        if (0 != ::GetScrollInfo(inHandle, SB_CTL, &si))
        {
            outTotalHeight = si.nMax;
            outPageHeight = si.nPage;
            outCurrentPosition = si.nPos;
        }
        else
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    int Scrollbar_GetPos(HWND inHandle)
    {
        return (int)::GetScrollPos(inHandle, SB_CTL);
    }


    void Scrollbar_SetPos(HWND inHandle, int inPos)
    {
        if (0 == ::SetScrollPos(inHandle, SB_CTL, inPos, TRUE))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void Tab_AddPanel(HWND inHandle, const std::string & inTitle)
    {
        TCITEM tabItem;
        tabItem.mask = TCIF_TEXT | TCIF_IMAGE;
        tabItem.iImage = -1;
        std::wstring text = ToUTF16(inTitle);
        tabItem.pszText = const_cast<LPWSTR>(text.c_str());
        if (-1 == TabCtrl_InsertItem(inHandle, TabCtrl_GetItemCount(inHandle), &tabItem))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void Menu_InsertItem(HMENU inMenuHandle, UINT inIndex)
    {
        MENUITEMINFO mii;
        memset(&mii, 0, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_FTYPE;
        mii.fType = MFT_SEPARATOR;
        if (0 == ::InsertMenuItem(inMenuHandle, inIndex, TRUE, &mii))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void Menu_InsertItem(HMENU inMenuHandle, UINT inIndex, UInt32 inComponentId, const std::string & inText)
    {
        MENUITEMINFO mii;
        memset(&mii, 0, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_FTYPE | MIIM_ID | MIIM_STRING;
        mii.fType = 0;

        std::wstring itemText = ToUTF16(inText);
        std::vector<TCHAR> buffer;
        buffer.resize(itemText.length() + 1);
        for (std::size_t i = 0; i < itemText.length(); i++)
        {
            buffer[i] = itemText[i];
        }
        buffer[itemText.length()] = 0;
        mii.dwTypeData = (LPTSTR)&buffer[0];
        mii.cch = (UINT)itemText.size();
        mii.wID = inComponentId;
        if (0 == ::InsertMenuItem(inMenuHandle, inIndex, TRUE, &mii))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void Menu_InsertSubMenu(HMENU inMenuHandle, UINT inIndex, HMENU inSubMenu, const std::string & inText)
    {
        MENUITEMINFO mii;
        memset(&mii, 0, sizeof(mii));
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_SUBMENU | MIIM_STRING;

        std::wstring text = ToUTF16(inText);
        std::vector<TCHAR> buffer;
        buffer.resize(text.length() + 1);
        for (std::size_t i = 0; i < text.length(); i++)
        {
            buffer[i] = text[i];
        }
        buffer[text.length()] = 0;
        mii.dwTypeData = (LPTSTR)&buffer[0];
        mii.cch = (UINT)inText.size();
        mii.hSubMenu = inSubMenu;

        if (0 == ::InsertMenuItem(inMenuHandle, inIndex, TRUE, &mii))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    HMENU Menu_Create(const MenuNode & inMenuNode)
    {
        HMENU result = CreateMenu();
        MenuNode::const_iterator it = inMenuNode.begin(), end = inMenuNode.end();
        for (; it != end; ++it)
        {
            MenuNode::ChildPtr subNode(*it);
            if (!subNode->empty())
            {
                Menu_InsertSubMenu(result, ::GetMenuItemCount(result), Menu_Create(*subNode), subNode->data().label);
            }
            else if (subNode->data().id != 0)
            {
                Menu_InsertItem(result, ::GetMenuItemCount(result), subNode->data().id, subNode->data().label);
            }
            else
            {
                Menu_InsertItem(result, ::GetMenuItemCount(result));
            }
        }
        return result;
    }


    void Menu_SetItemEnabled(HMENU inMenuHandle, UInt32 inComponentId, bool inEnabled)
    {
        // The return value specifies the previous state of the menu item (it
        // is either MF_DISABLED, MF_ENABLED, or MF_GRAYED). If the menu item
        // does not exist, the return value is -1.
        if (-1 == ::EnableMenuItem(inMenuHandle, inComponentId, MF_BYCOMMAND | inEnabled ? MF_ENABLED : MF_DISABLED | MF_GRAYED))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    void Menu_SetItemChecked(HMENU inMenuHandle, UInt32 inComponentId, bool inChecked)
    {
        // The return value specifies the previous state of the menu item (either MF_CHECKED or
        // MF_UNCHECKED). If the menu item does not exist, the return value is -1.
        if (-1 == ::CheckMenuItem(inMenuHandle, inComponentId, MF_BYCOMMAND | inChecked ? MF_CHECKED : MF_UNCHECKED))
        {
            ReportError(System_GetLastError(::GetLastError()));
        }
    }


    Timer::TimerMapping Timer::sTimerMapping;


    Timer::Timer() :
        mTimerId(0)
    {
    }


    Timer::~Timer()
    {
        stop();
    }


    void Timer::start(const TimerAction & inAction, int inDelayInMilliseconds)
    {
        if (!mTimerId)
        {
            mTimerAction = inAction;
            if (!(mTimerId = ::SetTimer(NULL, NULL, inDelayInMilliseconds, &Timer::OnTimerEvent)))
            {
                throw std::runtime_error("Failed to start the timer.");
            }            
            sTimerMapping.insert(std::make_pair(this, mTimerId));
        }
        else
        {
            throw std::logic_error("Timer already started.");
        }
    }


    void Timer::stop()
    {
        if (mTimerId)
        {
            TimerMapping::iterator it = sTimerMapping.find(this);
            assert(it != sTimerMapping.end());
            if (it != sTimerMapping.end())
            {
                sTimerMapping.erase(it);
                ::KillTimer(NULL, mTimerId);
                mTimerId = 0;
            }
        }
    }


    void Timer::OnTimerEvent(HWND inHandle, UINT inMessage, UINT_PTR inTimerId, DWORD inTime)
    {
        for (TimerMapping::iterator it = sTimerMapping.begin(); it != sTimerMapping.end(); ++it)
        {
            if (it->second == inTimerId)
            {
                it->first->mTimerAction();
                return;
            }
        }
    }


    UInt32 Toolbar_GetToolbarButtonSize(HWND inToolbarHandle, UInt32 inToolbarButtonId)
    {
        TBBUTTONINFO tbButtonInfo;
        ::ZeroMemory(&tbButtonInfo, sizeof(tbButtonInfo));
        tbButtonInfo.cbSize = sizeof(tbButtonInfo);
        tbButtonInfo.dwMask = TBIF_SIZE;
        ::SendMessage(inToolbarHandle, TB_GETBUTTONINFO, inToolbarButtonId, (LPARAM)&tbButtonInfo);
        return tbButtonInfo.cx;
    }


    RECT Toolbar_GetToolbarButtonRect(HWND inToolbarHandle, UInt32 inToolbarButtonId)
    {
        RECT result = {0, 0, 0, 0 };
        SendMessage(inToolbarHandle,
                    TB_GETRECT,
                    (WPARAM)inToolbarButtonId,
                    (LPARAM)&result);
        return result;
    }


    void Toolbar_SetButtonWidth(HWND inToolbarHandle, UInt32 inToolbarButtonId, int inWidth)
    {
        TBBUTTONINFO info;
        ZeroMemory(&info, sizeof(info));
        info.cbSize = sizeof(TBBUTTONINFO);
        info.dwMask  = TBIF_SIZE;
        info.cx = inWidth;
        if (!SendMessage(inToolbarHandle,
                         TB_SETBUTTONINFO,
                         (WPARAM)inToolbarButtonId,
                         (LPARAM)(LPTBBUTTONINFO)&info))
        {
            ReportError("Could not change the toolbar button width. Last error: " + WinAPI::System_GetLastError(::GetLastError()));
        }
    }


} // namespace WinAPI

} // namespace XULWin
