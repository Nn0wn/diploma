#ifndef STANDPROTO_H
#define STANDPROTO_H

#include <QVector3D>
#include <QVector>
#include <QSettings>
#include <algorithm>
#include <cassert>
#include <QMutex>

#define POSITION_RANGE 50000
#define VELOCITY_RANGE 1000
#define ACCELERATION_RANGE 200

enum GenerateBoundings
{
    NO,
    OLDVALS
};

enum GenerateType
{
    RANDOM,
    UNIFIED
};

class StandProto
{
    bool            ready;
    unsigned char   activeAxis;
    unsigned char   standEventReg;
    unsigned char   moving;

    QVector3D       maxFactoryPosLimits;
    QVector3D       minFactoryPosLimits;
    QVector3D       factoryAccLimits;
    QVector3D       factoryVelLimits;

    QVector3D       maxPosLimits;
    QVector3D       minPosLimits;
    QVector3D       accLimits;
    QVector3D       velLimits;

    QVector3D       curPos;
    QVector3D       curAcc;
    QVector3D       curVel;

    QMutex          mutex;

public:
    StandProto(QSettings *limits = nullptr);
    ~StandProto() = default;

    static QVector<QVector3D> generateFactoryLimits(int pRange = POSITION_RANGE,
                                                    int vRange = VELOCITY_RANGE,
                                                    int aRange = ACCELERATION_RANGE);
    static QVector<QVector3D> generateLimits(const StandProto &ref, GenerateBoundings gtype = NO);
    static QVector<QVector3D> generateCurVals(const StandProto &ref, GenerateType type = RANDOM);
    static QVector<QVector3D> generateCurVals(QVector3D maxPosLimits, QVector3D minPosLimits,
                                              QVector3D velLimits, QVector3D accLimits,
                                              GenerateType type = RANDOM);

    bool getReady();
    unsigned char getActiveAxis();
    unsigned char getStandEventReg();
    unsigned char getMoving();

    QVector3D getMaxFactoryPosLimits();
    QVector3D getMinFactoryPosLimits();
    QVector3D getFactoryAccLimits();
    QVector3D getFactoryVelLimits();

    QVector3D getMaxPosLimits();
    QVector3D getMinPosLimits();
    QVector3D getAccLimits();
    QVector3D getVelLimits();

    QVector3D getPos();
    QVector3D getAcc();
    QVector3D getVel();

    void setReady(bool val);
    void setActiveAxis(unsigned char val);
    void setStandEventReg(unsigned char val);
    void setMoving(unsigned char val);

    void setFactoryLimits(QVector3D mxPos, QVector3D mnPos,
                          QVector3D acc, QVector3D vel);

    void setMaxFactoryPosLimits(QVector3D vec);
    void setMinFactoryPosLimits(QVector3D vec);
    void setFactoryAccLimits(QVector3D vec);
    void setFactoryVelLimits(QVector3D vec);

    void setMaxPosLimits(QVector3D vec);
    void setMinPosLimits(QVector3D vec);
    void setAccLimits(QVector3D vec);
    void setVelLimits(QVector3D vec);

    void setLimits(QVector3D mxPos, QVector3D mnPos,
                   QVector3D vel, QVector3D acc);

    void setPos(QVector3D vec);
    void setAcc(QVector3D vec);
    void setVel(QVector3D vec);

    void setCurs(QVector3D pos, QVector3D vel, QVector3D acc);
};

#endif // STANDPROTO_H
