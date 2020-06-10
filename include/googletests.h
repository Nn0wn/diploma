#ifndef GOOGLETESTS_H
#define GOOGLETESTS_H

#include <tuple>

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>
#include "ConsoleInputProcessing.h"
#include "simnetworktranslator.h"
#include "rlnetworktranslator.h"
#include "RotationTable.h"
#include "FltToStr.h"
#include "InitialWindow.h"

#define PROTO_NUMBER 10

using namespace testing;

class TestEnvironment : public testing::Environment
{

public:
#if __cplusplus >= 201703L
    inline static int t_argc;
    inline static char** t_argv;
#else
    static int t_argc;
    static char** t_argv;
#endif
    explicit TestEnvironment(int argc, char** argv)
    {
        t_argc = argc;
        t_argv = argv;
    }
    virtual ~TestEnvironment() = default;
};

TEST(InitTest, InitName)
{
    ASSERT_EQ(1, 1);
    ASSERT_THAT(0, Eq(0));
}

TEST(ConsoleArgumentsParsingTest, EqualExpected)
{
    QList<QStringList> inputs = {{"-e", "-m", "-m", "-c", "-sim",
                          "../../program_examples/test_Vinogradov.acu", "6000", "6001"}};
    QList<QList<std::string>> expReturns = {{"../../program_examples/test_Vinogradov.acu",
                              "127.0.0.1", "6000", "-sim", "../../program_examples/limits.cfg", "1"}};
    for(int i = 0 ; i < inputs.size() ; i++)
    {
        QStringList resList = ConsoleCommandAnalyser(inputs[i]).analyse();
        for(int j = 0 ; j < resList.size() ; j++)
        {
            EXPECT_STREQ(resList[j].toLocal8Bit().data(), expReturns[i][j].data());
        }
    }
}

TEST(ConnectionModulesTest, EqualExpected)
{
    QApplication a(TestEnvironment::t_argc, TestEnvironment::t_argv);

    MessageMode mode = SILENT;
    SimNetworkTranslator net(nullptr, mode);
    RlNetworkTranslator clt(nullptr, mode);
    QEventLoop loop;



    net.setupServer(QHostAddress::LocalHost, STANDART_SERVER_LISTENING_PORT);
    clt.setupConnection(QHostAddress(QHostAddress::LocalHost).toString(), STANDART_SIMULATOR_CONNECTION_PORT, STANDART_SERVER_LISTENING_PORT,
                        QAbstractSocket::IPv4Protocol, net.getServer()->serverPort());

    QObject::connect(&net, &SimNetworkTranslator::dataDelivered, &loop, &QEventLoop::quit);
    QObject::connect(&clt, &RlNetworkTranslator::dataDelivered, &loop, &QEventLoop::quit);

    QStringList commands = {{"Power on"}};

    for(QStringList::iterator it = commands.begin(); it != commands.end(); it++)
    {
        clt.encodeData(*it);
        loop.exec();
        EXPECT_STREQ((*it).toStdString().data(), net.getMsg().toStdString().data());
        loop.exec();
        EXPECT_STREQ(net.getOutputMsg().toStdString().data(), clt.getMsg().toStdString().data());
    }

    QObject::disconnect(&clt, &RlNetworkTranslator::dataDelivered, &loop, &QEventLoop::quit);
    QObject::disconnect(&net, &SimNetworkTranslator::dataDelivered, &loop, &QEventLoop::quit);

    clt.disconnect();
    net.disconnect();
    net.stopServer();

    a.exit();
}

