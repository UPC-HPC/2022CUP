#ifndef SEISCODEMODEL_H
#define SEISCODEMODEL_H

#include <memory>
#include <string>

namespace ngp {
namespace bo {
class BOSeismic;
class BOSeisCube;
}
}

/*!
\class SeisCodeModel
\breif 地震数据处理的模型代码，所谓模型代码就是代码框架已经写好了，
只需要按照开发需要填空即可，这里的用户代码就是processTrace函数,
本类编写了三个最简单的单道处理（处理完成了之后可以使用SeismicView软件来查看和对比），
invalidTrace 将某一道的样点值设置为无效道
reverse 道样点值翻转，负号取反
twice 将道样点值放大两倍
*/
class SeisCodeModel
{
public:
    /*!
    构造函数 
    \param seis 要处理的地震数据
    \param newSeis 处理之后输出的地震数据（没有该地震数据会自动创建，如果有会被删除）
    */
    SeisCodeModel(const std::shared_ptr<ngp::bo::BOSeismic>& seis,
                  const std::string& newSeis);

    /*!
    执行模型代码，调用processTrace逐道的处理地震数据
    */
    void run() ;
private:
    //处理每一道
    void processTrace(char *hdr, float *data) ;
private:
    //=====算法部分====

    //invalidTrace 将某一道的样点值设置为无效道
    void invalidTrace(float* data) ;

    //reverse 道样点值翻转，负号取反
    void reverse(float* data) ;

    //twice 将道样点值放大两倍
    void twice(float* data);
private:
    //当前处理的地震数据
    std::shared_ptr<ngp::bo::BOSeismic> m_seis;

    //道头字跑号的字节偏移、道头字道号的字节偏移、单道的样点个数
    int m_bytePosShot {0}, m_bytePosTrace{0}, m_sampleCnt {0};

    //样点的无效值
    float m_invalidVal {0};

    //新地震数据的名称
    std::string m_newSeis;
};

#endif // SEISCODEMODEL_H
