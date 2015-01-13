/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2007  Michael Woodward
 * Copyright (C) 2007  Michael Wybrow
 * Copyright (C) 2008, 2010  Monash University
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
 * Author(s): Michael Woodward
 *            Michael Wybrow  <http://michael.wybrow.info/>
 *            Yiya Li <http://yiya.li/>
*/

#include <string>
#include <algorithm>
#include <vector>

#include <QApplication>
#include <QPainter>
#include <QTextEdit>

#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/graphlayout.h"

#include "umlclass.h"

class GraphLayout;

#if 0
static void center_text(QPixmap*, int, int, int, int, const char*);
static void get_text_dimensions(QString text, int *w, int *h);
static void get_text_width(QString text, int *w);
#endif

unsigned int umlFontSize = 10;
QFont mono;

static const int minSectHeight = 23;
static const int classLineHeight = 12;


ClassShape::ClassShape()
    : RectangleShape()
{
    do_init();

    setItemType("umlClass");
    setSizeLocked(true);

}

void ClassShape::setEditDialog(EditUmlClassInfoDialog *editDialog)
{
    m_editDialog = editDialog;
}

QString ClassShape::getClassNameAreaText()
{
    return classNameAreaText;
}

QString ClassShape::getClassAttributesAreaText()
{
    return classAttributesAreaText;
}

QString ClassShape::getClassMethodsAreaText()
{
    return classMethodsAreaText;
}

void ClassShape::setupForShapePickerPreview()
{
    classNameAreaText = "";
    classAttributesAreaText = "";
    classMethodsAreaText = "";
}

void ClassShape::setupForShapePickerDropOnCanvas()
{
#if 0
    classNameAreaText = "";
    classAttributesAreaText = "";
    classMethodsAreaText = "";
#else
    classNameAreaText = "<<Stereotype>>\nClassName";
    classAttributesAreaText = "<<Stereotype>> # attrName: AttrType";
    classMethodsAreaText = "+ methodName(in paramA: ParamAType, out paramB: ParamBType): ReturnType";
#endif
}

/*
void ClassShape::mousePressEvent ( QGraphicsSceneMouseEvent * event )
{
    std::cout << "HERE" << std::endl;

    disconnect(m_editDialog->getClassNameArea(), 0, 0, 0);
    disconnect(m_editDialog->getClassAttributesArea(), 0, 0, 0);
    disconnect(m_editDialog->getClassMethodsArea(), 0, 0, 0);

    connect( m_editDialog->getClassNameArea(), SIGNAL( textChanged() ), this, SLOT( text_changed() ));
    connect( m_editDialog->getClassAttributesArea(), SIGNAL( textChanged() ), this, SLOT( text_changed() ));
    connect( m_editDialog->getClassMethodsArea(), SIGNAL( textChanged() ), this, SLOT( text_changed() ));

    QGraphicsItem::mousePressEvent(event);
}*/

QPainterPath ClassShape::buildPainterPath(void)
{
    std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
    QPainterPath painter_path;
    QRectF classRectangle (QPointF(-width() / 2, -height() / 2), QPointF(width() / 2, height() / 2));
    painter_path.addRect(classRectangle);

    if (isSelected())
    {
        std::cout << "HERE~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        painter_path.moveTo(QPointF(-width() / 2, -height() / 2 + classNameSectionHeight + 10));
        painter_path.lineTo(QPointF(width() / 2, -height() / 2 + classNameSectionHeight + 10));
        painter_path.moveTo(QPointF(-width() / 2, -height() / 2 + classNameSectionHeight + classAttributesSectionHeight + 20));
        painter_path.lineTo(QPointF(width() / 2, -height() / 2 + classNameSectionHeight + classAttributesSectionHeight + 20));
    }
    else
    {
        painter_path.moveTo(QPointF(-width() / 2, -height() / 6));
        painter_path.lineTo(QPointF(width() / 2, -height() / 6));

        painter_path.moveTo(QPointF(-width() / 2, height() / 6));
        painter_path.lineTo(QPointF(width() / 2, height() / 6));
    }
    return painter_path;
}

void ClassShape::setIdealPos(QPointF pos)
{
    ideal_pos = pos;
    setCentrePos(pos);
}

QPointF ClassShape::idealPos()
{
    if (ideal_pos.isNull())
    {
        return centrePos();
    }
    else
    {
        return ideal_pos;
    }
}

void ClassShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    /*
    QPainterPath painter_path;
    QRectF classRectangle (QPointF(-width() / 2, -height() / 2), QPointF(width() / 2, height() / 2));
    painter_path.addRect(classRectangle);

    painter_path.moveTo(QPointF(-width() / 2, -height() / 2 + classNameSectionHeight + 10));
    painter_path.lineTo(QPointF(width() / 2, -height() / 2 + classNameSectionHeight + 10));
    painter_path.moveTo(QPointF(-width() / 2, -height() / 2 + classNameSectionHeight + classAttributesSectionHeight + 20));
    painter_path.lineTo(QPointF(width() / 2, -height() / 2 + classNameSectionHeight + classAttributesSectionHeight + 20));

    setPainterPath(painter_path);*/

    // Call the parent paint method, to draw the node and label
    ShapeObj::paint(painter, option, widget);

    painter->setPen(strokeColour());
    if (canvas())
    {
        painter->setFont(canvas()->canvasFont());
    }
    painter->setRenderHint(QPainter::TextAntialiasing, true);

    QRectF classNameRectangle(QPointF(-width() / 2 + 5, -height() / 2 + 5), QSizeF(qreal(classWidth), qreal(classNameSectionHeight)));
    QRectF classAttributesRectangle(QPointF(-width() / 2 + 5, -height() / 2 + classNameSectionHeight + 15), QSizeF(qreal(classWidth), qreal(classAttributesSectionHeight)));
    QRectF classMethodsRectangle(QPointF(-width() / 2 + 5, -height() / 2 + classNameSectionHeight + classAttributesSectionHeight + 25), QSizeF(qreal(classWidth), qreal(classMethodsSectionHeight)));

    QString classNameAreaText;
    if (class_name.isEmpty())
    {
        class_stereotype = "";
    }
    else
    {
        if (!class_stereotype.isEmpty() && currentDetailLevel() < NO_CLASS_STEREOTYPES)
        {
            classNameAreaText = "<<" + class_stereotype + ">>\n";
        }
        classNameAreaText += class_name;
    }

    QString classAttributesAreaText;
    for (int i = 0; i < attributes.size() && currentDetailLevel() < NO_ATTRS; i++)
    {
        if (attributes[i].name.isEmpty())
        {
            attributes[i].stereotype = "";
            attributes[i].accessModifier = DEFAULT;
            attributes[i].type = "";

            if (i == attributes.size() - 1)
            {
                classAttributesAreaText = classAttributesAreaText.trimmed();
            }
        }
        else
        {
            if (!attributes[i].stereotype.isEmpty() && currentDetailLevel() < NO_ATTR_STEREOTYPES)
            {
                classAttributesAreaText += "<<" + attributes[i].stereotype + ">> ";
            }
            if (attributes[i].accessModifier == PRIVATE)
            {
                classAttributesAreaText += "- ";
            }
            else if (attributes[i].accessModifier == PROTECTED)
            {
                classAttributesAreaText += "# ";
            }
            else if (attributes[i].accessModifier == PUBLIC)
            {
                classAttributesAreaText += "+ ";
            }
            else if (attributes[i].accessModifier == PACKAGE)
            {
                classAttributesAreaText += "~ ";
            }
            classAttributesAreaText += attributes[i].name;
            if (!attributes[i].type.isEmpty() && currentDetailLevel() < NO_ATTR_TYPES)
            {
                classAttributesAreaText += ": " + attributes[i].type;
            }
            if (i != attributes.size() - 1)
            {
                classAttributesAreaText += "\n";
            }
        }
    }

    QString classMethodsAreaText;
    for (int i = 0; i < methods.size() && currentDetailLevel() < NO_METHODS; i++)
    {
        QString parameterListText("");
        for (int j = 0; j < methods[i].params.size() && currentDetailLevel() < NO_PARAMS; j++)
        {
            if (methods[i].params[j].name.isEmpty())
            {
                methods[i].params[j].type = "";
                methods[i].params[j].mode = UNSPECIFIED;

                if (j == methods[i].params.size() - 1)
                {
                   parameterListText = parameterListText.left(parameterListText.length() - 2);
                }
            }
            else
            {
                if (methods[i].params[j].mode == PARAM_IN && currentDetailLevel() < NO_PARAM_MODES)
                {
                    parameterListText += "in ";
                }
                else if (methods[i].params[j].mode == PARAM_OUT && currentDetailLevel() < NO_PARAM_MODES)
                {
                    parameterListText += "out ";
                }
                parameterListText += methods[i].params[j].name;
                if (!methods[i].params[j].type.isEmpty() && currentDetailLevel() < NO_PARAM_TYPES)
                {
                    parameterListText += ": " + methods[i].params[j].type;
                }
                if (j != methods[i].params.size() - 1)
                {
                    parameterListText += ", ";
                }
            }
        }

        if (methods[i].name.isEmpty())
        {
            methods[i].accessModifier = DEFAULT;
            methods[i].params.resize(0);
            methods[i].return_type = "";

            if (i == methods.size() - 1)
            {
                classMethodsAreaText = classMethodsAreaText.trimmed();
            }
        }
        else if (currentDetailLevel() < NO_METHODS)
        {
            if (methods[i].accessModifier == PRIVATE)
            {
                classMethodsAreaText += "- ";
            }
            else if (methods[i].accessModifier == PROTECTED)
            {
                classMethodsAreaText += "# ";
            }
            else if (methods[i].accessModifier == PUBLIC)
            {
                classMethodsAreaText += "+ ";
            }
            else if (methods[i].accessModifier == PACKAGE)
            {
                classMethodsAreaText += "~ ";
            }
            classMethodsAreaText += methods[i].name + "(";
            if (!parameterListText.isEmpty() && currentDetailLevel() < NO_PARAMS)
            {
                classMethodsAreaText += parameterListText;
            }
            classMethodsAreaText += ")";
            if (!methods[i].return_type.isEmpty() && currentDetailLevel() < NO_RETURN_TYPES)
            {
                classMethodsAreaText += ": " + methods[i].return_type;
            }
            if (i != methods.size() - 1)
            {
                classMethodsAreaText += "\n";
            }
        }
    }

    painter->drawText(classNameRectangle, Qt::AlignTop | Qt::AlignHCenter, classNameAreaText);
    painter->drawText(classAttributesRectangle, Qt::AlignTop, classAttributesAreaText);
    painter->drawText(classMethodsRectangle, Qt::AlignTop, classMethodsAreaText);

    std::cout << "Paint" << std::endl;
}

