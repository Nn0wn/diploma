#include "StandProto.h"
#include <QDebug>

StandProto::StandProto(QSettings *limits)
{
    mutex.lock();
    ready = static_cast<bool>(rand() % 2);
    //activeAxis = static_cast<unsigned char>(rand() % 7);
    standEventReg = static_cast<unsigned char>(rand() % 256);
    moving = 0;
    mutex.unlock();

    if(limits != nullptr)
    {
        setMaxFactoryPosLimits(QVector3D(limits->value("/x axis/FactoryMaxPosition", 1000).toFloat(),
                                         limits->value("/y axis/FactoryMaxPosition", 1000).toFloat(),
                                         limits->value("/z axis/FactoryMaxPosition", 1000).toFloat()));
        setMinFactoryPosLimits(QVector3D(limits->value("/x axis/FactoryMinPosition", -1000).toFloat(),
                                         limits->value("/y axis/FactoryMinPosition", -1000).toFloat(),
                                         limits->value("/z axis/FactoryMinPosition", -1000).toFloat()));
        setFactoryVelLimits(QVector3D(limits->value("/x axis/FactoryMaxVelocity", 1000).toFloat(),
                                      limits->value("/y axis/FactoryMaxVelocity", 1000).toFloat(),
                                      limits->value("/z axis/FactoryMaxVelocity", 1000).toFloat()));
        setFactoryAccLimits(QVector3D(limits->value("/x axis/FactoryMaxAcceleration", 1000).toFloat(),
                                      limits->value("/y axis/FactoryMaxAcceleration", 1000).toFloat(),
                                      limits->value("/z axis/FactoryMaxAcceleration", 1000).toFloat()));

        setMaxPosLimits(QVector3D(limits->value("/x axis/MaxPosition", 1000).toFloat(),
                                  limits->value("/y axis/MaxPosition", 1000).toFloat(),
                                  limits->value("/z axis/MaxPosition", 1000).toFloat()));
        setMinPosLimits(QVector3D(limits->value("/x axis/MinPosition", -1000).toFloat(),
                                  limits->value("/y axis/MinPosition", -1000).toFloat(),
                                  limits->value("/z axis/MinPosition", -1000).toFloat()));
        setVelLimits(QVector3D(limits->value("/x axis/MaxVelocity", 1000).toFloat(),
                               limits->value("/y axis/MaxVelocity", 1000).toFloat(),
                               limits->value("/z axis/MaxVelocity", 1000).toFloat()));
        setAccLimits(QVector3D(limits->value("/x axis/MaxAcceleration", 1000).toFloat(),
                               limits->value("/y axis/MaxAcceleration", 1000).toFloat(),
                               limits->value("/z axis/MaxAcceleration", 1000).toFloat()));
    }
    else
    {
        auto facLims = generateFactoryLimits();

        setMaxFactoryPosLimits(facLims.takeFirst());
        setMinFactoryPosLimits(facLims.takeFirst());
        setFactoryVelLimits(facLims.takeFirst());
        setFactoryAccLimits(facLims.takeFirst());

        auto lims = generateLimits(*this);

        setMaxPosLimits(lims.takeFirst());
        setMinPosLimits(lims.takeFirst());
        setVelLimits(lims.takeFirst());
        setAccLimits(lims.takeFirst());
    }

    auto curs = generateCurVals(*this);

    setPos(curs.takeFirst());
    setVel(curs.takeFirst());
    setAcc(curs.takeFirst());

    setMoving(curVel.x() != 0.f || curAcc.x() != 0.f ? 1 : 0 +
              curVel.y() != 0.f || curAcc.y() != 0.f ? 2 : 0 +
              curVel.z() != 0.f || curAcc.y() != 0.f ? 4 : 0);

    setActiveAxis(curVel.x() != 0.f || curAcc.x() != 0.f ? 1 : rand() % 2 +
                  curVel.y() != 0.f || curAcc.y() != 0.f ? 2 : rand() % 3 +
                  curVel.z() != 0.f || curAcc.y() != 0.f ? 4 : rand() % 5);
}

