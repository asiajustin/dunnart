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

#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QSvgGenerator>
#include <QStyleOptionGraphicsItem>

#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/freehand.h"
#include "libdunnartcanvas/textshape.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/polygon.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/svgshape.h"
#include "libdunnartcanvas/pluginshapefactory.h"
#include "libdunnartcanvas/connector.h"

#include "connectorlabel.h"

#include "libavoid/router.h"


namespace dunnart {


typedef QMap<ConnectorLabel *, QDomElement> ConnectorLabelXmlList;
static ConnectorLabelXmlList inactiveObjXml;
static ConnectorLabelList inactiveObjList;

ConnectorLabel::ConnectorLabel(QGraphicsItem *parent, QString id, unsigned int lev)
        : QGraphicsSvgItem(parent),
          m_string_id(id),
          m_internal_id(0),
          m_is_collapsed(false),
          m_is_inactive(false),
          m_connection_distance(-1),
          m_connection_cascade_glow(false),
          m_size(QSizeF(30, 20)),
          m_constraint_conflict(false),
          m_fill_colour(QColor(0,0,0,0)),
          m_stroke_colour(QColor(0,255,255,0)),
          m_label("")
{
    //Q_UNUSED (parent)

    // QGraphicsSvgItem sets a cache type that causes problems for printing,
    // so disable this for our normal ConnectorLabels.
    setCacheMode(QGraphicsItem::NoCache);

    setZValue(lev);
    m_connected_objects[0] = m_connected_objects[1] = NULL;

    setItemType("connectorLabel");
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, false); // true --> false
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif
    setCursor(Qt::OpenHandCursor);
    //setCanvasItemFlag(CanvasItem::ItemIsMovable, true);

    // Initial painter path.
    m_painter_path = buildPainterPath();

}

// This method reads object properties from an xml node.
void ConnectorLabel::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    Q_UNUSED (canvas)
    Q_UNUSED (ns)
    // Set the object id.
    this->m_string_id = essentialProp<QString>(node, x_id);

    // Set dynamic properties for any attributes not recognised and handled
    // by Dunnart.
    QDomNamedNodeMap attrs = node.attributes();
    for (int i = 0; i < attrs.length(); ++i)
    {
        QDomNode prop = attrs.item(i);
        QString name = prop.localName();
        if ( ! prop.prefix().isEmpty() && prop.prefix() != x_dunnartNs)
        {
            QString propName = prop.prefix() + ":" + prop.localName();
            setProperty(propName.toLatin1().constData(), prop.nodeValue());
        }
    }
}


ConnectorLabel::~ConnectorLabel()
{
    resetQueryModeIllumination(false);
}


void ConnectorLabel::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (canvas() == NULL)
    {
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        // Drop through to parent handler.
    }
    else if (event->button() == Qt::RightButton)
    {
        QMenu menu;
        QAction *action = buildAndExecContextMenu(event, menu);

        if (action)
        {
            event->accept();
        }
    }

    QGraphicsItem::mousePressEvent(event);
}


void ConnectorLabel::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragReleaseEvent(event);
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

bool ConnectorLabel::constraintConflict(void) const
{
    return m_constraint_conflict;
}

void ConnectorLabel::setConstraintConflict(const bool conflict)
{
    m_constraint_conflict = conflict;
    this->update();
}

QSizeF ConnectorLabel::size(void) const
{
    return m_size;
}

void ConnectorLabel::dragReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED (event)

    canvas()->setDraggedItem(NULL);
}

/*
void ConnectorLabel::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Actions& actions = canvas()->getActions();
    QPointF diff = event->pos() - event->lastPos();

    canvas()->setDraggedItem(this);

    foreach (ConnectorLabel *item, canvas()->selectedItems())
    {
        if (/*item->canvasItemFlags() & ItemIsMovable)
        {
            // Only move movable items.
            item->userMoveBy(diff.x(), diff.y());
            actions.moveList.push_back(item);
            canvas()->highlightIndicatorsForItemMove(item);
        }
    }
    canvas()->processResponseTasks();

    // Still do routing even when the layout thread is suspended.
    // XXX We need to only reroute connectors attached to moved shape
    //     during dragging if topology layout is turned on.
    if (canvas()->isLayoutSuspended())
    {
        reroute_connectors(canvas());
    }

    // We handle the moving of the shape, so no need to
    // call QGraphicsItem::mouseMoveEvent().
}
*/

