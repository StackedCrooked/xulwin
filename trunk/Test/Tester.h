#ifndef TESTER_H_INCLUDED
#define TESTER_H_INCLUDED


#include <string>


namespace XULWin
{

    class Tester
    {
    public:
        void runXULSample(const std::string & inAppName);

        void runNonXULSample();

    private:
        std::string mPathToXULRunnerSamples;
    };


} // namespace XULWin


#endif // TESTER_H_INCLUDED