QVector<QVector3D> StandProto::generateFactoryLimits(int pRange, int vRange, int aRange)
{
    assert(pRange > 0);
    assert(vRange > 0);
    assert(aRange > 0);

    QVector3D maxPos, minPos, vel, acc;

    float minMaxPos = 0.f, maxMinPos = pRange;

    while(static_cast<int>(minMaxPos - maxMinPos) < pRange / 4)
    {
        maxPos.setX(static_cast<float>(rand() % pRange * drand48() + 1 - pRange / 2));
        maxPos.setY(static_cast<float>(rand() % pRange * drand48() + 1 - pRange / 2));
        maxPos.setZ(static_cast<float>(rand() % pRange * drand48() + 1 - pRange / 2));

        assert(static_cast<int>(maxPos.x()) + pRange / 2 > 0);
        assert(static_cast<int>(maxPos.y()) + pRange / 2 > 0);
        assert(static_cast<int>(maxPos.z()) + pRange / 2 > 0);

        minPos.setX(static_cast<float>(rand() % (static_cast<int>(maxPos.x()) + pRange / 2) *
                                           drand48() - pRange / 2));
        minPos.setY(static_cast<float>(rand() % (static_cast<int>(maxPos.y()) + pRange / 2) *
                                           drand48() - pRange / 2));
        minPos.setZ(static_cast<float>(rand() % (static_cast<int>(maxPos.z()) + pRange / 2) *
                                           drand48() - pRange / 2));

        QVector<float> maxPs = {maxPos.x(), maxPos.y(), maxPos.z()};
        QVector<float> minPs = {minPos.x(), minPos.y(), minPos.z()};

        minMaxPos = *std::min_element(QVector<float>::iterator(maxPs.begin()), QVector<float>::iterator(maxPs.end()));
        maxMinPos = *std::max_element(QVector<float>::iterator(minPs.begin()), QVector<float>::iterator(minPs.end()));
    }

    vel.setX(static_cast<float>(rand() % vRange * drand48() + 1));
    vel.setY(static_cast<float>(rand() % vRange * drand48() + 1));
    vel.setZ(static_cast<float>(rand() % vRange * drand48() + 1));

    acc.setX(static_cast<float>(rand() % aRange * drand48() + 1));
    acc.setY(static_cast<float>(rand() % aRange * drand48() + 1));
    acc.setZ(static_cast<float>(rand() % aRange * drand48() + 1));

    /*QVector3D maxPos, minPos, vel, acc;

    float minMaxPos = 0.f, maxMinPos = 0.f;

    if(pRange > 0)
    {
        maxPos.setX(static_cast<float>(rand() % pRange * drand48() + 1 - pRange / 2));
        maxPos.setY(static_cast<float>(rand() % pRange * drand48() + 1 - pRange / 2));
        maxPos.setZ(static_cast<float>(rand() % pRange * drand48() + 1 - pRange / 2));
    }

    if(static_cast<int>(maxPos.x()) + pRange / 2 > 0)
    {   minPos.setX(static_cast<float>(rand() % (static_cast<int>(maxPos.x()) + pRange / 2) *
                                       drand48() - pRange / 2));    }
    if(static_cast<int>(maxPos.y()) + pRange / 2 > 0)
    {   minPos.setY(static_cast<float>(rand() % (static_cast<int>(maxPos.y()) + pRange / 2) *
                                       drand48() - pRange / 2));    }
    if(static_cast<int>(maxPos.z()) + pRange / 2 > 0)
    {   minPos.setZ(static_cast<float>(rand() % (static_cast<int>(maxPos.z()) + pRange / 2) *
                                       drand48() - pRange / 2));    }

    if(vRange > 0)
    {
        vel.setX(static_cast<float>(rand() % vRange * drand48() + 1));
        vel.setY(static_cast<float>(rand() % vRange * drand48() + 1));
        vel.setZ(static_cast<float>(rand() % vRange * drand48() + 1));
    }

    if(aRange > 0)
    {
        acc.setX(static_cast<float>(rand() % aRange * drand48() + 1));
        acc.setY(static_cast<float>(rand() % aRange * drand48() + 1));
        acc.setZ(static_cast<float>(rand() % aRange * drand48() + 1));
    }*/

    return  QVector<QVector3D>{maxPos, minPos, vel, acc};
}

