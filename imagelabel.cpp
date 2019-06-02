#include "imagelabel.h"


ImageLabel::ImageLabel(QWidget *parent, Qt::WindowFlags f) : QLabel (parent)
{}

ImageLabel::~ImageLabel()
{}

int ImageLabel::getX()
{
    int pos = xPos - (this->width() - this->pixmap()->width()) / 2.f;
    return (pos < 0 || pos > this->pixmap()->width()) ? 0 : pos;
}

int ImageLabel::getY()
{
    int pos = yPos - (this->height() - this->pixmap()->height()) / 2.f;
    return (pos < 0 || pos > this->pixmap()->height()) ? 0 : pos;
}

void ImageLabel::mousePressEvent(QMouseEvent *event)
{
    xPos = event->x();
    yPos = event->y();
}
