#include "searchwidget.h"
#include "canvasitem.h"
#include "shape.h"
#include "connector.h"
#include "plugins/shapes/uml/umlclass.h"
#include "plugins/shapes/uml/umlnote.h"
#include "plugins/shapes/uml/umlpackage.h"

namespace dunnart {

SearchWidget::SearchWidget(CanvasView *canvasView, QWidget *parent) :
    QDockWidget(parent),
    m_canvas(NULL),
    m_canvasview(NULL)
{
    setupUi(this);

    changeCanvasView(canvasView);

    typeCombo->addItem("Any Types");
    typeCombo->addItem("Association");
    typeCombo->addItem("Inheritance");
    typeCombo->addItem("Aggregation");
    typeCombo->addItem("Composition");

    strengthCombo->addItem("Any Strength");
    strengthCombo->addItem("Strong");
    strengthCombo->addItem("Weak");

    navigabilityCombo->addItem("Any Navigability");
    navigabilityCombo->addItem("Unspecified");
    navigabilityCombo->addItem("Navigable");
    navigabilityCombo->addItem("Non-navigable");

    connect(searchBox, SIGNAL(textChanged(QString)), this, SLOT(search()));
    connect(typeCombo, SIGNAL(activated(QString)), this, SLOT(search()));
    connect(strengthCombo, SIGNAL(activated(QString)), this, SLOT(search()));
    connect(navigabilityCombo, SIGNAL(activated(QString)), this, SLOT(search()));
    connect(resultsList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(locateResult(QListWidgetItem*)));
}

SearchWidget::~SearchWidget()
{
}

void SearchWidget::search(void)
{
    resultsList->clear();

    qDebug("Searching......................................");
    QList<CanvasItem *> canvasItems = m_canvas->items();

    QString searchText = searchBox->text().simplified();
    QString type = typeCombo->currentText();
    QString strength = strengthCombo->currentText();
    QString navigability = navigabilityCombo->currentText();

    if (type == "Any Types" && strength == "Any Strength" && navigability == "Any Navigability")
    {
        if (searchText.isEmpty())
        {
            return;
        }
        for (int i = 0; i < canvasItems.size(); ++i)
        {
            if (ClassShape *classShape = dynamic_cast<ClassShape*>(canvasItems[i]))
            {
                QString className(classShape->classNameAreaText);
                if (!className.isEmpty() && className.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Class " + QString::number(classShape->internalId()) + " Name: " + className.simplified());
                }
                QString classAttributes(classShape->classAttributesAreaText);
                if (!classAttributes.isEmpty() && classAttributes.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Class " + QString::number(classShape->internalId()) + " Attributes: " + classAttributes.simplified());
                }
                QString classMethods(classShape->classMethodsAreaText);
                if (!classMethods.isEmpty() && classMethods.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Class " + QString::number(classShape->internalId()) + " Methods: " + classMethods.simplified());
                }
            }
            else if (NoteShape *noteShape = dynamic_cast<NoteShape*>(canvasItems[i]))
            {
                QString noteLabel(noteShape->label());
                if (!noteLabel.isEmpty() && noteLabel.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Note " + QString::number(noteShape->internalId()) + " Text: " + noteLabel.simplified());
                }
            }
            else if (PackageShape *packageShape = dynamic_cast<PackageShape*>(canvasItems[i]))
            {
                QString packageLabel(packageShape->label());
                if (!packageLabel.isEmpty() && packageLabel.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Package " + QString::number(packageShape->internalId()) + " Label: " + packageLabel);
                }
            }
            else if (ShapeObj *shapeObj = dynamic_cast<ShapeObj*>(canvasItems[i]))
            {
                QString shapeLabel(shapeObj->label());
                if (!shapeLabel.isEmpty() && shapeLabel.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Shape " + QString::number(shapeObj->internalId()) + " Label: " + shapeLabel.simplified());
                }
            }
            else if (Connector *connector = dynamic_cast<Connector*>(canvasItems[i]))
            {
                QString connSrcLabel1(connector->getSrcLabelRectangle1()->label());
                if (!connSrcLabel1.isEmpty() && connSrcLabel1.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Connector Label " + QString::number(connector->getSrcLabelRectangle1()->internalId()) + " Text: " + connSrcLabel1.simplified());
                }
                QString connSrcLabel2(connector->getSrcLabelRectangle2()->label());
                if (!connSrcLabel2.isEmpty() && connSrcLabel2.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Connector Label " + QString::number(connector->getSrcLabelRectangle2()->internalId()) + " Text: " + connSrcLabel2.simplified());
                }
                QString connMidLabel1(connector->getMiddleLabelRectangle1()->label());
                if (!connMidLabel1.isEmpty() && connMidLabel1.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Connector Label " + QString::number(connector->getMiddleLabelRectangle1()->internalId()) + " Text: " + connMidLabel1.simplified());
                }
                QString connMidLabel2(connector->getMiddleLabelRectangle2()->label());
                if (!connMidLabel2.isEmpty() && connMidLabel2.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Connector Label " + QString::number(connector->getMiddleLabelRectangle2()->internalId()) + " Text: " + connMidLabel2.simplified());
                }
                QString connDstLabel1(connector->getDstLabelRectangle1()->label());
                if (!connDstLabel1.isEmpty() && connDstLabel1.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Connector Label " + QString::number(connector->getDstLabelRectangle1()->internalId()) + " Text: " + connDstLabel1.simplified());
                }
                QString connDstLabel2(connector->getDstLabelRectangle2()->label());
                if (!connDstLabel2.isEmpty() && connDstLabel2.contains(searchText, Qt::CaseInsensitive))
                {
                    resultsList->addItem("Connector Label " + QString::number(connector->getDstLabelRectangle2()->internalId()) + " Text: " + connDstLabel2.simplified());
                }
            }
        }
    }
    else
    {
        QList<Connector *> foundConnectors;
        for (int i = 0; i < canvasItems.size(); ++i)
        {
            if (Connector *connector = dynamic_cast<Connector*>(canvasItems[i]))
            {
                if (strength != "Any Strength" && (connector->dashedStroke() && strength == "Weak" ||
                                                   !connector->dashedStroke() && strength == "Strong"))
                {
                    foundConnectors.push_back(connector);
                }
                else if (strength == "Any Strength")
                {
                    foundConnectors.push_back(connector);
                }
            }
        }
        int foundConnectorsSize = foundConnectors.size();

        for (int i = 0; i < foundConnectorsSize; ++i)
        {
            Connector *connector = foundConnectors[0];
            foundConnectors.removeFirst();
            if (navigability != "Any Navigability" && (connector->getDirected() != Connector::both && navigability == "Unspecified" ||
                (connector->getDirected() == Connector::either && connector->arrowHeadHeadType() != Connector::nonNavigable ||
                 connector->getDirected() == Connector::both && (connector->arrowHeadHeadType() != Connector::nonNavigable ||
                 connector->arrowTailHeadType() != Connector::nonNavigable)) && navigability == "Navigable" ||
                (connector->getDirected() == Connector::either && connector->arrowHeadHeadType() == Connector::nonNavigable ||
                 connector->getDirected() == Connector::both && (connector->arrowHeadHeadType() == Connector::nonNavigable ||
                 connector->arrowTailHeadType() == Connector::nonNavigable)) && navigability == "Non-navigable"))
            {
                foundConnectors.push_back(connector);
            }
            else if (navigability == "Any Navigability")
            {
                foundConnectors.push_back(connector);
            }
        }
        foundConnectorsSize = foundConnectors.size();

        for (int i = 0; i < foundConnectorsSize; ++i)
        {
            Connector *connector = foundConnectors[0];
            foundConnectors.removeFirst();
            if (type != "Any Types" &&
                    ((connector->getDirected() == Connector::either && connector->arrowHeadHeadType() == Connector::normal ||
                      connector->getDirected() == Connector::both && (connector->arrowHeadHeadType() == Connector::normal ||
                      connector->arrowTailHeadType() == Connector::normal) || connector->getDirected() == Connector::neither) && type == "Association" ||
                     connector->getDirected() == Connector::either && connector->arrowHeadHeadType() == Connector::triangle_outline && type == "Inheritance" ||
                     (connector->getDirected() == Connector::either && connector->arrowHeadHeadType() == Connector::diamond_outline ||
                      connector->getDirected() == Connector::both && (connector->arrowHeadHeadType() == Connector::diamond_outline && connector->arrowTailHeadType() == Connector::normal ||
                      connector->arrowTailHeadType() == Connector::diamond_outline && connector->arrowHeadHeadType() == Connector::normal)&& type == "Aggregation") ||
                     (connector->getDirected() == Connector::either && connector->arrowHeadHeadType() == Connector::diamond_filled ||
                      connector->getDirected() == Connector::both && (connector->arrowHeadHeadType() == Connector::diamond_filled && connector->arrowTailHeadType() == Connector::normal ||
                      connector->arrowTailHeadType() == Connector::diamond_filled && connector->arrowHeadHeadType() == Connector::normal)&& type == "Composition")))
            {
                foundConnectors.push_back(connector);
            }
            else if (type == "Any Types")
            {
                foundConnectors.push_back(connector);
            }
        }

        QList<ShapeObj*> associatedShapes;
        for (int i = 0; i < foundConnectors.size(); ++i)
        {
            Connector *connector = foundConnectors[i];
            const QString matchRelationship = "matches specified relationship";
            bool isKeywordsAssociatedWithTheConnector = false;

            ShapeObj *headShape = connector->getSrcShape();
            ShapeObj *tailShape = connector->getDstShape();

            if (!associatedShapes.contains(headShape))
            {
                associatedShapes.push_back(headShape);
                searchWithinEndShapes(headShape, searchText) ? isKeywordsAssociatedWithTheConnector = true : isKeywordsAssociatedWithTheConnector = false;
            }
            if (!associatedShapes.contains(tailShape))
            {
                associatedShapes.push_back(tailShape);
                if (searchWithinEndShapes(headShape, searchText) && !isKeywordsAssociatedWithTheConnector)
                {
                    isKeywordsAssociatedWithTheConnector = true;
                }
            }

            QString connSrcLabel1(connector->getSrcLabelRectangle1()->label());
            if (!connSrcLabel1.isEmpty() && connSrcLabel1.contains(searchText, Qt::CaseInsensitive))
            {
                resultsList->addItem("Connector Label " + QString::number(connector->getSrcLabelRectangle1()->internalId()) + " Text: " + connSrcLabel1.simplified());
                isKeywordsAssociatedWithTheConnector = true;
            }
            QString connSrcLabel2(connector->getSrcLabelRectangle2()->label());
            if (!connSrcLabel2.isEmpty() && connSrcLabel2.contains(searchText, Qt::CaseInsensitive))
            {
                resultsList->addItem("Connector Label " + QString::number(connector->getSrcLabelRectangle2()->internalId()) + " Text: " + connSrcLabel2.simplified());
                isKeywordsAssociatedWithTheConnector = true;
            }
            QString connMidLabel1(connector->getMiddleLabelRectangle1()->label());
            if (!connMidLabel1.isEmpty() && connMidLabel1.contains(searchText, Qt::CaseInsensitive))
            {
                resultsList->addItem("Connector Label " + QString::number(connector->getMiddleLabelRectangle1()->internalId()) + " Text: " + connMidLabel1.simplified());
                isKeywordsAssociatedWithTheConnector = true;
            }
            QString connMidLabel2(connector->getMiddleLabelRectangle2()->label());
            if (!connMidLabel2.isEmpty() && connMidLabel2.contains(searchText, Qt::CaseInsensitive))
            {
                resultsList->addItem("Connector Label " + QString::number(connector->getMiddleLabelRectangle2()->internalId()) + " Text: " + connMidLabel2.simplified());
                isKeywordsAssociatedWithTheConnector = true;
            }
            QString connDstLabel1(connector->getDstLabelRectangle1()->label());
            if (!connDstLabel1.isEmpty() && connDstLabel1.contains(searchText, Qt::CaseInsensitive))
            {
                resultsList->addItem("Connector Label " + QString::number(connector->getDstLabelRectangle1()->internalId()) + " Text: " + connDstLabel1.simplified());
                isKeywordsAssociatedWithTheConnector = true;
            }
            QString connDstLabel2(connector->getDstLabelRectangle2()->label());
            if (!connDstLabel2.isEmpty() && connDstLabel2.contains(searchText, Qt::CaseInsensitive))
            {
                resultsList->addItem("Connector Label " + QString::number(connector->getDstLabelRectangle2()->internalId()) + " Text: " + connDstLabel2.simplified());
                isKeywordsAssociatedWithTheConnector = true;
            }

            if (isKeywordsAssociatedWithTheConnector)
            {
                resultsList->addItem("Connector " + QString::number(connector->internalId()) + " " + matchRelationship);
            }
        }
    }
}

bool SearchWidget::searchWithinEndShapes(ShapeObj* shape, QString searchText)
{
    bool isKeywordsAssociatedWithTheConnector = false;

    if (ClassShape *classShape = dynamic_cast<ClassShape*>(shape))
    {
        QString className(classShape->classNameAreaText);
        if (!className.isEmpty() && className.contains(searchText, Qt::CaseInsensitive))
        {
            resultsList->addItem("Class " + QString::number(classShape->internalId()) + " Name: " + className.simplified());
            isKeywordsAssociatedWithTheConnector = true;
        }
        QString classAttributes(classShape->classAttributesAreaText);
        if (!classAttributes.isEmpty() && classAttributes.contains(searchText, Qt::CaseInsensitive))
        {
            resultsList->addItem("Class " + QString::number(classShape->internalId()) + " Attributes: " + classAttributes.simplified());
            isKeywordsAssociatedWithTheConnector = true;
        }
        QString classMethods(classShape->classMethodsAreaText);
        if (!classMethods.isEmpty() && classMethods.contains(searchText, Qt::CaseInsensitive))
        {
            resultsList->addItem("Class " + QString::number(classShape->internalId()) + " Methods: " + classMethods.simplified());
            isKeywordsAssociatedWithTheConnector = true;
        }
    }
    else if (NoteShape *noteShape = dynamic_cast<NoteShape*>(shape))
    {
        QString noteLabel(noteShape->label());
        if (!noteLabel.isEmpty() && noteLabel.contains(searchText, Qt::CaseInsensitive))
        {
            resultsList->addItem("Note " + QString::number(noteShape->internalId()) + " Text: " + noteLabel.simplified());
            isKeywordsAssociatedWithTheConnector = true;
        }
    }
    else if (PackageShape *packageShape = dynamic_cast<PackageShape*>(shape))
    {
        QString packageLabel(packageShape->label());
        if (!packageLabel.isEmpty() && packageLabel.contains(searchText, Qt::CaseInsensitive))
        {
            resultsList->addItem("Package " + QString::number(packageShape->internalId()) + " Label: " + packageLabel);
            isKeywordsAssociatedWithTheConnector = true;
        }
    }
    else
    {
        QString shapeLabel(shape->label());
        if (!shapeLabel.isEmpty() && shapeLabel.contains(searchText, Qt::CaseInsensitive))
        {
            resultsList->addItem("Shape " + QString::number(shape->internalId()) + " Label: " + shapeLabel.simplified());
            isKeywordsAssociatedWithTheConnector = true;
        }
    }

    return isKeywordsAssociatedWithTheConnector;
}

void SearchWidget::locateResult(QListWidgetItem* widgetItem)
{
    m_canvas->deselectAll();
    // Recentre the canvas view, animating the "centre" property.
    QPropertyAnimation *animation =
            new QPropertyAnimation(m_canvasview, "centre");

    animation->setDuration(500);
    //animation->setEasingCurve(QEasingCurve::OutInCirc);
    animation->setStartValue(m_canvasview->centre());

    QStringList splitList = widgetItem->text().split(" ");
    if (splitList[0] == "Shape")
    {
        ShapeObj *shapeObj = dynamic_cast<ShapeObj*>(m_canvas->getItemByInternalId(splitList[1].toInt()));
        if (shapeObj)
        {
            animation->setEndValue(shapeObj->pos());
            shapeObj->setSelected(true);
        }
        else
        {
            qDebug("1111111:::::::::Failed to Cast");
        }
    }
    else if (splitList[0] == "Package")
    {
        PackageShape *packageShape = dynamic_cast<PackageShape*>(m_canvas->getItemByInternalId(splitList[1].toInt()));
        if (packageShape)
        {
            animation->setEndValue(packageShape->pos());
            packageShape->setSelected(true);
        }
        else
        {
            qDebug("2222222:::::::::Failed to Cast");
        }
    }
    else if (splitList[0] == "Note")
    {
        NoteShape *noteShape = dynamic_cast<NoteShape*>(m_canvas->getItemByInternalId(splitList[1].toInt()));
        if (noteShape)
        {
            animation->setEndValue(noteShape->pos());
            noteShape->setSelected(true);
        }
        else
        {
            qDebug("3333333:::::::::Failed to Cast");
        }
    }
    else if (splitList[0] == "Class")
    {
        ClassShape *classShape = dynamic_cast<ClassShape*>(m_canvas->getItemByInternalId(splitList[1].toInt()));
        if (classShape)
        {
            animation->setEndValue(classShape->pos());
            classShape->setSelected(true);
        }
        else
        {
            qDebug("4444444:::::::::Failed to Cast");
        }
    }
    else if (splitList[0] == "Connector")
    {
        if (splitList[1] == "Label")
        {
            ConnectorLabel *connectorLabel = dynamic_cast<ConnectorLabel*>(m_canvas->getConnectorLabelByInternalId(splitList[2].toInt()));
            if (connectorLabel)
            {
                animation->setEndValue(connectorLabel->parentItem()->mapToScene(connectorLabel->pos()));
                connectorLabel->setSelected(true);
            }
            else
            {
                qDebug("5555555:::::::::Failed to Cast");
                std::cout << "Content: " << splitList[2].toStdString() <<std::endl;
            }
        }
        else
        {
            Connector *connector = dynamic_cast<Connector*>(m_canvas->getItemByInternalId(splitList[1].toInt()));
            if (connector)
            {
                animation->setEndValue(connector->mapToScene(connector->painterPath().pointAtPercent(0.5)));
                connector->setSelected(true);
            }
            else
            {
                qDebug("6666666:::::::::Failed to Cast");
            }
        }
    }
    else
    {
        qDebug("Unexpected Type!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        std::cout << "Content: " << splitList[0].toStdString() <<std::endl;
    }
    animation->start();
}

void SearchWidget::changeCanvasView(CanvasView *canvasview)
{
    if (m_canvasview)
    {
        disconnect(m_canvasview, 0, this, 0);
        disconnect(this, 0, m_canvasview, 0);
    }
    m_canvasview = canvasview;
    m_canvas = m_canvasview->canvas();

    searchBox->clear();
    typeCombo->setCurrentIndex(0);
    strengthCombo->setCurrentIndex(0);
    navigabilityCombo->setCurrentIndex(0);
    resultsList->clear();
    //connect(m_canvas, SIGNAL(selectionChanged()), this, SLOT(selection_changed()));
}

}