QVector<QVector3D> StandProto::generateLimits(const StandProto &ref, GenerateBoundings gtype)
{
    assert(static_cast<int>(ref.maxFactoryPosLimits.x() - ref.minFactoryPosLimits.x()) > 0);
    assert(static_cast<int>(ref.maxFactoryPosLimits.y() - ref.minFactoryPosLimits.y()) > 0);
    assert(static_cast<int>(ref.maxFactoryPosLimits.z() - ref.minFactoryPosLimits.z()) > 0);

    assert(static_cast<int>(ref.factoryVelLimits.x()) > 0);
    assert(static_cast<int>(ref.factoryVelLimits.y()) > 0);
    assert(static_cast<int>(ref.factoryVelLimits.z()) > 0);

    assert(static_cast<int>(ref.factoryAccLimits.x()) > 0);
    assert(static_cast<int>(ref.factoryAccLimits.y()) > 0);
    assert(static_cast<int>(ref.factoryAccLimits.z()) > 0);

    QVector3D maxPos, minPos, vel, acc;

    float minMaxRefPos, maxMinRefPos, minMaxPos = 0.f, maxMinPos = 1.f;

    QVector<float> refMaxPs, refMinPs, maxPs, minPs;

    switch(gtype)
    {
    case NO:

        refMaxPs = {ref.maxFactoryPosLimits.x(), ref.maxFactoryPosLimits.y(), ref.maxFactoryPosLimits.z()};
        refMinPs = {ref.minFactoryPosLimits.x(), ref.minFactoryPosLimits.y(), ref.minFactoryPosLimits.z()};

        minMaxRefPos = *std::min_element(QVector<float>::iterator(refMaxPs.begin()), QVector<float>::iterator(refMaxPs.end()));
        maxMinRefPos = *std::max_element(QVector<float>::iterator(refMinPs.begin()), QVector<float>::iterator(refMinPs.end()));

        while(static_cast<int>(minMaxPos - maxMinPos) < (minMaxRefPos - maxMinRefPos) / 4)
        {
            maxPos.setX(static_cast<float>(rand() % static_cast<int>(ref.maxFactoryPosLimits.x() - ref.minFactoryPosLimits.x()) *
                                               drand48()) + ref.minFactoryPosLimits.x() + 1);
            maxPos.setY(static_cast<float>(rand() % static_cast<int>(ref.maxFactoryPosLimits.y() - ref.minFactoryPosLimits.y()) *
                                               drand48()) + ref.minFactoryPosLimits.y() + 1);
            maxPos.setZ(static_cast<float>(rand() % static_cast<int>(ref.maxFactoryPosLimits.z() - ref.minFactoryPosLimits.z()) *
                                               drand48()) + ref.minFactoryPosLimits.z() + 1);

            assert(static_cast<int>(maxPos.x() - ref.minFactoryPosLimits.x()) > 0);
            assert(static_cast<int>(maxPos.y() - ref.minFactoryPosLimits.y()) > 0);
            assert(static_cast<int>(maxPos.z() - ref.minFactoryPosLimits.z()) > 0);

            minPos.setX(static_cast<float>(rand() % static_cast<int>(maxPos.x() - ref.minFactoryPosLimits.x()) *
                                               drand48()) + ref.minFactoryPosLimits.x());
            minPos.setY(static_cast<float>(rand() % static_cast<int>(maxPos.y() - ref.minFactoryPosLimits.y()) *
                                               drand48()) + ref.minFactoryPosLimits.y());
            minPos.setZ(static_cast<float>(rand() % static_cast<int>(maxPos.z() - ref.minFactoryPosLimits.z()) *
                                               drand48()) + ref.minFactoryPosLimits.z());

            maxPs = {maxPos.x(), maxPos.y(), maxPos.z()};
            minPs = {minPos.x(), minPos.y(), minPos.z()};

            maxMinPos = *std::max_element(QVector<float>::iterator(minPs.begin()), QVector<float>::iterator(minPs.end()));
            minMaxPos = *std::min_element(QVector<float>::iterator(maxPs.begin()), QVector<float>::iterator(maxPs.end()));
        }

        vel.setX(static_cast<float>(rand() % static_cast<int>(ref.factoryVelLimits.x()) * drand48()) + 1);
        vel.setY(static_cast<float>(rand() % static_cast<int>(ref.factoryVelLimits.y()) * drand48()) + 1);
        vel.setZ(static_cast<float>(rand() % static_cast<int>(ref.factoryVelLimits.z()) * drand48()) + 1);

        acc.setX(static_cast<float>(rand() % static_cast<int>(ref.factoryAccLimits.x()) * drand48()) + 1);
        acc.setY(static_cast<float>(rand() % static_cast<int>(ref.factoryAccLimits.y()) * drand48()) + 1);
        acc.setZ(static_cast<float>(rand() % static_cast<int>(ref.factoryAccLimits.z()) * drand48()) + 1);

        break;
    case OLDVALS:

        refMaxPs = {ref.maxPosLimits.x(), ref.maxPosLimits.y(), ref.maxPosLimits.z()};
        refMinPs = {ref.minPosLimits.x(), ref.minPosLimits.y(), ref.minPosLimits.z()};

        minMaxRefPos = *std::min_element(QVector<float>::iterator(refMaxPs.begin()), QVector<float>::iterator(refMaxPs.end()));
        maxMinRefPos = *std::max_element(QVector<float>::iterator(refMinPs.begin()), QVector<float>::iterator(refMinPs.end()));

        while(static_cast<int>(minMaxPos - maxMinPos) < (minMaxRefPos - maxMinRefPos) / 4)
        {
            maxPos.setX(static_cast<float>(rand() % static_cast<int>(ref.maxPosLimits.x() - ref.minPosLimits.x()) *
                                               drand48()) + ref.minPosLimits.x() + 1);
            maxPos.setY(static_cast<float>(rand() % static_cast<int>(ref.maxPosLimits.y() - ref.minPosLimits.y()) *
                                               drand48()) + ref.minPosLimits.y() + 1);
            maxPos.setZ(static_cast<float>(rand() % static_cast<int>(ref.maxPosLimits.z() - ref.minPosLimits.z()) *
                                               drand48()) + ref.minPosLimits.z() + 1);

            assert(static_cast<int>(maxPos.x() - ref.minPosLimits.x()) > 0);
            assert(static_cast<int>(maxPos.y() - ref.minPosLimits.y()) > 0);
            assert(static_cast<int>(maxPos.z() - ref.minPosLimits.z()) > 0);

            minPos.setX(static_cast<float>(rand() % static_cast<int>(maxPos.x() - ref.minPosLimits.x()) *
                                               drand48()) + ref.minPosLimits.x());
            minPos.setY(static_cast<float>(rand() % static_cast<int>(maxPos.y() - ref.minPosLimits.y()) *
                                               drand48()) + ref.minPosLimits.y());
            minPos.setZ(static_cast<float>(rand() % static_cast<int>(maxPos.z() - ref.minPosLimits.z()) *
                                               drand48()) + ref.minPosLimits.z());

            maxPs = {maxPos.x(), maxPos.y(), maxPos.z()};
            minPs = {minPos.x(), minPos.y(), minPos.z()};

            maxMinPos = *std::max_element(QVector<float>::iterator(minPs.begin()), QVector<float>::iterator(minPs.end()));
            minMaxPos = *std::min_element(QVector<float>::iterator(maxPs.begin()), QVector<float>::iterator(maxPs.end()));
        }

        vel.setX(static_cast<float>(rand() % static_cast<int>(ref.velLimits.x()) * drand48()) + 1);
        vel.setY(static_cast<float>(rand() % static_cast<int>(ref.velLimits.y()) * drand48()) + 1);
        vel.setZ(static_cast<float>(rand() % static_cast<int>(ref.velLimits.z()) * drand48()) + 1);

        acc.setX(static_cast<float>(rand() % static_cast<int>(ref.accLimits.x()) * drand48()) + 1);
        acc.setY(static_cast<float>(rand() % static_cast<int>(ref.accLimits.y()) * drand48()) + 1);
        acc.setZ(static_cast<float>(rand() % static_cast<int>(ref.accLimits.z()) * drand48()) + 1);

        break;
    }

    return  QVector<QVector3D>{maxPos, minPos, vel, acc};
}

