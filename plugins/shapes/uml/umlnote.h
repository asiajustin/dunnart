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
        virtual QString label(void) const;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QDomElement to_QDomElement(const unsigned int subset, QDomDocument& doc);
        virtual void initWithXMLProperties(Canvas *canvas, const QDomElement& node, const QString& ns);

        /**
         *  Overrided, to set up any label text, colours, etc for the
         *  instance of the shape to be shown in the shape picker.
         */
        virtual void setupForShapePickerPreview(void);


        /**
         *  Overrided, to set up any label text, colours, etc for the
         *  instance of the shape created on the canvas when the users
         *  drags shapes from the shape picker.
         */
        virtual void setupForShapePickerDropOnCanvas(void);

    private:
        QString m_label;

        QDomElement newTextChild(QDomElement& node, const QString& ns, const QString& name, QString arg, QDomDocument& doc);

};
#endif // UMLNOTE_H