void ConnectorLabel::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED (event)

    if (canvas())
    {
        canvas()->pushStatusMessage(m_hover_message.arg(m_internal_id));
    }
    setStrokeColour(QColor(0, 255, 255, 255));
}

void ConnectorLabel::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED (event)

    if (canvas())
    {
        canvas()->popStatusMessage();
    }
    setStrokeColour(QColor(0, 255, 255, 0));
}

void ConnectorLabel::setHoverMessage(const QString& message)
{
    m_hover_message = message;
}


void ConnectorLabel::userMoveBy(qreal dx, qreal dy)
{
    // The standard case is to just move the shape.
    // Subclasses may provide their own version to do other work.
    moveBy(dx, dy);
}

Canvas *ConnectorLabel::canvas(void) const
{
    return dynamic_cast<Canvas *> (scene());
}

void ConnectorLabel::setIdealPos(QPointF pos)
{
    ideal_pos = pos;
    setPos(pos);
}

QPointF ConnectorLabel::idealPos()
{
    if (ideal_pos.isNull())
    {
        return pos();
    }
    else
    {
        return ideal_pos;
    }
}

QRectF ConnectorLabel::boundingRect(void) const
{
    // We expect the painter path to have a valid bounding rect if this
    // is called.  If it is invalid, it may be being called incorrectly
    // from within buildPainterPath().
    assert(!m_painter_path.boundingRect().isNull());

    // Return the boundingRect, with padding for drawing selection cue.
    return expandRect(m_painter_path.boundingRect(), 0);
}


QPainterPath ConnectorLabel::shape() const
{
    return m_painter_path;
}

QString ConnectorLabel::label(void) const
{
    return m_label;
}


void ConnectorLabel::setLabel(const QString& label)
{
    m_label = label;
    if (m_label.isEmpty())
    {
        setSize(QSizeF(30, 20));
    }
    update();

    if (isSelected())
    {
        canvas()->fully_restart_graph_layout();
    }
}

QColor ConnectorLabel::strokeColour(void) const
{
    return m_stroke_colour;
}

void ConnectorLabel::setStrokeColour(const QColor& colour)
{
    m_stroke_colour = colour;
    update();
}


QColor ConnectorLabel::fillColour(void) const
{
    if (/*(this == queryObj) ||*/ m_connection_cascade_glow || constraintConflict())
    {
        return HAZARD_COLOUR;
    }
    return m_fill_colour;
}

void ConnectorLabel::setFillColour(const QColor& colour)
{
    m_fill_colour = colour;
    update();
}

void ConnectorLabel::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED (option)
    Q_UNUSED (widget)
    //assert(painter->isActive());

    bool showDecorations = canvas() && ! canvas()->isRenderingForPrinting();

    if ( isSelected() && showDecorations && canvas()->inSelectionMode() )
    {
        QPen highlight;
        highlight.setColor(QColor(0, 255, 255, 255));
        highlight.setWidth(3.5);    // 7-->3.5
        highlight.setCosmetic(true);
        // Draw selection cue.
        painter->setPen(highlight);
        painter->drawPath(painterPath());
    }

#if 0
    // Shadow.
    if (canvas()->isLayoutSuspended(); && ((type == SHAPE_DRAW_HIGHLIGHTED) ||
                (type == SHAPE_DRAW_LEAD_HIGHLIGHTED)))
    {
        boxColor(surface, x, y + 6, x + w - 8, y + h - 1,
                QColor(66, 60, 60, 100));
    }
