#ifndef QUICKBOMANAGER_H
#define QUICKBOMANAGER_H

#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include <initializer_list>
#include <regex>

namespace ngp {
namespace bo {
class BOProject;
class BOSurvey;
class BOLine;
class BOWell;
class BOHorizon2d;
class BOHorizon3d;
class BOFault2d;
class BOFault3d;
class BOSeismic;
class BOSeisCube;
}
}

namespace qk {
using stringlist_t = std::vector<std::string>;
using baid_t = long long;

std::vector<std::string> getDatabases() ;
baid_t toBAID(const std::string& str, bool *ok = nullptr);
std::string toBAIDString(baid_t id) ;
stringlist_t splitToken(const std::string& str, const std::string& tok = "/");

template<class T>
extern std::shared_ptr<T>  getBODataT(const stringlist_t& list,std::string *error) ;


template<class T>
std::shared_ptr<T>  getBODataT(const std::initializer_list<std::string>& list,std::string *error) {
    stringlist_t r;
    for(const auto& i : list) {
        auto v = splitToken(i);
        std::copy(v.begin(), v.end(), std::back_inserter(r));
    }
    return qk::getBODataT<T>(r, error);
}

template<class T>
std::shared_ptr<T>  getBODataT(const std::initializer_list<std::string>& list) {
    return qk::getBODataT<T>(list, nullptr);
}


template<class T>
std::shared_ptr<T>  getBODataT(const stringlist_t& list) {
    return qk::getBODataT<T>(list, nullptr);
}

template<class T>
std::shared_ptr<T>  getBODataT(const std::string & str,std::string *error) {
    auto list = splitToken(str);
    return qk::getBODataT<T>(list, error);
}

template<class T>
std::shared_ptr<T>  getBODataT(const std::string & str) {
    return qk::getBODataT<T>(str,nullptr);
}

} //namespsace qk


namespace qk{

std::shared_ptr<ngp::bo::BOProject>     operator"" _p(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOSurvey>      operator"" _s(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOLine>        operator"" _l(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOWell>        operator"" _w(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOHorizon2d>   operator"" _h2(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOHorizon3d>   operator"" _h3(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOFault2d>     operator"" _f2(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOFault3d>     operator"" _f3(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOSeismic>     operator"" _se(const char * s, size_t n);
std::shared_ptr<ngp::bo::BOSeisCube>    operator"" _sc(const char * s, size_t n);

}


#endif // QUICKBOMANAGER_H