QAction *ClassShape::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
        QMenu& menu)
{

    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction* editClassInfo = menu.addAction(tr("Edit UML Class Information"));

    QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

    if (action == editClassInfo)
    {
        m_editDialog->show();
    }
    delete editClassInfo;
    editClassInfo = NULL;

    return action;
}

void ClassShape::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    RectangleShape::initWithXMLProperties(canvas, node, ns);

    std::cout << "XML___DO_______INIT" << std::endl;
    do_init();

    /* load class name, params, and methods from xml */
    QDomNodeList children = node.childNodes();

    std::cout << "Children Length: " << children.length() << std::endl;

    for (int i = 0; i < children.length(); ++i)
    {
        QDomElement element = children.item(i).toElement();
        QString name = element.nodeName();

        std::cout << "Element Name: " << name.toStdString() << std::endl;
        if (name == "class_name")
        {
            class_name = element.text();
            std::cout << "class_name: " << class_name.toStdString() << std::endl;
        }
        else if (name == "class_stereotype")
        {
            class_stereotype = element.text();
            std::cout << "class_stereotype: " << class_stereotype.toStdString() << std::endl;
        }
        else if (name == "mode")
        {
            setDetailLevel(UML_Class_Abbrev_Mode(element.text().toInt()));
            std::cout << "mode: " << currentDetailLevel() << std::endl;
        }
        else if (name == "class_width")
        {
            classWidth = element.text().toInt();
            std::cout << "classWidth: " << classWidth << std::endl;
        }
        else if (name == "name_section_height")
        {
            classNameSectionHeight = element.text().toInt();
            std::cout << "classNameSectionHeight: " << classNameSectionHeight << std::endl;
        }
        else if (name == "attr_section_height")
        {
            classAttributesSectionHeight = element.text().toInt();
            std::cout << "classAttributesSectionHeight: " << classAttributesSectionHeight << std::endl;
        }
        else if (name == "method_section_height")
        {
            classMethodsSectionHeight = element.text().toInt();
            std::cout << "classMethodsSectionHeight: " << classMethodsSectionHeight << std::endl;
        }
        else if (name == "attribute")
        {   
            /* makes assumption properties are in order */        
            attributes.resize(attributes.size()+1);
            if (element.hasAttribute("stereotype"))
            {
                attributes[attributes.size()-1].stereotype =
                        element.attribute("stereotype");
            }
            attributes[attributes.size()-1].accessModifier =
                    ACCESS_MODIFIER(element.attribute("accessModifier").toInt());
            std::cout << "attributes accessModifier: " << attributes[attributes.size()-1].accessModifier << std::endl;
            attributes[attributes.size()-1].name = 
                    element.attribute("name");
            std::cout << "attributes name: " << attributes[attributes.size()-1].name.toStdString() << std::endl;
            attributes[attributes.size()-1].type = 
                    element.attribute("type");
            std::cout << "attributes type: " << attributes[attributes.size()-1].type.toStdString() << std::endl;
        }
        else if (name == "method")
        {
            /* makes assumption properties are in order */      
            methods.resize(methods.size()+1);
            methods[methods.size()-1].accessModifier =
                    ACCESS_MODIFIER(element.text().toInt());
            std::cout << "methods accessModifier: " << methods[methods.size()-1].accessModifier << std::endl;
            methods[methods.size()-1].name = 
                    element.attribute("name");
            std::cout << "methods name: " << methods[methods.size()-1].name.toStdString() << std::endl;
            methods[methods.size()-1].return_type = 
                    element.attribute("return_type");
            std::cout << "methods return_type: " << methods[methods.size()-1].return_type.toStdString() << std::endl;

            QDomNodeList params = element.childNodes();

            for (int i = 0; i < params.length(); ++i)
            {
                QDomElement param = params.item(i).toElement();

                if (param.nodeName() == "parameter")
                {
                    /* makes assumption properties are in order */      
                    methods[methods.size()-1].params.resize(
                            methods[methods.size()-1].params.size()+1);
                    Parameter *p = &(methods[methods.size()-1].params[
                            methods[methods.size()-1].params.size()-1]);
                    
                    if (param.hasAttribute("mode"))
                    {
                        p->mode = (param.attribute("mode") == "in") ? PARAM_IN : PARAM_OUT;
                    }
					else 
                    {
                        p->mode = UNSPECIFIED;
                    }
                    p->name = param.attribute("name");
                    p->type = param.attribute("type");
                    std::cout << "methods param name: " << p->name.toStdString() << std::endl;
                    std::cout << "methods param type: " << p->type.toStdString() << std::endl;
                    std::cout << "methods param mode: " << p->mode << std::endl;
                }
            }
        }
    }
    updateAllClassAreas();

    QPainterPath painter_path;
    QRectF classRectangle (QPointF(-width() / 2, -height() / 2), QPointF(width() / 2, height() / 2));
    painter_path.addRect(classRectangle);

    painter_path.moveTo(QPointF(-width() / 2, -height() / 2 + classNameSectionHeight + 10));
    painter_path.lineTo(QPointF(width() / 2, -height() / 2 + classNameSectionHeight + 10));
    painter_path.moveTo(QPointF(-width() / 2, -height() / 2 + classNameSectionHeight + classAttributesSectionHeight + 20));
    painter_path.lineTo(QPointF(width() / 2, -height() / 2 + classNameSectionHeight + classAttributesSectionHeight + 20));

    setPainterPath(painter_path);
}

