#include "seiscodemodel.h"

//mgr
#include <bosystemroot.h>
#include <boproject.h>
#include <bosurvey.h>
#include <boline.h>

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

#include <iostream>

using namespace ngp::bo;
using namespace ngp::dp;
using namespace std;
float proceeMat[1387][361][813];
float  data2[361*813] = {0};
namespace {

//从一段缓存中，按照类型读取数据，一般是道头的int,long,double和float
//int 4字节
//long 8字节
//double 8字节
//float 4字节
template<class T>
T readValue(void* buf, int bytePos = 0) {
    char* addr = reinterpret_cast<char*>(buf);
    return *reinterpret_cast<T*>(addr+bytePos);
}

//从公共道头中读取地震数据的无效样点值
float getInvalidValue(const shared_ptr<ngp::bo::BOSeismic>& seis) {
    const shared_ptr<BAHWDefinitions> def = seis->commonHWDefinitions();
    const int bytePos {def->find(seis::NMNullValue)->bytePos()};

    const shared_ptr<BACommonHeader> chdr = seis->commonHeader();
    float val{0};
    chdr->getValue(bytePos, val);
    return val;
}

//创建地震数据的模板函数，如果创建失败返回nullptr，从error中获取错误信息
template<class T>
shared_ptr<BOSeismic> createSeisPrivate(shared_ptr<T>p, const string& newSeis, string& error) {
    if(!p) {
        error = "Invalid parent pointer";
        return nullptr;
    }

    if(p->template hasChild<BOSeismic>(newSeis)) {
        NStatus st =p->template eraseChild<BOSeismic>(newSeis);
        if(st.Fail()) {
            error = "Delete Child error : " + newSeis + "," + st.Message();
            return nullptr;
        }
    }

    shared_ptr<BOSeismic> seis = p->template createChild<BOSeismic>(newSeis);
    NStatus st = p->template save();
    if(st.Fail()) {
        error = "Save Child error:" + st.Message();
        return nullptr;
    }

    return seis;
}

//创建地震数据，如果创建失败返回nullptr，从error中获取错误信息
shared_ptr<BOSeismic> createNewSeis(const shared_ptr<ngp::bo::BOSeismic> &seis,
                                    const string& newSeis,
                                    string &error) {
    std::shared_ptr<BOParent> parent = seis->getParent();
    if(!parent) {
        error = "Invalid parent pointer";
        return nullptr;
    }
    if(parent->getClassName() == BOSurvey::getClassName()) {
        return createSeisPrivate<BOSurvey>(dynamic_pointer_cast<BOSurvey>(parent), newSeis, error);
    }
    return createSeisPrivate<BOLine>(dynamic_pointer_cast<BOLine>(parent), newSeis, error);
}

//拷贝地震数据的道头字定义和公共道头 src---->dest
void copySeisHeader(const shared_ptr<BOSeismic>& src, shared_ptr<BOSeismic>& dest) {
    //等价于 *dest->commonHWDefinitions() = *src->commonHWDefinitions();
    dest->setCommonHWDefinitions(src->commonHWDefinitions());
    dest->setTraceHWDefinitions(src->traceHWDefinitions());
    dest->setCommonHeader(src->commonHeader());
}

}

SeisCodeModel::SeisCodeModel(const shared_ptr<ngp::bo::BOSeismic> &seis,
                             const string& newSeis)
{
    m_seis = seis;
    m_newSeis = newSeis;
}

