#ifndef _236360_3_
#define _236360_3_

#include <vector>
#include <string>
using namespace std;

namespace output
{
    void endScope();

    void printID(const string& id, int offset, const string& type);

    /* Do not save the string returned from this function in a data structure
        as it is not dynamically allocated and will be destroyed(!) at the end of the calling scope.
    */
    string makeFunctionType(const string& retType, vector<string>& argTypes);

    [[noreturn]] void errorLex(int lineno);

    [[noreturn]] void errorSyn(int lineno);

    [[noreturn]] void errorUndef(int lineno, const string& id);

    [[noreturn]] void errorDef(int lineno, const string& id);

    [[noreturn]] void errorUndefFunc(int lineno, const string& id);

    [[noreturn]] void errorMismatch(int lineno);

    [[noreturn]] void errorPrototypeMismatch(int lineno, const string& id, vector<string>& argTypes);

    [[noreturn]] void errorUnexpectedBreak(int lineno);

    [[noreturn]] void errorUnexpectedContinue(int lineno);

    [[noreturn]] void errorMainMissing();

    [[noreturn]] void errorByteTooLarge(int lineno, const string& value);
}

#endif