void ClassShape::updateAllClassAreas()
{
    classNameAreaText = "";
    if (class_name.isEmpty())
    {
        class_stereotype = "";
    }
    else
    {
        if (!class_stereotype.isEmpty())
        {
            classNameAreaText = "<<" + class_stereotype + ">>\n";
        }
        classNameAreaText += class_name;
    }

    classAttributesAreaText = "";
    for (int i = 0; i < attributes.size(); i++)
    {
        if (attributes[i].name.isEmpty())
        {
            attributes[i].stereotype = "";
            attributes[i].accessModifier = DEFAULT;
            attributes[i].type = "";

            if (i == attributes.size() - 1)
            {
                classAttributesAreaText = classAttributesAreaText.trimmed();
            }
        }
        else
        {
            if (!attributes[i].stereotype.isEmpty())
            {
                classAttributesAreaText += "<<" + attributes[i].stereotype + ">> ";
            }
            if (attributes[i].accessModifier == PRIVATE)
            {
                classAttributesAreaText += "- ";
            }
            else if (attributes[i].accessModifier == PROTECTED)
            {
                classAttributesAreaText += "# ";
            }
            else if (attributes[i].accessModifier == PUBLIC)
            {
                classAttributesAreaText += "+ ";
            }
            else if (attributes[i].accessModifier == PACKAGE)
            {
                classAttributesAreaText += "~ ";
            }
            classAttributesAreaText += attributes[i].name;
            if (!attributes[i].type.isEmpty())
            {
                classAttributesAreaText += ": " + attributes[i].type;
            }
            if (i != attributes.size() - 1)
            {
                classAttributesAreaText += "\n";
            }
        }
    }

    classMethodsAreaText = "";
    for (int i = 0; i < methods.size(); i++)
    {
        QString parameterListText("");
        for (int j = 0; j < methods[i].params.size(); j++)
        {
            if (methods[i].params[j].name.isEmpty())
            {
                methods[i].params[j].type = "";
                methods[i].params[j].mode = UNSPECIFIED;

                if (j == methods[i].params.size() - 1)
                {
                   parameterListText = parameterListText.left(parameterListText.length() - 2);
                }
            }
            else
            {
                if (methods[i].params[j].mode == PARAM_IN)
                {
                    parameterListText += "in ";
                }
                else if (methods[i].params[j].mode == PARAM_OUT)
                {
                    parameterListText += "out ";
                }
                parameterListText += methods[i].params[j].name;
                if (!methods[i].params[j].type.isEmpty())
                {
                    parameterListText += ": " + methods[i].params[j].type;
                }
                if (j != methods[i].params.size() - 1)
                {
                    parameterListText += ", ";
                }
            }
        }

        if (methods[i].name.isEmpty())
        {
            methods[i].accessModifier = DEFAULT;
            methods[i].params.resize(0);
            methods[i].return_type = "";

            if (i == methods.size() - 1)
            {
                classMethodsAreaText = classMethodsAreaText.trimmed();
            }
        }
        else
        {
            if (methods[i].accessModifier == PRIVATE)
            {
                classMethodsAreaText += "- ";
            }
            else if (methods[i].accessModifier == PROTECTED)
            {
                classMethodsAreaText += "# ";
            }
            else if (methods[i].accessModifier == PUBLIC)
            {
                classMethodsAreaText += "+ ";
            }
            else if (methods[i].accessModifier == PACKAGE)
            {
                classMethodsAreaText += "~ ";
            }
            classMethodsAreaText += methods[i].name + "(";
            if (!parameterListText.isEmpty())
            {
                classMethodsAreaText += parameterListText;
            }
            classMethodsAreaText += ")";
            if (!methods[i].return_type.isEmpty())
            {
                classMethodsAreaText += ": " + methods[i].return_type;
            }
            if (i != methods.size() - 1)
            {
                classMethodsAreaText += "\n";
            }
        }
    }
}

/**
  * Overrided, to specify how many levels of detail your shape has.
 */
uint ClassShape::levelsOfDetail(void) const
{
    return 10;
}

/**
  * Overrided, to specify the size that your shape should be
  * expanded to at each detail level.  The base level is 1.  Any
  * subsequent levels will be 2, 3, ...
 */
QSizeF ClassShape::sizeForDetailLevel(uint level)
{
    return recalculateSize();
}

void ClassShape::refresh()
{
    setSize(recalculateSize());
    update(CanvasItem::boundingRect());
    canvas()->repositionAndShowSelectionResizeHandles(true);
    canvas()->fully_restart_graph_layout();
}

void ClassShape::classNameAreaChanged()
{
    classNameAreaChanged(true);
}

void ClassShape::classNameAreaChanged(bool textChanged)
{
    std::cout << "Inside classNameAreaChanged()" << std::endl;

    if (textChanged && currentDetailLevel() == NO_CLASS_STEREOTYPES)
    {
        setDetailLevel(NO_ATTRS);
    }

    QString tempClassNameAreaText(m_editDialog->getClassNameArea()->toPlainText().trimmed());

    if (tempClassNameAreaText.startsWith("<<") && tempClassNameAreaText.contains(">>"))
    {
        class_stereotype = tempClassNameAreaText.mid(2, tempClassNameAreaText.indexOf(">>") - 2).simplified().replace(" ", "_");
        class_name = tempClassNameAreaText.mid(tempClassNameAreaText.indexOf(">>") + 2).simplified().replace(" ", "_");
    }
    else if (!tempClassNameAreaText.contains("<") && !tempClassNameAreaText.contains(">"))
    {
        class_stereotype = "";
        class_name = tempClassNameAreaText.simplified().replace(" ", "_");;
    }
    else {
        class_stereotype = "";
        class_name = "";
    }

    classNameAreaText = "";
    if (class_name.isEmpty())
    {
        class_stereotype = "";
    }
    else
    {
        if (!class_stereotype.isEmpty())
        {
            classNameAreaText = "<<" + class_stereotype + ">>\n";
        }
        classNameAreaText += class_name;
    }


    std::cout << "Class Stereotype: " << class_stereotype.toStdString() << std::endl;
    std::cout << "Class Name: " << class_name.toStdString() << std::endl;
    std::cout << "Class Name Area has been updated" << std::endl;

    refresh();
}

void ClassShape::classAttributesAreaChanged()
{
    classAttributesAreaChanged(true);
}