#endif

    QPen pen;
    pen.setColor(strokeColour());
    pen.setWidth(3.5);
    pen.setCosmetic(true);

    painter->setPen(pen);
    painter->setBrush(QBrush(fillColour()));

    painter->drawPath(painterPath());

    if (!m_label.isEmpty())
    {
        QFontMetrics qfm(canvas()->canvasFont());
        int maxWordLength = 0;
        QStringList wordList = m_label.split(" ");

        for (int i = 0; i < wordList.length(); i++)
        {
            int currentWordLength = qfm.width(wordList.value(i));
            maxWordLength = (maxWordLength >= currentWordLength) ? maxWordLength : currentWordLength;
        }

        double singleLineLength = qfm.width(m_label);

        if (width() > singleLineLength)
        {
            setSize(QSizeF(singleLineLength, qreal(height())));
        }

        double currentWidth = qMax(width(), qreal(maxWordLength));

        double tempWidth = 0.0;
        int lineCount = 1;

        for (int i = 0; i < wordList.length(); i++)
        {
            bool spaceAdded = false;
            double currentWordLength = qreal(qfm.width(wordList.value(i)));
            if (i != wordList.length() - 1 && qfm.width(wordList.value(i)) != maxWordLength)
            {
                spaceAdded = true;
                currentWordLength += qreal(qfm.width(" "));
            }
            double currentTotalLength = tempWidth + currentWordLength;
            if (currentTotalLength <= currentWidth)
            {
                tempWidth += currentWordLength;
            }
            else
            {
                if (spaceAdded && currentTotalLength - qreal(qfm.width(" ")) <= currentWidth)
                {
                    tempWidth += qreal(qfm.width(wordList.value(i)));
                }
                else
                {
                    tempWidth = currentWordLength;
                    ++lineCount;
                }
            }
        }

        double currentHeight = qMax(20.0, qreal(lineCount * qfm.height() + (lineCount - 1) * qfm.leading()));

        setSize(QSizeF(currentWidth, currentHeight));
        canvas()->repositionAndShowSelectionResizeHandles(true);
    }

    painter->setPen(Qt::black);
    if (canvas())
    {
        painter->setFont(canvas()->canvasFont());
    }
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->drawText(boundingRect(), Qt::AlignTop | Qt::AlignHCenter | Qt::TextWordWrap, m_label);

}


void ConnectorLabel::setIdString(const QString& id)
{
    if (canvas())
    {
        m_string_id = canvas()->assignStringId(id);
    }
    else
    {
        m_string_id = id;
    }
}

QString ConnectorLabel::idString(void) const
{
    return m_string_id;
}

uint ConnectorLabel::internalId(void) const
{
    return m_internal_id;
}

QString ConnectorLabel::itemType(void) const
{
    return m_type;
}

void ConnectorLabel::setItemType(const QString& type)
{
    m_type = type;
}

void ConnectorLabel::bringToFront(void)
{
    QList<QGraphicsItem *> overlapItems = collidingItems();

    foreach (QGraphicsItem *item, overlapItems)
    {
        if (item->zValue() == zValue())
        {
            item->stackBefore(this);
            item->update();
        }
    }
}


void ConnectorLabel::sendToBack(void)
{
    QList<QGraphicsItem *> overlapItems = collidingItems();

    foreach (QGraphicsItem *item, overlapItems)
    {
        if (item->zValue() == zValue())
        {
            this->stackBefore(item);
            this->update();
        }
    }
}

