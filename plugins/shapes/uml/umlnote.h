#ifndef UMLNOTE_H
#define UMLNOTE_H

#include "libdunnartcanvas/shape.h"
using namespace dunnart;

class NoteShape: public RectangleShape {

    public:
        NoteShape();
        virtual ~NoteShape() { }

        virtual QPainterPath buildPainterPath(void);
        virtual void setLabel(const QString& label);
        virtual QString getLabel(void) const;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QDomElement to_QDomElement(const unsigned int subset, QDomDocument& doc);
        virtual void initWithXMLProperties(Canvas *canvas, const QDomElement& node, const QString& ns);

    private:
        QString m_label;

        QDomElement newTextChild(QDomElement& node, const QString& ns, const QString& name, QString arg, QDomDocument& doc);

};
#endif // UMLNOTE_H