void ClassShape::classAttributesAreaChanged(bool textChanged)
{
    std::cout << "Inside classAttributesAreaChanged()" << std::endl;

    if (textChanged && currentDetailLevel() > NO_METHODS)
    {
        setDetailLevel(NO_METHODS);
    }

    QString tempClassAttributesAreaText(m_editDialog->getClassAttributesArea()->toPlainText().trimmed());

    unsigned int i;

    if (tempClassAttributesAreaText.isEmpty())
    {
        attributes.resize(0);
        classAttributesAreaText = "";
        refresh();
        return;
    }
    else
    {
        attributes.resize(tempClassAttributesAreaText.count("\n") + 1);
    }

    QStringList attributeList = tempClassAttributesAreaText.split("\n");
    classAttributesAreaText = "";

    for (i = 0; i < attributeList.length(); i++)
    {
        QString currentAttribute(attributeList.value(i).trimmed());

        if (currentAttribute.startsWith("<<") && currentAttribute.contains(">>"))
        {
            attributes[i].stereotype = currentAttribute.mid(2, currentAttribute.indexOf(">>") - 2).simplified().replace(" ", "_");
            currentAttribute = currentAttribute.mid(currentAttribute.indexOf(">>") + 2).trimmed();
        }
        else
        {
            attributes[i].stereotype = "";
        }

        if (currentAttribute.startsWith("+") || currentAttribute.startsWith("-") || currentAttribute.startsWith("#"))
        {
            attributes[i].accessModifier = currentAttribute.startsWith("~") ? PACKAGE : currentAttribute.startsWith("+") ? PUBLIC : currentAttribute.startsWith("-") ? PRIVATE : PROTECTED;
            currentAttribute = currentAttribute.mid(1).trimmed();
        }
        else {
            attributes[i].accessModifier = DEFAULT;
        }

        if (!currentAttribute.contains(":"))
        {
            attributes[i].name = currentAttribute.simplified().replace(" ", "_");
            attributes[i].type = "";
        }
        else if (!currentAttribute.startsWith(":") && !currentAttribute.endsWith(":"))
        {
            attributes[i].name = currentAttribute.mid(0, currentAttribute.indexOf(":")).simplified().replace(" ", "_");
            attributes[i].type = currentAttribute.mid(currentAttribute.indexOf(":") + 1).simplified().replace(" ", "_");
        }
        else if (currentAttribute.endsWith(":"))
        {
            attributes[i].name = currentAttribute.mid(0, currentAttribute.indexOf(":")).simplified().replace(" ", "_");
            attributes[i].type = "";
        }
        else
        {
            attributes[i].name = "";
            attributes[i].type = "";
        }

        if (currentAttribute.contains("<") || currentAttribute.contains(">"))
        {
            attributes[i].name = "";
            attributes[i].type = "";
        }

        if (attributes[i].name.isEmpty())
        {
            attributes[i].stereotype = "";
            attributes[i].accessModifier = DEFAULT;
            attributes[i].type = "";

            if (i == attributeList.length() - 1)
            {
                classAttributesAreaText = classAttributesAreaText.trimmed();
            }
        }
        else
        {
            if (!attributes[i].stereotype.isEmpty())
            {
                classAttributesAreaText += "<<" + attributes[i].stereotype + ">> ";
            }
            if (attributes[i].accessModifier == PRIVATE)
            {
                classAttributesAreaText += "- ";
            }
            else if (attributes[i].accessModifier == PROTECTED)
            {
                classAttributesAreaText += "# ";
            }
            else if (attributes[i].accessModifier == PUBLIC)
            {
                classAttributesAreaText += "+ ";
            }
            else if (attributes[i].accessModifier == PACKAGE)
            {
                classAttributesAreaText += "~ ";
            }
            classAttributesAreaText += attributes[i].name;
            if (!attributes[i].type.isEmpty())
            {
                classAttributesAreaText += ": " + attributes[i].type;
            }
            if (i != attributeList.length() - 1)
            {
                classAttributesAreaText += "\n";
            }
        }

        std::cout << "Attribute Stereotype: " << attributes[i].stereotype.toStdString() << std::endl;
        std::cout << "Attribute Access Modifier: " << attributes[i].accessModifier << std::endl;
        std::cout << "Attribute Name: " << attributes[i].name.toStdString() << std::endl;
        std::cout << "Attribute Type: " << attributes[i].type.toStdString() << std::endl;

    }

    std::cout << "Attributes Count: " << attributeList.length() << std::endl;
    std::cout << "Class Attributes Area has been updated" << std::endl;

    refresh();
}

void ClassShape::classMethodsAreaChanged()
{
    classMethodsAreaChanged(true);
}

void ClassShape::classMethodsAreaChanged(bool textChanged)
{
    std::cout << "Inside classMethodsAreaChanged()" << std::endl;

    if (textChanged && currentDetailLevel() > NO_ABBREV)
    {
        setDetailLevel(NO_ABBREV);
    }

    QString tempClassMethodsAreaText(m_editDialog->getClassMethodsArea()->toPlainText().trimmed());

    unsigned int i, j;

    if (tempClassMethodsAreaText.isEmpty())
    {
        classMethodsAreaText = "";
        methods.resize(0);
        refresh();
        return;
    }
    else
    {
        methods.resize(tempClassMethodsAreaText.count("\n") + 1);
    }

    QStringList methodList = tempClassMethodsAreaText.split("\n");
    classMethodsAreaText = "";

    for (i = 0; i < methodList.length(); i++)
    {
        QString currentMethod(methodList.value(i).trimmed());

        if (currentMethod.startsWith("+") || currentMethod.startsWith("-") || currentMethod.startsWith("#"))
        {
            methods[i].accessModifier = currentMethod.startsWith("~") ? PACKAGE : currentMethod.startsWith("+") ? PUBLIC : currentMethod.startsWith("-") ? PRIVATE : PROTECTED;
            currentMethod = currentMethod.mid(1).trimmed();
        }
        else {
            methods[i].accessModifier = DEFAULT;
        }

        QString parameterListText("");

        if (currentMethod.contains("(") && currentMethod.contains(")") && !currentMethod.startsWith("("))
        {
            methods[i].name = currentMethod.left(currentMethod.indexOf("(")).simplified().replace(" ", "_");
            currentMethod = currentMethod.mid(currentMethod.indexOf("("));

            QStringList parameterTempList = currentMethod.mid(1, currentMethod.indexOf(")") - 1).trimmed().split(",");
            QStringList parameterList;

            for (j = 0; j < parameterTempList.length(); j++)
            {
                QString tempParameter(parameterTempList.value(j).trimmed());
                if (!tempParameter.isEmpty())
                {
                    parameterList.append(tempParameter);
                }
            }

            methods[i].params.resize(parameterList.length());

            for (j = 0; j < parameterList.length(); j++)
            {
                QString currentParameter(parameterList.value(j));

                if (currentParameter.toLower().startsWith("in "))
                {
                    methods[i].params[j].mode = PARAM_IN;
                    currentParameter = currentParameter.mid(3).trimmed();
                }
                else if (currentParameter.toLower().startsWith("out "))
                {
                    methods[i].params[j].mode = PARAM_OUT;
                    currentParameter = currentParameter.mid(4).trimmed();
                }
                else
                {
                    methods[i].params[j].mode = UNSPECIFIED;
                }

                if (!currentParameter.contains(":"))
                {
                    methods[i].params[j].name = currentParameter.simplified().replace(" ", "_");
                    methods[i].params[j].type = "";
                }
                else if (!currentParameter.startsWith(":") && !currentParameter.endsWith(":"))
                {
                    methods[i].params[j].name = currentParameter.mid(0, currentParameter.indexOf(":")).simplified().replace(" ", "_");
                    methods[i].params[j].type = currentParameter.mid(currentParameter.indexOf(":") + 1).simplified().replace(" ", "_");
                }
                else if (currentParameter.endsWith(":"))
                {
                    methods[i].params[j].name = currentParameter.mid(0, currentParameter.indexOf(":")).simplified().replace(" ", "_");
                    methods[i].params[j].type = "";
                }
                else
                {
                    methods[i].params[j].name = "";
                    methods[i].params[j].type = "";
                    methods[i].params[j].mode = UNSPECIFIED;
                }

                if (methods[i].params[j].name.isEmpty())
                {
                    methods[i].params[j].type = "";
                    methods[i].params[j].mode = UNSPECIFIED;

                    if (j == parameterList.length() - 1)
                    {
                       parameterListText = parameterListText.left(parameterListText.length() - 2);
                    }
                }
                else
                {
                    if (methods[i].params[j].mode == PARAM_IN)
                    {
                        parameterListText += "in ";
                    }
                    else if (methods[i].params[j].mode == PARAM_OUT)
                    {
                        parameterListText += "out ";
                    }
                    parameterListText += methods[i].params[j].name;
                    if (!methods[i].params[j].type.isEmpty())
                    {
                        parameterListText += ": " + methods[i].params[j].type;
                    }
                    if (j != parameterList.length() - 1)
                    {
                        parameterListText += ", ";
                    }
                }

                std::cout << "Method Parameter Mode: " << methods[i].params[j].mode << std::endl;
                std::cout << "Method Parameter Name: " << methods[i].params[j].name.toStdString() << std::endl;
                std::cout << "Method Parameter Type: " << methods[i].params[j].type.toStdString() << std::endl;

            }

            currentMethod = currentMethod.mid(currentMethod.indexOf(")") + 1);

            if (currentMethod.isNull() || !currentMethod.contains(":") || currentMethod.trimmed().endsWith(":"))
            {
                methods[i].return_type = "";
            }
            else
            {
                methods[i].return_type = currentMethod.mid(currentMethod.indexOf(":") + 1).simplified().replace(" ", "_");
            }
        }
        else
        {
            methods[i].name = "";
            methods[i].params.resize(0);
            methods[i].return_type = "";
            methods[i].accessModifier = DEFAULT;
        }

        if (methods[i].name.isEmpty())
        {
            methods[i].accessModifier = DEFAULT;
            methods[i].params.resize(0);
            methods[i].return_type = "";

            if (i == methodList.length() - 1)
            {
                classMethodsAreaText = classMethodsAreaText.trimmed();
            }
        }
        else
        {
            if (methods[i].accessModifier == PRIVATE)
            {
                classMethodsAreaText += "- ";
            }
            else if (methods[i].accessModifier == PROTECTED)
            {
                classMethodsAreaText += "# ";
            }
            else if (methods[i].accessModifier == PUBLIC)
            {
                classMethodsAreaText += "+ ";
            }
            else if (methods[i].accessModifier == PACKAGE)
            {
                classMethodsAreaText += "~ ";
            }
            classMethodsAreaText += methods[i].name + "(";
            if (!parameterListText.isEmpty())
            {
                classMethodsAreaText += parameterListText;
            }
            classMethodsAreaText += ")";
            if (!methods[i].return_type.isEmpty())
            {
                classMethodsAreaText += ": " + methods[i].return_type;
            }
            if (i != methodList.length() - 1)
            {
                classMethodsAreaText += "\n";
            }
        }

        std::cout << "Method Access Modifier: " << methods[i].accessModifier << std::endl;
        std::cout << "Method Name: " << methods[i].name.toStdString() << std::endl;
        std::cout << "Method Parameter Count: " << methods[i].params.size() << std::endl;
        std::cout << "Method Return Type: " << methods[i].return_type.toStdString() << std::endl;
    }

    std::cout << "Methods Count: " << methodList.length() << std::endl;
    std::cout << "Class Methods Area has been updated" << std::endl;

    refresh();
}

