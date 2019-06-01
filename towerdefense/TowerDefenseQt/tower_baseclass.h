#ifndef TOWER_BASECLASS_H
#define TOWER_BASECLASS_H

#include <QObject>
#include <QPoint>

class tower_baseclass
{
public:
    virtual const QPoint getpos()=0;
    virtual void setrange(int range)= 0;
    virtual void setdamage(int damage)=0;




};

#endif // TOWER_BASECLASS_H
