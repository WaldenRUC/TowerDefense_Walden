#ifndef BULLET_WANDERING_H
#define BULLET_WANDERING_H
#include <QPoint>
#include <QSize>
#include <QPixmap>
#include <QObject>
#include <QPainter>
#include <QTimer>

class QPainter;
class Enemy;
class MainWindow;

class bullet_wandering : QObject
{
    Q_OBJECT
    //Q_PROPERTY(QPoint m_currentPos READ currentPos WRITE setCurrentPos)

public:
    bullet_wandering(QPoint startPos, int damage, MainWindow *game,
                     const QPixmap &sprite = QPixmap(":/image/bullet.png"));
    void draw(QPainter *painter) const;
    //void move();
    void hitTarget();
    QTimer          *move_timer;
    void setCurrentPos(QPoint pos);
    QPoint currentPos() const;
    qreal          angle;
    Enemy*          target;
public slots:
    void move();
private:
    const QPoint    m_startPos;
    const QPixmap   m_sprite;
    qreal           m_currentX;
    qreal           m_currentY;

    QPoint          m_currentPos;
    MainWindow *    m_game;
    int             m_damage;

    static const QSize ms_fixedSize;
};

#endif // BULLET_WANDERING_H