QDomElement newTextChild(QDomElement& node, const QString& ns, 
        const QString& name, QString arg, QDomDocument& doc)
{
    /*
    QByteArray bytes;
    QDataStream o(&bytes, QIODevice::WriteOnly);
    o << arg;
    */
    Q_UNUSED (ns)

    QDomElement new_node = doc.createElement(name);

    QDomText text = doc.createTextNode(arg);
    new_node.appendChild(text);

    node.appendChild(new_node);

    return new_node;
}


QDomElement ClassShape::to_QDomElement(const unsigned int subset, QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newProp(node, x_type, "umlClass");
    }

    addXmlProps(subset, node, doc);
    
    if (subset & XMLSS_IOTHER)
    {
        /* UML Class specific xml (represented as children */
        //class name
        newTextChild(node, x_dunnartNs, "class_name", class_name, doc);
        newTextChild(node, x_dunnartNs, "class_stereotype", class_stereotype, doc);

        //class mode
        newTextChild(node, x_dunnartNs, "mode", QString::number(currentDetailLevel()), doc);

        //section heights
        newTextChild(node, x_dunnartNs, 
                "attr_section_height", QString::number(classAttributesSectionHeight), doc);
        newTextChild(node, x_dunnartNs, 
                "method_section_height", QString::number(classMethodsSectionHeight), doc);
        newTextChild(node, x_dunnartNs,
                "name_section_height", QString::number(classNameSectionHeight), doc);
        newTextChild(node, x_dunnartNs,
                "class_width", QString::number(classWidth), doc);
        
        //attributes
        for (unsigned int i=0; i<attributes.size(); i++)
        {
            QDomElement attr_node = doc.createElement("attribute");
            if (attributes[i].stereotype.length() > 0)
            {
                newProp(attr_node, "stereotype", attributes[i].stereotype);
            }
            newProp(attr_node, "accessModifier", attributes[i].accessModifier);
            newProp(attr_node, "name", attributes[i].name);
            newProp(attr_node, "type", attributes[i].type);
            node.appendChild(attr_node);
        }
        for (unsigned int i=0; i<methods.size(); i++)
        {
            QDomElement method_node = doc.createElement("method");
            newProp(method_node, "accessModifier", methods[i].accessModifier);
            newProp(method_node, "name", methods[i].name);
            newProp(method_node, "return_type", methods[i].return_type);

            for (unsigned int j=0; j<methods[i].params.size(); j++)
            {
                QDomElement param_node = doc.createElement("parameter");
                if (methods[i].params[j].mode != UNSPECIFIED)
                {
                    newProp(param_node, "mode", 
                            methods[i].params[j].mode == PARAM_IN ? "in" : "out");
                }
                newProp(param_node, "name", methods[i].params[j].name);
                newProp(param_node, "type", methods[i].params[j].type);
                method_node.appendChild(param_node);
            }
            node.appendChild(method_node);
        }
    }

    return node;
}

void ClassShape::do_init()
{
    std::cout << "DO_____________INIT" << std::endl;
    //mode_changed_manually = false;

    //setDetailLevel(NO_ABBREV);

    //m_detail_level = 1;   // updated shape.h sets the default detail level as 1

    //QT mono = FONT_LoadTTF("VeraMono.ttf", umlFontSize);
    mono = QApplication::font();

}

QSizeF ClassShape::recalculateSize()
{
    std::cout << "Current Detail Level: " << currentDetailLevel() << std::endl;

    // 50 is current default height; a line is considered as 5px thick for padding section inwards.
    return QSizeF(qreal(getMaxWidth() + 10), qreal(qMax(getClassNameSectionHeight() + getClassAttributesSectionHeight() + getClassMethodsSectionHeight() + 30, 50)));

}

int ClassShape::getMaxWidth()
{
    QFontMetrics qfm(canvas()->canvasFont());
    int maxWidth, i, j, currentLineWidth;

    maxWidth = qfm.width(class_name);

    if (!class_stereotype.isEmpty() && currentDetailLevel() < NO_CLASS_STEREOTYPES)
    {
        maxWidth = qMax(maxWidth, qfm.width("<<" + class_stereotype + ">>"));
    }

    for (i = 0; i < attributes.size() && currentDetailLevel() < NO_ATTRS; i++)
    {
        currentLineWidth = qfm.width(attributes[i].name);
        if (currentLineWidth != 0 && !attributes[i].type.isEmpty() && currentDetailLevel() < NO_ATTR_TYPES)
        {
            currentLineWidth += qfm.width(": " + attributes[i].type);
        }
        if (currentLineWidth != 0 && !attributes[i].stereotype.isEmpty() && currentDetailLevel() < NO_ATTR_STEREOTYPES)
        {
            currentLineWidth += qfm.width("<<" + attributes[i].stereotype + ">> ");
        }
        if (currentLineWidth != 0 && attributes[i].accessModifier == PACKAGE)
        {
            currentLineWidth += qfm.width("~ ");
        }
        else if (currentLineWidth != 0 && attributes[i].accessModifier == PUBLIC)
        {
            currentLineWidth += qfm.width("+ ");
        }
        else if (currentLineWidth != 0 && attributes[i].accessModifier == PROTECTED)
        {
            currentLineWidth += qfm.width("# ");
        }
        else if (currentLineWidth != 0 && attributes[i].accessModifier == PRIVATE)
        {
            currentLineWidth += qfm.width("- ");
        }
        maxWidth = qMax(maxWidth, currentLineWidth);
    }

    for (i = 0; i < methods.size() && currentDetailLevel() < NO_METHODS; i++)
    {
        currentLineWidth = 0;
        if (!methods[i].name.isEmpty())
        {
            currentLineWidth += qfm.width(methods[i].name + "()");
        }
        if (currentLineWidth != 0 && !methods[i].return_type.isEmpty() && currentDetailLevel() < NO_RETURN_TYPES)
        {
            currentLineWidth += qfm.width(": " + methods[i].return_type);
        }
        if (currentLineWidth != 0 && currentDetailLevel() < NO_PARAMS)
        {
            for (j = 0; j < methods[i].params.size(); j++)
            {
                if (!methods[i].params[j].name.isEmpty())
                {
                    currentLineWidth += qfm.width(methods[i].params[j].name);
                    if (j != methods[i].params.size() - 1)
                    {
                        currentLineWidth += qfm.width(", ");
                    }
                    if (methods[i].params[j].mode == PARAM_IN && currentDetailLevel() < NO_PARAM_MODES)
                    {
                        currentLineWidth += qfm.width("in ");
                    }
                    else if (methods[i].params[j].mode == PARAM_OUT && currentDetailLevel() < NO_PARAM_MODES)
                    {
                        currentLineWidth += qfm.width("out ");
                    }
                    if (!methods[i].params[j].type.isEmpty() && currentDetailLevel() < NO_PARAM_TYPES)
                    {
                        currentLineWidth += qfm.width(": " + methods[i].params[j].type);
                    }
                }
                else if (j != 1)
                {
                    currentLineWidth -= qfm.width(", ");
                }
            }
        }
        if (currentLineWidth != 0 && methods[i].accessModifier == PACKAGE)
        {
            currentLineWidth += qfm.width("~ ");
        }
        else if (currentLineWidth != 0 && methods[i].accessModifier == PUBLIC)
        {
            currentLineWidth += qfm.width("+ ");
        }
        else if (currentLineWidth != 0 && methods[i].accessModifier == PROTECTED)
        {
            currentLineWidth += qfm.width("# ");
        }
        else if (currentLineWidth != 0 && methods[i].accessModifier == PRIVATE)
        {
            currentLineWidth += qfm.width("- ");
        }
        maxWidth = qMax(maxWidth, currentLineWidth);
    }

    std::cout << "Calculated Max Width: " << maxWidth << std::endl;
    std::cout << "Current Shape Width: " << size().width() << std::endl;

    return classWidth = qMax(maxWidth, 60);  // 60 (+10 = 70) is current default width;
}

