#ifndef UMLPACKAGE_H
#define UMLPACKAGE_H

#include "libdunnartcanvas/shape.h"
using namespace dunnart;

class PackageShape: public RectangleShape {
    Q_OBJECT

    Q_PROPERTY (bool expanded READ isExpanded WRITE setExpanded)

    public:
        PackageShape();
        virtual ~PackageShape() { }

        virtual QAction *buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu);
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

        bool isExpanded();
        void setExpanded(bool expanded);

    private:
        QString m_label;
        int lineWidth;
        int fontHeight;
        bool m_expanded;

        QDomElement newTextChild(QDomElement& node, const QString& ns, const QString& name, QString arg, QDomDocument& doc);

};
#endif // UMLPACKAGE_H