QVector<QVector3D> StandProto::generateCurVals(const StandProto &ref, GenerateType type)
{
    assert(static_cast<int>(ref.maxPosLimits.x() - ref.minPosLimits.x()) > 0);
    assert(static_cast<int>(ref.maxPosLimits.y() - ref.minPosLimits.y()) > 0);
    assert(static_cast<int>(ref.maxPosLimits.z() - ref.minPosLimits.z()) > 0);
    assert(static_cast<int>(ref.velLimits.x()) > 0);
    assert(static_cast<int>(ref.velLimits.y()) > 0);
    assert(static_cast<int>(ref.velLimits.z()) > 0);
    assert(static_cast<int>(ref.accLimits.x()) > 0);
    assert(static_cast<int>(ref.accLimits.y()) > 0);
    assert(static_cast<int>(ref.accLimits.z()) > 0);

    QVector3D pos, vel, acc;

    switch(type)
    {
    case RANDOM:

        pos.setX(static_cast<float>(rand() % static_cast<int>(ref.maxPosLimits.x() - ref.minPosLimits.x()) *
                                           drand48()) + ref.minPosLimits.x());
        pos.setY(static_cast<float>(rand() % static_cast<int>(ref.maxPosLimits.y() - ref.minPosLimits.y()) *
                                           drand48()) + ref.minPosLimits.y());
        pos.setZ(static_cast<float>(rand() % static_cast<int>(ref.maxPosLimits.z() - ref.minPosLimits.z()) *
                                           drand48()) + ref.minPosLimits.z());

        vel.setX(static_cast<float>(rand() % static_cast<int>(ref.velLimits.x()) * drand48()));
        vel.setY(static_cast<float>(rand() % static_cast<int>(ref.velLimits.y()) * drand48()));
        vel.setZ(static_cast<float>(rand() % static_cast<int>(ref.velLimits.z()) * drand48()));

        acc.setX(static_cast<float>(rand() % static_cast<int>(ref.accLimits.x()) * drand48()));
        acc.setY(static_cast<float>(rand() % static_cast<int>(ref.accLimits.y()) * drand48()));
        acc.setZ(static_cast<float>(rand() % static_cast<int>(ref.accLimits.z()) * drand48()));
        break;
    case UNIFIED:
        QVector<float> minPs = {ref.minPosLimits.x(), ref.minPosLimits.y(), ref.minPosLimits.z()};
        QVector<float> maxPs = {ref.maxPosLimits.x(), ref.maxPosLimits.y(), ref.maxPosLimits.z()};
        QVector<float> vels = {ref.velLimits.x(), ref.velLimits.y(), ref.velLimits.z()};
        QVector<float> accs = {ref.accLimits.x(), ref.accLimits.y(), ref.accLimits.z()};

        float maxMinPos = *std::max_element(QVector<float>::iterator(minPs.begin()), QVector<float>::iterator(minPs.end()));
        float minMaxPos = *std::min_element(QVector<float>::iterator(maxPs.begin()), QVector<float>::iterator(maxPs.end()));
        float minVel = *std::min_element(QVector<float>::iterator(vels.begin()), QVector<float>::iterator(vels.end()));
        float minAcc = *std::min_element(QVector<float>::iterator(accs.begin()), QVector<float>::iterator(accs.end()));

        pos.setX(static_cast<float>(rand() % static_cast<int>(minMaxPos - maxMinPos) *
                                           drand48()) + maxMinPos);
        pos.setY(static_cast<float>(rand() % static_cast<int>(minMaxPos - maxMinPos) *
                                           drand48()) + maxMinPos);
        pos.setZ(static_cast<float>(rand() % static_cast<int>(minMaxPos - maxMinPos) *
                                           drand48()) + maxMinPos);

        vel.setX(static_cast<float>(rand() % static_cast<int>(minVel) * drand48()) + 1);
        vel.setY(static_cast<float>(rand() % static_cast<int>(minVel) * drand48()) + 1);
        vel.setZ(static_cast<float>(rand() % static_cast<int>(minVel) * drand48()) + 1);

        acc.setX(static_cast<float>(rand() % static_cast<int>(minAcc) * drand48()) + 1);
        acc.setY(static_cast<float>(rand() % static_cast<int>(minAcc) * drand48()) + 1);
        acc.setZ(static_cast<float>(rand() % static_cast<int>(minAcc) * drand48()) + 1);
        break;
    }
    return  QVector<QVector3D>{pos, vel, acc};
}

