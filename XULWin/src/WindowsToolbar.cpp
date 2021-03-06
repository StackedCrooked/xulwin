#include "XULWin/WindowsToolbar.h"
#include "XULWin/ErrorReporter.h"
#include "XULWin/Gdiplus.h"
#include "XULWin/Unicode.h"
#include "XULWin/WindowsToolbarItem.h"
#include "XULWin/WinUtils.h"
#include <boost/bind.hpp>
#include <commctrl.h>


namespace XULWin
{

    namespace WinAPI
    {

        extern const int cDownArrowWidth = 10;
        extern const int cMarginForCustomWindow = 4;
        extern const int cSpacingBetweenIconAndText = 4;

        WindowsToolbar::ParentMapping WindowsToolbar::sInstancesParent;
        WindowsToolbar::InstanceMapping WindowsToolbar::sInstances;

        WindowsToolbar::WindowsToolbar (EventHandler * inEventHandler, HMODULE inModuleHandle, HWND inParentWindow, int inID) :
            mEventHandler(inEventHandler),
            mModuleHandle(inModuleHandle),
            mParentWindow(inParentWindow),
            mID(inID),
            mHandle(0),
            mParentProc(0),
            mToolbarProc(0),
            mActiveDropDown(0),
            mIsBuilt(false)
        {
            mHandle = CreateWindowEx(0,
                                     TOOLBARCLASSNAME,
                                     0,
                                     WS_CHILD
                                      | TBSTYLE_FLAT
                                      | WS_CLIPSIBLINGS
                                      | WS_CLIPCHILDREN
                                      | TBSTYLE_TRANSPARENT
                                      | TBSTYLE_LIST
                                      | TBSTYLE_TOOLTIPS
                                      | CCS_NODIVIDER
                                      | CCS_NOPARENTALIGN
                                      | CCS_NORESIZE
                                      | CCS_TOP
                                      | CCS_NODIVIDER
                                      | CCS_NORESIZE,
                                     0, 0, 0, 0,
                                     mParentWindow,
                                     (HMENU)(INT_PTR)inID,
                                     mModuleHandle,
                                     0);
            ::SendMessage(mHandle, TB_BUTTONSTRUCTSIZE, (WPARAM) sizeof(TBBUTTON), 0);
            mFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);

            mParentProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(mParentWindow, GWLP_WNDPROC, (LONG)(LONG_PTR)WindowsToolbar::ParentProc);
            sInstancesParent.insert(std::make_pair(this, mParentWindow));

            mToolbarProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(mHandle, GWLP_WNDPROC, (LONG)(LONG_PTR)WindowsToolbar::ToolbarProc);
            sInstances.insert(std::make_pair(this, mHandle));
        }


        WindowsToolbar::~WindowsToolbar ()
        {
            if (mActiveDropDown)
            {
                mActiveDropDown = 0;
            }

            mToolbarItems.clear();

            (WNDPROC)(LONG_PTR)SetWindowLongPtr(mHandle, GWLP_WNDPROC, (LONG)(LONG_PTR)mToolbarProc);

            (WNDPROC)(LONG_PTR)SetWindowLongPtr(mParentWindow, GWLP_WNDPROC, (LONG)(LONG_PTR)mParentProc);

            InstanceMapping::iterator toolbarIt = sInstances.find(this);
            bool foundThisToolbar = toolbarIt != sInstances.end();
            assert(foundThisToolbar);
            if (foundThisToolbar)
            {
                sInstances.erase(toolbarIt);
            }

            ParentMapping::iterator parentIt = sInstancesParent.find(this);
            bool foundParent = parentIt != sInstancesParent.end();
            assert(foundParent);
            if (foundParent)
            {
                sInstancesParent.erase(parentIt);
            }

            if (mHandle)
            {
                ::DeleteObject(mHandle);
                mHandle = 0;
            }
        }


