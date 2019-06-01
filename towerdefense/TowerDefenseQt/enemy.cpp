#include "enemy.h"
#include "waypoint.h"
#include "tower.h"
#include "utility.h"
#include "mainwindow.h"
#include "audioplayer.h"
#include <QPainter>
#include <QColor>
#include <QDebug>
#include <QMatrix>
#include <QVector2D>
#include <QtMath>
#include <QTime>

static const int Health_Bar_Width = 20;
static const int HP = 60000;
static const int TIMES = 2;

const QSize Enemy::ms_fixedSize(52, 52);

Enemy::Enemy(WayPoint *startWayPoint, MainWindow *game, const QPixmap &sprite/* = QPixmap(":/image/enemy.png")*/)
	: QObject(0)
    , slow(1.0)//减速比例
    , times(1.0)
    , invincible(0)//可以瞄准
    , stop(1.0)//如果stop=0,则不动
	, m_active(false)
    , m_maxHp(HP)//原先40
    , m_currentHp(HP)//原先40
    , m_walkingSpeed(0.5)//原先1.0
	, m_rotationSprite(0.0)
	, m_pos(startWayPoint->pos())
	, m_destinationWayPoint(startWayPoint->nextWayPoint())
	, m_game(game)
	, m_sprite(sprite)
{
    //加速
    m_accelerate = new QTimer(this);
    connect(m_accelerate, SIGNAL(timeout()),this, SLOT(changespeed()));
    //m_accelerate->start(1000);//这个在创建的时候就激活了，所以所有的敌人加速效果一样
    //加速
    m_back_to_origin = new QTimer(this);
    connect(m_back_to_origin, SIGNAL(timeout()), this, SLOT(back_to_origin()));
}

Enemy::~Enemy()
{
	m_attackedTowersList.clear();
    m_destinationWayPoint = NULL;

    m_accelerate=NULL;
    m_back_to_origin = NULL;
    //m_active=false;////
    m_game = NULL;
}

void Enemy::doActivate()
{
	m_active = true;
    m_accelerate->start(1000);//当进入战场时再激活，但当敌人到达终点时被打会闪退！此时操作：在析构函数中m_accelerate=NULL;
}

void Enemy::move()
{
    /*
    QPoint A(15,250);
    if(collisionWithCircle(A,1,this->pos(),1)){
        invincible=1;//invincible
    }
    */
    /*if(m_attackedTowersList.size()==0)slow=1.0;//减速机制：如果没被攻击，则保持原速
    else slow=0.5;//如果被这些塔攻击，则slow系数改变为0.5
    */
    if (!m_active)
		return;

	if (collisionWithCircle(m_pos, 1, m_destinationWayPoint->pos(), 1))
	{
		// 敌人抵达了一个航点
		if (m_destinationWayPoint->nextWayPoint())
		{
			// 还有下一个航点
			m_pos = m_destinationWayPoint->pos();
            m_destinationWayPoint = m_destinationWayPoint->nextWayPoint();//将目标点设置为下一个航点的位置
		}
		else
		{
			// 表示进入基地
            m_game->getHpDamage();//基地损失血量
            m_game->removedEnemy(this);//移除敌人
			return;
		}
	}

	// 还在前往航点的路上
	// 目标航点的坐标
    QPoint targetPoint = m_destinationWayPoint->pos();//用targetPoint获取下一个目标点的坐标
	// 未来修改这个可以添加移动状态,加快,减慢,m_walkingSpeed是基准值
	// 向量标准化
    qreal movementSpeed = m_walkingSpeed */* times * */slow * stop ;//乘以减速比例

	QVector2D normalized(targetPoint - m_pos);
    normalized.normalize();//标准化
	m_pos = m_pos + normalized.toPoint() * movementSpeed;

	// 确定敌人选择方向
	// 默认图片向左,需要修正180度转右
    m_rotationSprite = qRadiansToDegrees(qAtan2(normalized.y(), normalized.x())) + 180;//normalize()函数是做什么用的？
    //if(qRadiansToDegrees(qAtan2(normalized.y(),normalized.x()))<=90)m_rotationSprite = 180;
    //else m_rotationSprite = 0;
}
void Enemy::changespeed()//改变敌人的速度
{
    if(times>=TIMES)times=1;
    else times=TIMES;
}
void Enemy::draw(QPainter *painter) const
{
	if (!m_active)
		return;

	painter->save();

	QPoint healthBarPoint = m_pos + QPoint(-Health_Bar_Width / 2 - 5, -ms_fixedSize.height() / 3);
	// 绘制血条
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::red);

	QRect healthBarBackRect(healthBarPoint, QSize(Health_Bar_Width, 2));
	painter->drawRect(healthBarBackRect);

    if(m_currentHp>=15000)painter->setBrush(Qt::green);//原先：green
    else if(m_currentHp>=5000)painter->setBrush(Qt::yellow);//
    else painter->setBrush(Qt::black);//
    //以上：敌人的血量与血条颜色相关

	QRect healthBarRect(healthBarPoint, QSize((double)m_currentHp / m_maxHp * Health_Bar_Width, 2));
	painter->drawRect(healthBarRect);

	// 绘制偏转坐标,由中心+偏移=左上
	static const QPoint offsetPoint(-ms_fixedSize.width() / 2, -ms_fixedSize.height() / 2);
	painter->translate(m_pos);
	painter->rotate(m_rotationSprite);
	// 绘制敌人
	painter->drawPixmap(offsetPoint, m_sprite);

	painter->restore();
}

