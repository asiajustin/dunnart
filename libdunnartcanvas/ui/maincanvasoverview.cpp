/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2012  Monash University
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
 * Author: Michael Wybrow <mjwybrow@users.sourceforge.net>
 *         Yiya Li <asiajustin@gmail.com>
*/

#include <QtCore>
#include <QtWidgets>

#include "maincanvasoverview.h"
#include "ui_maincanvasoverview.h"

#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/cluster.h"

#include "libcola/cola.h"

namespace dunnart {

struct NodeLevelInfo
{
        NodeLevelInfo()
            : isRoot(true),
              sighted(false),
              level(0),
              parentVisits(0)
        {
        }

    std::set<unsigned> children;
    std::set<unsigned> parents;
    bool isRoot;
    bool sighted;
    unsigned level;
    unsigned parentVisits;
};

// This is similar to Kahn's 1962 topological sorting algorithm.
// It is O(|V|+|E|).  It starts from each root, incrementing the levels
// as it moves down the tree.  It waits when it gets to a node with multiple
// parents and waits until it has been visited from every parent, and then
// continues from there with the maximum level.
// It also keeps track of the highestLevel, for later convenience.
static void levelAssignmentTraverse(unsigned ind, unsigned newLevel,
        unsigned& maxLevel, std::vector<NodeLevelInfo>& nodeInfo)
{
    // Set the node level.
    nodeInfo[ind].level = qMax(nodeInfo[ind].level, newLevel);
    maxLevel = qMax(maxLevel, nodeInfo[ind].level);
    nodeInfo[ind].parentVisits++;

    if (nodeInfo[ind].parentVisits < nodeInfo[ind].parents.size())
    {
        // Stop until we've reached this from every parent.
        return;
    }

    // Recurse to set node levels of children.
    for (std::set<unsigned>::iterator it = nodeInfo[ind].children.begin();
            it != nodeInfo[ind].children.end(); ++it)
    {
        levelAssignmentTraverse(*it, nodeInfo[ind].level + 1, maxLevel,
                nodeInfo);
    }
}

class MainCanvasOverviewWidget : public QWidget
{

    public:
        MainCanvasOverviewWidget(QWidget *parent = 0, Qt::WindowFlags f = 0)
            : QWidget(parent, f),
              m_canvasview(NULL),
              k_undefined(-1)
        {
        }
        void setCanavasView(CanvasView *canvasview)
        {
            m_canvasview = canvasview;
            this->update();
        }
    protected:
        void mouseReleaseEvent(QMouseEvent *event)
        {
            // Centre the canvas where the user clicked on the overview.

            // Recentre the canvas view, animating the "centre" property.
            QPropertyAnimation *animation =
                    new QPropertyAnimation(m_canvasview, "centre");

            animation->setDuration(500);
            //animation->setEasingCurve(QEasingCurve::OutInCirc);
            animation->setStartValue(m_canvasview->centre());

            Canvas *canvas = m_canvasview->canvas();
            std::vector<int> highLightList;

            QRectF dummyRect;
            dummyRect.setSize(QSizeF(5, 5));
            dummyRect.moveCenter(event->pos());

            QList<QRectF*> overviewRectList = overviewRectIdLookup.keys();
            for (int i = 0; i < overviewRectList.size(); ++i)
            {
                if (overviewRectList[i]->intersects(dummyRect))
                {
                    highLightList.push_back(overviewRectIdLookup.value(overviewRectList[i]));
                    break;
                }
            }

            double intersectionPercent;

            if (highLightList.empty())
            {
                QLineF dummyLine1(dummyRect.topLeft(), dummyRect.bottomRight());
                QLineF dummyLine2(dummyRect.topRight(), dummyRect.bottomLeft());

                QList<QLineF*> overviewLineList = overviewLineIdLookup.keys();
                for (int i = 0; i < overviewLineList.size(); ++i)
                {
                    QPointF *lineIntersection = new QPointF();
                    if (overviewLineList[i]->intersect(dummyLine1, lineIntersection) == QLineF::BoundedIntersection
                            || overviewLineList[i]->intersect(dummyLine2, lineIntersection) == QLineF::BoundedIntersection)
                    {
                        highLightList.push_back(overviewLineIdLookup.value(overviewLineList[i]));

                        intersectionPercent = sqrt(pow((overviewLineList[i]->x2() - lineIntersection->x()), 2) + pow((overviewLineList[i]->y2() - lineIntersection->y()), 2))
                                / overviewLineList[i]->length();

                        break;
                    }
                    delete lineIntersection;
                    lineIntersection = NULL;
                }
            }

            canvas->deselectAll();
            if (highLightList.size() == 1)
            {
                CanvasItem *itemToBeSelected = canvas->getItemByInternalId(highLightList[0]);
                itemToBeSelected->setSelected(true);
                if (ShapeObj *shape = dynamic_cast<ShapeObj *>(itemToBeSelected))
                {
                    animation->setEndValue(shape->pos());
                    animation->start();
                }
                else if (Connector *conn = dynamic_cast<Connector *>(itemToBeSelected))
                {
                    std::cout << "HERRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR" << std::endl;
                    std:: cout << intersectionPercent << std::endl;
                    animation->setEndValue(conn->mapToScene(conn->painterPath().pointAtPercent(intersectionPercent)));
                    animation->start();
                }
                else
                {
                    std::cout << "22222222222222222222222222222222222222222" << std::endl;
                }
            }
            else if (highLightList.size() > 0)
            {
                std::cout << "Multiple Canvas Items Selected in Overview!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
            }
        }