        void WindowsToolbar::rebuildLayout()
        {
            if (!mIsBuilt)
            {
                return;
            }

            if (!mToolbarItems.empty())
            {
                updateToolbarButtonSizes(mHandle, mFont, mToolbarItems);
            }

            int toolbarSpringID = -1;
            for (ToolbarItems::const_iterator it = mToolbarItems.begin(); it != mToolbarItems.end(); ++it)
            {
                const AbstractToolbarItem * item = it->get();
                if (dynamic_cast<const ToolbarSpring *>(item))
                {
                    toolbarSpringID = item->componentId();
                }
            }
            if (toolbarSpringID != -1)
            {
                // apply the middle divide
                applySpring(mHandle, mToolbarItems, toolbarSpringID);
            }

            // Do it again to update the position of any
            // custom windows that come after the spring.
            if (!mToolbarItems.empty())
            {
                updateToolbarButtonSizes(mHandle, mFont, mToolbarItems);
            }

            for (size_t idx = 0; idx != mToolbarItems.size(); ++idx)
            {
                mToolbarItems[idx]->onPostRebuildLayout();
            }
        }


        void WindowsToolbar::buildToolbar()
        {
            buildToolbar(mHandle,
                         mFont,
                         mToolbarItems,
                         mCustomWindowPositions);

            for (size_t idx = 0; idx != mToolbarItems.size(); ++idx)
            {
                mToolbarItems[idx]->onPostRebuildLayout();
            }
            mIsBuilt = true;
        }


