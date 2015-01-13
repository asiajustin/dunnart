#include <QPainter>

#include "umlnote.h"
#include "libdunnartcanvas/graphlayout.h"

class GraphLayout;

NoteShape::NoteShape() : RectangleShape()
{
    setItemType("umlNote");
}

void NoteShape::setupForShapePickerPreview()
{
    m_label = "";
}

void NoteShape::setupForShapePickerDropOnCanvas()
{
    m_label = "Note";
}

uint NoteShape::levelsOfDetail(void) const
{
    return 2;
}

QPainterPath NoteShape::buildPainterPath(void)
{
    std::cout << "UML Note: build painter path" << std::endl;
    QPainterPath painter_path;

    painter_path.moveTo(QPointF(-width() / 2, -height() / 2));
    painter_path.lineTo(QPointF(width() / 2 - 10, -height() / 2));
    painter_path.lineTo(QPointF(width() / 2 - 10, -height() / 2 + 10));
    painter_path.lineTo(QPointF(width() / 2, -height() / 2 + 10));
    painter_path.lineTo(QPointF(width() / 2, height() / 2));
    painter_path.lineTo(QPointF(-width() / 2, height() / 2));
    painter_path.closeSubpath();
    painter_path.moveTo(QPointF(width() / 2 - 10, -height() / 2));
    painter_path.lineTo(QPointF(width() / 2, -height() / 2 + 10));

    return painter_path;
}

QString NoteShape::label() const
{
    return m_label;
}

void NoteShape::setLabel(const QString& label)
{
    m_label = label.simplified();

    if (m_label.isEmpty())
    {
        setSize(QSizeF(70, 50));
    }
    update();
    canvas()->repositionAndShowSelectionResizeHandles(true);
    canvas()->fully_restart_graph_layout();
}

void NoteShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    std::cout << "UML Note: Paint" << std::endl;

    QString m_label(this->m_label);

    if (!m_label.isEmpty())
    {
        if (currentDetailLevel() == 2)
        {
            m_label = "......";
        }
        QFontMetrics qfm(canvas()->canvasFont());
        int maxWordLength = 0;
        QStringList wordList = m_label.split(" ");

        std::cout << "Word List Length(): " << wordList.length() << std::endl;

        for (int i = 0; i < wordList.length(); i++)
        {
            int currentWordLength = qfm.width(wordList.value(i));
            maxWordLength = (maxWordLength >= currentWordLength) ? maxWordLength : currentWordLength;
        }

        double singleLineLength = qfm.width(m_label) + 10;

        if (width() > singleLineLength)
        {
            setSize(QSizeF(singleLineLength, qreal(height())));
        }

        double currentWidth = qMax(width(), qreal(maxWordLength + 10));

        std::cout << "Current Width: " << currentWidth << std::endl;

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
            double currentTotalLength = tempWidth + currentWordLength + 10;
            if (currentTotalLength <= currentWidth)
            {
                tempWidth += currentWordLength;
                std::cout << "Temp Width: " << tempWidth << std::endl;
            }
            else
            {
                if (spaceAdded && currentTotalLength - qreal(qfm.width(" ")) <= currentWidth)
                {
                    tempWidth += qreal(qfm.width(wordList.value(i)));
                    std::cout << "Trailing space!!!! " << std::endl;
                }
                else
                {
                    tempWidth = currentWordLength;
                    ++lineCount;
                    std::cout << "Temp Width Reset: " << tempWidth << std::endl;
                }
            }
        }
        double currentHeight = qMax(50.0, qreal(lineCount * qfm.height() + (lineCount - 1) * qfm.leading() + 15));

        std::cout << "Line Count: " << lineCount << std::endl;
        std::cout << "Current Width: " << currentWidth << std::endl;
        std::cout << "Current Height: " << currentHeight << std::endl;
        std::cout << "Width: " << width() << std::endl;
        std::cout << "Height: " << height() << std::endl;

        setSize(QSizeF(currentWidth, currentHeight));
        canvas()->repositionAndShowSelectionResizeHandles(true);
    }

    // Call the parent paint method, to draw the node and label
    ShapeObj::paint(painter, option, widget);

    painter->setPen(strokeColour());
    if (canvas())
    {
        painter->setFont(canvas()->canvasFont());
    }
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    QRectF noteRectangle(QPointF(-width() / 2 + 5, -height() / 2 + 10), QPointF(width() / 2 - 5, height() / 2 - 5));
    painter->drawText(noteRectangle, Qt::AlignTop | Qt::TextWordWrap, m_label);
}

QDomElement NoteShape::to_QDomElement(const unsigned int subset, QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newProp(node, x_type, "umlNote");
    }

    addXmlProps(subset, node, doc);

    if (subset & XMLSS_IOTHER)
    {
        newTextChild(node, x_dunnartNs, "note_text", m_label, doc);
    }

    return node;
}

QDomElement NoteShape::newTextChild(QDomElement& node, const QString& ns,
        const QString& name, QString arg, QDomDocument& doc)
{
    Q_UNUSED (ns)

    QDomElement new_node = doc.createElement(name);

    QDomText text = doc.createTextNode(arg);
    new_node.appendChild(text);

    node.appendChild(new_node);

    return new_node;
}

void NoteShape::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    RectangleShape::initWithXMLProperties(canvas, node, ns);

    QDomElement element = node.childNodes().item(0).toElement();
    QString name = element.nodeName();

    if (name == "note_text")
    {
        m_label = element.text();
    }
}