        void paintEvent(QPaintEvent *event)
        {
            std::cout << "Main Canvas Overview Paint Event!" << std::endl;

            /*for (int i = 0; i < rs.size(); ++i)
            {
                delete rs[i];
                rs[i] = NULL;
            }*/
            if (lineConnLookup.size() > 0)
            {
                QList<QLineF*> lineConnLookupKeys = lineConnLookup.keys();
                qDeleteAll(lineConnLookupKeys.begin(), lineConnLookupKeys.end());
            }
            if (overviewRectIdLookup.size() > 0)
            {
                QList<QRectF*> overviewRectIdLookupKeys = overviewRectIdLookup.keys();
                qDeleteAll(overviewRectIdLookupKeys.begin(), overviewRectIdLookupKeys.end());
            }
            if (overviewLineIdLookup.size() > 0)
            {
                QList<QLineF*> overviewLineIdLookupKeys = overviewLineIdLookup.keys();
                qDeleteAll(overviewLineIdLookupKeys.begin(), overviewLineIdLookupKeys.end());
            }

            shapeIndexLookup.clear();
            rect_vec.clear();
            shape_vec.clear();
            conn_vec.clear();
            lineConnLookup.clear();
            rs.clear();
            edges.clear();
            ccs.clear();
            edgeLengths.clear();
            overviewRectIdLookup.clear();
            overviewLineIdLookup.clear();

            Q_UNUSED (event)
            Canvas *canvas = NULL;

            // XXX Maybe we should cache this and recompute separately for
            //     transform changes, shape movement and viewport changes?

            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);

            // Fill the overview with the canvas background colour.
            QColor backgroundColour = Qt::white;
            if (m_canvasview)
            {
                canvas = m_canvasview->canvas();
                backgroundColour = canvas->optCanvasBackgroundColour();
            }
            painter.setPen(backgroundColour);
            painter.setBrush(backgroundColour);
            QRectF drawingRect = QRectF(0, 0, width(), height());
            painter.drawRect(drawingRect);

            if (m_canvasview == NULL)
            {
                return;
            }

            // Draw edges in overview for each connector on the canvas.
            QList<CanvasItem *> items = canvas->items();
            QRectF shapeRect;

            // Get normal shapes from canvas
            /*for (int i = 0; i < items.count(); ++i)
            {
                ShapeObj *shape = dynamic_cast<ShapeObj *> (items.at(i));
                Cluster *cluster = dynamic_cast<Cluster *> (items.at(i));
                if (shape && !cluster)
                {
                    // Clusters are also shapes.
                    shapeRect.setSize(shape->size());
                    shapeRect.moveCenter(shape->centrePos());
                    shapeIndexLookup.insert(shape, rs.size());
                    shape_vec.push_back(shape);

                    rect_vec.push_back(shapeRect);

                    vpsc::Rectangle *vpscShape = new vpsc::Rectangle(shapeRect.left(), shapeRect.right(),
                            shapeRect.top(), shapeRect.bottom(), false);
                    vpscShape->moveCentre(shapeRect.x(), shapeRect.y());
                    rs.push_back(vpscShape);
                }
            }*/

