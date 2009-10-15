#include "XULWin/ListBoxImpl.h"
#include "XULWin/WinUtils.h"


namespace XULWin
{


    ListBoxImpl::ListBoxImpl(ElementImpl * inParent, const AttributesMapping & inAttributesMapping) :
        NativeControl(inParent, inAttributesMapping, TEXT("LISTBOX"), WS_EX_CLIENTEDGE, 0),
        mRows(0)
    {
        mRows.setInvalid();
    }

    
    bool ListBoxImpl::initImpl()
    {
        return Super::initImpl();
    }


    bool ListBoxImpl::initAttributeControllers()
    {
        setAttributeController("label", static_cast<LabelController*>(this));
        setAttributeController("rows", static_cast<RowsController*>(this));
        return Super::initAttributeControllers();
    }
    
    
    int ListBoxImpl::getRows() const
    {
        return mRows.or(1);
    }

    
    void ListBoxImpl::setRows(int inRows)
    {
        mRows = inRows;
    }


    int ListBoxImpl::calculateWidth(SizeConstraint inSizeConstraint) const
    {
        int result = 0;
        for (size_t idx = 0; idx != getChildCount(); ++idx)
        {
            int width = getChild(idx)->calculateWidth(inSizeConstraint);
            if (width > result)
            {
                result = width;
            }
        }
        return result;
    }


    int ListBoxImpl::calculateHeight(SizeConstraint inSizeConstraint) const
    {
        int result = 0;
        int itemCount = mRows.or(getChildCount());
        for (size_t idx = 0; idx != itemCount; ++idx)
        {
            if (idx < getChildCount())
            {
                result += getChild(idx)->calculateHeight(inSizeConstraint);
            }
            else
            {
                result += getChild(0)->calculateHeight(inSizeConstraint);
            }
        }
        int extraHeight = Windows::getSizeDifferenceBetweenWindowRectAndClientRect(handle()).cy;
        return result + extraHeight;
    }
    
} // namespace XULWin