#include "bullet_wandering.h"
#include "mainwindow.h"
#include <QPainter>
#include <QPropertyAnimation>
#include <utility.h>
#include <QVector2D>
#include <QtMath>
#include <QList>
#include <enemy.h>
#include <QTimer>
#include <qmath.h>

const QSize bullet_wandering::ms_fixedSize(8, 8);

bullet_wandering::bullet_wandering(QPoint startPos,  int damage, MainWindow *game,
                                   const QPixmap &sprite/* = QPixmap(":/image/bullet.png")*/)
    : m_startPos(startPos)
    , m_damage(damage)
    , m_game(game)
    , m_sprite(sprite)
    , m_currentPos(startPos)
    , m_currentX(startPos.x())
    , m_currentY(startPos.y())
{
    target = NULL;
    move_timer = new QTimer(this);
    connect(move_timer, SIGNAL(timeout()), this, SLOT(move()));
    move_timer->start(50);
}

void bullet_wandering::draw(QPainter *painter) const
{
    //painter->drawPixmap(m_currentX,m_currentY,10,10,m_sprite);
    painter->drawPixmap(m_currentPos, m_sprite);
}

void bullet_wandering::move()
{
    int flag=0;
    foreach(Enemy *enemy, m_game->enemyList()){
        if(collisionWithCircle(enemy->pos(), 10, m_currentPos, 10)){
            flag=1;
            target = enemy;
            move_timer->stop();
            hitTarget();
            break;
        }
    }
    if(flag==0){
        //移动
        if(m_currentPos.x()>=1000||m_currentPos.x()<=0||m_currentPos.y()>=1000||m_currentPos.x()<=0){
        //if(m_currentX>=1000||m_currentX<=0||m_currentY>=1000||m_currentY<=1000){
            m_game->removedBullet_wandering(this);
        }
        else{
        qreal movementSpeed = 10.0;
        qreal xtemp = movementSpeed * qCos(angle);
        qreal ytemp = movementSpeed * qSin(angle);
        //m_currentX += xtemp;
        //m_currentY += ytemp;
        m_currentPos = m_currentPos + QPoint(xtemp, ytemp);
        }
    }
}

void bullet_wandering::hitTarget(){
    if(target && !m_game->enemyList().empty() && m_game->enemyList().indexOf(target)!=-1){
        target->getDamage(m_damage);
        if(target->getcurrentHp()<=0){
            target->getRemoved();
            m_game->removedEnemy(target);
        }
    }

    m_game->removedBullet_wandering(this);
}

QPoint bullet_wandering::currentPos() const{
    return m_currentPos;
}

void bullet_wandering::setCurrentPos(QPoint pos){
    m_currentPos = pos;
}