            // Get connectors from canvas
            for (int i = 0; i < items.count(); ++i)
            {
                Connector *connector = dynamic_cast<Connector *> (items.at(i));
                if (connector && connector->getDirected() == Connector::either && connector->obeysDirectedEdgeConstraints() &&
                        connector->arrowHeadHeadType() == Connector::triangle_outline)
                {
                    QPair<CPoint, CPoint> connpts = connector->get_connpts();
                    ShapeObj *firstShape = connpts.first.shape;
                    ShapeObj *secondShape = connpts.second.shape;
                    if(!firstShape || !secondShape) {
                        qWarning("dangling connector!");
                        continue;
                    }

                    if (shapeIndexLookup.value(firstShape, -1) == -1)
                    {
                        shapeRect.setSize(firstShape->size());
                        shapeRect.moveCenter(firstShape->centrePos());
                        shapeIndexLookup.insert(firstShape, rs.size());
                        shape_vec.push_back(firstShape);

                        rect_vec.push_back(shapeRect);

                        vpsc::Rectangle *vpscShape = new vpsc::Rectangle(shapeRect.left(), shapeRect.right(),
                                shapeRect.top(), shapeRect.bottom(), false);
                        vpscShape->moveCentre(shapeRect.x(), shapeRect.y());
                        rs.push_back(vpscShape);
                    }

                    if (shapeIndexLookup.value(secondShape, -1) == -1)
                    {
                        shapeRect.setSize(secondShape->size());
                        shapeRect.moveCenter(secondShape->centrePos());
                        shapeIndexLookup.insert(secondShape, rs.size());
                        shape_vec.push_back(secondShape);

                        rect_vec.push_back(shapeRect);

                        vpsc::Rectangle *vpscShape = new vpsc::Rectangle(shapeRect.left(), shapeRect.right(),
                                shapeRect.top(), shapeRect.bottom(), false);
                        vpscShape->moveCentre(shapeRect.x(), shapeRect.y());
                        rs.push_back(vpscShape);
                    }

                    conn_vec.push_back(connector);
                    QLineF line(firstShape->centrePos(), secondShape->centrePos());

                    edges.push_back(std::make_pair(shapeIndexLookup.value(secondShape), shapeIndexLookup.value(firstShape)));
                    edgeLengths.push_back(line.length());
                    lineConnLookup.insert(new QLineF(line), connector);
                }
            }

            Canvas::LayeredAlignment layeredAlignment = Canvas::ShapeMiddle;

            // Set up downward edge constraints.
            // In the case that the graph isn't a DAG, i.e., has cycles, we
            // don't constrain any of the edges that form a cycle.  We do
            // this by computing the strongly connected components for the
            // Graph and only add a downward edge constraint between two
            // nodes (connector endpoints) if they are not part of the same
            // strongly connected component.
            QVector<int> sccIndexes = stronglyConnectedComponentIndexes();

            std::vector<NodeLevelInfo> nodeInfo(shape_vec.size());

            for (uint i = 0; i < conn_vec.size(); ++i)
            {
                if (!(conn_vec[i]->getDirected() == Connector::either && conn_vec[i]->obeysDirectedEdgeConstraints()))
                {
                    // Don't constrain undirected edges, or those
                    // ignoring downward edge constraints.
                    conn_vec[i]->setHasDownwardConstraint(false);
                    continue;
                }

                cola::Edge edge = edges[i];
                if (sccIndexes[edge.first] != sccIndexes[edge.second])
                {
                    unsigned firstIndex = edge.first;
                    unsigned secondIndex = edge.second;

                    // Save the hierarchy information so we can align the
                    // layers together.
                    nodeInfo[firstIndex].children.insert(secondIndex);
                    nodeInfo[secondIndex].parents.insert(firstIndex);
                    nodeInfo[firstIndex].sighted = true;
                    nodeInfo[secondIndex].sighted = true;
                    nodeInfo[secondIndex].isRoot = false;

                    conn_vec[i]->setHasDownwardConstraint(true);
                }
                else
                {
                    conn_vec[i]->setHasDownwardConstraint(false);
                }
            }