int ClassShape::getClassNameSectionHeight()
{
    QFontMetrics qfm(canvas()->canvasFont());
    int lineCount = class_name.isEmpty() ? 0 : (class_stereotype.isEmpty() || currentDetailLevel() == NO_CLASS_STEREOTYPES) ? 1 : 2;
    return classNameSectionHeight = qMax(lineCount * qfm.height() + (lineCount - 1) * qfm.leading(), 7);
}

int ClassShape::getClassAttributesSectionHeight()
{
    QFontMetrics qfm(canvas()->canvasFont());
    int lineCount = 0;

    if (currentDetailLevel() < NO_ATTRS)
    {
        for (int i = 0; i < attributes.size(); i++)
        {
            if(!attributes[i].name.isEmpty())
            {
                ++lineCount;
            }
        }
    }
    return classAttributesSectionHeight = qMax(lineCount * qfm.height() + (lineCount - 1) * qfm.leading(), 7);
}

int ClassShape::getClassMethodsSectionHeight()
{
    QFontMetrics qfm(canvas()->canvasFont());
    int lineCount = 0;

    if (currentDetailLevel() < NO_METHODS)
    {
        for (int i = 0; i < methods.size(); i++)
        {
            if(!methods[i].name.isEmpty())
            {
                ++lineCount;
            }
        }
    }
    return classMethodsSectionHeight = qMax(lineCount * qfm.height() + (lineCount - 1) * qfm.leading(), 7);
}

#if 0
void ClassShape::change_label(void)
{
    do_edit(EDIT_CLASS_NAME);
}

//middle click will open a text box for editing, section edited depends on
//mouse co-ordinates
void ClassShape::middle_click(const int& mouse_x, const int& mouse_y)
{
    if (mode == CLASS_NAME_ONLY)
        do_edit(EDIT_CLASS_NAME);
    else
    {
        // ??? ypos() === CanvasItem::boundingRect().center().y()
        qreal ypos = CanvasItem::boundingRect().center().y();
        if (mouse_y - ypos  - this->get_parent()->get_absypos() < (int) get_class_name_section_height())
            do_edit(EDIT_CLASS_NAME);
        else if (mouse_y - ypos - this->get_parent()->get_absypos() < (int) get_class_name_section_height() +
                                                     (int) get_attr_section_height())
            do_edit(EDIT_ATTRIBUTES);
        else
            do_edit(EDIT_METHODS);
    }
}

//after editing, parse the contents of the text area and update class contents
//no seperate tokeniser. If the grammars are extended it may be worth using one.
//information that could not be parsed is simply left blank.
void ClassShape::update_contents(UML_Class_Edit_Type edit_type, const std::vector<QString>& lines, const bool store_undo)
{
    class_name = lines[0];
    QRectF qrf = CanvasItem::boundingRect();
    /*std::cout << qrf.width() << " " << qrf.height() << std::endl;
    std::cout << this->size().width() << " " << this->size().height() << std::endl;*/

    //check to see if a resize is needed
    /*
    int new_width, new_height;
    determine_best_dimensions(&new_width, &new_height);
    setPosAndSize(QPointF(qrf.center().x(), qrf.center().y()), QSizeF(new_width+16, new_height));
    */
    //QT mathematicallyRepositionLabels();
    //canvas()->layout()->setRestartFromDunnart();
    canvas()->layout()->setRestartFromDunnart();
    update(qrf);
    std::cout << " has been updated" << std::endl;
    return;

    Q_UNUSED (store_undo)
    if (edit_type == EDIT_CLASS_NAME)
    {
        std::cout << "Class Name";
        char *s = strdup(lines[0].toUtf8().constData());
        if (*s == '<' && *(s+1) == '<')
        {
            s+=2;
            char *stype = s;    //beginning of stereotype

            while (*s != '>') s++;
            *s = '\0';
            class_stereotype = stype;
            s += 2;
            while (*s == ' ') s++;
            class_name = s;
        }
        else
            class_name = lines[0];
    }
    else if (edit_type == EDIT_ATTRIBUTES)
    {
        unsigned int i;

        if (lines.size() == 1 && lines[0].length() == 0)
            attributes.resize(0);
        else
            attributes.resize(lines.size());

        for (i=0; i<lines.size(); i++)
        {
            char *s = strdup(lines[i].toUtf8().constData());

            //optionally a stereotype first
            if (*s == '<' && *(s+1) == '<')
            {
                s += 2;
                char *stype = s;    //beginning of stereotype

                while (*s != '>') s++;
                *s = '\0';
                attributes[i].stereotype = stype;
                s += 2;
                while (*s == ' ') s++;

            }
            else
            {
                attributes[i].stereotype = "";
            }

            while (*s == ' ') s++;

            if (*s == '+' || *s == '-')
            {
                attributes[i].is_public = (*s == '+');
                s++;
                while (*s == ' ') s++;
            }


            //at the start of a word
            char *attr_name = s;

            //find end of word
            while (*s != ' ' && *s != '\0' && *s != ':') s++;

            if (*s == '\0')
                attributes[i].name = attr_name;
            else
            {
                int ch = *s;
                *s = '\0';
                attributes[i].name = attr_name;
                *s = ch;
                while (*s == ' ') s++;
                if (*s == ':')
                {
                    s++;
                    while (*s == ' ') s++;
                    char *attr_type = s;
                    while (*s != ' ' && *s != '\0') s++;
                    if (*s != '\0')
                        *s = '\0';
                    attributes[i].type = attr_type;
                }
                else
                    attributes[i].type = "";
            }

        }
    }
    else if (edit_type == EDIT_METHODS)
    {
        unsigned int i;
        methods.resize(lines.size());
        for (i=0; i<lines.size(); i++)
        {
            char *s = strdup(lines[i].toUtf8().constData());

            while (*s == ' ') s++;

            if (*s == '+' || *s == '-')
            {
                methods[i].is_public = (*s == '+');
                s++;
                while (*s == ' ') s++;

            }

            //at the start of a word
            char *method_name = s;

            //find end of word
            while (*s != ' ' && *s != '\0' && *s != '(') s++;

            int ch = *s;
            *s = '\0';
            methods[i].name = method_name;
            *s = ch;

            while (*s == ' ') s++;
            if (*s == '(')
            {
                int j = 0;
                methods[i].params.resize(0);
                s++;
                while (*s == ' ') s++;

                while (*s != ')' && *s != '\0')
                {
                    methods[i].params.resize(j+1);

                    //parse param
                    while (*s == ' ') s++;
                    if (*s == ',') s++;
                    while (*s == ' ') s++;

                    //optionally an 'in' or 'out' first.
                    if (*s == 'i' && *(s+1) == 'n')
                    {
                        methods[i].params[j].mode = PARAM_IN;
                        s += 2;
                        while (*s == ' ') s++;
                    }
                    else if (*s == 'o' && *(s+1) == 'u' && *(s+2) == 't')
                    {
                        methods[i].params[j].mode = PARAM_OUT;
                        s += 3;
                        while (*s == ' ') s++;
                    }
                    else
                        methods[i].params[j].mode = UNSPECIFIED;

                    char *param_name = s;

                    while (*s != ' ' && *s != '\0' && *s != ':' && *s != ')') s++;

                    //record param name
                    int ch = *s;
                    *s = '\0';
                    methods[i].params[j].name = param_name;
                    *s = ch;
                    while (*s == ' ') s++;

                    //param type
                    if (*s == ':')
                    {
                        s++;
                        while (*s == ' ') s++;
                        char *param_type = s;
                        while (*s != ' ' && *s != '\0' && *s != ')' && *s != ',') s++;

                        //record param type
                        ch = *s;
                        *s = '\0';
                        methods[i].params[j].type = param_type;
                        *s = ch;
                    }
                    else
                        methods[i].params[j].type = "";
                    while (*s == ' ') s++;
                    j++;
                }
                if (*s == ')') s++;
                while (*s == ' ') s++;
            }

            //method type
            if (*s == ':')
            {
                s++;
                while (*s == ' ') s++;
                char *method_type = s;
                while (*s != ' ' && *s != '\0') s++;
                if (*s != '\0')
                    *s = '\0';
                methods[i].return_type = method_type;
            }
            else
                methods[i].return_type = "";
        }
    }
//#if 0
    //QT

    qrf = CanvasItem::boundingRect();
    /*std::cout << qrf.width() << " " << qrf.height() << std::endl;
    std::cout << this->size().width() << " " << this->size().height() << std::endl;*/

    //check to see if a resize is needed
    /*
    int new_width, new_height;
    determine_best_dimensions(&new_width, &new_height);
    setPosAndSize(QPointF(qrf.center().x(), qrf.center().y()), QSizeF(new_width+16, new_height));
    */
    //QT mathematicallyRepositionLabels();
    //canvas()->layout()->setRestartFromDunnart();
    canvas()->layout()->setRestartFromDunnart();
    update(qrf);
    std::cout << " has been updated" << std::endl;
    //GraphLayout::getInstance()->setRestartFromDunnart();
    //repaint_canvas();
//#endif
}



