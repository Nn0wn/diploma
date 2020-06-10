#ifndef FLTTOSTR_H
#define FLTTOSTR_H

#include <cfloat>
#include <string>
#include <iomanip>
#include <QString>
#include <sstream>

namespace {
class FltToStr
{
#if __cplusplus >= 201703L
    inline static int fltMantDig = FLT_MANT_DIG;
    inline static int fltMinExp = FLT_MIN_EXP;
    inline static int additDig = 3;
#else
    static int fltMantDig;
    static int fltMinExp;
    static int additDig;
#endif

    static inline int maxStrFltSise()
    { return fltMantDig - fltMinExp + additDig; }

public:
    static std::string toStdString(float val)
    {
        std::ostringstream stream;
        stream << std::fixed;
        stream << std::setprecision(maxStrFltSise());
        stream << val;
        std::string str = stream.str();
        size_t pos = str.size();
        for(auto it = str.rbegin(); *it == '0'; ++it)
        {   --pos;  }
        str.erase(pos, str.size() - pos);
        return str;
    }

    static QString toQString(float val)
    {
        QString str = QString::number(static_cast<double>(val), 'f', maxStrFltSise());
        int pos = str.size();
        for(auto it = str.rbegin(); *it == '0'; ++it)
        {   --pos;  }
        str.remove(pos, str.size() - pos);
        return str;
    }
};

#if __cplusplus < 201703L
    int FltToStr::fltMantDig = FLT_MANT_DIG;
    int FltToStr::fltMinExp = FLT_MIN_EXP;
    int FltToStr::additDig = 3;
#endif
}


#endif // FLTTOSTR_H