/*
QColor QColorFromRRGGBBAA(char *str)
{
    unsigned int rrggbbaa = strtoul(str, NULL, 16);

    int alpha = rrggbbaa & 0xFF;
    unsigned int rrggbb = rrggbbaa >> 8;

    QColor colour((QRgb) rrggbb);
    colour.setAlpha(alpha);

    return colour;
}


QString nodeAttribute(const QDomElement& node, const QString& ns,
        const QString& prop)
{
    QDomNamedNodeMap attribs = node.attributes();

    for (int i = 0; i < attribs.count(); ++i)
    {
        QDomNode node = attribs.item(i);

        if (qualify(ns, prop) == node.nodeName())
        {
            return node.nodeValue();
        }
    }
    return QString();
}

bool nodeHasAttribute(const QDomElement& node, const QString& ns,
        const QString& prop)
{
    return !nodeAttribute(node, ns, prop).isNull();
}

ConnectorLabel *ConnectorLabel::create(Canvas *canvas, const QDomElement& node,
        const QString& dunnartURI, int pass)
{
    QString type = nodeAttribute(node, dunnartURI, x_type);
    assert(!type.isEmpty());

    QStringList oldConnTypes;
    oldConnTypes << "connStraight" << "connAvoidCurved" << "connAvoidPoly" <<
                    "connOrthogonal" << "connAvoidOrtho";

    ConnectorLabel *newObj = NULL;

    if (pass == PASS_SHAPES)
    {
        ShapeObj *shape = NULL;

        // Support legacy shapes from Dunnart v1.
        if (type == "flowEndOProc")
        {
            type = "roundedRect";
        }
        else if (type == "flowDiamond")
        {
            type = "diamond";
        }
        else if (type == "indGuide")
        {
            type = x_guideline;
        }

        if (type == x_guideline)
        {
            newObj = new Guideline(canvas, node, dunnartURI);
        }
        else if (type == x_distribution)
        {
            newObj = new Distribution(node, dunnartURI);
        }
        else if (type == x_separation)
        {
            newObj = new Separation(node, dunnartURI);
        }
        else if (!type.isEmpty() && (type != x_cluster) &&
                (type != x_constraint) && (type != x_connector) &&
                ! oldConnTypes.contains(type))
        {
            if (type == x_svgNode)
            {
                shape = new SvgShape();
            }
            else if (type == x_shTextShape)
            {
                shape = new TextShape();
            }
            else if (type == x_shPolygon)
            {
                shape = new PolygonShape();
            }
            else if (type == x_shFreehand)
            {
                shape = new FreehandShape();
            }
            else
            {
                // Load this shape from a plugin if the factory supports it,
                // or else create a generic RectangleShape().
                PluginShapeFactory *factory = sharedPluginShapeFactory();
                shape = factory->createShape(type);
            }
        }

        if (shape)
        {
            shape->initWithXMLProperties(canvas, node, dunnartURI);
            newObj = shape;
        }
    }
    else if (pass == PASS_CLUSTERS)
    {
        if (type == x_cluster)
        {
            newObj = new Cluster(canvas, node, dunnartURI);
        }
    }
    else if (pass == PASS_CONNECTORS)
    {
        if (type == x_connector)
        {
            Connector *conn = new Connector();
            conn->initWithXMLProperties(canvas, node, dunnartURI);
            newObj = conn;
        }
        else if (oldConnTypes.contains(type))
        {
            QString id = essentialProp<QString>(node, x_id);
            qWarning("Conn [%s] specified with old syntax: %s.",
                    qPrintable(id), qPrintable(type));

            Connector *conn = new Connector();
            conn->initWithXMLProperties(canvas, node, dunnartURI);
            newObj = conn;
        }
    }
    else if (pass == PASS_RELATIONSHIPS)
    {
        if (type == x_constraint)
        {
            new Relationship(canvas, node, dunnartURI);
        }
    }

    if (newObj)
    {
        QUndoCommand *cmd = new CmdCanvasSceneAddItem(canvas, newObj);
        canvas->currentUndoMacro()->addCommand(cmd);
    }

    return newObj;
}
*/

void ConnectorLabel::addXmlProps(const unsigned int subset, QDomElement& node,
        QDomDocument& doc)
{
    Q_UNUSED (doc)

    if (subset & XMLSS_IOTHER)
    {
        newProp(node, "id", idString());

        newProp(node, x_type, itemType());

        // Add saved properties for props not used by Dunnart
        QList<QByteArray> propertyList = this->dynamicPropertyNames();
        for (int i = 0; i < propertyList.size(); ++i)
        {
            const char *propName = propertyList.at(i).constData();
            QStringList nameParts = QString(propName).split(":");
            assert(nameParts.length() <= 2);
            QString localName = nameParts.at(nameParts.length() - 1);
            QString prefix = "";
            if (nameParts.length() > 1)
            {
                prefix = nameParts.at(nameParts.length() - 2);
            }

            if ((prefix == node.prefix()) || (prefix == "svg"))
            {
                // Don't put this attribute in a namespace.
                newProp(node, localName, this->property(propName).toString());
            }
            else
            {
                // Add attribute in a NS prefix.
                newProp(node, qualify(prefix, localName),
                          this->property(propName).toString());
            }
        }
    }
}