            // Apply level constraints.
            // Assign levels for each node.
            unsigned maxLevel = 0;
            for (size_t ind = 0; ind < shape_vec.size(); ++ind)
            {
                if (nodeInfo[ind].isRoot && nodeInfo[ind].sighted)
                {
                    levelAssignmentTraverse(ind, nodeInfo[ind].level + 1,
                            maxLevel, nodeInfo);
                }
            }

            // Create list of nodes in each level and remember the size
            // of the largest
            std::vector<std::set<unsigned> > levelLists(maxLevel);
            std::vector<double> levelShapeLength(maxLevel, 0.0);
            for (size_t ind = 0; ind < shape_vec.size(); ++ind)
            {
                // Clear the layer channel padding information.
                /*shape_vec[ind]->setProperty("layeredStartChannelPadding",
                        QVariant());
                shape_vec[ind]->setProperty("layeredEndChannelPadding",
                        QVariant());
                shape_vec[ind]->setProperty("layeredEndConnectorGroup",
                        QVariant());*/
                // Store level information.
                if (nodeInfo[ind].level > 0)
                {
                    size_t level = nodeInfo[ind].level - 1;
                    levelLists[level].insert(ind);
                    levelShapeLength[level] = qMax(
                            levelShapeLength[level],
                            rs[ind]->length(vpsc::YDIM));
                }
            }

            // Align each level.
            cola::AlignmentConstraint *prevLevelAlignment = NULL;
            for (size_t level = 0; level < maxLevel; ++level)
            {
                // Set of nodes in this level.
                std::set<unsigned>& nodes = levelLists[level];

                int prevSources = 0;
                int currSources = 0;
                if (level > 0)
                {
                    // Find the number of connector sources on each side
                    // of the channel between levels.  The nodes if the
                    // previous level are in "prevNodes" and the nodes of
                    // this level are in "nodes".
                    for (std::set<unsigned>::const_iterator it = nodes.begin();
                            it != nodes.end(); ++it)
                    {
                        if (nodeInfo[*it].parents.size() > 0)
                        {
                            currSources++;
                        }
                    }
                    std::set<unsigned>& prevNodes = levelLists[level - 1];
                    for (std::set<unsigned>::const_iterator it = prevNodes.begin();
                            it != prevNodes.end(); ++it)
                    {
                        if (nodeInfo[*it].children.size() > 0)
                        {
                            prevSources++;
                        }
                    }

                    // Based on the numer of sources on the smaller side,
                    // give an ordering to the nodes where these
                    // connectors terminate.  For example in a directed
                    // tree flowing to th left, three nodes on one level
                    // might point to seven nodes, so each of the seven
                    // nodes would be given a group based on the three
                    // sources [0-2].  This is not the best solution, but
                    // it is fairly simple and easy to compute.
                    if (prevSources < currSources)
                    {
                        // Connectors point to nodes in "nodes".
                        int number = 0;
                        for (std::set<unsigned>::const_iterator it = prevNodes.begin();
                                it != prevNodes.end(); ++it)
                        {
                            if (nodeInfo[*it].children.size() > 0)
                            {
                                /*shape_vec[*it]->setProperty("layeredEndConnectorGroup",
                                        QVariant(number++));*/
                            }
                        }
                    }
                    else
                    {
                        // Connectors point to nodes in "prevNodes".
                        int number = 0;
                        for (std::set<unsigned>::const_iterator it = nodes.begin();
                                it != nodes.end(); ++it)
                        {
                            for (std::set<unsigned>::const_iterator ch =
                                    nodeInfo[*it].parents.begin();
                                    ch != nodeInfo[*it].parents.end(); ++ch)
                            {
                                /*shape_vec[*ch]->setProperty("layeredEndConnectorGroup",
                                        QVariant(number));*/
                            }
                            if (nodeInfo[*it].parents.size() > 0)
                            {
                                number++;
                            }
                        }
                    }
                }

                // Set layer channel padding information for each shape on
                // this level, and add it to the alignment relationship.
                // The channel padding information is used for centring
                // all connectors at the same point between levels.
                cola::AlignmentConstraint *alignment =
                        new cola::AlignmentConstraint(vpsc::YDIM);
                for (std::set<unsigned>::const_iterator it = nodes.begin();
                        it != nodes.end(); ++it)
                {
                    double offset = 0;
                    double modifier = -1;
                    double distToEdge = rs[*it]->length(vpsc::YDIM) / 2.0;
                    double startPadding = 0.0;
                    double endPadding = 0.0;
                    if (layeredAlignment == Canvas::ShapeStart)
                    {
                        offset = modifier * distToEdge;
                        startPadding = distToEdge;
                        endPadding = levelShapeLength[level] - distToEdge;
                    }
                    else if (layeredAlignment == Canvas::ShapeEnd)
                    {
                        offset = modifier * -distToEdge;
                        startPadding = levelShapeLength[level] - distToEdge;
                        endPadding = distToEdge;
                    }
                    else
                    {
                        startPadding = levelShapeLength[level] / 2.0;
                        endPadding = levelShapeLength[level] / 2.0;
                    }
                    /*shape_vec[*it]->setProperty("layeredStartChannelPadding",
                            QVariant(startPadding));
                    shape_vec[*it]->setProperty("layeredEndChannelPadding",
                            QVariant(endPadding));*/

                    alignment->addShape(*it, offset);
                }
                ccs.push_back(alignment);

                if (prevLevelAlignment)
                {

                    // Determine padding between this level and previous.
                    double padding = 0;
                    if (layeredAlignment == Canvas::ShapeMiddle)
                    {
                        padding = (levelShapeLength[level] / 2.0) +
                                (levelShapeLength[level - 1] / 2.0);
                    }
                    else if (layeredAlignment == Canvas::ShapeStart)
                    {
                        padding = levelShapeLength[level];
                    }
                    else if (layeredAlignment == Canvas::ShapeEnd)
                    {
                        padding = levelShapeLength[level - 1];
                    }

                    // Create a separation constraint between this level's
                    // alignment and the one for the previous level.
                    ccs.push_back(new cola::SeparationConstraint(vpsc::YDIM, prevLevelAlignment, alignment, padding + (100 * 1.0 * 0.5)));
                }
                prevLevelAlignment = alignment;
            }