//bring up a text box for editing
void ClassShape::do_edit(UML_Class_Edit_Type edit_type)
{
    Q_UNUSED (edit_type)

    int fypos = 0;
    int absxpos = 0; // What is absxpos?
    int fxpos = absxpos + (width() / 2);
    FieldLines lines;

    QFontMetrics qfm(mono);
    int texth = qfm.height();
    int textw;
    int padding = 7;
    int fheight = texth + padding;
    int fwidth = 360;

    int lineCount = 1;
    int extraLines = 3;
    if (edit_type == EDIT_CLASS_NAME)
    {
        lines.push_back(class_name);
        if (mode == CLASS_NAME_ONLY)
        {
            fypos = absypos + (height / 2) - (fheight / 2);
        }
        else
        {
            fypos = absypos + 5;
        }
    }

    if (edit_type == EDIT_ATTRIBUTES)
    {
        lines.resize(attributes.size());
        for (unsigned int i = 0; i < attributes.size(); ++i)
        {
            QString str = attribute_to_string(i);
            lines[i] = str;
            //TTF_SizeText(mono, str.c_str(), &textw, NULL);
            textw = qfm.width(str);
            fwidth = qMax(fwidth, textw);
        }
        fypos = absypos + 25;
        lineCount = attributes.size() + extraLines;
    }
    if (edit_type == EDIT_METHODS)
    {
        lines.resize(methods.size());
        for (unsigned int i = 0; i < methods.size(); ++i)
        {
            QString str = method_to_string(i);
            lines[i] = str;
            //TTF_SizeText(mono, str.c_str(), &textw, NULL);
            textw = qfm.width(str);
            fwidth = qMax(fwidth, textw);
        }
        fypos = absypos + 27 + ((height - 23) / 2);
        lineCount = methods.size() + extraLines;
    }
    // Set minimum number of lines if inputing methods or attributes, or
    // use the current number of lines plus a couple of extra:
    if (edit_type != EDIT_CLASS_NAME) lineCount = qMax(13, lineCount);
    fheight = (lineCount * texth) + padding;
    fwidth += padding + 40;


    fxpos -= (fwidth / 2);

    int screenPadding = 8;
    // Make sure the text field doesn't go off the screen:
    fxpos = qMin(fxpos, screen->w - (fwidth + screenPadding));
    fxpos = qMax(fxpos, screenPadding);
    fypos = qMin(fypos, screen->h - (fheight + screenPadding));
    fypos = qMax(fypos, screenPadding);


    Field *field = new Field(NULL, fxpos, fypos, fwidth, fheight);
    if (edit_type == EDIT_CLASS_NAME)
    {
        // Limit field to one line.
        field->setLimits(1, 0);
    }
    field->setFont(mono);
    field->setValue(lines);
    SDL_FastFlip(screen);
    bool modified = field->editText();

    if (modified)
    {
        //Save undo information:
        // canvas()->beginUndoMacro(tr("Change Label Property"));
        //add_undo_record(DELTA_LABEL, this);
        //end_undo_scope();
        FieldLines& lines = field->getLines();
        update_contents(edit_type, lines);
    }
    delete field;

}

int ClassShape::get_longest_text_width(UML_Class_Abbrev_Mode mode)
{
    int i, j, longest = 0, width = 0;

    QString s;

    //name of class.
    s = class_stereotype.length() > class_name.length() ? "<<" + class_stereotype + ">>" : class_name;
    get_text_width(s, &longest);

    if (mode == CLASS_NAME_ONLY)
        return longest;

    //attributes
    for (i=0; i<(int)attributes.size(); i++)
    {
        if (attributes[i].stereotype.length() > 0)
            s = "<<" + attributes[i].stereotype + ">> ";
        else
            s = "";
        s += attributes[i].name;
        if (mode < NO_TYPES)
            s += " : " + attributes[i].type;
        get_text_width(s, &width);
        longest=qMax(longest, width+9);      //the + or - takes up 9 pixels.
    }

    //methods
    for (i=0; i<(int)methods.size(); i++)
    {
        s = methods[i].name + "(";
        if (mode < NO_PARAMS)
        {
            for (j = 0; j < (int)methods[i].params.size(); j++)
            {
                s += j == 0 ? "" : ", ";
                if (methods[i].params[j].mode == PARAM_IN)
                    s += "in ";
                if (methods[i].params[j].mode == PARAM_OUT)
                    s += "out ";
                s += methods[i].params[j].name;
                if (mode < NO_PARAM_TYPES)
                    s += ":" + methods[i].params[j].type;

            }
        }
        s += ")";

        if (mode < NO_TYPES)
            s += ":" + methods[i].return_type;
        get_text_width(s, &width);
        longest=qMax(longest, width+9); //the + or - takes up 9 pixels..
    }

    return longest;
}

void ClassShape::determine_best_mode(void)
{
    //width irrelevant if height too small.
    if (height() < (int) get_class_name_section_height() + (HANDLE_PADDING * 2))
    {
        mode = CLASS_NAME_ONLY;
        //m_detail_level = 5;
        //QT m_detail_level = 0;
        return;
    }

    UML_Class_Abbrev_Mode i;
    for (i = NOABBREV; i < CLASS_NAME_ONLY; i = (UML_Class_Abbrev_Mode) (i+1))
    {
        if (get_longest_text_width(i) <= width() - 15)
        {
            mode = i;
            return;
        }
    }
    mode = CLASS_NAME_ONLY;
}


void ClassShape::determine_small_dimensions(int *w, int *h)
{
    *w = 22;
    *h = 7;
}


void ClassShape::determine_best_dimensions(int *inner_width, int *h)
{
    *inner_width = get_longest_text_width(static_cast<UML_Class_Abbrev_Mode>(ShapeObj::currentDetailLevel())) + 15;
    *h = get_class_name_section_height();
    if (static_cast<UML_Class_Abbrev_Mode>(ShapeObj::currentDetailLevel()) != CLASS_NAME_ONLY)
    {
        attr_section_size = qMax(classLineHeight,
                (int)attributes.size()*classLineHeight) + 2;
        method_section_size = qMax(classLineHeight,
                (int)methods.size()*classLineHeight) + 2;
        *h += attr_section_size + method_section_size;
    }
}