QDomElement ConnectorLabel::to_QDomElement(const unsigned int subset,
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");
    addXmlProps(subset, node, doc);

    return node;
}


QVariant ConnectorLabel::itemChange(GraphicsItemChange change,
        const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        routerMove();
    }
    else if (change == QGraphicsItem::ItemSceneChange)
    {
        if (canvas())
        {
            // Being removed from the canvas
            routerRemove();
        }
    }
    else if (change == QGraphicsItem::ItemSceneHasChanged)
    {
        if (canvas())
        {
            // Give this item an id if it doesn't have one.
            m_string_id = canvas()->assignStringId(m_string_id);
            m_internal_id = canvas()->assignInternalId();
            // Being added to canvas
            routerAdd();
        }
    }
    return value;
}


void ConnectorLabel::routerAdd(void)
{
    // Do nothing for ConnectorLabel.  Subclasses will implement behaviour.
}
void ConnectorLabel::routerRemove(void)
{
    // Do nothing for ConnectorLabel.  Subclasses will implement behaviour.
}
void ConnectorLabel::routerMove(void)
{
    // Do nothing for ConnectorLabel.  Subclasses will implement behaviour.
}
void ConnectorLabel::routerResize(void)
{
    // Do nothing for ConnectorLabel.  Subclasses will implement behaviour.
}

QPainterPath ConnectorLabel::buildPainterPath(void)
{
    QPainterPath painter_path;
    painter_path.addRect(-width() / 2, -height() / 2, width(), height());
    return painter_path;
}


QPainterPath ConnectorLabel::painterPath(void) const
{
    return m_painter_path;
}

void ConnectorLabel::setPainterPath(QPainterPath path)
{
    m_painter_path = path;
}


void ConnectorLabel::setSizeAndUpdatePainterPath(const QSizeF& newSize)
{
    if (newSize == size())
    {
        // setSize won't update the painter path if the shape size
        // hasn't changed, so force that here.
        m_painter_path = buildPainterPath();
        update();
    }
    else
    {
        // If the size is different, then let setSize do the work.
        setSize(newSize);
    }
}

void ConnectorLabel::setSize(const QSizeF& newSize)
{
    if (newSize == size() && ! m_painter_path.isEmpty())
    {
        // Don't do anything if the size hasn't changed and
        // shape has a painter path.
        return;
    }
    prepareGeometryChange();
    m_size = newSize;
    // Update the painter path used to draw the shape.
    m_painter_path = buildPainterPath();

    //assert(m_painter_path.boundingRect().isNull() == false);

    // Update visibility graph for connector routing.
    routerResize();
}

void ConnectorLabel::setSize(const double w, const double h)
{
    ConnectorLabel::setSize(QSizeF(w, h));
}

double ConnectorLabel::width(void) const
{
    return m_size.width();
}

double ConnectorLabel::height(void) const
{
    return m_size.height();
}

QAction *ConnectorLabel::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
        QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
#if 0
    QAction *cutAction = menu.addAction(tr("Cut"));
    QAction *copyAction = menu.addAction(tr("Copy"));
    QAction *deleteAction = menu.addAction(tr("Delete"));
#endif

    QAction *action = NULL;
    if (!menu.isEmpty())
    {
        QApplication::restoreOverrideCursor();
        action = menu.exec(event->screenPos());
    }

#if 0
    if (action == cutAction)
    {

    }
    else if (action == copyAction)
    {

    }
    else if (action == deleteAction)
    {

    }
#endif
    return action;
}

/*
void CanvasItem::setCanvasItemFlag(CanvasItemFlag flag, bool enabled)
{
    if (enabled)
    {
        m_flags = m_flags | flag;
    }
    else
    {
        m_flags = m_flags & ~flag;
    }
}

CanvasItem::CanvasItemFlags CanvasItem::canvasItemFlags(void) const
{
    return m_flags;
}
*/

bool ConnectorLabel::isCollapsed(void)
{
    return m_is_collapsed;
}


void ConnectorLabel::setAsCollapsed(bool collapsed)
{
    m_is_collapsed = collapsed;
    if (m_is_collapsed)
    {
        setSelected(false);
    }
}


bool ConnectorLabel::isInactive(void) const
{
    return m_is_inactive;
}