TEST(RealStandGetFunctionsTest, EqualExpected)
{
    QApplication a(TestEnvironment::t_argc, TestEnvironment::t_argv);

    MessageMode mode = SILENT;

    auto ptable = std::unique_ptr<CRTableReal>(new CRTableReal);
    auto initWin = std::unique_ptr<CInitialWindow>(new CInitialWindow(mode, ptable.get(), ON));

    initWin->changeLoadType();
    initWin->setupSimulatorConnection();

    QVector<int> axis = {0, 1, 2, 3};

    /* bool return, 0 arg, get functions start */

    QVector<bool (CRTableReal::*)()> boolFuncsNoArgs = {&CRTableReal::RTready, &CRTableReal::powerOn, &CRTableReal::powerOff};
    QVector<bool (StandProto::*)()> protoBoolFuncs = {&StandProto::getReady};
    QVector<unsigned char (StandProto::*)()> protoCharFuncs = {&StandProto::getStandEventReg, &StandProto::getActiveAxis};
    QVector<bool> boolFuncsNoArgsAnswers;

    /* bool return, 0 arg, get functions end */

    /* QVector3D return, 1 int arg, get functions start */

    QVector<QVector3D (StandProto::*)()> vec3D1IntArgProtoFuncs = {&StandProto::getPos, &StandProto::getVel, &StandProto::getAcc,
            &StandProto::getMinPosLimits, &StandProto::getMaxPosLimits, &StandProto::getVelLimits, &StandProto::getVelLimits,
            &StandProto::getAccLimits, &StandProto::getAccLimits, &StandProto::getMinFactoryPosLimits, &StandProto::getMaxFactoryPosLimits,
            &StandProto::getFactoryVelLimits, &StandProto::getFactoryVelLimits, &StandProto::getFactoryAccLimits, &StandProto::getFactoryAccLimits};

    QVector<QVector3D (CRTableReal::*)(int)> vec3D1IntArgFuncs = {&CRTableReal::RTgetPosition, &CRTableReal::RTgetVelocity,
            &CRTableReal::RTgetAcceleration, &CRTableReal::RTgetMinPositionLimit, &CRTableReal::RTgetMaxPositionLimit,
            &CRTableReal::RTgetMinVelocityLimit, &CRTableReal::RTgetMaxVelocityLimit, &CRTableReal::RTgetMinAccelerationLimit,
            &CRTableReal::RTgetMaxAccelerationLimit, &CRTableReal::RTgetFactoryMinPositionLimit,
            &CRTableReal::RTgetFactoryMaxPositionLimit, &CRTableReal::RTgetFactoryMinVelocityLimit,
            &CRTableReal::RTgetFactoryMaxVelocityLimit, &CRTableReal::RTgetFactoryMinAccelerationLimit,
            &CRTableReal::RTgetFactoryMaxAccelerationLimit};
    QVector<QVector3D> vec3D1IntArgFuncsAnswers;

    /* QVector3D return, 1 int arg, get functions end */

    /* state return, 1 int arg, get functions start */

    QVector<QVector<QVector3D (StandProto::*)()>> state1IntArgProtoFuncs = {{&StandProto::getMaxPosLimits, &StandProto::getVelLimits,
            &StandProto::getAccLimits}, {&StandProto::getMinPosLimits, &StandProto::getVelLimits, &StandProto::getAccLimits},
            {&StandProto::getMaxFactoryPosLimits, &StandProto::getFactoryVelLimits, &StandProto::getFactoryAccLimits},
            {&StandProto::getMinFactoryPosLimits, &StandProto::getFactoryVelLimits, &StandProto::getFactoryAccLimits}};

    QVector<state (CRTableReal::*)(int)> state1IntArgFuncs = {&CRTableReal::RTgetMaxLimits, &CRTableReal::RTgetMinLimits,
            &CRTableReal::RTgetFactoryMaxLimits, &CRTableReal::RTgetFactoryMinLimits};
    QVector<state> state1IntArgFuncsAnswers;

    /* state return, 1 int arg, get functions end */

    /* bool return, 1 int arg, get functions start */

    QVector<bool> bool1IntArgFuncsAnswers;

    /* bool return, 1 int arg, get functions end */

    /* bool return, 1 int arg and 1 double arg, set functions start */

    QVector<bool (CRTableReal::*)(int, double)> bool1Int1DoubleArgsFuncs = {&CRTableReal::startRotation, &CRTableReal::RTsetMaxPositionLimit,
            &CRTableReal::RTsetMinPositionLimit, &CRTableReal::RTsetMaxVelocityLimit, &CRTableReal::RTsetMinVelocityLimit,
            &CRTableReal::RTsetMaxAccelerationLimit, &CRTableReal::RTsetMinAccelerationLimit};
    QVector<QVector3D (StandProto::*)()> bool1Int1DoubleArgProtoFuncs = {&StandProto::getVel, &StandProto::getMaxPosLimits, &StandProto::getMinPosLimits,
            &StandProto::getVelLimits, &StandProto::getVelLimits, &StandProto::getAccLimits, &StandProto::getAccLimits};
    QVector<int> bool1Int1DoubleArgsDoubleArgsSeq = {5, 0, 1, 2, 2, 3, 3}/* = {5.0, 10.0, 50,0}*/;
    QVector<QVector3D> bool1Int1DoubleArgsDoubleArgs;

    /* bool return, 1 int arg and 1 double arg, set functions end */

    /* bool return, 1 int arg and 3 double args, set functions start */

    QVector<bool (CRTableReal::*)(int, double, double, double)> bool1Int3DoubleArgsFirstFuncs = {&CRTableReal::sinusMove, &CRTableReal::sinusRelMove};
    QVector<bool (CRTableReal::*)(int, double, double, double)> bool1Int3DoubleArgsSecondFuncs = {&CRTableReal::RTsetMaxLimits, &CRTableReal::RTsetMinLimits};
    QVector<unsigned char (StandProto::*)()> bool1Int3DoubleArgFirstProtoFuncs = {&StandProto::getMoving, &StandProto::getMoving};
    QVector<QVector<QVector3D (StandProto::*)()>> bool1Int3DoubleArgSecondProtoFuncs = {{&StandProto::getMaxPosLimits, &StandProto::getVelLimits,
            &StandProto::getAccLimits}, {&StandProto::getMinPosLimits, &StandProto::getVelLimits, &StandProto::getAccLimits}};
    QVector<QVector3D> bool1Int3DoubleArgsDoubleArgs;

    /* bool return, 1 int arg and 3 double args, set functions end */

    /* bool return, 1 int arg and 2 double args, set functions start */

    QVector<bool (CRTableReal::*)(int, double, double)> bool1Int2DoubleArgsFuncs = {&CRTableReal::setPosition, &CRTableReal::setRelPosition};
    QVector<QVector<QVector3D (StandProto::*)()>> bool1Int2DoubleArgsProtoFuncs = {{&StandProto::getPos, &StandProto::getVel},
            {&StandProto::getPos, &StandProto::getVel}};
    QVector<QVector3D> bool1Int2DoubleArgsDoubleArgs;
    QVector<QVector<QVector3D>> bool1Int2DoubleArgsDoubleAns;

    /* bool return, 1 int arg and 2 double args, set functions end */

    QVector<StandProto*> protoVec;
    for(int i = 0; i < PROTO_NUMBER; ++i)
    {   protoVec.push_back(new StandProto); }

    for(QVector<StandProto*>::iterator vit = protoVec.begin(); vit != protoVec.end(); ++vit)
    {
        ptable->getSrvNet()->setProto(*vit);

        qInfo() << "============== Starting " + QString::number(protoVec.indexOf((*vit)) + 1) + " test round ===============";
        /* checking first group
         *
         * RTready,
         * powerOn,
         * powerOff
         *
         * */

        boolFuncsNoArgsAnswers.clear();

        boolFuncsNoArgsAnswers.push_back((*vit)->getReady());
        boolFuncsNoArgsAnswers.push_back(static_cast<int>((*vit)->getStandEventReg()) > 127);
        boolFuncsNoArgsAnswers.push_back(static_cast<int>((*vit)->getStandEventReg()) < 128);

        for(auto iter = std::make_tuple(boolFuncsNoArgs.begin(), boolFuncsNoArgsAnswers.begin());
            std::get<0>(iter) != boolFuncsNoArgs.end() && std::get<1>(iter) != boolFuncsNoArgsAnswers.end();
            ++std::get<0>(iter), ++std::get<1>(iter))
        {   ASSERT_EQ(*std::get<1>(iter), ((*ptable).*(*std::get<0>(iter)))());   }

        /* checking second group
         *
         * RTgetPosition,
         * RTgetVelocity,
         * RTgetAcceleration,
         * RTgetMinPositionLimit,
         * RTgetMaxPositionLimit,
         * RTgetMinVelocityLimit,
         * RTgetMaxVelocityLimit,
         * RTgetMinAccelerationLimit,
         * RTgetMaxAccelerationLimit,
         * RTgetFactoryMinPositionLimit,
         * RTgetFactoryMaxPositionLimit,
         * RTgetFactoryMinVelocityLimit,
         * RTgetFactoryMaxVelocityLimit,
         * RTgetFactoryMinAccelerationLimit,
         * RTgetFactoryMaxAccelerationLimit
         *
         * */

        vec3D1IntArgFuncsAnswers.clear();

        for(auto func: vec3D1IntArgProtoFuncs)
        {   vec3D1IntArgFuncsAnswers.push_back(QVector3D(((*vit)->*func)().x(), 0., 0.)); }
        for(auto func: vec3D1IntArgProtoFuncs)
        {   vec3D1IntArgFuncsAnswers.push_back(QVector3D(0., ((*vit)->*func)().y(), 0.)); }
        for(auto func: vec3D1IntArgProtoFuncs)
        {   vec3D1IntArgFuncsAnswers.push_back(QVector3D(0., 0., ((*vit)->*func)().z())); }
        for(auto func: vec3D1IntArgProtoFuncs)
        {   vec3D1IntArgFuncsAnswers.push_back(((*vit)->*func)()); }

        for(auto topIter = std::make_tuple(axis.begin(), vec3D1IntArgFuncsAnswers.begin());
            std::get<0>(topIter) != axis.end(); ++std::get<0>(topIter))
        {
            for(auto funcIt = vec3D1IntArgFuncs.begin(); funcIt != vec3D1IntArgFuncs.end();
                ++funcIt,  ++std::get<1>(topIter))
            {   ASSERT_EQ(*std::get<1>(topIter), ((*ptable).*(*funcIt))(*std::get<0>(topIter))); }
        }

        /* checking third group
         *
         * RTgetMaxLimits,
         * RTgetMinLimits,
         * RTgetFactoryMaxLimits,
         * RTgetFactoryMinLimits
         *
         * */

        state1IntArgFuncsAnswers.clear();

        for(auto funcSet: state1IntArgProtoFuncs)
        {
            QVector<double> tempv;
            for(auto func: funcSet)
            {
                tempv.push_back(static_cast<double>(((*vit)->*func)().x()));
                tempv.push_back(static_cast<double>(((*vit)->*func)().y()));
                tempv.push_back(static_cast<double>(((*vit)->*func)().z()));
            }

            state1IntArgFuncsAnswers.push_back(state(tempv.at(0), 0.0, 0.0, tempv.at(3), 0.0, 0.0, tempv.at(6), 0.0, 0.0));
            state1IntArgFuncsAnswers.push_back(state(0.0, tempv.at(1), 0.0, 0.0, tempv.at(4), 0.0, 0.0, tempv.at(7), 0.0));
            state1IntArgFuncsAnswers.push_back(state(0.0, 0.0, tempv.at(2), 0.0, 0.0, tempv.at(5), 0.0, 0.0, tempv.at(8)));
            state1IntArgFuncsAnswers.push_back(state(tempv.at(0), tempv.at(1), tempv.at(2),
                                                     tempv.at(3), tempv.at(4), tempv.at(5),
                                                     tempv.at(6), tempv.at(7), tempv.at(8)));
        }

        for(auto topIt = std::make_tuple(state1IntArgFuncs.begin(), state1IntArgFuncsAnswers.begin());
            std::get<0>(topIt) != state1IntArgFuncs.end(); ++std::get<0>(topIt))
        {
            for(auto argIt = axis.begin(); argIt != axis.end(); ++argIt, ++std::get<1>(topIt))
            {   ASSERT_EQ(*std::get<1>(topIt), ((*ptable).*(*std::get<0>(topIt)))(*argIt)); }
        }

        /* checking fourth group
         *
         * RTgetState
         *
         * */

        ASSERT_EQ(state(static_cast<double>((*vit)->getPos().x()), static_cast<double>((*vit)->getPos().y()),
                        static_cast<double>((*vit)->getPos().z()), static_cast<double>((*vit)->getVel().x()),
                        static_cast<double>((*vit)->getVel().y()), static_cast<double>((*vit)->getVel().z()),
                        static_cast<double>((*vit)->getAcc().x()), static_cast<double>((*vit)->getAcc().y()),
                        static_cast<double>((*vit)->getAcc().z())), ptable->RTgetState());

        /* checking fifth group
         *
         * RTgetInterlock
         *
         * */

        bool1IntArgFuncsAnswers.clear();

        bool1IntArgFuncsAnswers.push_back((*vit)->getActiveAxis() > 0);
        bool1IntArgFuncsAnswers.push_back((*vit)->getActiveAxis() > 1);
        bool1IntArgFuncsAnswers.push_back((*vit)->getActiveAxis() > 3);
        bool1IntArgFuncsAnswers.push_back((*vit)->getActiveAxis() == 7);


        for(auto iter = std::make_tuple(bool1IntArgFuncsAnswers.begin(),axis.begin());
            std::get<0>(iter) != bool1IntArgFuncsAnswers.end() && std::get<1>(iter) != axis.end();
            ++std::get<0>(iter), ++std::get<1>(iter))
        {   ASSERT_EQ(*std::get<0>(iter), ptable->RTgetInterlock(*std::get<1>(iter)));   }

        /* checking sixth group
         *
         * startRotation,
         * RTsetMaxPositionLimit,
         * RTsetMinPositionLimit,
         * RTsetMaxVelocityLimit,
         * RTsetMinVelocityLimit,
         * RTsetMaxAccelerationLimit,
         * RTsetMinAccelerationLimit
         *
         * */

        bool1Int1DoubleArgsDoubleArgs.clear();

        auto temp = StandProto::generateLimits(*(*vit), OLDVALS);
        while (!temp.isEmpty())
        {   bool1Int1DoubleArgsDoubleArgs.push_back(temp.takeFirst());  }
        temp = StandProto::generateCurVals(bool1Int1DoubleArgsDoubleArgs.first(), bool1Int1DoubleArgsDoubleArgs.at(1),
                                           bool1Int1DoubleArgsDoubleArgs.at(2), bool1Int1DoubleArgsDoubleArgs.last());
        while(!temp.isEmpty())
        {   bool1Int1DoubleArgsDoubleArgs.push_back(temp.takeFirst());  }

        QEventLoop setLoop;

        QObject::connect(ptable->getSrvNet(), &SimNetworkTranslator::valueSet, &setLoop, &QEventLoop::quit);

        for(auto iter = std::make_tuple(bool1Int1DoubleArgsFuncs.begin(),bool1Int1DoubleArgProtoFuncs.begin(),
            bool1Int1DoubleArgsDoubleArgsSeq.begin()); std::get<0>(iter) != bool1Int1DoubleArgsFuncs.end() &&
            std::get<1>(iter) != bool1Int1DoubleArgProtoFuncs.end() && std::get<2>(iter) != bool1Int1DoubleArgsDoubleArgsSeq.end();
            ++std::get<0>(iter), ++std::get<1>(iter), ++std::get<2>(iter))
        {
            for(auto axie: axis)
            {
                switch (axie)
                {
                case 0:
                    ((*ptable).*(*std::get<0>(iter)))(axie,
                                static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).x()));
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))().x(),
                              static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).x()));
                    break;
                case 1:
                    ((*ptable).*(*std::get<0>(iter)))(axie,
                                static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).y()));
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))().y(),
                              static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).y()));
                    break;
                case 2:
                    ((*ptable).*(*std::get<0>(iter)))(axie,
                                static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).z()));
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))().z(),
                              static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).z()));
                    break;
                case 3:
                    QVector<QVector3D> uniArgs = StandProto::generateCurVals(*(*vit), UNIFIED);
                    bool1Int1DoubleArgsDoubleArgs.replace(4,StandProto::generateCurVals(*(*vit), UNIFIED).first());
                    bool1Int1DoubleArgsDoubleArgs.replace(5,StandProto::generateCurVals(*(*vit), UNIFIED).at(1));
                    bool1Int1DoubleArgsDoubleArgs.replace(6,StandProto::generateCurVals(*(*vit), UNIFIED).last());
                    ((*ptable).*(*std::get<0>(iter)))(axie,
                                static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).x()));
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))().x(),
                              static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).x()));
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))().y(),
                              static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).x()));
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))().z(),
                              static_cast<double>(bool1Int1DoubleArgsDoubleArgs.at(*std::get<2>(iter)).x()));
                    break;
                }
                for(auto axie: ptable->getAnimAxis()){   axie->stopAnimation(); }
            }
        }

        /* checking seventh group
         *
         * sinusMove,
         * sinusRelMove
         * RTsetMaxLimits,
         * RTsetMinLimits
         *
         * */


        /*
         *
         * sinusMove,
         * sinusRelMove
         *
         * */

        bool1Int3DoubleArgsDoubleArgs.clear();

        temp = StandProto::generateCurVals(*(*vit), UNIFIED);
        (*vit)->setPos(temp.takeFirst());
        bool1Int3DoubleArgsDoubleArgs.push_back(
                    QVector3D(std::min(std::abs((*vit)->getMaxPosLimits().x() - (*vit)->getPos().x()),
                                       std::abs((*vit)->getMinPosLimits().x() - (*vit)->getPos().x())),
                              std::min(std::abs((*vit)->getMaxPosLimits().y() - (*vit)->getPos().y()),
                                       std::abs((*vit)->getMinPosLimits().y() - (*vit)->getPos().y())),
                              std::min(std::abs((*vit)->getMaxPosLimits().z() - (*vit)->getPos().z()),
                                       std::abs((*vit)->getMinPosLimits().z() - (*vit)->getPos().z()))));
        while (!temp.isEmpty())
        {   bool1Int3DoubleArgsDoubleArgs.push_back(temp.takeFirst());  }

        for(auto iter = std::make_tuple(bool1Int3DoubleArgsFirstFuncs.begin(), bool1Int3DoubleArgFirstProtoFuncs.begin());
            std::get<0>(iter) != bool1Int3DoubleArgsFirstFuncs.end() && std::get<1>(iter) != bool1Int3DoubleArgFirstProtoFuncs.end();
            ++std::get<0>(iter), ++std::get<1>(iter))
        {
            for(auto axie: axis)
            {
                switch(axie)
                {
                case 0:
                    ((*ptable).*(*std::get<0>(iter)))(axie,
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.first().x()),
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.at(1).x()),
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.last().x()));
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))() == 1 || ((*vit)->*(*std::get<1>(iter)))() == 3 ||
                              ((*vit)->*(*std::get<1>(iter)))() == 5 || ((*vit)->*(*std::get<1>(iter)))() == 7,
                              true);
                    break;
                case 1:
                    ((*ptable).*(*std::get<0>(iter)))(axie,
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.first().y()),
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.at(1).y()),
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.last().y()));
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))() == 2 || ((*vit)->*(*std::get<1>(iter)))() == 3 ||
                              ((*vit)->*(*std::get<1>(iter)))() == 6 || ((*vit)->*(*std::get<1>(iter)))() == 7,
                              true);
                    break;
                case 2:
                    ((*ptable).*(*std::get<0>(iter)))(axie,
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.first().z()),
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.at(1).z()),
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.last().z()));
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))() == 4 || ((*vit)->*(*std::get<1>(iter)))() == 5 ||
                              ((*vit)->*(*std::get<1>(iter)))() == 6 || ((*vit)->*(*std::get<1>(iter)))() == 7,
                              true);
                    break;
                case 3:
                    ((*ptable).*(*std::get<0>(iter)))(axie,
                                static_cast<double>(std::min(std::min(bool1Int3DoubleArgsDoubleArgs.first().x(),
                                                             bool1Int3DoubleArgsDoubleArgs.first().y()),
                                                             bool1Int3DoubleArgsDoubleArgs.first().z())),
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.at(1).x()),
                                static_cast<double>(bool1Int3DoubleArgsDoubleArgs.last().x()));
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*(*std::get<1>(iter)))() == 7, true);
                    break;
                }
                for(auto axie: ptable->getAnimAxis()){   axie->stopAnimation(); }
            }
        }

        /*
         *
         * RTsetMaxLimits,
         * RTsetMinLimits
         *
         * */

        bool1Int3DoubleArgsDoubleArgs.clear();

        temp = StandProto::generateLimits(*(*vit), GenerateBoundings::OLDVALS);
        while(!temp.isEmpty())
        {   bool1Int3DoubleArgsDoubleArgs.push_back(temp.takeFirst());  }

        temp = StandProto::generateCurVals(bool1Int3DoubleArgsDoubleArgs.first(), bool1Int3DoubleArgsDoubleArgs.at(1),
                                           bool1Int3DoubleArgsDoubleArgs.at(2), bool1Int3DoubleArgsDoubleArgs.last(), UNIFIED);
        (*vit)->setPos(temp.takeFirst());
        (*vit)->setVel(temp.takeFirst());
        (*vit)->setAcc(temp.takeFirst());

        for(auto iter = std::make_tuple(bool1Int3DoubleArgsSecondFuncs.begin(), bool1Int3DoubleArgSecondProtoFuncs.begin());
            std::get<0>(iter) != bool1Int3DoubleArgsSecondFuncs.end() && std::get<1>(iter) != bool1Int3DoubleArgSecondProtoFuncs.end();
            ++std::get<0>(iter), ++std::get<1>(iter))
        {
            for(auto axie : axis)
            {
                if(std::get<0>(iter) != bool1Int3DoubleArgsSecondFuncs.begin() + 1)
                {
                    if(axie != axis.last())
                    {
                        ((*ptable).*(*std::get<0>(iter)))(axie, bool1Int3DoubleArgsDoubleArgs.first()[axie],
                                                      bool1Int3DoubleArgsDoubleArgs.at(2)[axie],
                                                      bool1Int3DoubleArgsDoubleArgs.last()[axie]);
                        setLoop.exec();
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).first())()[axie], bool1Int3DoubleArgsDoubleArgs.first()[axie]);
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).at(1))()[axie], bool1Int3DoubleArgsDoubleArgs.at(2)[axie]);
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).last())()[axie], bool1Int3DoubleArgsDoubleArgs.last()[axie]);
                    }
                    else
                    {
                        ((*ptable).*(*std::get<0>(iter)))(axie, bool1Int3DoubleArgsDoubleArgs.first().x(),
                                                          bool1Int3DoubleArgsDoubleArgs.at(2).x(),
                                                          bool1Int3DoubleArgsDoubleArgs.last().x());
                        setLoop.exec();
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).first())().x(), bool1Int3DoubleArgsDoubleArgs.first().x());
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).at(1))().x(), bool1Int3DoubleArgsDoubleArgs.at(2).x());
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).last())().x(), bool1Int3DoubleArgsDoubleArgs.last().x());
                    }
                }
                else
                {
                    if(axie != axis.last())
                    {
                        ((*ptable).*(*std::get<0>(iter)))(axie, bool1Int3DoubleArgsDoubleArgs.at(1)[axie],
                                                      bool1Int3DoubleArgsDoubleArgs.at(2)[axie],
                                                      bool1Int3DoubleArgsDoubleArgs.last()[axie]);
                        setLoop.exec();
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).first())()[axie], bool1Int3DoubleArgsDoubleArgs.at(1)[axie]);
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).at(1))()[axie], bool1Int3DoubleArgsDoubleArgs.at(2)[axie]);
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).last())()[axie], bool1Int3DoubleArgsDoubleArgs.last()[axie]);
                    }
                    else
                    {
                        ((*ptable).*(*std::get<0>(iter)))(axie, bool1Int3DoubleArgsDoubleArgs.at(1).x(),
                                                          bool1Int3DoubleArgsDoubleArgs.at(2).x(),
                                                          bool1Int3DoubleArgsDoubleArgs.last().x());
                        setLoop.exec();
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).first())().x(), bool1Int3DoubleArgsDoubleArgs.at(1).x());
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).at(1))().x(), bool1Int3DoubleArgsDoubleArgs.at(2).x());
                        ASSERT_EQ(((*vit)->*(*std::get<1>(iter)).last())().x(), bool1Int3DoubleArgsDoubleArgs.last().x());
                    }
                }
            }
        }

        /* checking eights group
         *
         * setPosition
         * setRelPosition
         *
         * */

        bool1Int2DoubleArgsDoubleArgs.clear();
        bool1Int2DoubleArgsDoubleAns.clear();

        temp = StandProto::generateCurVals(*(*vit), UNIFIED);
        bool1Int2DoubleArgsDoubleArgs.push_back(temp.first());
        bool1Int2DoubleArgsDoubleArgs.push_back(temp.at(1));

        for(auto iter = std::make_tuple(bool1Int2DoubleArgsFuncs.begin(), bool1Int2DoubleArgsProtoFuncs.begin());
            std::get<0>(iter) != bool1Int2DoubleArgsFuncs.end() && std::get<1>(iter) != bool1Int2DoubleArgsProtoFuncs.end();
            ++std::get<0>(iter), ++std::get<1>(iter))
        {
            for(auto axie : axis)
            {
                if(axie != axis.last())
                {
                    ((*ptable).*(*std::get<0>(iter)))(axie, std::get<0>(iter) == bool1Int2DoubleArgsFuncs.begin() + 1 ?
                                                      bool1Int2DoubleArgsDoubleArgs.first()[axie] - (*vit)->getPos()[axie] :
                                                      bool1Int2DoubleArgsDoubleArgs.first()[axie],
                                                      bool1Int2DoubleArgsDoubleArgs.last()[axie]);
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*((*std::get<1>(iter)).first()))()[axie], bool1Int2DoubleArgsDoubleArgs.first()[axie]);
                    ASSERT_EQ(((*vit)->*((*std::get<1>(iter)).last()))()[axie], bool1Int2DoubleArgsDoubleArgs.last()[axie]);
                }
                else
                {
                    ((*ptable).*(*std::get<0>(iter)))(axie, std::get<0>(iter) == bool1Int2DoubleArgsFuncs.begin() + 1 ?
                                                      bool1Int2DoubleArgsDoubleArgs.first().x() - (*vit)->getPos().x() :
                                                      bool1Int2DoubleArgsDoubleArgs.first().x(),
                                                      bool1Int2DoubleArgsDoubleArgs.last().x());
                    setLoop.exec();
                    ASSERT_EQ(((*vit)->*((*std::get<1>(iter)).first()))().x(), bool1Int2DoubleArgsDoubleArgs.first().x());
                    ASSERT_EQ(((*vit)->*((*std::get<1>(iter)).last()))().x(), bool1Int2DoubleArgsDoubleArgs.last().x());
                }
                for(auto axie: ptable->getAnimAxis()){   axie->stopAnimation(); }
            }
        }

        /* checking nineth group
         *
         * stopRotation
         *
         * */

        (*vit)->setMoving((*vit)->getMoving());

        for(auto axie : axis)
        {
            if(axie != axis.last())
            {   (*vit)->setMoving(static_cast<int>(round(pow(2., static_cast<float>(axie)))));   }
            else
            {   (*vit)->setMoving(static_cast<int>(round(pow(2., static_cast<float>(axie)))) - 1);   }

            ptable->stopRotation(axie);
            setLoop.exec();

            ASSERT_EQ((*vit)->getMoving(), 0);
        }

        /* checking tenth group
         *
         * setAxis
         *
         * */

        (*vit)->setActiveAxis(0);

        for(auto axie : axis)
        {
            switch(axie)
            {
            case 0:
                ptable->setAxis(true, false, false);
                setLoop.exec();
                ASSERT_EQ((*vit)->getActiveAxis(), 1);
                break;
            case 1:
                ptable->setAxis(false, true, false);
                setLoop.exec();
                ASSERT_EQ((*vit)->getActiveAxis(), 2);
                break;
            case 2:
                ptable->setAxis(false, false, true);
                setLoop.exec();
                ASSERT_EQ((*vit)->getActiveAxis(), 4);
                break;
            case 3:
                ptable->setAxis(true, true, true);
                setLoop.exec();
                ASSERT_EQ((*vit)->getActiveAxis(), 7);
                break;
            }
        }

        QObject::disconnect(ptable->getSrvNet(), &SimNetworkTranslator::valueSet, &setLoop, &QEventLoop::quit);

    }

    initWin->disconnect();
    initWin->stopServer();

    a.exit();
}

#if __cplusplus < 201703L
    int TestEnvironment::t_argc;
    char** TestEnvironment::t_argv;
#endif

#endif // GOOGLETESTS_H