#if 0
void ClassShape::draw(QPixmap *surface, const int x, const int y,
        const int type, const int w, const int h)
{
    if (!surface)
        return;

    Rect::draw(surface, x, y, type, w, h);
    const QColor black = Qt::black;
    //SDL_Color SDL_black = {0x00, 0x00, 0x00, 0};

    int classSectHeight = get_class_name_section_height();

    if (currentDetailLevel() > 0)
    {
        Draw_HLine(surface, x+3, y+classSectHeight, x+w-6, black);
        Draw_HLine(surface, x+3, y+classSectHeight+get_attr_section_height(),
                x+w-6, black);

        //print class name in top section
        if (class_stereotype.length() > 0)
        {
                        QString s = "<<" + class_stereotype + ">>";
            center_text(surface, x+6, x+w-7, y+5, y+19, s);
            center_text(surface, x+6, x+w-7, y+19, y+33, class_name);
        }
        else
            center_text(surface, x+6, x+w-7, y+5, y+21,  class_name);

        QString s;
        QPainter qp(surface);
        unsigned int lines = get_attr_section_height() / classLineHeight;
        for (unsigned int i = 0; (i < lines) && (i < attributes.size()); ++i)
        {
            int lineYPos = y+classSectHeight+1+i*classLineHeight;
            if ((i == lines - 1) && (i < attributes.size() - 1))
            {
                // Last line, but not the last attribute, so print "...":
                //SDL_WriteText(surface, x+15, lineYPos, "...", &SDL_black, mono);
                qp.drawText(QPointF(x+15, lineYPos), "...");
                break;
            }
            if (attributes[i].stereotype.length() > 0)
                s = "<<" + attributes[i].stereotype + ">> ";
            else
                s = "";
            s += attributes[i].name;
            if (mode < NO_TYPES)
                s += ":" + attributes[i].type;
            if (attributes[i].stereotype.length() > 0)
            {
                //SDL_WriteText(surface, x+6, lineYPos, s.c_str(), &SDL_black, mono);
                qp.drawText(QPointF(x+6, lineYPos), s);
            }
            else
            {
               // SDL_WriteText(surface, x+6, lineYPos, attributes[i].is_public ? "+" : "-", &SDL_black, mono);
                //SDL_WriteText(surface, x+15, lineYPos, s.c_str(), &SDL_black, mono);
                qp.drawText(QPointF(x+6, lineYPos), attributes[i].is_public ? "+" : "-");
                qp.drawText(QPointF(x+15, lineYPos), s);
            }
        }

        lines = get_method_section_height() / classLineHeight;
        for (unsigned int i = 0; (i < lines) && (i < methods.size()); ++i)
        {
            int lineYPos = y+classSectHeight+1+i*classLineHeight +
                    get_attr_section_height();
            if ((i == lines - 1) && (i < methods.size() - 1))
            {
                // Last line, but not the last method, so print "...":
                //SDL_WriteText(surface, x+15, lineYPos, "...", &SDL_black, mono);
                qp.drawText(QPointF(x+15, lineYPos), "...");
                break;
            }
            s = methods[i].name + "(";
            unsigned int j;
            if (mode < NO_PARAMS)
            {
                for (j = 0; j < methods[i].params.size(); j++)
                {
                    s += j == 0 ? "" : ", ";
                    if (methods[i].params[j].mode == PARAM_IN)
                        s += "in ";
                    if (methods[i].params[j].mode == PARAM_OUT)
                        s += "out ";
                    s += methods[i].params[j].name;
                    if (mode < NO_PARAM_TYPES)
                        s += ":" + methods[i].params[j].type;
                };
            }

            s += ")";
            if (mode < NO_TYPES && methods[i].return_type.length() > 0)
                s += ":" + methods[i].return_type;

            //SDL_WriteText(surface, x+6, lineYPos, methods[i].is_public ? "+" : "-", &SDL_black, mono);
            //SDL_WriteText(surface, x+15, lineYPos, s.c_str(), &SDL_black, mono);
            qp.drawText(QPointF(x+6, lineYPos), methods[i].is_public ? "+" : "-");
            qp.drawText(QPointF(x+15, lineYPos), s);
        }
    }
    else
    {
        //print class name in center
        if (class_stereotype.length() > 0)
        {
                        QString s = "<<" + class_stereotype + ">>";
            center_text(surface, x+6, x+w-7, y+5, y+h-23, s);
            center_text(surface, x+6, x+w-7, y+19, y+h-9, class_name);
        }
        else
            center_text(surface, x+6, x+w-7, y+5, y+h-9, class_name);
    }

}
#endif


QString ClassShape::attribute_to_string(int i, UML_Class_Abbrev_Mode mode)
{
    QString s;
    if (attributes[i].stereotype.length() > 0)
        s = "<<" + attributes[i].stereotype + ">> ";
    else
        s = attributes[i].is_public ? "+" : "-";
    s += attributes[i].name;
    if (static_cast<UML_Class_Abbrev_Mode>(ShapeObj::currentDetailLevel()) < NO_TYPES)
        s += ":" + attributes[i].type;
    return s;
}

QString ClassShape::method_to_string(int i, UML_Class_Abbrev_Mode mode)
{
    QString s;
    s = (methods[i].is_public ? "+" : "-") + methods[i].name + "(";
    unsigned int j;
    if (static_cast<UML_Class_Abbrev_Mode>(ShapeObj::currentDetailLevel()) < NO_PARAMS)
    {
        for (j = 0; j < methods[i].params.size(); j++)
        {
            s += j == 0 ? "" : ", ";
            if (methods[i].params[j].mode == PARAM_IN)
                s += "in ";
            if (methods[i].params[j].mode == PARAM_OUT)
                s += "out ";
            s += methods[i].params[j].name;
            if (static_cast<UML_Class_Abbrev_Mode>(ShapeObj::currentDetailLevel()) < NO_PARAM_TYPES)
                s += ":" + methods[i].params[j].type;
        };
    }

    s += ")";
    if (static_cast<UML_Class_Abbrev_Mode>(ShapeObj::currentDetailLevel()) < NO_TYPES && methods[i].return_type.length() > 0)
        s += ":" + methods[i].return_type;
    return s;
}

QString ClassShape::class_name_to_string(bool one_line)
{
    return class_stereotype.length() > 0 ? "<<" + class_stereotype + ">>" + (one_line ? "" : "\n") + class_name : class_name;
}

static void center_text(QPixmap* surface, const int left, const int right,
    const int top, const int bottom, const QString text)
{
    //SDL_Color SDL_black = {0x00, 0x00, 0x00, 0};
    int tw, th;
    get_text_dimensions(text, &tw, &th);
    QPainter qp(surface);
    qp.drawText(QPointF(left + (right-(left+tw))/2, top + (bottom-(top+th))/2), text);
    //SDL_WriteText(surface, left + (right-(left+tw))/2, top + (bottom-(top+th))/2, text, &SDL_black, mono);

}


static void get_text_dimensions(QString text, int *w, int *h)
{
    //Q_UNUSED (text)
    //Q_UNUSED (w)
    //Q_UNUSED (h)

    //QT TTF_SizeText(mono, text, w, h);
    QFontMetrics qfm(mono);
    *w = qfm.width(text);
    *h = qfm.height();
}

static void get_text_width(QString text, int *w)
{
    int h;
    get_text_dimensions(text, w, &h);
}

int ClassShape::get_min_width()
{
    int w=0;
    get_text_width(class_name, &w);
    return w+10;
}

unsigned int ClassShape::get_class_name_section_height(void)
{
    return minSectHeight + (class_stereotype.length() > 0 ? 14 : 0);
}

unsigned int ClassShape::get_attr_section_height(void)
{
    return attr_section_size;
}

unsigned int ClassShape::get_method_section_height(void)
{
    return method_section_size;
}

void ClassShape::detailLevelChanged(void)
{

    mode_changed_manually = true;
    determine_best_mode();
    int section_sizes(height() - (HANDLE_PADDING * 2) -
            get_class_name_section_height());
    attr_section_size = 2 +
            qMax(classLineHeight, (int)attributes.size() * classLineHeight);
    method_section_size = 2 +
            qMax(classLineHeight, (int)methods.size() * classLineHeight);
    double proportion = attr_section_size /
            (double) (attr_section_size + method_section_size);
    attr_section_size = static_cast<int>(proportion * section_sizes);
    method_section_size = section_sizes - attr_section_size;

    update();

}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