/*
void returnAllInactive(void)
{
    // Return shapes.
    ConnectorLabelList tmpList = inactiveObjList;
    returnToCanvas(tmpList);
}


void returnAppropriateConnectors(void)
{
    ConnectorLabelList tmpList = inactiveObjList;
    for (ConnectorLabelList::iterator curr = tmpList.begin(); curr != tmpList.end();
            ++curr)
    {
        Connector *conn = dynamic_cast<Connector *> (*curr);
        if (conn)
        {
            conn->maybeReturn();
        }
    }
}
*/

void ConnectorLabel::maybeReturn(void)
{
    assert(dynamic_cast<Connector *> (this));

    QDomElement node = inactiveObjXml[this];
    assert(!node.isNull());

    QString sshape, dshape;

    // XXX: This could have faster lookup with a set.
    if (optionalProp(node, x_srcID, sshape) && canvas()->getItemByID(sshape) &&
        optionalProp(node, x_dstID, dshape) && canvas()->getItemByID(dshape))
    {
        setAsInactive(false);
    }
}


void ConnectorLabel::setAsInactive(bool inactive, ConnectorLabelSet fullSet)
{
    if (m_is_inactive == inactive)
    {
        qFatal("Existing inactive state passed to ConnectorLabel::setAsInactive");
    }

    Connector *conn = dynamic_cast<Connector *> (this);
    Guideline *guide = dynamic_cast<Guideline *> (this);
    ShapeObj *shape = dynamic_cast<ShapeObj *> (this);

    QDomDocument doc("XML");
    m_is_inactive = inactive;
    if (m_is_inactive)
    {
        QDomElement xn = to_QDomElement(XMLSS_ALL, doc);
        assert(!xn.isNull());

        inactiveObjXml[this] = xn;

        setSelected(false);

        routerRemove();
        if (conn)
        {
            //deactivateAll(fullSet);
        }

        setVisible(false);

        inactiveObjList.push_back(this);
    }
    else
    {
        inactiveObjList.removeAll(this);

        setVisible(true);
        if (shape)
        {
            //deactivateAll(fullSet);
        }
        routerAdd();

        if (conn)
        {
            // This is a connector, so reattach.
            // QT conn->updateFromXmlRep(DELTA_REROUTE, inactiveObjXml[this]);
        }
        else if (guide)
        {
            // Reattach shapes to this guideline.
            // QT recursiveReadSVG(NULL, inactiveObjXml[this], NULL,
            //                 PASS_RELATIONSHIPS);
        }
        inactiveObjXml.remove(this);
    }
}


#if 0
void CanvasItem::glowSetClipRect(SDL_Surface *surface)
{
    // Only display guides to edge of things they are connected to.
    const bool two_tier_indicators = true;

    Guideline *guide = dynamic_cast<Guideline *> (this);

    if (connectedObjs[0] && connectedObjs[1])
    {
        // We're doing a pair query, only show relevant section
        // of the connector

        int offsetX = cxoff;
        int offsetY = cyoff;
        int x1 = connectedObjs[0]->get_xpos() + offsetX;
        int x2 = connectedObjs[1]->get_xpos() + offsetX;
        int y1 = connectedObjs[0]->get_ypos() + offsetY;
        int y2 = connectedObjs[1]->get_ypos() + offsetY;
        int w1 = connectedObjs[0]->get_width();
        int w2 = connectedObjs[1]->get_width();
        int h1 = connectedObjs[0]->get_height();
        int h2 = connectedObjs[1]->get_height();

        int cx = qMin(x1, x2);
        int cy = qMin(y1, y2);
        int cw = qMax(x1 + w1, x2 + w2) - cx;
        int ch = qMax(y1 + h1, y2 + h2) - cy;
        SDL_Rect crect = { cx, cy, cw, ch };
        SDL_SetClipRect(surface, &crect);
    }
    else if (two_tier_indicators && guide)
    {
        int cx1, cy1, cx2, cy2;

        // Extra pixels past shapes.
        int buffer = 15;
        getAttachedObjectsBounds<ShapeObj *>(guide, &cx1, &cy1, &cx2, &cy2,
                buffer);

        cx1 += cxoff;
        cy1 += cyoff;
        cx2 += cxoff;
        cy2 += cyoff;

        SDL_Rect crect = { cx1, cy1, cx2 - cx1, cy2 - cy1 };
        SDL_SetClipRect(surface, &crect);
    }
}



