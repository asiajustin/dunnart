#include <QPainter>

#include "umlpackage.h"
#include "libdunnartcanvas/graphlayout.h"

class GraphLayout;

PackageShape::PackageShape() : RectangleShape()
{
    setItemType("umlPackage");
    m_expanded = false;
}

void PackageShape::setupForShapePickerPreview()
{
    m_label = "";
    lineWidth = 12;
    fontHeight = 0;
}

void PackageShape::setupForShapePickerDropOnCanvas()
{
    m_label = "package";
    lineWidth = 46;
    fontHeight = 20;
    setContainmentPadding(QMarginsF(qreal(5), qreal(fontHeight + 15), qreal(5), qreal(5)));
}

QAction *PackageShape::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu)
{

    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction* editExpanded = m_expanded ? menu.addAction(tr("Collapse the Package")) : menu.addAction(tr("Expand the Package"));

    QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

    if (action == editExpanded)
    {
        m_expanded ? setExpanded(false) : setExpanded(true);
    }

    return action;
}

bool PackageShape::isExpanded()
{
    return m_expanded;
}

void PackageShape::setExpanded(bool expanded)
{
    m_expanded = expanded;
    if (m_expanded)
    {
        sendToBack();
    }
    else
    {
        bringToFront();
    }
}

QPainterPath PackageShape::buildPainterPath(void)
{
    std::cout << "UML Package: build painter path" << std::endl;
    QPainterPath painter_path;

    QRectF packageGroupingRectangle (QPointF(-width() / 2, -height() / 2 + fontHeight + 10), QPointF(width() / 2, height() / 2));
    painter_path.addRect(packageGroupingRectangle);

    painter_path.moveTo(QPointF(-width() / 2, -height() / 2 + fontHeight + 10));
    painter_path.lineTo(QPointF(-width() / 2, -height() / 2));
    painter_path.lineTo(QPointF(-width() / 2 + lineWidth + 10, -height() / 2));
    painter_path.lineTo(QPointF(-width() / 2 + lineWidth + 10, -height() / 2 + fontHeight + 10));

    return painter_path;
}

QString PackageShape::label() const
{
    return m_label;
}

void PackageShape::setLabel(const QString& label)
{
    m_label = label.simplified();

    update();
    canvas()->layout()->setRestartFromDunnart();
}

void PackageShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    std::cout << "UML Package: Paint" << std::endl;

    if (fontHeight != 0)
    {
        if (width() < 70)
        {
            setSize(QSizeF(qreal(70), qreal(height())));
            canvas()->repositionAndShowSelectionResizeHandles(true);
        }

        if (height() < 50)
        {
            setSize(QSizeF(qreal(width()), qreal(50)));
            canvas()->repositionAndShowSelectionResizeHandles(true);
        }
    }

    int minLabelWidth = width() * 0.2 - 10;
    int maxLabelWidth = width() * 0.8 - 10;

    if (fontHeight != 0)
    {
        QFontMetrics qfm(canvas()->canvasFont());
        int realLineWidth = qfm.width(m_label);
        lineWidth = realLineWidth > maxLabelWidth ? maxLabelWidth : realLineWidth < minLabelWidth ? minLabelWidth : realLineWidth;
        fontHeight = qfm.height();
    }

    QPainterPath painter_path;

    QRectF packageGroupingRectangle (QPointF(-width() / 2, -height() / 2 + fontHeight + 10), QPointF(width() / 2, height() / 2));
    painter_path.addRect(packageGroupingRectangle);

    painter_path.moveTo(QPointF(-width() / 2, -height() / 2 + fontHeight + 10));
    painter_path.lineTo(QPointF(-width() / 2, -height() / 2));
    painter_path.lineTo(QPointF(-width() / 2 + lineWidth + 10, -height() / 2));
    painter_path.lineTo(QPointF(-width() / 2 + lineWidth + 10, -height() / 2 + fontHeight + 10));

    setPainterPath(painter_path);

    // Call the parent paint method, to draw the node and label
    ShapeObj::paint(painter, option, widget);

    painter->setPen(Qt::black);
    if (canvas())
    {
        painter->setFont(canvas()->canvasFont());
    }
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    QRectF noteRectangle (QPointF(-width() / 2 + 5, -height() / 2 + 5), QPointF(-width() / 2 + 5 + lineWidth, -height() / 2 + 5 + fontHeight));

    int drawTextFlags = Qt::AlignTop | Qt::TextSingleLine;
    QString drawTextContent(m_label);

    std::cout << "lineWidth: " << lineWidth << std::endl;
    std::cout << "minLabelWidth: " << minLabelWidth << std::endl;
    std::cout << "maxLabelWidth: " << maxLabelWidth << std::endl;

    if (lineWidth == minLabelWidth)
    {
        drawTextFlags |= Qt::AlignHCenter;
    }
    else if (lineWidth == maxLabelWidth)
    {
        std::cout << "LineWidth equals to the maximum of width()" << std::endl;
        QFontMetrics qfm(canvas()->canvasFont());
        int maxVisibleWidth = lineWidth - qfm.width("...");
        int estimatedTruncatedIndex = maxVisibleWidth / (qfm.width(m_label) / m_label.length());

        while (true)
        {
            int widthA = qfm.width(m_label.left(estimatedTruncatedIndex));
            int widthB = qfm.width(m_label.left(estimatedTruncatedIndex + 1));

            if (widthB <= maxVisibleWidth)
            {
                ++estimatedTruncatedIndex;
                continue;
            }
            else if (maxVisibleWidth < widthA)
            {
                --estimatedTruncatedIndex;
                continue;
            }
            break;
        }
        drawTextContent = m_label.left(estimatedTruncatedIndex).append("...");
    }
    painter->drawText(noteRectangle, drawTextFlags, drawTextContent);
}

QDomElement PackageShape::to_QDomElement(const unsigned int subset, QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newProp(node, x_type, "umlPackage");
    }

    addXmlProps(subset, node, doc);

    if (subset & XMLSS_IOTHER)
    {
        newTextChild(node, x_dunnartNs, "package_name", m_label, doc);
        newTextChild(node, x_dunnartNs, "line_width", QString::number(lineWidth), doc);
        newTextChild(node, x_dunnartNs, "font_height", QString::number(fontHeight), doc);
    }

    return node;
}

QDomElement PackageShape::newTextChild(QDomElement& node, const QString& ns,
        const QString& name, QString arg, QDomDocument& doc)
{
    Q_UNUSED (ns)

    QDomElement new_node = doc.createElement(name);

    QDomText text = doc.createTextNode(arg);
    new_node.appendChild(text);

    node.appendChild(new_node);

    return new_node;
}

void PackageShape::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    RectangleShape::initWithXMLProperties(canvas, node, ns);

    //QDomElement element = node.childNodes().item(0).toElement();
    //QString name = element.nodeName();

    QDomNodeList children = node.childNodes();

    std::cout << "Children Length: " << children.length() << std::endl;

    for (int i = 0; i < children.length(); ++i)
    {
        QDomElement element = children.item(i).toElement();
        QString name = element.nodeName();

        if (name == "package_name")
        {
            m_label = element.text();
            std::cout << "m_label: " << m_label.toStdString() << std::endl;
        }
        else if (name == "line_width")
        {
            lineWidth = element.text().toInt();
            std::cout << "lineWidth: " << lineWidth << std::endl;
        }
        else if (name == "font_height")
        {
            fontHeight = element.text().toInt();
            std::cout << "fontHeight: " << fontHeight << std::endl;
        }
    }
    setContainmentPadding(QMarginsF(qreal(5), qreal(fontHeight + 15), qreal(5), qreal(5)));
}
