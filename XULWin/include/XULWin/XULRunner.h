#ifndef XULRUNNER_H_INCLUDED
#define XULRUNNER_H_INCLUDED


#include "XULWin/Fallible.h"
#include "XULWin/XULParser.h"
#include "XULWin/Windows.h"
#include <string>


namespace XULWin
{

    class XULRunner
    {
    public:
        // Gets the previously set locale.
        // If no locale has been set en-US is returned.
        static std::string GetLocale();

        // Sets the desired locale (en-US, fr, nl, ...)
        // NOTE: the locale statements in the chrome.manifest file
        // are ignored, instead we depend on the foldernames in the chrome/locale directory.
        // This means that the locale value should be identical to its foldername.
        static void SetLocale(const std::string & inLocale);

        static HMODULE GetModuleHandle();

        static void SetModuleHandle(HMODULE inModuleHandle);

        /**
         * Searches for an image file with the name <inWindowId>.<inExtension>
         * in the directory <mozilla-directory>/chrome/icons/default/.
         *
         * See also https://developer.mozilla.org/en/xul/window.
         *
         * Params:
         * inAppDir		path of the application (parent directory of the chrome directory)
         * inWindowId	id attribute of the root window element
         * inExtension  extension of the image file found (supports .ico and all other GDI+ image types (png, bmp, jpeg, ...)
         */
        static HICON GetDefaultIcon(const std::string & inAppDir,
                                    const std::string & inWindowId,
                                    const std::string & inExtension);


        /**
         * Same as above without specifying the extension. 
         * Searches inside "chrome/icons/default" for WindowId.ico or WindowId.png.
         */
        static HICON GetDefaultIcon(const std::string & inWindowId,
                                    const std::string & inExtension);


        /**
         * Same as above but searches but auto-detects extension.
         */
        static HICON GetDefaultIcon(const std::string & inWindowId);

        XULRunner();

        XULRunner(HMODULE inModuleHandle);

        /**
         * Runs a XUL application.
         *
         * Root element must be <window>.
         */
        void run(const std::string & inApplicationIniFile);

        /**
         * loadApplication
         *
         * Loads a XUL application:
         *
         *  - Finds out which is the main XUL file. The location of the main XUL file
         *    must be specified in the file prefs.js which is found in the
         *    "<Application>/defaults/preferences" folder.
         *  - Parses the main XUL file.
         *  - Returns the root element of the parsed XUL file.
         */
        ElementPtr loadApplication(const std::string & inApplicationIniFile);


        /**
         * loadXULFromFile
         *
         * Load a XUL file:
         *
         *  - Finds the requested XUL file. This requires the
         *    chrome url to be translated to a native path.
         *  - Parses the XUL file.
         *  - Returns the root element of the parsed XUL file.
         */
        ElementPtr loadXULFromFile(const std::string & inXULUrl);


        /**
         * loadXULFromString
         *
         * Parses a XUL String.
         */
        ElementPtr loadXULFromString(const std::string & inXULString);


        /**
         * loadOverlay
         *
         * Loads a XUL overlay.
         *
         * The parameter is a chrome url to the overlay XUL document.
         * This document must have a <overlay> element as root element.
         * See the Mozilla documention on XUL overlays for more information.
         */
        void loadOverlay(const std::string & inXULUrl);

        ElementPtr rootElement() const;

        HMODULE getModuleHandle() const;

    private:
        static Fallible<std::string> sLocale;
        static HMODULE sModuleHandle;

        static ElementPtr ParseFile(AbstractXULParser & inParser, const std::string & inXULURL);
        static ElementPtr ParseString(AbstractXULParser & inParser, const std::string & inXULURL);
        std::string getOverlayElementId(const std::string & inXULURL);

        HMODULE mModuleHandle;
        XULParser mParser;
        ElementPtr mRootElement;
    };

} // XULWin


#endif // XULRUNNER_H_INCLUDED