        void WindowsToolbar::buildToolbar(HWND inToolbarHandle, HFONT hFont, const ToolbarItems & inToolbarItems, CustomWindowPositions & outCustomWindowPositions)
        {
            std::vector< TBBUTTON > theToolbarButtons;

            // we need to remember the position of the ToolbarSpring
            // index value of -1 means that none was found
            int toolbarSpringID = -1;

            ToolbarItems::const_iterator it = inToolbarItems.begin(), end = inToolbarItems.end();
            for (; it != end; ++it)
            {
                const AbstractToolbarItem * item = it->get();
                if (dynamic_cast<const ToolbarSpring *>(item))
                {
                    toolbarSpringID = item->componentId();
                }

                TBBUTTON theToolbarButton;
                memset(&theToolbarButton, 0, sizeof(theToolbarButton));
                theToolbarButton.idCommand = item->componentId();
                theToolbarButton.dwData = 0;
                if (const IECustomWindow * customWindow = dynamic_cast<const IECustomWindow *>(item)) // if custom window
                {
                    // make room for custom window
                    RECT rc;
                    GetClientRect(customWindow->handle(), &rc);
                    theToolbarButton.fsState = TBSTATE_ENABLED;
                    theToolbarButton.fsStyle = BTNS_SEP;
                    theToolbarButton.iBitmap = rc.right-rc.left + 2*cMarginForCustomWindow; // if fsStyle is set to BTNS_SEP, iBitmap determines the width of the separator, in pixels.
                    theToolbarButton.iString = -1;
                    if (FALSE == SendMessage(inToolbarHandle, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)(LPTBBUTTON) &theToolbarButton))
                    {
                        std::string lastError = WinAPI::System_GetLastError(::GetLastError());
                        ReportError("Failed to add the toolbar button. Reason: " + lastError);
                    }
                }
                else
                {
                    theToolbarButton.iBitmap = I_IMAGENONE;
                    theToolbarButton.fsState = TBSTATE_ENABLED;
                    theToolbarButton.fsStyle = item->flags() | (item->flags() & BTNS_SEP ? 0 : BTNS_AUTOSIZE);

                    theToolbarButton.iString = -1;

                    // Add the button to the toolbar
                    if (FALSE == SendMessage(inToolbarHandle, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)(LPTBBUTTON) &theToolbarButton))
                    {
                        std::string lastError = WinAPI::System_GetLastError(::GetLastError());
                        ReportError("Failed to add the toolbar button. Reason: " + lastError);
                    }
                }
                theToolbarButtons.push_back(theToolbarButton);
            }

            if (!theToolbarButtons.empty())
            {
                updateToolbarButtonSizes(inToolbarHandle, hFont, inToolbarItems);
            }

            if (toolbarSpringID != -1)
            {
                // apply the middle divide
                applySpring(inToolbarHandle, inToolbarItems, toolbarSpringID);
            }

            if (!theToolbarButtons.empty())
            {
                updateToolbarButtonSizes(inToolbarHandle, hFont, inToolbarItems);
            }
        }


        void WindowsToolbar::updateToolbarButtonSizes(HWND inToolbarHandle, HFONT hFont, const ToolbarItems & inToolbarItems)
        {
            SendMessage((HWND) inToolbarHandle, (UINT) TB_SETEXTENDEDSTYLE, (WPARAM)0, (LPARAM) TBSTYLE_EX_DRAWDDARROWS);
            int maxButtonHeight = 0;

            // Position custom windows and resize buttons
            int offset_x = 0;
            ToolbarItems::const_iterator it = inToolbarItems.begin(), end = inToolbarItems.end();
            for (; it != end; ++it)
            {
                const AbstractToolbarItem * abstractItem = it->get();

                if (const IECustomWindow * customWindow = dynamic_cast<const IECustomWindow *>(abstractItem)) // if custom window
                {
                    DWORD buttonSize = (DWORD)SendMessage(inToolbarHandle, TB_GETBUTTONSIZE, 0, 0);
                    int toolbarHeight = HIWORD(buttonSize);
                    RECT rc;
                    ::GetWindowRect(customWindow->handle(), &rc);
                    if (maxButtonHeight < rc.bottom - rc.top)
                    {
                        maxButtonHeight = rc.bottom - rc.bottom;
                    }

                    RECT rcToolbar;
                    ::GetClientRect(inToolbarHandle, &rcToolbar);

                    offset_x += cMarginForCustomWindow;
                    RECT position;
                    position.left = rcToolbar.left + offset_x;
                    position.top = rcToolbar.top + (toolbarHeight-(rc.bottom-rc.top))/2;
                    position.right = position.left + rc.right-rc.left;
                    position.bottom = position.top + rc.bottom-rc.top;
                    ::MoveWindow(customWindow->handle(), position.left, position.top, position.right-position.left, position.bottom-position.top, TRUE);
                    offset_x = position.right + cMarginForCustomWindow;
                }
                else if (const ToolbarSpring * spring = dynamic_cast<const ToolbarSpring *>(abstractItem))
                {
                    // Do nothing.
                }
                else if (const ConcreteToolbarItem * item = dynamic_cast<const ConcreteToolbarItem *>(abstractItem)) // if normal button
                {
                    TBBUTTONINFO buttonInfo;
                    buttonInfo.cbSize = sizeof(TBBUTTONINFO);

                    buttonInfo.cx = item->getLeftMargin() + item->getRightMargin();
                    if (!item->text().empty())
                    {
                        buttonInfo.cx += (WORD)WinAPI::Window_GetTextSize(inToolbarHandle, item->text()).cx;
                    }
                    if (item->image())
                    {
                        double resizeFactor = static_cast<double>(std::min<size_t>(item->maxIconHeight(), item->image()->GetHeight()))/static_cast<double>(item->image()->GetHeight());
                        size_t w = static_cast<size_t>(static_cast<double>(item->image()->GetWidth() * resizeFactor) + 0.5);
                        buttonInfo.cx += w;
                        if (!item->text().empty())
                        {
                            buttonInfo.cx += cSpacingBetweenIconAndText;
                        }
                        if (item->image()->GetHeight() > (UINT)maxButtonHeight)
                        {
                            maxButtonHeight = item->image()->GetHeight();
                        }
                    }
                    if (dynamic_cast<const ToolbarDropDown *>(item))
                    {
                        buttonInfo.cx += cDownArrowWidth;
                    }

                    // Finally, make the button width an even number, so that alignment of icons is neatly centered
                    if (buttonInfo.cx % 2 != 0)
                    {
                        buttonInfo.cx += 1;
                    }

                    // If the resulting width is smaller than the min width, then set to min width
                    if (buttonInfo.cx < item->minimumWidth())
                    {
                        buttonInfo.cx = item->minimumWidth();
                    }

                    // If the resulting width is wider than the max width, then set to max width
                    if (buttonInfo.cx > item->maximumWidth())
                    {
                        buttonInfo.cx = item->maximumWidth();
                    }

                    buttonInfo.dwMask  = TBIF_SIZE;
                    ::SendMessage(inToolbarHandle, TB_SETBUTTONINFO, (WPARAM)item->componentId(), (LPARAM)(LPTBBUTTONINFO) &buttonInfo);
                    offset_x += buttonInfo.cx;
                }
            }
            ::SendMessage(inToolbarHandle, TB_SETBUTTONSIZE, 0, MAKELPARAM(0, maxButtonHeight));
        }


        WindowsToolbar::ToolbarItems::const_iterator WindowsToolbar::findByCommandID(const ToolbarItems & inToolbarItems, UInt32 inComponentId)
        {
            ToolbarItems::const_iterator it = inToolbarItems.begin(), end = inToolbarItems.end();
            for (; it != end; ++it)
            {
                const AbstractToolbarItem * item = it->get();
                if (item->componentId() == inComponentId)
                {
                    break;
                }
            }
            return it;
        }


        void WindowsToolbar::setActiveDropDownToNull()
        {
            mActiveDropDown = 0;
        }


        void WindowsToolbar::applySpring(HWND inToolbarHandle, const ToolbarItems & inToolbarItems, int inSpringId)
        {
            ToolbarItems::const_iterator it = std::find_if(inToolbarItems.begin(), inToolbarItems.end(), boost::bind(&AbstractToolbarItem::componentId, _1) == inSpringId);
            ToolbarItems::const_iterator end = inToolbarItems.end();
            assert(it != end);
            if (it != end)
            {
                RECT rc = {0, 0, 0, 0};
                GetClientRect(::GetParent(inToolbarHandle), &rc);
                int actualWidth = rc.right - rc.left;

                assert(inSpringId >= 1);

                RECT rcItemBeforeSpring = {0, 0, 0, 0};
                ToolbarItems::const_iterator itemBeforeSpring = findByCommandID(inToolbarItems, inSpringId-1);
                if (itemBeforeSpring != end)
                {
                    SendMessage(inToolbarHandle, TB_GETRECT, (WPARAM)(*itemBeforeSpring)->componentId(), (LPARAM)&rcItemBeforeSpring);
                }

                RECT rcItemAfterSpring = {0, 0, 0, 0};
                ToolbarItems::const_iterator itemAfterSpring = findByCommandID(inToolbarItems, inSpringId+1);
                if (itemAfterSpring == end)
                {
                    return;
                }
                SendMessage(inToolbarHandle, TB_GETRECT, (WPARAM)(*itemAfterSpring)->componentId(), (LPARAM)&rcItemAfterSpring);

                RECT rcLast = {0, 0, 0, 0};
                ToolbarItems::const_reverse_iterator lastItem = inToolbarItems.rbegin();
                if (lastItem != inToolbarItems.rend())
                {
                    SendMessage(inToolbarHandle, TB_GETRECT, (WPARAM)(*lastItem)->componentId(), (LPARAM)&rcLast);
                }

                int width = actualWidth - rcItemBeforeSpring.right;
                width -= (rcLast.right-rcItemAfterSpring.left);

                Toolbar_SetButtonWidth(inToolbarHandle, inSpringId, width);
            }
        }


        HMODULE WindowsToolbar::moduleHandle() const
        {
            return mModuleHandle;
        }


        HWND WindowsToolbar::handle() const
        {
            return mHandle;
        }


        bool WindowsToolbar::hasFocus() const
        {
            if (::GetFocus() == mHandle)
            {
                return true;
            }
            else
            {
                ToolbarItems::const_iterator it = mToolbarItems.begin(), end = mToolbarItems.end();
                for (; it != end; ++it)
                {
                    const AbstractToolbarItem * item = it->get();
                    if (const IFocusableToolbarItem * focusableItem = dynamic_cast<const IFocusableToolbarItem *>(item))
                    {
                        if (focusableItem->hasFocus())
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }


        void WindowsToolbar::setFocus()
        {
            ToolbarItems::const_iterator it = mToolbarItems.begin(), end = mToolbarItems.end();
            for (; it != end; ++it)
            {
                const AbstractToolbarItem * item = it->get();
                if (const IFocusableToolbarItem * focusableItem = dynamic_cast<const IFocusableToolbarItem *>(item))
                {
                    if (!focusableItem->hasFocus())
                    {
                        focusableItem->setFocus();
                        break;
                    }
                }
            }
        }


        AbstractToolbarItem * WindowsToolbar::getToolbarItemByCommandId(UInt32 inComponentId)
        {
            ToolbarItems::iterator it = std::find_if(mToolbarItems.begin(), mToolbarItems.end(), boost::bind(&AbstractToolbarItem::componentId, _1) == inComponentId);
            if (it != mToolbarItems.end())
            {
                return it->get();
            }
            return 0;
        }


        const AbstractToolbarItem * WindowsToolbar::getToolbarItemByCommandId(UInt32 inComponentId) const
        {
            ToolbarItems::const_iterator it = std::find_if(mToolbarItems.begin(), mToolbarItems.end(), boost::bind(&AbstractToolbarItem::componentId, _1) == inComponentId);
            if (it != mToolbarItems.end())
            {
                return it->get();
            }
            return 0;
        }


        void WindowsToolbar::add(AbstractToolbarItem * inToolbarItem)
        {
            ToolbarItems::iterator it = std::find_if(mToolbarItems.begin(), mToolbarItems.end(), boost::bind(&AbstractToolbarItem::componentId, _1) == inToolbarItem->componentId());
            bool found = it != mToolbarItems.end();
            assert(!found);
            if (!found)
            {
                boost::shared_ptr<AbstractToolbarItem> item(inToolbarItem);
                mToolbarItems.push_back(item);
            }
        }


        size_t WindowsToolbar::size() const
        {
            return mToolbarItems.size();
        }


        bool WindowsToolbar::empty() const
        {
            return mToolbarItems.empty();
        }


        void WindowsToolbar::remove(size_t inIndex)
        {
            ToolbarItems::iterator it = mToolbarItems.begin() + inIndex, end = mToolbarItems.end();
            bool found = it != end;
            assert(found);
            if (found)
            {
                mToolbarItems.erase(it);
                ::SendMessage(mHandle, TB_DELETEBUTTON, (LPARAM)inIndex, (WPARAM)0);
            }
        }


        void WindowsToolbar::clear()
        {
            mActiveDropDown = 0;
            while (!empty())
            {
                remove(0);
            }
            assert(empty());
        }


        void WindowsToolbar::enable(size_t inIndex)
        {
            int componentId = get(inIndex)->componentId();
            ::SendMessage(mHandle, TB_ENABLEBUTTON, (WPARAM)componentId, (LPARAM)MAKELONG(TRUE, 0));
        }


        void WindowsToolbar::disable(size_t inIndex)
        {
            int componentId = get(inIndex)->componentId();
            ::SendMessage(mHandle, TB_ENABLEBUTTON, (WPARAM)componentId, (LPARAM)MAKELONG(FALSE, 0));
        }


        const AbstractToolbarItem * WindowsToolbar::get(size_t inIndex) const
        {
            return mToolbarItems[inIndex].get();
        }


        AbstractToolbarItem * WindowsToolbar::get(size_t inIndex)
        {
            return mToolbarItems[inIndex].get();
        }


        LRESULT CALLBACK WindowsToolbar::ToolbarProc(HWND hWnd, UINT inMessage, WPARAM wParam, LPARAM lParam)
        {
            InstanceMapping::iterator it = sInstances.begin(), end = sInstances.end();
            for (; it != end; ++it)
            {
                if (it->second == hWnd)
                {
                    break;
                }
            }

            if (it == sInstances.end())
            {
                assert(false);
                return DefWindowProc(hWnd, inMessage, wParam, lParam);
            }

            WindowsToolbar * pThis = it->first;

            switch (inMessage)
            {
                case WM_SIZE:
                {
                    RECT rc;
                    ::GetClientRect(pThis->handle(), &rc);
                    for (size_t idx = 0; idx != pThis->mToolbarItems.size(); ++idx)
                    {
                        pThis->mToolbarItems[idx]->onClientRectChanged(rc);
                    }
                    pThis->rebuildLayout();
                    return 0;
                }
                case WM_COMMAND:
                {
                    HWND sender = (HWND)lParam;
                    WORD code = HIWORD(wParam);
                    WORD paramLo = LOWORD(wParam);
                    if (code == EN_SETFOCUS)
                    {
                        pThis->mEventHandler->onRequestFocus();
                        return 0;
                    }
                    break;
                }
            }
            return CallWindowProc(pThis->mToolbarProc, hWnd, inMessage, wParam, lParam);
        }


        LRESULT CALLBACK WindowsToolbar::ParentProc(HWND hWnd, UINT inMessage, WPARAM wParam, LPARAM lParam)
        {
            ParentMapping::iterator it = sInstancesParent.begin(), end = sInstancesParent.end();
            for (; it != end; ++it)
            {
                if (it->second == hWnd)
                {
                    break;
                }
            }
            if (it == sInstancesParent.end())
            {
                assert(false);
                return DefWindowProc(hWnd, inMessage, wParam, lParam);
            }

            WindowsToolbar * pThis = it->first;

            switch (inMessage)
            {
                case WM_NOTIFY:
                {
                    LPNMHDR lpnm = (LPNMHDR)lParam;
                    LPNMTOOLBAR lpnmTB = (LPNMTOOLBAR)lParam;

                    if (lpnm->code == TTN_GETDISPINFO)
                    {
                        LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lParam;
                        ToolbarItems::const_iterator it = pThis->findByCommandID(pThis->mToolbarItems, lpttt->hdr.idFrom);
                        bool found = it != pThis->mToolbarItems.end();
                        assert(found);
                        if (found)
                        {
                            AbstractToolbarItem * abstractItem = it->get();
                            if (ConcreteToolbarItem * item = dynamic_cast<ConcreteToolbarItem *>(abstractItem))
                            {
                                if (!item->tooltipText().empty())
                                {
                                    static std::wstring fText;
                                    fText = ToUTF16(item->tooltipText());
                                    lpttt->lpszText = const_cast<TCHAR *>(fText.c_str());
                                    lpttt->hinst = pThis->moduleHandle();
                                }
                            }
                        }
                    }
                    else if (lpnm->hwndFrom == pThis->handle() || lpnmTB->hdr.hwndFrom == pThis->handle())
                    {
                        switch (lpnm->code)
                        {
                            case TBN_DROPDOWN:
                            {
                                ToolbarItems::const_iterator it = pThis->findByCommandID(pThis->mToolbarItems, lpnmTB->iItem);
                                bool found = it != pThis->mToolbarItems.end();
                                assert(found);
                                if (found)
                                {
                                    ToolbarDropDown * item = dynamic_cast<ToolbarDropDown *>(it->get());
                                    assert(item);
                                    if (item)
                                    {
                                        RECT itemRect = item->getRect();
                                        ::MapWindowPoints(pThis->handle(), HWND_DESKTOP, (LPPOINT)&itemRect, 2);
                                        item->showToolbarMenu(itemRect);
                                    }
                                }
                                break;
                            }
                            case NM_CUSTOMDRAW:
                            {
                                LPNMTBCUSTOMDRAW customDrawMessage = (LPNMTBCUSTOMDRAW)lParam;
                                LPNMCUSTOMDRAW customDrawOldMessage = (LPNMCUSTOMDRAW)lParam;
                                LRESULT result = 0;
                                //Version 4.70. Pointer to an NMCUSTOMDRAW structure that contains
                                //information about the drawing operation. The dwItemSpec member
                                //of this structure contains the command identifier of the item
                                //being drawn. The lItemlParam member of this structure contains
                                //the dwData value for the item being drawn.

                                //switch(customDrawMessage->nmcd.dwDrawStage)
                                switch (customDrawOldMessage->dwDrawStage)
                                {
                                    case CDDS_PREPAINT:
                                    {
                                        return CDRF_NOTIFYITEMDRAW;
                                    }
                                    case CDDS_ITEMPREPAINT:
                                    {
                                        ToolbarItems::const_iterator it = pThis->findByCommandID(pThis->mToolbarItems, lpnmTB->cchText);
                                        bool found = it != pThis->mToolbarItems.end();
                                        if (found)
                                        {
                                            AbstractToolbarItem * abstractItem = it->get();
                                            if (ConcreteToolbarItem * item = dynamic_cast<ConcreteToolbarItem *>(abstractItem))
                                            {
                                                if (item->noHover())
                                                {
                                                    item->draw(customDrawMessage->nmcd.hdc, customDrawMessage->nmcd.rc, pThis->mFont, WinAPI::Window_GetTextSize(pThis->handle(), item->text()));
                                                    return CDRF_SKIPDEFAULT;
                                                }
                                            }
                                        }
                                        return CDRF_NOTIFYPOSTPAINT;
                                    }
                                    case CDDS_ITEMPOSTPAINT:
                                    {
                                        ToolbarItems::const_iterator it = pThis->findByCommandID(pThis->mToolbarItems, lpnmTB->cchText);
                                        bool found = it != pThis->mToolbarItems.end();
                                        if (!found)
                                        {
                                            break;
                                        }
                                        AbstractToolbarItem * abstractItem = it->get();
                                        if (ConcreteToolbarItem * item = dynamic_cast<ConcreteToolbarItem *>(abstractItem))
                                        {
                                            item->draw(customDrawMessage->nmcd.hdc, customDrawMessage->nmcd.rc, pThis->mFont, WinAPI::Window_GetTextSize(pThis->handle(), item->text()));
                                        }
                                        return CDRF_DODEFAULT;
                                    }
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }
            return CallWindowProc(pThis->mParentProc, hWnd, inMessage, wParam, lParam);
        }


    } // namespace WinAPI

} // namespace XULWin