QVector<QVector3D> StandProto::generateCurVals(QVector3D maxPosLimits, QVector3D minPosLimits, QVector3D velLimits,
                                               QVector3D accLimits, GenerateType type)
{
    assert(static_cast<int>(maxPosLimits.x() - minPosLimits.x()) > 0);
    assert(static_cast<int>(maxPosLimits.y() - minPosLimits.y()) > 0);
    assert(static_cast<int>(maxPosLimits.z() - minPosLimits.z()) > 0);
    assert(static_cast<int>(velLimits.x()) > 0);
    assert(static_cast<int>(velLimits.y()) > 0);
    assert(static_cast<int>(velLimits.z()) > 0);
    assert(static_cast<int>(accLimits.x()) > 0);
    assert(static_cast<int>(accLimits.y()) > 0);
    assert(static_cast<int>(accLimits.z()) > 0);

    QVector3D pos, vel, acc;

    switch(type)
    {
    case RANDOM:

        pos.setX(static_cast<float>(rand() % static_cast<int>(maxPosLimits.x() - minPosLimits.x()) *
                                           drand48()) + minPosLimits.x());
        pos.setY(static_cast<float>(rand() % static_cast<int>(maxPosLimits.y() - minPosLimits.y()) *
                                           drand48()) + minPosLimits.y());
        pos.setZ(static_cast<float>(rand() % static_cast<int>(maxPosLimits.z() - minPosLimits.z()) *
                                           drand48()) + minPosLimits.z());

        vel.setX(static_cast<float>(rand() % static_cast<int>(velLimits.x()) * drand48()));
        vel.setY(static_cast<float>(rand() % static_cast<int>(velLimits.y()) * drand48()));
        vel.setZ(static_cast<float>(rand() % static_cast<int>(velLimits.z()) * drand48()));

        acc.setX(static_cast<float>(rand() % static_cast<int>(accLimits.x()) * drand48()));
        acc.setY(static_cast<float>(rand() % static_cast<int>(accLimits.y()) * drand48()));
        acc.setZ(static_cast<float>(rand() % static_cast<int>(accLimits.z()) * drand48()));
        break;
    case UNIFIED:
        QVector<float> minPs = {minPosLimits.x(), minPosLimits.y(), minPosLimits.z()};
        QVector<float> maxPs = {maxPosLimits.x(), maxPosLimits.y(), maxPosLimits.z()};
        QVector<float> vels = {velLimits.x(), velLimits.y(), velLimits.z()};
        QVector<float> accs = {accLimits.x(), accLimits.y(), accLimits.z()};

        float maxMinPos = *std::max_element(QVector<float>::iterator(minPs.begin()), QVector<float>::iterator(minPs.end()));
        float minMaxPos = *std::min_element(QVector<float>::iterator(maxPs.begin()), QVector<float>::iterator(maxPs.end()));
        float minVel = *std::min_element(QVector<float>::iterator(vels.begin()), QVector<float>::iterator(vels.end()));
        float minAcc = *std::min_element(QVector<float>::iterator(accs.begin()), QVector<float>::iterator(accs.end()));

        pos.setX(static_cast<float>(rand() % static_cast<int>(minMaxPos - maxMinPos) *
                                           drand48()) + maxMinPos);
        pos.setY(static_cast<float>(rand() % static_cast<int>(minMaxPos - maxMinPos) *
                                           drand48()) + maxMinPos);
        pos.setZ(static_cast<float>(rand() % static_cast<int>(minMaxPos - maxMinPos) *
                                           drand48()) + maxMinPos);

        vel.setX(static_cast<float>(rand() % static_cast<int>(minVel) * drand48()));
        vel.setY(static_cast<float>(rand() % static_cast<int>(minVel) * drand48()));
        vel.setZ(static_cast<float>(rand() % static_cast<int>(minVel) * drand48()));

        acc.setX(static_cast<float>(rand() % static_cast<int>(minAcc) * drand48()));
        acc.setY(static_cast<float>(rand() % static_cast<int>(minAcc) * drand48()));
        acc.setZ(static_cast<float>(rand() % static_cast<int>(minAcc) * drand48()));
        break;
    }
    return  QVector<QVector3D>{pos, vel, acc};
}