void SeisCodeModel::run()
{
    if(!m_seis) {
        cout<<"Invalid seismic data." << endl;
        return ;
    }

    string error;
    //根据名称在同一级创建一个新的地震数据
    shared_ptr<BOSeismic> newSeis = createNewSeis(m_seis, m_newSeis, error);
    if(!newSeis) {
        cout << error <<endl;
        return;
    }

    //拷贝道头字定义和公共道头
    copySeisHeader(m_seis, newSeis);
    shared_ptr<BATraceWriter> writer = newSeis->traceWriter();
    //设置地震数据的存储方式
    newSeis->setStorageType(seis::NAS);
    //设置地震数据的状态，正在写入，避免地震数据误用
    newSeis->setUnderWriting(true);

    //获取地震数据辅助类，用来获取单道的样点个数
    shared_ptr<BASeisHelper> helper(new BASeisHelper(m_seis.get()));


    //获取地震数据的一道道头的大小（字节数）
    const int bytesOfTraceHeader = helper->bytesPerHeaderTrace();
    //获取地震数据每一道采样点的个数
    const int sampleCnt = helper->samplesPerTrace();
    m_sampleCnt =sampleCnt;




    const shared_ptr<BAHWDefinitions> thdr = m_seis->traceHWDefinitions();
    const int bytesOfHdr = thdr->bytesPerHeaderTrace();

    //从单道道头字定义中获取source_no和trace_no这两个道头字在道头内存中的字节偏移量，用来读取道头字数据
    BAHWDefinition* hwshot = thdr->find(seis::NMShot);
    BAHWDefinition* hwtrace = thdr->find(seis::NMTrace);
    assert(hwshot && hwtrace);
    m_bytePosShot  = hwshot->bytePos();
    m_bytePosTrace = hwtrace->bytePos();




    //使用两种方式的Reader来读取一个道集数据
    //1.traceReader 读取一个道集
    //2.gatherReader 读取一个道集
    shared_ptr<BATraceReader>  treader = m_seis->traceReader();
    shared_ptr<BAGatherReader> greader = m_seis->gatherReader();

    //按照道集来设置数据的索引
    BAIndex index;
    //指定Shot和Trace的排序方式
    index.addKey(seis::NMShot).addKey(seis::NMTrace);
    treader->setGatherType(index, seis::FirstKey);
    greader->setGatherType(index, seis::FirstKey);

    //道集的数据结构 shared_ptr<BATraces>，可以一次读取多个道集
    shared_ptr<BATraces> gt;
    NStatus st;

    int shotCnt {0};
    int traceCnt {0};


    st = greader->begin(gt);
    assert(st.Succeed() && gt);

     while(!(greader->isLast(gt))){
         shotCnt++;
         greader->next(gt);
     }

     shotCnt++;
     cout<<shotCnt<<endl;
     traceCnt = gt->ntraces();




//===========================transform=============================


     greader->begin(gt);
     int shotCnt2 {0};


    while(shotCnt2<shotCnt){

     int traceCnt2 {0};
     int samplenum {0};


    //获取道集中的样点数据
    void* samples = gt->sampleTraces();
    float* data = samples;

    while(samplenum<(sampleCnt*traceCnt)){

        if((samplenum%sampleCnt)==0){
           traceCnt2++;
        }

        proceeMat[shotCnt2][traceCnt2][(samplenum%sampleCnt)]=data[samplenum];

        samplenum++;


    }

     greader->next(gt);
     shotCnt2++;

   }


//====================================================================


//==============================process===============================



int i {80};


while(i<100){
    int j {400};
    while(j<600){
        proceeMat[1][i][j]=0;
        j++;
    }
    i++;
}




//====================================================================



//===========================intransform==============================



       greader->begin(gt);
       shotCnt2 = 0;


      while(shotCnt2<shotCnt){

       int traceCnt2 {0};
       int samplenum {0};



       void* samples = gt->sampleTraces();
       float* data3 = samples;



      while(samplenum<(sampleCnt*traceCnt)){

          if((samplenum%sampleCnt)==0){
             traceCnt2++;
          }
          data3[samplenum]=proceeMat[shotCnt2][traceCnt2][(samplenum%sampleCnt)];


          samplenum++;


      }
      //获取道集中的道头数据
      shared_ptr<int> headers = gt->headerTraces()->getValues();
      char* hdr2 = headers.get();
      st = writer->write(reinterpret_cast<int*>(hdr2), data3, traceCnt);
              if(st.Fail()) {
                  cout<<"Write error:"<<st.Message()<<endl;
                  return;
              }


       greader->next(gt);
       shotCnt2++;
       cout<<shotCnt2<<endl;

     }


    st = writer->close();
            if(!st.Succeed()) {
                cout <<"Close error:"<<st.Message() << endl;
                return ;
            }

            newSeis->setUnderWriting(false);
            st = newSeis->save();
            if(st.Fail()) {
                cout<<"Save error:"<<st.Message()<<endl;
                return;
     }



}