            /*
            // Code for aligning just children of each parent node.
            for (size_t ind = 0; ind < childLists.size(); ++ind)
            {
                std::set<unsigned>& children = childLists[ind];
                if (children.empty())
                {
                    continue;
                }

                cola::AlignmentConstraint *alignment =
                        new cola::AlignmentConstraint(vpsc::YDIM);
                for (std::set<unsigned>::iterator it = children.begin();
                        it != children.end(); ++it)
                {
                    if (parentCount[*it] == 1)
                    {
                        alignment->addShape(*it, 0);
                    }
                }
                ccs.push_back(alignment);
            }
            */

            // start layout
            std::vector<cola::Edge> emptyEdges;
            cola::ConstrainedFDLayout cfdl(rs, emptyEdges, 0, true);
            cfdl.setConstraints(ccs);
            cfdl.makeFeasible();
            cfdl.run();

            QRectF laidOutBoundingRect;

            for (int i = 0; i < rect_vec.size(); ++i)
            {
                rect_vec[i].moveCenter(QPointF(rs[i]->getCentreX(), rs[i]->getCentreY()));
                laidOutBoundingRect = laidOutBoundingRect.united(rect_vec[i]);
            }
            // Compute the diagram bounds.
            double buffer = 50;
            QRectF diagramBounds = expandRect(laidOutBoundingRect, buffer);

            // Compute the scale to with the drawing into the overview rect.
            qreal xscale = drawingRect.width() / diagramBounds.width();
            qreal yscale = drawingRect.height() /  diagramBounds.height();

            // Choose the smallest of the two scale values.
            qreal scale = qMin(xscale, yscale);

            // Scale uniformly, and transform to center in the overview.
            QTransform scaleTransform = QTransform::fromScale(scale, scale);
            QRectF targetRect = scaleTransform.mapRect(diagramBounds);
            QPointF diff = drawingRect.center() - targetRect.center();
            m_transform = QTransform();
            m_transform.translate(diff.x(), diff.y());
            m_transform.scale(scale, scale);

            // Draw page outline on the canvas.
            /*
            painter.setPen(Qt::darkGray);
            painter.setBrush(Qt::white);
            painter.drawRect(m_transform.mapRect(canvas->pageRect()));*/