bool StandProto::getReady()
{
    return ready;
}

unsigned char StandProto::getMoving()
{
    return moving;
}

unsigned char StandProto::getActiveAxis()
{
    return activeAxis;
}

unsigned char StandProto::getStandEventReg()
{
    return standEventReg;
}

QVector3D StandProto::getMaxFactoryPosLimits()
{
    return maxFactoryPosLimits;
}

QVector3D StandProto::getMinFactoryPosLimits()
{
    return minFactoryPosLimits;
}

QVector3D StandProto::getFactoryAccLimits()
{
    return factoryAccLimits;
}

QVector3D StandProto::getFactoryVelLimits()
{
    return factoryVelLimits;
}

QVector3D StandProto::getMaxPosLimits()
{
    return maxPosLimits;
}

QVector3D StandProto::getMinPosLimits()
{
    return minPosLimits;
}

QVector3D StandProto::getAccLimits()
{
    return accLimits;
}

QVector3D StandProto::getVelLimits()
{
    return velLimits;
}

QVector3D StandProto::getPos()
{
    return curPos;
}

QVector3D StandProto::getAcc()
{
    return curAcc;
}

QVector3D StandProto::getVel()
{
    return curVel;
}

void StandProto::setReady(bool val)
{
    mutex.lock();
    ready = val;
    mutex.unlock();
}

