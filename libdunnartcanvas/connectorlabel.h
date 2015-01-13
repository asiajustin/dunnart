/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2008  Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA  02110-1301, USA.
 *
 *
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
 *            Yiya Li         <http://yiya.li/>
*/

#ifndef CONNECTORLABEL_H
#define CONNECTORLABEL_H

#include <QtXml>
#include <QRectF>
#include <QString>
#include <QGraphicsItem>
#include <QGraphicsSvgItem>
#include <QSet>
#include <QMenu>
#include <QAction>
#include <QUndoCommand>


#include <set>
#include <iostream>

class QGraphicsSceneMouseEvent;

namespace dunnart {

class Canvas;
class ConnectorLabel;

typedef QSet<ConnectorLabel *> ConnectorLabelSet;

class ConnectorLabel: public QGraphicsSvgItem
{
    Q_OBJECT
    Q_PROPERTY (QString type READ itemType)
    Q_PROPERTY (QString id READ idString WRITE setIdString)
    Q_PROPERTY (QString label READ label WRITE setLabel)
    Q_PROPERTY (QColor fillColour READ fillColour WRITE setFillColour)
    Q_PROPERTY (QColor strokeColour READ strokeColour WRITE setStrokeColour)

    public:

        ConnectorLabel(QGraphicsItem *parent, QString id, unsigned int lev);
        virtual ~ConnectorLabel();

        virtual void initWithXMLProperties(Canvas *canvas,
                const QDomElement& node, const QString& ns);

        void setIdString(const QString& id);
        QString idString(void) const;

        uint internalId(void) const;

        QString itemType(void) const;
        void setItemType(const QString& type);

        Canvas *canvas(void) const;
        void bringToFront(void);
        void sendToBack(void);
        static ConnectorLabel *create(Canvas *canvas, const QDomElement& node,
                const QString& dunnartURI, int pass);
        virtual QDomElement to_QDomElement(const unsigned int subset,
                QDomDocument& doc);
        /*virtual void cascade_distance(int dist, unsigned int dir,
                ConnectorLabel **path) = 0;*/
        void update_after_unhide(void);
        bool cascade_logic(int& nextval, int dist, unsigned int dir,
                ConnectorLabel **path);
        void move_to(const int x, const int y, bool store_undo);
        void glowSetClipRect(QPixmap *surface);
        void glowClearClipRect(QPixmap *surface);
        virtual bool isCollapsed(void);

        virtual QString label(void) const;
        virtual void setLabel(const QString& label);
        QColor strokeColour(void) const;
        void setStrokeColour(const QColor& colour);
        QColor fillColour(void) const;
        void setFillColour(const QColor& colour);
#if 0
        CanvasItemFlags canvasItemFlags(void) const;
#endif
        void setAsCollapsed(bool collapsed);
        /*virtual void deactivateAll(ConnectorLabelSet& selSet) = 0;*/
        bool isInactive(void) const;
        void setAsInactive(bool inactive,
                ConnectorLabelSet fullSet = ConnectorLabelSet());
        void maybeReturn(void);
        virtual void findAttachedSet(ConnectorLabelSet& objSet)
        {
            Q_UNUSED(objSet);
        }
        //! @brief   Returns the bounding rectangle required to render the
        //!          canvas item within.
        //! @note    This can't be called from within the buildPainterPath()
        //!          method, since it depends on the painter path already
        //!          hacing been built.
        //! @returns A QRectF preresenting the item's bounding rectangle.
        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
        double width(void) const;
        double height(void) const;
        void setSize(const double w, const double h);
        void dragReleaseEvent(QGraphicsSceneMouseEvent *event);
        QSizeF size(void) const;
        virtual void setSize(const QSizeF& size);
        void setConstraintConflict(const bool conflict);
        bool constraintConflict(void) const;
        virtual void loneSelectedChange(const bool value);
        QString svgCodeAsString(const QSize& size, const QRectF& viewBox);
        void setIdealPos(QPointF pos);
        QPointF idealPos();

    protected:
        void setHoverMessage(const QString& message);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        //virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual QPainterPath buildPainterPath(void);
        QPainterPath painterPath(void) const;
        virtual void setPainterPath(QPainterPath path);

        // This method resizes the canvas item and also triggers the
        // painter path used for drawing to be recreated.
        void setSizeAndUpdatePainterPath(const QSizeF& newSize);

        QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                const QVariant &value);
        virtual void addXmlProps(const unsigned int subset, QDomElement& node,
                QDomDocument& doc);
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);

        // Libavoid related methods
        virtual void routerAdd(void);
        virtual void routerRemove(void);
        virtual void routerMove(void);
        virtual void routerResize(void);
#if 0
        void setCanvasItemFlag(CanvasItemFlag flag, bool enabled);
#endif
        QString m_string_id;
        uint m_internal_id;
        bool m_is_collapsed;
        bool m_is_inactive;

        // Used for constraint debugging:
        int m_connection_distance;
        int m_connection_cascade_glow;
        ConnectorLabel *m_connected_objects[2];

    private:
        friend class AlterConnectorLabelProperty;
        virtual void userMoveBy(qreal dx, qreal dy);

        QString m_type;
        QPainterPath m_painter_path;
        QSizeF m_size;
        QString m_hover_message;

        QString m_label;
        QColor m_fill_colour;
        QColor m_stroke_colour;
        QPointF ideal_pos;
#if 0
        CanvasItemFlags m_flags;
#endif
        bool m_constraint_conflict;
};

typedef QList<ConnectorLabel *> ConnectorLabelList;

}
#endif // CONNECTORLABEL_H

