#ifndef ERRORREPORTER_H_INCLUDED
#define ERRORREPORTER_H_INCLUDED


#include <stack>
#include <vector>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>


namespace XULWin
{


    /**
     * Error contains error info.
     */
    class Error
    {
    public:
        Error(const std::string & inError);
        
        Error(const std::string & inError, const std::string & inFile, int inLine);

        const std::string & message() const;

        const std::string & file() const;

        int line() const;

    private:
        std::string mError;
        std::string mFile;
        int mLine;
    };


    /** Forward declaration */
    class ErrorReporter;


    /**
     * ErrorCatcher objects can be used to limit error handling to a certain scope.
     * A ErrorCatcher object should always be created on the stack, and usually at
     * the beginning of its surrounding scope.
     */
    class ErrorCatcher
    {
    public:
        ErrorCatcher();

        ErrorCatcher(const ErrorCatcher & inErrorCatcher);

        ~ErrorCatcher();

        /**
         * Logging is enabled by default and will occur on destruction
         * of the ErrorCatcher object. This call allows you to disable
         * this behavior.
         * This is handy for creating a 'error silencer' object.
         */
        void disableLogging(bool inDisable);


        /**
         * Returns true if an error was reported during the lifetime of this object
         * and not caught by a deeper nested ErrorCatcher object.
         */
        bool hasCaught() const;

        /**
         * On destruction the ErrorCatcher will log its error information.
         * However, you can also propagate the error, so that the next
         * ErrorCatcher up in the hierarchy will deal with it.
         */
        void propagate();

        /**
         * Gets the error messages.
         * Instead of calling this method everytime you can also set a logger
         * on the ErrorReporter instance.
         */
        void getError(std::stringstream & outStream) const;

        const std::vector<Error> & errors() const;

    private:
        friend class ErrorReporter;
        ErrorCatcher & operator =(const ErrorCatcher & inErrorCatcher);

        /**
         * Log the error message using the callback set on the ErrorReporter.
         */
        void log();

        void push(const Error & inError);

        void getErrorImpl(std::stringstream & outStream) const;

        bool mOwns;
        std::vector<Error> mErrors;
        bool mDisableLogging;
    };


    /**
     * ErrorReporter serves as a global stack for ErrorCatcher objects.
     */
    class ErrorReporter
    {
    public:
        /**
         * Initialize must be called once (and only once) at program startup, and
         * before any ErrorCatcher objects are created.
         */
        static void Initialize();


        /**
         * Returns the singleton object.
         */
        static ErrorReporter & Instance();

        /**
         * Finalize should be called before shutting down your program.
         */
        static void Finalize();

        typedef boost::function<void(const std::string &)> LogFunction;

        void setLogger(const LogFunction & inLogFunction);

        /**
         * You can use this method to report an error, however
         * the ReportError functions below are more convenient.
         * NOTE: Calling this will not cause an exception to be thrown, so
         *       program flow will not be altered. If you want to return to
         *       the caller you still have to write a return statement.
         */
        void reportError(const Error & inError);

    private:
        friend class ErrorCatcher;

        ErrorReporter();

        ~ErrorReporter();

        void push(ErrorCatcher * inError);

        void pop(ErrorCatcher * inError);

        void log(ErrorCatcher * inError);

        void log(const std::string & inError);

        std::stack<ErrorCatcher *> mErrorCatchers;
        LogFunction mLogFunction;
        bool mEnableMessageBoxLogging;
        static ErrorReporter * sInstance;
    };


    ///**
    // * Try to execute a function and log any caught exceptions.
    // */
    //typedef boost::function<void()> TryAction;
    //typedef boost::function<void(const std::exception &)> CatchAction;

    ////bool TryCatch(const TryAction & inAction, const CatchAction & inCatchAction);

    //bool TryOrReportError(const TryAction & inAction);

} // namespace XULWin


/**
 * ReportError is a helper function to quickly log a message.
 */
#define ReportError(msg) \
    ErrorReporter::Instance().reportError(XULWin::Error(msg, __FILE__, __LINE__))


#endif // ERRORREPORTER_H_INCLUDED
