#include <iostream>
#include <cassert>
#include <sstream>
#include <vector>
#include <algorithm>

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

//intp
#include <bohorizon2d.h>
#include <bohorizon3d.h>
#include <bofault2d.h>
#include <bofault3d.h>
#include <bafault.h>

//geometry
#include <bobinsetinfo.h>
#include <bogeophonestatic.h>
#include <bogeophonestatich.h>
#include <bosourcestatic.h>
#include <bosourcestatich.h>
#include <bobinsetstatic.h>
//#include <bobinsetstatich.h>

//seis
#include <boseismic.h>
#include <boseiscube.h>
#include <boseismicset.h>
#include <baseishelper.h>
#include <seis/bahwdefinition.h>
#include <seis/bahwdefinitions.h>
#include <seis/baindexmanipulator.h>
#include <seis/baindex.h>
#include <seis/batracereader.h>
#include <seis/baheadertrace.h>
#include <seis/bacommonheader.h>
#include <seis/baheadertraces.h>
#include <seis/bagatherreader.h>
#include <seis/batracewriter.h>

#include <badatagroupuid.h>
#include <bodatagroup.h>
#include <bodatamember.h>
#include <brmanageparent.h>
#include <bodatainfo.h>
#include <brdatagather.h>

#include "quickbomanager.h"
#include "seiscodemodel.h"

using namespace ngp::bo;
using namespace std;
using namespace qk ; //quickbomanager



int main()
{

    SeisCodeModel scm("ndp/BGPCup/process-3-adaptsub/line1/shot-multiple"_se, "shot-multiple_proceed2");
    scm.run();

    return 0;
}