            QList<QLineF*> lineList = lineConnLookup.keys();
            for (int i = 0; i < lineList.size(); ++i)
            {
                Connector* conn = lineConnLookup.value(lineList[i]);
                QPair<CPoint, CPoint> connpts = conn->get_connpts();

                unsigned firstIndex = shapeIndexLookup.value(connpts.second.shape);
                unsigned secondIndex = shapeIndexLookup.value(connpts.first.shape);

                lineList[i]->setP1(m_transform.map(QPointF(rs[firstIndex]->getCentreX(), rs[firstIndex]->getCentreY())));
                lineList[i]->setP2(m_transform.map(QPointF(rs[secondIndex]->getCentreX(), rs[secondIndex]->getCentreY())));

                overviewLineIdLookup.insert(lineList[i], conn->internalId());

                QPen pen(QColor(0, 0, 0, 100));
                pen.setCosmetic(true);
                if (conn->dashedStroke())
                {
                    QVector<qreal> dashes;
                    dashes << 5 << 3;
                    pen.setDashPattern(dashes);
                }
                painter.setPen(pen);
                painter.drawLine(QLineF(lineList[i]->p1(), lineList[i]->p2()));

                // Ignore arrow head drawing, the default drawing is hierarchical

                /*connector-->lineConnLookup[i]
                // Add the Arrowhead.
                if (connector->getDirected() != Connector::neither)
                {
                    // There is an arrowhead.
                    if (connector->arrowHeadOutline())
                    {
                        // Use white fill.
                        painter.setBrush(QBrush(Qt::white));
                    }
                    else
                    {
                        painter.setBrush(QBrush(QColor(0, 0, 0, 100)));
                    }

                    painter.drawPath(m_transform.map(connector->mapToScene(connector->getArrowHeadPath())));

                    if (connector->getDirected() == Connector::both)
                    {
                        if (connector->arrowTailOutline())
                        {
                            // Use white fill.
                            painter.setBrush(QBrush(Qt::white));
                        }
                        else
                        {
                            painter.setBrush(QBrush(QColor(0, 0, 0, 100)));
                        }
                        painter.drawPath(m_transform.map(connector->mapToScene(connector->getArrowTailPath())));
                    }
                }*/
            }

            // Ignore clusters for hierarchical drawing.

            /*painter.setPen(Qt::darkGray);
            painter.setBrush(Qt::lightGray);
            for (int i = 0; i < items.count(); ++i)
            {
                Cluster *cluster = dynamic_cast<Cluster *> (items.at(i));
                if (cluster)
                {
                    shapeRect.setSize(cluster->size());
                    shapeRect.moveCenter(cluster->centrePos());
                    QRectF transformedShapeRect(m_transform.mapRect(shapeRect));
                    painter.drawRect(transformedShapeRect);

                    //overviewRectIdLookup.insert(new QRectF(transformedShapeRect), cluster->internalId());
                }
            }*/


            // Draw Rectangles in overview for each shape.
            painter.setPen(Qt::black);
            painter.setBrush(Qt::darkGray);
            for (int i = 0; i < rect_vec.size(); ++i)
            {
                rect_vec[i] = m_transform.mapRect(rect_vec[i]);
                overviewRectIdLookup.insert(new QRectF(rect_vec[i]), shape_vec[i]->internalId());
                painter.drawRect(rect_vec[i]);
            }

            cfdl.freeAssociatedObjects();

            // Show where the visible viewport is (by making everything
            // outside this have a light grey overlay).
            /*
            QColor grey(0, 0, 0, 60);
            painter.setPen(QPen(Qt::transparent));
            painter.setBrush(QBrush(grey));
            QRectF viewRect = m_transform.mapRect(m_canvasview->viewportRect());
            QPolygon polygon = QPolygon(drawingRect.toRect()).subtracted(
                    QPolygon(viewRect.toRect()));
            painter.drawPolygon(polygon);*/

        }
    private:
        QVector<int> stronglyConnectedComponentIndexes()
        {
            m_index = 0;
            m_scc_index = 0;
            int shapesCount = shape_vec.size();

            m_indexes = QVector<int>(shapesCount, k_undefined);
            m_scc_indexes = QVector<int>(shapesCount, 0);
            m_lowlinks = QVector<int>(shapesCount, 0);
            m_in_stack = QVector<bool>(shapesCount, false);
            m_stack.clear();

            for (int i = 0; i < shapesCount; ++i)
            {
                if (m_indexes[i] == k_undefined)
                {
                    strongConnect(i);
                }
            }
            return m_scc_indexes;
        }


