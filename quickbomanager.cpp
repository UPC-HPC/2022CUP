#include "quickbomanager.h"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <regex>
#include <map>
#include <sstream>
#include <string>
#include <cassert>
#include <charconv>


#include <baquery.h>
#include <bodefinitions.h>

//mgr
#include <bosystemroot.h>
#include <boproject.h>
#include <bosurvey.h>
#include <boline.h>

//well
#include <bowell.h>
#include <boage.h>
#include <bowellcurve.h>
#include <bocycle.h>
#include <bologfacies.h>
#include <bowellformation.h>
#include <bogram.h>
#include <bowelllith.h>
#include <bowellogw.h>
#include <bowellpath.h>
#include <bowavelet.h>

//seis
#include <boseismic.h>
#include <boseiscube.h>
#include <boseismicset.h>

//intp
#include <bohorizon2d.h>
#include <bohorizon3d.h>
#include <bofault2d.h>
#include <bofault3d.h>

//geometry
#include <bobinsetinfo.h> //2d->BOLine 3d->BOSurvey
#include <bosourceline.h>
#include <bogeophoneline.h>
#include <bobinsetline.h> //bobinsetinfo

//static
#include <bogeophonestatic.h>
#include <bogeophonestatich.h>
#include <bosourcestatic.h>
#include <bosourcestatich.h>
#include <bobinsetstatic.h>
//#include <bobinsetstatich.h>

//velocity
#include <bovelocityanisotropy.h> //2d->BOLine 3d-Swath

//mute
#include <bomute.h>

//filterop
#include <bofilteroperator.h>

//dataprovider
#include <dataproviderconf.h>

using namespace qk;
using namespace std;
using namespace ngp::bo;

namespace {
static setError(const string& msg, string* error) {
    if(!error) return ;
    *error = msg;
}

}

