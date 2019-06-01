#ifndef TOWERPOSITION_H
#define TOWERPOSITION_H

#include <QPoint>
#include <QSize>
#include <QPixmap>

class QPainter;

class TowerPosition
{
public:
	TowerPosition(QPoint pos, const QPixmap &sprite = QPixmap(":/image/open_spot.png"));

    void setHasTower(bool hasTower = true);
	bool hasTower() const;
	const QPoint centerPos() const;
    bool containPoint(const QPoint &pos) const;//是否包括了pos这个点的坐标
    /*void donothastower() const{
        !m_hasTower;
    }*/
	void draw(QPainter *painter) const;


    mutable bool		m_hasTower;//是否有防御塔
private:
    QPoint		m_pos;//这个点应该是左上角？然后用ms_fixedSize扩展出四个顶点
	QPixmap		m_sprite;

	static const QSize ms_fixedSize;
};

#endif // TOWERPOSITION_H