        void strongConnect(uint v)
        {
            // Set the depth index for v to the smallest unused index
            m_indexes[v] = m_index;
            m_lowlinks[v] = m_index;
            m_index++;
            m_stack.push(v);
            m_in_stack[v] = true;


            // Consider successors of v
            for (uint edgeInd = 0; edgeInd < conn_vec.size(); ++edgeInd)
            {
                // for each (v, w) in E
                cola::Edge edge = edges[edgeInd];
                if ( (edge.first == v) && conn_vec[edgeInd]->getDirected() == Connector::either &&
                        conn_vec[edgeInd]->obeysDirectedEdgeConstraints() )
                {
                    int w = edge.second;
                    if (m_indexes[w] == k_undefined)
                    {
                        // Successor w has not yet been visited; recurse on it
                        strongConnect(w);
                        m_lowlinks[v] = qMin(m_lowlinks[v], m_lowlinks[w]);
                    }
                    else if (m_in_stack[w])
                    {
                        // Successor w is in stack S and hence in the current SCC
                        m_lowlinks[v] = qMin(m_lowlinks[v], m_indexes[w]);
                    }
                }
            }

            // If v is a root node, pop the stack and generate an SCC
            if (m_lowlinks[v] == m_indexes[v])
            {
                // start a new strongly connected component
                m_scc_index++;
                uint w;
                do
                {
                    w = m_stack.pop();
                    m_in_stack[w] = false;
                    // add w to current strongly connected component
                    m_scc_indexes[w] = m_scc_index;
                }
                while (w != v);
            }
        }


        CanvasView *m_canvasview;
        QTransform m_transform;
        QMap<QRectF*, int> overviewRectIdLookup;
        QMap<QLineF*, int> overviewLineIdLookup;
        QMap<ShapeObj*, int> shapeIndexLookup;
        QMap<QLineF*, Connector*> lineConnLookup;
        std::vector<ShapeObj*> shape_vec;
        std::vector<Connector*> conn_vec;
        std::vector<QRectF> rect_vec;
        std::vector<vpsc::Rectangle*> rs;
        std::vector<cola::Edge> edges;
        cola::CompoundConstraints ccs;
        std::vector<double> edgeLengths;

        const int k_undefined;
        int m_index;
        int m_scc_index;
        QVector<int> m_indexes;
        QVector<int> m_scc_indexes;
        QVector<int> m_lowlinks;
        QVector<bool> m_in_stack;
        QStack<int> m_stack;
};

void MainCanvasOverviewDialog::canvasViewportChanged(QRectF viewRect)
{
    Q_UNUSED (viewRect)

    update();
}

void MainCanvasOverviewDialog::canvasSceneChanged(QList<QRectF> rects)
{
    Q_UNUSED (rects)

    update();
}

MainCanvasOverviewDialog::MainCanvasOverviewDialog(CanvasView *view, QWidget *parent)
    : QDockWidget(parent),
      ui(new Ui::MainCanvasOverview),
      m_canvasview(view)
{
    ui->setupUi(this);

    m_canvasoverview = new MainCanvasOverviewWidget();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_canvasoverview);

    ui->frame->setLayout(layout);

    changeCanvasView(view);
}

MainCanvasOverviewDialog::~MainCanvasOverviewDialog()
{
    delete ui;
}

void MainCanvasOverviewDialog::changeCanvasView(CanvasView *canvasview)
{
    if (m_canvasview)
    {
        disconnect(m_canvasview, 0, this, 0);
        disconnect(this, 0, m_canvasview, 0);
    }
    m_canvasview = canvasview;
    m_canvasoverview->setCanavasView(m_canvasview);

    // Trigger redraw when the viewport is scrolled or changes size.
    connect(m_canvasview, SIGNAL(viewportChanged(QRectF)),
            this, SLOT(canvasViewportChanged(QRectF)));

    // Trigger redraw when the diagram layout changes.
    connect(m_canvasview->canvas(), SIGNAL(changed(QList<QRectF>)),
            this, SLOT(canvasSceneChanged(QList<QRectF>)));
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
