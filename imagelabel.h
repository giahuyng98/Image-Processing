#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QWidget>
#include <QLabel>
#include <Qt>
#include <QMouseEvent>
#include <QPoint>
#include <QDebug>

class ImageLabel : public QLabel{
    Q_OBJECT
public:
    explicit ImageLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~ImageLabel();
    int getX();
    int getY();

protected:
    void mousePressEvent(QMouseEvent* event);
    int xPos, yPos;
};

#endif // IMAGELABEL_H
