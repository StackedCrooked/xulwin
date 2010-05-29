#include "XULWin/ListColElement.h"
#include "XULWin/ListCol.h"
#include "XULWin/Decorator.h"


namespace XULWin
{

    ListColElement::ListColElement(Element * inParent, const AttributesMapping & inAttributesMapping) :
        Element(ListColElement::TagName(),
                inParent,
                new ListCol(inParent->component(), inAttributesMapping))
    {
    }


    bool ListColElement::init()
    {
        return Element::init();
    }

} // namespace XULWin
