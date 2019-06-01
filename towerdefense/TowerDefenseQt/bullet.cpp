#include "bullet.h"
#include "enemy.h"
#include "mainwindow.h"
#include <QPainter>
#include <QPropertyAnimation>

const QSize Bullet::ms_fixedSize(8, 8);

Bullet::Bullet(QPoint startPos, QPoint targetPoint, int damage, Enemy *target,
			   MainWindow *game, const QPixmap &sprite/* = QPixmap(":/image/bullet.png")*/)
	: m_startPos(startPos)
	, m_targetPos(targetPoint)
	, m_sprite(sprite)
	, m_currentPos(startPos)
	, m_target(target)
	, m_game(game)
	, m_damage(damage)
{
}

void Bullet::draw(QPainter *painter) const
{
	painter->drawPixmap(m_currentPos, m_sprite);
}

void Bullet::move()
{
	// 100毫秒内击中敌人
    if(m_target && !m_game->enemyList().empty()){//如果有敌人，才能进攻 补充empty
    static const int duration = 50;//原来100
	QPropertyAnimation *animation = new QPropertyAnimation(this, "m_currentPos");
    animation->setDuration(duration);//动画持续时间

    animation->setStartValue(m_startPos);//动画起始点坐标
    animation->setEndValue(m_targetPos);//动画结束点坐标
    connect(animation, SIGNAL(finished()), this, SLOT(hitTarget()));
    //信号与槽：animation 的 finished() 函数作为一个信号， 发送到bullet(就是this) 的hitTarget()槽中，
    //含义：如果动画结束了（子弹打中目标了，执行hitTarget()函数，判断敌人是否存在
    //connect(animation, SIGNAL(finished()), this, SLOT());//打中目标，减速。
    //animation->setEasingCurve(QEasingCurve::OutInCirc);
    animation->start();
    }
}

void Bullet::hitTarget()
{
	// 这样处理的原因是:
	// 可能多个炮弹击中敌人,而其中一个将其消灭,导致敌人delete
	// 后续炮弹再攻击到的敌人就是无效内存区域
	// 因此先判断下敌人是否还有效

    if (m_target && !m_game->enemyList().empty() && m_game->enemyList().indexOf(m_target) != -1)//empty
        {
            if(m_target->invincible==0)//并非无敌
            m_target->getDamage(m_damage);//m_target指向的敌人受到伤害
            if(m_target->getcurrentHp()<=0)
            {
                m_target->getRemoved();
                m_game->removedEnemy(m_target);
            }
        }
        m_game->removedBullet(this);
}

void Bullet::setCurrentPos(QPoint pos)
{
	m_currentPos = pos;
}

QPoint Bullet::currentPos() const
{
	return m_currentPos;
}