namespace qk{
std::vector<std::string> getDatabases() {
    DataProviderConfig dpc;
    auto & r= dpc.Load();
    if(r.Fail()) {
        std::cerr<<"read config file error:"<<r.Message();
        return std::vector<string>();
    }
    auto dp = dpc.listofDpname();
    vector<string> res;
    for(const auto& i : dp) {
        auto s = BOSystemRoot::instance(i);
        if(!s) continue;
        if(s->Fail()) continue;
        res.push_back(i);
    }
    return res;

}


bool isBAID(const std::string& str, BAID& id) {
    static regex rx{"\\{(\\d+)\\}"};
    smatch m;
    if(!regex_match(str, m, rx)) return false;
    auto mstr = m[1].str();
    id = stoll(mstr);
    return true;
}


baid_t toBAID(const std::string& str, bool *ok ) {
    BAID id{0};
    static_assert(sizeof(BAID) == sizeof(baid_t), "(static_error) BAID != baid_t") ;
    auto b = isBAID(str, id);
    if(ok) *ok = b;
    return id;
}

std::string toBAIDString(baid_t id) {
    return "{" + to_string(id) + "}";
}

stringlist_t splitToken(const std::string& str, const std::string& tok ){
    regex rx{tok};
    sregex_token_iterator i{str.begin(), str.end(), rx, -1} ;
    return stringlist_t{i, std::sregex_token_iterator()};
}


//=============================getBODataT==================================
#define MAKE_ERROR(x) do {setError(#x, error);} while(0)
#define MAKE_ERROR_S(x) do {setError(x, error);} while(0)
#define E_INVALID_ARG_LENGTH do {setError("Invalid Argument Length.", error);}while(0)
#define E_NULL_OBJECT do {setError("Null Object.", error);}while(0)

template<class P, class C>
shared_ptr<C> getChildT(shared_ptr<P>& p, const string& k) {
    if(!p) return shared_ptr<C>();

    BAID id {0};
    auto ok = isBAID(k, id);
    auto c = ok ? p -> template getChild<C>(id) :
                p -> template getChild<C>(k);
    return c;
}

template<>
shared_ptr<BOSystemRoot>  getBODataT(const stringlist_t& list, string *error) {
    if(list.size() < 1) {
        E_INVALID_ARG_LENGTH;
        return shared_ptr<BOSystemRoot>();
    }
    auto inst = BOSystemRoot::instance(list[0]);
    if(!inst) {
        E_NULL_OBJECT;
        return shared_ptr<BOSystemRoot>();
    }
    if(inst->Fail()) {
        MAKE_ERROR_S(inst->Status().Message()) ;
        return shared_ptr<BOSystemRoot>();
    }
    return inst;
}

template<>
shared_ptr<BOProject>  getBODataT(const stringlist_t& list, string* error) {
    if(list.size() < 2) {
        E_INVALID_ARG_LENGTH;
        return shared_ptr<BOProject>();
    }

    auto root = getBODataT<BOSystemRoot>(list, error);
    if(!root) return shared_ptr<BOProject>();

    return getChildT<BOSystemRoot,BOProject>(root,list[1]);
}

#define GET_SUB(pty,ty,N)\
template<>\
shared_ptr<ty> getBODataT(const stringlist_t& list, string* error) {\
    if(list.size() < N) {\
        E_INVALID_ARG_LENGTH;\
        return shared_ptr<ty>();\
    }\
    \
    auto p = getBODataT<pty>(list,error);\
    if(!p) return shared_ptr<ty>();\
    return getChildT<pty,ty>(p,list[(N-1)]);\
\
}

GET_SUB(BOProject, BOSurvey, 3)
GET_SUB(BOProject, BOWell, 3)

GET_SUB(BOSurvey, BOLine, 4)
GET_SUB(BOSurvey, BOHorizon2d, 4)
GET_SUB(BOSurvey, BOHorizon3d, 4)
GET_SUB(BOSurvey, BOFault2d, 4)
GET_SUB(BOSurvey, BOFault3d, 4)

GET_SUB(BOWell,BOAge,5)
GET_SUB(BOWell,BOCycle,5)
GET_SUB(BOWell,BOLogFacies,5)
GET_SUB(BOWell,BOWellFormation,5)
GET_SUB(BOWell,BOGram,5)
GET_SUB(BOWell,BOWellLith,5)
GET_SUB(BOWell,BOWellOGW,5)
GET_SUB(BOWell,BOWellPath,5)


#define GET_SURORLINE_SUB(ty)\
template<>\
shared_ptr<ty> getBODataT(const stringlist_t& list ,string* error) {\
    if(list.size() < 4) {\
        E_INVALID_ARG_LENGTH;\
        return shared_ptr<ty>();\
    }\
    \
    auto p = getBODataT<BOSurvey>(list,error);\
    if(!p) return shared_ptr<ty>();\
    const auto &lineName = list[3];\
    if(list.size() == 4)\
        return getChildT<BOSurvey,ty>(p,lineName);\
    auto p2 = getChildT<BOSurvey,BOLine>(p,lineName);\
    return getChildT<BOLine,ty>(p2, list[4]);\
}

GET_SURORLINE_SUB(BOSeismic)
GET_SURORLINE_SUB(BOSeisCube)
GET_SURORLINE_SUB(BOSeismicSet)

//geometry
GET_SURORLINE_SUB(BOSourceLine)
GET_SURORLINE_SUB(BOGeophoneLine)

//velocity
GET_SURORLINE_SUB(BOVelocityAnisotropy)

//mute
GET_SURORLINE_SUB(BOMute)

//filter
GET_SURORLINE_SUB(BOFilterOperator)

}


namespace qk{
#define IMPL_OPER_D_QUOTE(t, sym) \
shared_ptr<t> operator"" sym(const char* s, size_t n) {\
    (void)n;\
    return getBODataT<t>(std::string(s));\
}

IMPL_OPER_D_QUOTE(BOProject, _p)
IMPL_OPER_D_QUOTE(BOSurvey, _s)
IMPL_OPER_D_QUOTE(BOLine, _l)
IMPL_OPER_D_QUOTE(BOWell, _w)
IMPL_OPER_D_QUOTE(BOHorizon2d, _h2)
IMPL_OPER_D_QUOTE(BOHorizon3d, _h3)
IMPL_OPER_D_QUOTE(BOFault2d, _f2)
IMPL_OPER_D_QUOTE(BOFault3d, _f3)
IMPL_OPER_D_QUOTE(BOSeismic, _se)
IMPL_OPER_D_QUOTE(BOSeisCube, _sc)

#undef IMPL_OPER_D_QUOTE
} //namespace qk





