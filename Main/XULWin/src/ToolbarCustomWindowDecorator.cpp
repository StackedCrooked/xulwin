#include "XULWin/ToolbarCustomWindowDecorator.h"


namespace XULWin
{    
    
    ToolbarCustomWindowDecorator::ToolbarCustomWindowDecorator(Component * inDecoratedElement, boost::weak_ptr<Windows::ToolbarElement> inToolbar) :
        Decorator(inDecoratedElement),
        IECustomWindow(),
        mCustomWindow(0)
    {        
        if (NativeComponent * native = mDecoratedElement->downcast<NativeComponent>())
        {
            mCustomWindow = new ConcreteCustomWindow(inToolbar, mDecoratedElement->commandId(), native->handle());
        }        
    }


    int ToolbarCustomWindowDecorator::commandId() const
    {
        if (mCustomWindow)
        {
            mCustomWindow->commandId();
        }
        return 0;
    }
        
    
    bool ToolbarCustomWindowDecorator::hasFocus() const
    {
        if (mCustomWindow)
        {
            mCustomWindow->hasFocus();
        }
        return false;
    }


    void ToolbarCustomWindowDecorator::setFocus() const
    {
        if (mCustomWindow)
        {
            mCustomWindow->setFocus();
        }
    }

    
    HWND ToolbarCustomWindowDecorator::handle() const
    {
        if (mCustomWindow)
        {
            return mCustomWindow->handle();
        }
        return 0;
    }


    ToolbarCustomWindowDecorator::ConcreteCustomWindow::ConcreteCustomWindow(boost::weak_ptr<Windows::ToolbarElement> inToolbar, int inCommandId, HWND inHandle) :
        mToolbar(inToolbar),
        mHandle(inHandle),
        mCommandId(inCommandId)
    {
    }


    bool ToolbarCustomWindowDecorator::ConcreteCustomWindow::hasFocus() const
    {
        return ::GetFocus() == mHandle;
    }


    void ToolbarCustomWindowDecorator::ConcreteCustomWindow::setFocus() const
    {
        ::SetFocus(mHandle);
    }


    HWND ToolbarCustomWindowDecorator::ConcreteCustomWindow::handle() const
    {
        return mHandle;
    }


    int ToolbarCustomWindowDecorator::ConcreteCustomWindow::commandId() const
    {
        return mCommandId;
    }


    bool ToolbarCustomWindowDecorator::init()
    {
        if (mCustomWindow)
        {
            if (Toolbar * toolbar = parent()->downcast<Toolbar>())
            {
                mDecoratedElement->move(0, 0, mDecoratedElement->getWidth(), mDecoratedElement->getHeight());
                mDecoratedElement->rebuildLayout();
                toolbar->nativeToolbar()->add(mCustomWindow);
            }
        }
        return mDecoratedElement->init();
    }

} // namespace XULWin