void StandProto::setMoving(unsigned char val)
{
    mutex.lock();
    moving ^= val;
    mutex.unlock();
}

void StandProto::setFactoryLimits(QVector3D mxPos, QVector3D mnPos,
                                  QVector3D acc, QVector3D vel)
{
    setMaxFactoryPosLimits(mxPos);
    setMinFactoryPosLimits(mnPos);
    setFactoryVelLimits(vel);
    setFactoryAccLimits(acc);
}

void StandProto::setActiveAxis(unsigned char val)
{
    mutex.lock();
    activeAxis = val;
    mutex.unlock();
}

void StandProto::setStandEventReg(unsigned char val)
{
    mutex.lock();
    standEventReg = val;
    mutex.unlock();
}

void StandProto::setMaxFactoryPosLimits(QVector3D vec)
{
    mutex.lock();
    maxFactoryPosLimits = vec;
    mutex.unlock();
}

void StandProto::setMinFactoryPosLimits(QVector3D vec)
{
    mutex.lock();
    minFactoryPosLimits = vec;
    mutex.unlock();
}

void StandProto::setFactoryAccLimits(QVector3D vec)
{
    mutex.lock();
    factoryAccLimits = vec;
    mutex.unlock();
}

void StandProto::setFactoryVelLimits(QVector3D vec)
{
    mutex.lock();
    factoryVelLimits = vec;
    mutex.unlock();
}

void StandProto::setMaxPosLimits(QVector3D vec)
{
    mutex.lock();
    maxPosLimits = vec;
    mutex.unlock();
}

void StandProto::setMinPosLimits(QVector3D vec)
{
    mutex.lock();
    minPosLimits = vec;
    mutex.unlock();
}

void StandProto::setAccLimits(QVector3D vec)
{
    mutex.lock();
    accLimits = vec;
    mutex.unlock();
}

void StandProto::setVelLimits(QVector3D vec)
{
    mutex.lock();
    velLimits = vec;
    mutex.unlock();
}

void StandProto::setLimits(QVector3D mxPos, QVector3D mnPos,
                           QVector3D vel, QVector3D acc)
{
    setMaxPosLimits(mxPos);
    setMinPosLimits(mnPos);
    setVelLimits(vel);
    setAccLimits(acc);
}

void StandProto::setPos(QVector3D vec)
{
    mutex.lock();
    curPos = vec;
    mutex.unlock();
}

void StandProto::setAcc(QVector3D vec)
{
    mutex.lock();
    curAcc = vec;
    mutex.unlock();
}

void StandProto::setVel(QVector3D vec)
{
    mutex.lock();
    curVel = vec;
    mutex.unlock();
}

void StandProto::setCurs(QVector3D pos, QVector3D vel, QVector3D acc)
{
    setPos(pos);
    setAcc(acc);
    setVel(vel);
}