void Enemy::getRemoved()
{
    if (m_attackedTowersList.empty())
        return;

    foreach (Tower *attacker, m_attackedTowersList)
        attacker->targetKilled();//通知所有打到敌人的防御塔，这个目标已经被清除
}

void Enemy::getRemoved_light_tower()
{
    if(m_attackedlight_towersList.empty())
        return;
    foreach(light_tower *attacker, m_attackedlight_towersList)
        attacker->targetKilled();
    //if()
    //if(this)
    //m_game->removedEnemy(this);
}

void Enemy::back_to_origin()
{
    stop=1;
}

void Enemy::getDamage(int damage)
{
    /*
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    int test=qrand()%10;

    if(test<=1){
        stop=0;
        //m_game->pp->setX(this->pos().rx());有问题
        //m_game->pp->setY(this->pos().ry());有问题
    }
    //m_game->pp=NULL;有问题
    m_back_to_origin->start(100);//10% 定身0.1s
    //以上，随机一个数使得子弹有定时效果*/

	m_game->audioPlayer()->playSound(LaserShootSound);
    if(this->invincible==0)
    m_currentHp = m_currentHp - damage;//enemy受到damage伤害//修改：*time
    /*
    foreach(Enemy *p, m_game->enemyList()){
        if(m_game->enemyList().size()!=0&&p!=NULL){
            if(collisionWithCircle(this->pos(),20,p->pos(),20))//被瞄准的目标和当前遍历到的目标距离小于40，则受到溅射伤害
            {
                if(p!=this)
                    p->m_currentHp -= damage*0.2;//如果不是瞄准的目标，只会受到20%的伤害
                else p->m_currentHp -= damage;//如果是瞄准的目标，则受到全额伤害

                //if(p->m_currentHp<=0)
                //{
                    //m_game->awardGold(200);//如果这里就奖励金钱，则金钱会突然变多
                    //if(p)
                    //m_game->removedEnemy(p);
                    //如果这里就把p释放了，空指针报错
                //}

            }
        }
    }
    */
    /*foreach(Enemy *p, m_game->enemyList()){//再循环一遍，把所有体力值小于等于零的敌人清除
        if(p->m_currentHp<=0){
            //p->getRemoved();//会闪退
            m_game->awardGold(200);//给玩家钱
            p->getRemoved();//这时候就可以了
        }
    }//溅射伤害设计,但好像到终点时会闪退
    */
// 阵亡,需要移除
    /*
    if (m_currentHp <= 0)
	{
		m_game->audioPlayer()->playSound(EnemyDestorySound);
        m_game->awardGold(200);
		getRemoved();
    }
    */

}

void Enemy::getAttacked_by_tower(Tower *attacker)
{
    m_attackedTowersList.push_back(attacker);
}

// 表明敌人已经逃离了攻击范围
void Enemy::gotLostSight_by_tower(Tower *attacker)
{
    m_attackedTowersList.removeOne(attacker);////如果列表里没有attacker也能移除吗
}

void Enemy::gotLostSight_by_light_tower(light_tower *attacker)
{
    m_attackedlight_towersList.removeOne(attacker);
}

void Enemy::getAttacked_by_light_tower(light_tower *attacker)
{
    m_attackedlight_towersList.push_back(attacker);
}

QPoint Enemy::pos() const
{
	return m_pos;
}