void CanvasItem::glowClearClipRect(SDL_Surface *surface)
{
    if (connectedObjs[0] && connectedObjs[1])
    {
        SDL_SetClipRect(surface, NULL);
    }
}

void CanvasItem::update_after_unhide(void)
{
    int nabsxpos = canvas->get_xpos() + cxoff + xpos;
    int nabsypos = canvas->get_ypos() + cyoff + ypos;

    Conn *c = dynamic_cast<Conn *> (this);
    if (c)
    {
        c->move_diff_points(nabsxpos - absxpos, nabsypos - absypos);
    }
    absxpos = nabsxpos;
    absypos = nabsypos;
}

#endif

void ConnectorLabel::move_to(const int x, const int y, bool store_undo)
{
    Q_UNUSED (store_undo)

    // UNDO undo record.

    // Visibility graph stuff:
    routerMove();

    setPos(x, y);
}


bool ConnectorLabel::cascade_logic(int& nextval, int dist, unsigned int dir,
        ConnectorLabel **path)
{
    Q_UNUSED (dir)

    // Clear stuff ahead:
    if (path)
    {
        int index = dist + 1;
        while (path[index])
        {
            path[index]->m_connection_distance = -1;
            path[index] = NULL;
            ++index;
        }
    }

    if (path && (this == path[0]))
    {
        if (m_connection_distance == -1)
        {
            m_connection_distance = dist;
        }
        path[dist + 1] = this;
        // Have reached destination
        for (int i = dist + 1; i > 0; --i)
        {
            // Mark path.
            path[i]->m_connection_distance = -2;
            path[i]->m_connection_cascade_glow = true;

            if ((i > 1) && (i < (dist + 1)))
            {
                path[i]->m_connected_objects[0] = path[i - 1];
                path[i]->m_connected_objects[1] = path[i + 1];
            }

#if 0
            bool regen_cache = true;
            path[i]->set_active_image(path[i]->get_active_image_n(),
                    regen_cache);
            path[i]->update();
#endif
        }
        // No more searching
        return false;
    }

    if (m_connection_distance == -1)
    {
        m_connection_distance = dist;
        path[dist + 1] = this;
    }
    else if (dist >= m_connection_distance)
    {
        return false;
    }

    nextval = dist + 1;
    return true;
}


void ConnectorLabel::loneSelectedChange(const bool value)
{
    Q_UNUSED(value)

    // Nothing.
}


// This method returns the SVG tags that can be used to draw the ConnectorLabel
// in an output SVG file.  It does this by outputing the canvas with just the
// one item and them stripping the header and footer SVG tags.  Doing this
// allows us to know the SVG for individual ConnectorLabels and transform it by
// injecting object IDs or onHover code if we desire.
// This is required because the QSvgGenerator just uses QPainter instructions
// to generate the SVG and this has now information about the objects that
// were the source of the drawing instructions.
//
QString ConnectorLabel::svgCodeAsString(const QSize& size, const QRectF& viewBox)
{
    // Write SVG to a QBuffer.
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    QSvgGenerator generator;
    generator.setOutputDevice(&buffer);

    // The the generator size and view the same as the set for the
    // wrapper SVG.
    generator.setSize(size);
    generator.setViewBox(viewBox);

    // Do the actual painting (output of SVG code)
    QPainter painter;
    if (painter.begin(&generator))
    {
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setMatrix(this->sceneMatrix(), true);
        QStyleOptionGraphicsItem styleItem;
        this->paint(&painter, &styleItem, NULL);
        painter.end();
    }
    buffer.close();

    QString svgStr(buffer.data());

    // Remove SVG tags up to beginning of first group tag, which will be
    // the beginning of the definition of the object.
    svgStr = svgStr.remove(0, svgStr.indexOf("<g "));

    // Remove "</svg>\n" from end of string.
    svgStr.chop(7);

    return svgStr;
}


}
