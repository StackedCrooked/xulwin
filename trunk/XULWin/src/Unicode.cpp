#include "XULWin/Unicode.h"
#include "Poco/UnicodeConverter.h"


namespace XULWin
{

    std::string ToUTF8(const std::wstring & inText)
    {
        std::string result;
        Poco::UnicodeConverter::toUTF8(inText, result);
        return result;
    }


    std::wstring ToUTF16(const std::string & inText)
    {
        std::wstring result;
        Poco::UnicodeConverter::toUTF16(inText, result);
        return result;
    }

} // namespace XULWin
