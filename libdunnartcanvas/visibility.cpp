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
*/

#include <cstdlib>
#include <cassert>
#include <map>
#include <utility>

#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/interferencegraph.h"
#include "libavoid/vertices.h"
#include "libavoid/viscluster.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/connector.h"

#include "libavoid/router.h"

namespace dunnart {


using namespace Avoid;
using namespace vpsc;

int afterPostProcessing = 3;

//===========================================================================
//  Visibility Graph code:


// ConnPairSet is defined in interferencegraph.h
typedef std::map<unsigned int, int> TallyMap;

// -===============

static int calcConnectorIntersections(Canvas *canvas, PtOrderMap *ptOrders, 
        ConnPairSet *touchingConns, TallyMap *tallyMap, 
        ConnPairSet *crossingConnectors, Connector *queryConn,
        PointSet *crossingPoints = NULL);


int noOfConnectorCrossings(Connector *conn)
{
    PointSet crossingPoints;
    return calcConnectorIntersections(conn->canvas(), NULL, NULL, NULL, 
            NULL, conn, &crossingPoints);
}


void nudgeConnectors(Canvas *canvas, const double nudgeDist, 
        const bool displayUpdate)
{
    PtOrderMap ptOrds;
    calcConnectorIntersections(canvas, &ptOrds, NULL, NULL, NULL, NULL);

    // Do the actual nudging.
    for (int dim = 0; dim < 2; ++dim)
    {
        PtOrderMap::iterator finish = ptOrds.end();
        for (PtOrderMap::iterator it = ptOrds.begin(); it != finish; ++it)
        {
            //const VertID& ptID = it->first;
            PtOrder& order = it->second;
            PointRepVector pointsOrder = order.sortedPoints(dim);

            printf("Nudging[%d] ", dim);
            int count = 0;
            for (PointRepVector::iterator curr = pointsOrder.begin();
                    curr != pointsOrder.end(); ++curr)
            {
                if (curr->first == NULL)
                {
                    continue;
                }
                Point *connPt = curr->first;

                if (count == 0)
                {
                    printf("%g, %g : ", connPt->x, connPt->y);
                }
                const VertID id(connPt->id, connPt->vn,
                        VertID::PROP_ConnPoint);
                id.print(); printf(" ");
               
                if (count > 0)
                {
                    // If not the first point.
                    printf("%lX ", (long) connPt);
                    printf("%g, %g : ", connPt->x, connPt->y);
                    if (dim == 0)
                    {
                        connPt->x += nudgeDist * (count);
                    }
                    else
                    {
                        connPt->y += nudgeDist * (count);
                    }
                }
                count++;
            }
            printf("\n");
        }
    }

    if (displayUpdate)
    {
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            Connector *conn =
                    dynamic_cast<Connector *> (canvas_items.at(i));
            if (conn)
            {
                conn->applyNewRoute(conn->avoidRef->displayRoute());
                conn->update();
            }
        }
    }
}


#if 0
// ADS used in colorInteferingConnectors() for debug
static void conn_set_red(Conn *conn)
{
    conn->colour = QColor(255,0,0);
    conn->update();
}

static void conn_set_green(Conn *conn)
{
    conn->colour = QColor(0,255,0);
    conn->update();
}
#endif


#if ADS_DEBUG
// ADS used in calcConnectorIntersections for debug
static void identifyPointWithCircle(double xc, double yc, int r, int g, int b)
{
    int cox = canvas->get_offabsxpos(), coy = canvas->get_offabsypos();
    int cx = canvas->get_xpos(), cy = canvas->get_ypos();
    SDL_Rect crect = { cx + 2, cy + 2,
                       canvas->get_width() - 4, canvas->get_height() - 4 };
    SDL_SetClipRect(screen, &crect);
    Avoid::Point p(xc+cox,yc+coy); // does same as unoffsetPoint() (canvas.cpp)
    filledCircleRGBA(screen,(int)p.x,(int)p.y,
                     4,r,g,b,80);
    SDL_Flip(screen);
    SDL_Rect wholeCanvasRect;    
    wholeCanvasRect.x = canvas->get_xpos();
    wholeCanvasRect.y = canvas->get_ypos();
    wholeCanvasRect.w = canvas->get_width();
    wholeCanvasRect.h = canvas->get_height();

    canvasRect = wholeCanvasRect;
    SDL_SetClipRect(screen, NULL);
}
#endif /* ADS_DEBUG */


//
// reset colors of connectors to the saved value that is stored when 
// color is set by set_value for interfering connectors. This is so
// that connectors that were colored because they inteferered, and have
// been re-routed so that they no longer interfere have their colours 
// restored to what they were before changed by colorInterferingConnectors().
//
static void resetConnectorColors(Canvas *canvas)
{
    QList<CanvasItem *> canvas_items = canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (!conn)
        {
            continue;
        }
        conn->restoreColour();
    }
}

//
// Color connectors that are intersecting or shared path ('nudgable').
// We use graph coloring algorithm (Welsh-Powell) to color all
// intersecting/shared path connectors different colors.
//
void colourInterferingConnectors(Canvas *canvas)
{
    ConnPairSet touchingConns;
    ConnPairSet crossingConns;

    calcConnectorIntersections(canvas, NULL, &touchingConns, NULL, 
            &crossingConns, NULL);
    ConnPairSet::iterator pathFinish = touchingConns.end();

#if 0
    // ADS begin old debug line coloring code
    printf("colorInterferingConnectors start\n");
    for (ConnPairSet::iterator it = touchingConns.begin(); 
            it != pathFinish; ++it)
    {
        Conn *conn1 = it->first;
        Conn *conn2 = it->second;
        printf("colorInterferingConnectors shared path %d,%d\n",
               conn1->get_ID(),
               conn2->get_ID());
        conn_set_red(conn1); conn_set_red(conn2);
    }
    for (ConnPairSet::iterator iter = crossingConns.begin();
         iter != crossingConns.end(); ++iter)
    {
        Conn *conn1 = iter->first;
        Conn *conn2 = iter->second;
        printf("colorInterferingConnectors crossing connectors %d,%d\n",
               conn1->get_ID(),
               conn2->get_ID());
        conn_set_green(conn1); conn_set_green(conn2);
    }
    printf("colorInterfering Connectors end\n");
    // ADS end old debug line coloring code
#endif

    // build inteference graph for crossings and shared paths
    ConnPairSet interferingConns = crossingConns; // start with crossings
    // then add shared paths
    for (ConnPairSet::iterator it = touchingConns.begin(); 
            it != pathFinish; ++it)
    {
        interferingConns.insert(*it);
    }
    interference_graph::InterferenceGraph *intgraph = NULL;
    intgraph = new interference_graph::InterferenceGraph(interferingConns);
    // color the graph so adjacent nodes have different colors
    int num_colors = intgraph->color_graph();

    std::cout << "colourInterferingConnectors: required " << num_colors << 
        " colours." << std::endl;

    // reset colors to values they had before any previous call here
    resetConnectorColors(canvas);

    // and now set connector colors according to the graph coloring
    // by using the color number as index into list of suitable line colors.
    const QList<QColor> connectorColours =
            canvas->interferingConnectorColours();
    if (num_colors > connectorColours.size())
    {
        std::cerr << "Too many colors required\n";
        // ADS TODO: need to decide what to do about this; just give up,
        // or generate colors rather than use fixed list?
    }
    else
    {
        for (interference_graph::NodePtrList::iterator iter = 
                 intgraph->nodes->begin();
             iter != intgraph->nodes->end(); ++iter)
        {
            Connector *conn = (*iter)->conn;
            conn->overrideColour(connectorColours[(*iter)->colornum-1]);
        }
    }
    delete intgraph;
}

static int calcConnectorIntersections(Canvas *canvas, PtOrderMap *ptOrders,
        ConnPairSet *touchingConns, TallyMap *tallyMap,
        ConnPairSet *crossingConns, Connector *queryConn,
        PointSet *crossingPoints)
{
    Q_UNUSED (ptOrders)

    int crossingsN = 0;

    // Do segment splitting.
    QList<CanvasItem *> canvas_items = canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Point lastInt(INFINITY, INFINITY);
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (!conn)
        {
            continue;
        }
        
        for (int j = (i + 1); j < canvas_items.size(); ++j)
        {
            Point lastInt2(INFINITY, INFINITY);
            Connector *conn2 = dynamic_cast<Connector *> (canvas_items.at(j));
            if (!conn2)
            {
                continue;
            }
            
            if (queryConn && (queryConn != conn) && (queryConn != conn2))
            {
                // Querying, and neither of these are the query connector.
                continue;
            }

            if (conn->internalId() == conn2->internalId())
            {
                continue;
            }
            
            Avoid::Polygon& route = conn->avoidRef->displayRoute();
            Avoid::Polygon& route2 = conn2->avoidRef->displayRoute();
            splitBranchingSegments(route2, true, route);
        }
    }

    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Point lastInt(INFINITY, INFINITY);
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (!conn)
        {
            continue;
        }
        
        for (int j = (i + 1); j < canvas_items.size(); ++j)
        {
            Point lastInt2(INFINITY, INFINITY);
            Connector *conn2 = dynamic_cast<Connector *> (canvas_items.at(j));
            if (!conn2)
            {
                continue;
            }
            
            if (queryConn && (queryConn != conn) && (queryConn != conn2))
            {
                // Querying, and neither of these are the query connector.
                continue;
            }

            if (conn->internalId() == conn2->internalId())
            {
                continue;
            }
            
            Avoid::Polygon& route = conn->avoidRef->displayRoute();
            Avoid::Polygon& route2 = conn2->avoidRef->displayRoute();
            //bool checkForBranchingSegments = false;
            int crossings = 0;
            bool touches = false;
            ConnectorCrossings cross(route2, true, route);
            cross.crossingPoints = crossingPoints;
            for (size_t i = 1; i < route.size(); ++i)
            {
                const bool finalSegment = ((i + 1) == route.size());
                cross.countForSegment(i, finalSegment);

                crossings += cross.crossingCount;
                touches |= (cross.crossingFlags & CROSSING_TOUCHES);
            }
            if (touchingConns && touches)
            {
                // Add to the list of touching connectors.
                touchingConns->insert(std::make_pair(conn, conn2));
            }
            assert(crossings <= 2);
            if (crossings > 0)
            {

                if (tallyMap)
                {
                    (*tallyMap)[conn->internalId()]++;
                    (*tallyMap)[conn2->internalId()]++;
                }
                if (crossingConns)
                {
                    crossingConns->insert(std::make_pair(conn, conn2));
                }
                crossingsN += crossings;
            }
        }
    }
#if ADS_DEBUG
    if (crossingPoints)
    {
        for (PointSet::const_iterator i = crossingPoints->begin();
                i != crossingPoints->end(); ++i)
        {
            identifyPointWithCircle(i->x, i->y, 255, 0, 0);
        }
    }
#endif

    return crossingsN;
}


void reroute_all_connectors(Canvas *canvas)
{
    Avoid::Router *router = canvas->router();
    bool lastSimpleRouting = router->SimpleRouting;
    router->SimpleRouting = false;
    router->ClusteredRouting = canvas->avoidClusterCrossings();
    bool force = true;
    reroute_connectors(canvas, force);
    router->SimpleRouting = lastSimpleRouting;
    canvas->interrupt_graph_layout();
}


void redraw_connectors(Canvas *canvas)
{
    QList<CanvasItem *> canvas_items = canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (conn)
        {
            conn->reapplyRoute();
        }
    }
}


void reroute_connectors(Canvas *canvas, const bool force,
        const bool postProcessing)
{
    Avoid::Router *router = canvas->router();
    qDebug("%d reroute_connectors(%d, %d)\n", (int) time(NULL),
           (int) force, (int) postProcessing);
    if (router->SimpleRouting)
    {
        router->processTransaction();
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
            if (conn)
            {
                conn->forceReroute();
                conn->updateIdealPosition();
            }
        }
        return;
    }

    if (force)
    {
        //printf("+++++ Making all libavoid paths invalid\n");
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));

            if (conn)
            {
                conn->avoidRef->makePathInvalid();
                conn->forceReroute();
            }
        }
    }
    bool changes = router->processTransaction();
    //router->outputInstanceToSVG("libavoid-debug-new");
    // Update connectors.
    if (changes)
    {
        int rconns = 0;
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            Connector *conn =
                    dynamic_cast<Connector *> (canvas_items.at(i));
            if (conn &&  (!(router->SelectiveReroute) ||
                     conn->avoidRef->needsRepaint() || force))
            {
                conn->updateFromLibavoid();
                rconns++;
            }
        }
        //printf("-- %3d conns rerouted! --\n", rconns);
        //fflush(stdout);
    }
    // -----------------
   
    if (postProcessing && canvas->avoidConnectorCrossings())
    {
        //SDL_Surface *sur = canvas->get_image(canvas->get_active_image_n());
        
        std::map<unsigned int, int> tallies;
        printf("Avoiding connector crossings...\n");

        calcConnectorIntersections(canvas, NULL, NULL, &tallies, NULL, NULL);
        
        TallyMap::iterator curr, finish = tallies.end();
        while (!tallies.empty())
        {
            unsigned int maxID = 0;
            int maxVal = 0;
            TallyMap::iterator maxIt;

            for (curr = tallies.begin(); curr != finish; ++curr)
            {
                if ((*curr).second > maxVal)
                {
                    maxVal = (*curr).second;
                    maxID  = (*curr).first;
                    maxIt  = curr;
                }
            }
            tallies.erase(maxIt);
            QList<CanvasItem *> canvas_items = canvas->items();
            for (int i = 0; i < canvas_items.size(); ++i)
            {
                Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
                if (!conn || (maxID != conn->internalId()))
                {
                    continue;
                }
                conn->rerouteAvoidingIntersections();
            }
        }
    }

    if (postProcessing && canvas->optColourInterferingConnectors())
    {
        colourInterferingConnectors(canvas);
    }

    QList<CanvasItem *> canvas_items = canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (conn)
        {
            conn->updateIdealPosition();
        }
    }

#if 1
    if (postProcessing)
    {
        afterPostProcessing = 0;
    }
    else
    {
        ++afterPostProcessing;
    }

    /*******************/
    if (afterPostProcessing <= 2 && afterPostProcessing >= 1)
    {
        vpsc::Rectangles allShapes, labelShapes, rs, dummyNodesOnMidConnectors;
        std::vector<unsigned> rsIDList;
        std::vector<ShapeObj *> normalShapes;
        std::vector<Connector *> connectors;
        std::vector<cola::Edge> edgesVector;
        QMap<int, int> idIndexLookup, dummyNodeEndShapeInternalIDMap;
        std::map<int, int> midLabelDummyNodeMap;
        std::vector<unsigned> dummyNodeIndices;
        std::vector<double> idealEdgeLengths;

        QList<CanvasItem *> canvasObjects = canvas->items();
        for (int i = 0; i < canvasObjects.size(); ++i)
        {
            if (ShapeObj *shape = isShapeForLayout(canvasObjects.at(i)))
            {
                normalShapes.push_back(shape);

                // copied and modified from shapeToNode() in graphdata.cpp
                double g=0;
                double buffer = 0; //shape->canvas()->optShapeNonoverlapPadding();
                QRectF rect = shape->shapeRect(buffer + g); // Make a rect with the size of the shape + some possible paddings

                bool allowOverlap = false;
                /*
                if(rect.width() < 1.0) {
                    qWarning("dummy node, size<1 found - allowing overlap");
                    allowOverlap=true;
                }*/
                double labelOverlapPreventionPadding = 5;
                vpsc::Rectangle *r = new vpsc::Rectangle(rect.left() - labelOverlapPreventionPadding, rect.right() + labelOverlapPreventionPadding,
                        rect.top() - labelOverlapPreventionPadding, rect.bottom() + labelOverlapPreventionPadding, allowOverlap);
                qWarning("Node id=%d, (x,y)=(%f,%f), (w,h)=%f,%f",
                        shape->internalId(),r->getCentreX(), r->getCentreY(),
                        r->width(), r->height());

                rs.push_back(r);
                idIndexLookup.insert(shape->internalId(), rs.size() - 1);

            }
            else if (Connector *conn = dynamic_cast<Connector *> (canvasObjects.at(i)))
            {
                if (!conn->get_connpts().second.shape) return;

                connectors.push_back(conn);

                double srcShapeWidth = (conn->getSrcShape() == NULL) ? 0 : conn->getSrcShape()->size().width();
                double srcShapeHeight = (conn->getSrcShape() == NULL) ? 0 : conn->getSrcShape()->size().height();
                double dstShapeWidth = (conn->getDstShape() == NULL) ? 0 : conn->getDstShape()->size().width();
                double dstShapeHeight = (conn->getDstShape() == NULL) ? 0 : conn->getDstShape()->size().height();

                double srcShapeMaxHiddenLength = sqrt(pow(srcShapeWidth, 2) + pow(srcShapeHeight, 2)) / 2;
                double dstShapeMaxHiddenLength = sqrt(pow(dstShapeWidth, 2) + pow(dstShapeHeight, 2)) / 2;
                double totalLength = conn->painterPath().length();
                double minVisibleLength = totalLength - srcShapeMaxHiddenLength - dstShapeMaxHiddenLength;

                double midLabelsAtConnectorLength = srcShapeMaxHiddenLength + 0.5 * minVisibleLength;
                double midLabelsPercentAtLength = conn->painterPath().percentAtLength(midLabelsAtConnectorLength);
                QPointF midLabelsAtConnectorPoint = conn->painterPath().pointAtPercent(midLabelsPercentAtLength);

                QRectF rect;
                rect.setSize(QSizeF(5, 5));
                rect.moveCenter(conn->mapToScene(midLabelsAtConnectorPoint));

                dummyNodeEndShapeInternalIDMap.insertMulti(dummyNodesOnMidConnectors.size(), conn->getSrcShape()->internalId());
                dummyNodeEndShapeInternalIDMap.insertMulti(dummyNodesOnMidConnectors.size(), conn->getDstShape()->internalId());
                dummyNodesOnMidConnectors.push_back(new vpsc::Rectangle(rect.left(), rect.right(),
                                                                        rect.top(), rect.bottom(), false));
            }

        }

        // Made a similar method as items() in cavas.cpp called connectorLabelItems();
        QList<ConnectorLabel *> connectorLabel = canvas->connectorLabelItems(false);
        for(int i = 0; i < connectorLabel.size(); ++i)
        {
            ConnectorLabel *cl = connectorLabel.at(i);
            QRectF rect;
            rect.setSize(cl->size());

            QList<QGraphicsItem *> intersectedSceneItemsList;
            QList<QGraphicsItem *> tempList = canvas->collidingItems(cl, Qt::IntersectsItemBoundingRect);

            for (int i = 0; i < tempList.size(); ++i)
            {
                if (dynamic_cast<ConnectorLabel *> (tempList[i]) || dynamic_cast<ShapeObj *> (tempList[i]))
                {
                    intersectedSceneItemsList.push_back(tempList[i]);
                }
            }

            if (intersectedSceneItemsList.size() > 0)
            {
                rect.moveCenter(cl->scenePos());
            }
            else
            {
                rect.moveCenter(cl->parentItem()->mapToScene(cl->idealPos()));
            }
            rect = expandRect(rect, 3);
            labelShapes.push_back(new vpsc::Rectangle(rect.left(), rect.right(),
                    rect.top(), rect.bottom(), false));
        }

        for (int i = 0; i < rs.size(); i++)
        {
            allShapes.push_back(rs[i]);
            rsIDList.push_back(i);
        }

        int normalShapeIndexEndTo = allShapes.size() - 1;

        for (int i = 0; i < dummyNodesOnMidConnectors.size(); i++)
        {
            allShapes.push_back(dummyNodesOnMidConnectors[i]);
            rsIDList.push_back(rsIDList.size());

            int newIndex = allShapes.size() - 1;
            dummyNodeIndices.push_back(newIndex);

            QList<int> connectorEndShapesInternalIDList = dummyNodeEndShapeInternalIDMap.values(i);
            QList<int> connectorEndShapesIndexList;
            for (int j = 0; j < connectorEndShapesInternalIDList.size(); ++j)
            {
                connectorEndShapesIndexList.push_back(idIndexLookup.value(connectorEndShapesInternalIDList[j]));
            }
    /*
            if (connectors[i]->getDstShape())
            {
                edgesVector.push_back(std::make_pair(idIndexLookup.value(connectors[i]->getSrcShape()->internalId()), allShapes.size()));
                edgesVector.push_back(std::make_pair(idIndexLookup.value(connectors[i]->getDstShape()->internalId()), allShapes.size()));
                idealEdgeLengths.push_back(euclideanDist(Point(connectors[i]->getSrcShape()->x(), connectors[i]->getSrcShape()->y()), dummyNodesOnMidConnectors[i]->getCentreX()));
                idealEdgeLengths.push_back(40);
            }*/
            for (int j = 0; j < rs.size(); j++)
            {
                double rsxDummyNodeDistance = euclideanDist(Point(dummyNodesOnMidConnectors[i]->getCentreX(), dummyNodesOnMidConnectors[i]->getCentreY()),
                                                            Point(rs[j]->getCentreX(), rs[j]->getCentreY()));
                if (rsxDummyNodeDistance < 60 + qMax(rs[j]->width(), rs[j]->height()) || connectorEndShapesIndexList.contains(j))
                {
                    edgesVector.push_back(std::make_pair(j, newIndex));
                    idealEdgeLengths.push_back(rsxDummyNodeDistance);
                }
            }
        }

        int labelIndexStartFrom = allShapes.size();

        for (int i = 0; i < labelShapes.size(); i++)
        {
            allShapes.push_back(labelShapes[i]);
            idIndexLookup.insert(connectorLabel[i]->internalId(), allShapes.size() - 1);
        }
        for (int i = 0; i < dummyNodesOnMidConnectors.size(); i++)
        {
            if (!connectors[i]->getMiddleLabelRectangle1()->label().isEmpty())
            {
                edgesVector.push_back(std::make_pair(idIndexLookup.value(connectors[i]->getMiddleLabelRectangle1()->internalId()), dummyNodeIndices[i]));
                idealEdgeLengths.push_back(connectors[i]->getIdOffsetMap().value(connectors[i]->getMiddleLabelRectangle1()->internalId()));
                midLabelDummyNodeMap[idIndexLookup.value(connectors[i]->getMiddleLabelRectangle1()->internalId())] = dummyNodeIndices[i];
            }
            if (!connectors[i]->getMiddleLabelRectangle2()->label().isEmpty())
            {
                edgesVector.push_back(std::make_pair(idIndexLookup.value(connectors[i]->getMiddleLabelRectangle2()->internalId()), dummyNodeIndices[i]));
                idealEdgeLengths.push_back(connectors[i]->getIdOffsetMap().value(connectors[i]->getMiddleLabelRectangle2()->internalId()));
                midLabelDummyNodeMap[idIndexLookup.value(connectors[i]->getMiddleLabelRectangle2()->internalId())] = dummyNodeIndices[i];
            }
        }
        if (rsIDList.size() >= 2 && labelShapes.size() > 0)
        {
    #if 1
            cola::CompoundConstraints cc;
            cola::FixedRelativeConstraint *frc = new cola::FixedRelativeConstraint(allShapes, rsIDList, true);
            cc.push_back(frc);
            cola::ConstrainedFDLayout cfdl(allShapes, edgesVector, 1, false, idealEdgeLengths);
            cfdl.run();
            cfdl.setConstraints(cc);
            cfdl.run();
            cfdl.outputInstanceToSVG("pre");

            cola::ConstrainedFDLayout cfdl2(allShapes, edgesVector, 1, true, idealEdgeLengths);
            cfdl2.setUmlEdgeLabelStartIndex(labelIndexStartFrom);
            cfdl2.setShapeEndIndex(normalShapeIndexEndTo);
            cfdl2.setUmlMidLabelDummyNodeMap(midLabelDummyNodeMap);
            cfdl2.setConstraints(cc);
            cfdl2.outputInstanceToSVG("before");
            cfdl2.makeFeasible();
            cfdl2.outputInstanceToSVG("middle");
            cfdl2.run();
            cfdl2.outputInstanceToSVG("after");

            for (int i = 0; i < connectorLabel.size(); i++)
            {
                connectorLabel[i]->setPos(connectorLabel[i]->parentItem()->mapFromScene(QPointF(labelShapes[i]->getCentreX(), labelShapes[i]->getCentreY())));
            }

            cfdl2.freeAssociatedObjects();
            qDebug() << ":LAYOUT";
    #else

            Variables vs(allShapes.size());
            unsigned i=0;
            for(Variables::iterator v=vs.begin();v!=vs.end();++v,++i) {
                *v=new Variable(i,0,1);
            }
            Constraints cs;
            generateXConstraints(allShapes,vs,cs,false);
            //generateYConstraints(allShapes,vs,cs);
            try {
                vpsc::IncSolver vpsc(vs,cs);
                vpsc.solve();
            } catch (char *str) {
                std::cerr<<str<<std::endl;
                for(vpsc::Rectangles::iterator r=allShapes.begin();r!=allShapes.end();++r) {
                    std::cerr << **r <<std::endl;
                }
            }
            vpsc::Rectangles::iterator r=allShapes.begin();
            for(vpsc::Variables::iterator v=vs.begin();v!=vs.end();++v,++r) {
                assert((*v)->finalPosition==(*v)->finalPosition);
                (*r)->moveCentreX((*v)->finalPosition);
                //(*r)->moveCentreY((*v)->finalPosition);
            }
            assert(r==allShapes.end());
            for_each(cs.begin(),cs.end(),delete_object());
            for_each(vs.begin(),vs.end(),delete_object());

            Variables vs1(allShapes.size());
            unsigned j=0;
            for(Variables::iterator v=vs1.begin();v!=vs1.end();++v,++j) {
                *v=new Variable(j,0,1);
            }
            Constraints cs1;
            //generateXConstraints(allShapes,vs,cs1,false);
            generateYConstraints(allShapes,vs1,cs1);
            try {
                vpsc::IncSolver vpsc(vs1,cs1);
                vpsc.solve();
            } catch (char *str) {
                std::cerr<<str<<std::endl;
                for(vpsc::Rectangles::iterator r=allShapes.begin();r!=allShapes.end();++r) {
                    std::cerr << **r <<std::endl;
                }
            }
            vpsc::Rectangles::iterator r1=allShapes.begin();
            for(vpsc::Variables::iterator v=vs1.begin();v!=vs1.end();++v,++r1) {
                assert((*v)->finalPosition==(*v)->finalPosition);
                //(*r)->moveCentreX((*v)->finalPosition);
                (*r1)->moveCentreY((*v)->finalPosition);
            }
            assert(r1==allShapes.end());
            for_each(cs1.begin(),cs1.end(),delete_object());
            for_each(vs1.begin(),vs1.end(),delete_object());

            if (labelShapes.size() > 0)
            {
                for (int i = 0; i < connectorLabel.size(); i++)
                {
                    connectorLabel[i]->setPos(connectorLabel[i]->parentItem()->mapFromScene(QPointF(labelShapes[i]->getCentreX(), labelShapes[i]->getCentreY())));
                }
            }
    #endif
            /*******************

            vpsc::removeoverlaps(allShapes);

            for (int i = 0; i < normalShapes.size(); i++)
            {
                normalShapes[i]->setCentrePos(QPoint(rs[i]->getCentreX(), rs[i]->getCentreY()));
            }

            if (labelShapes.size() > 0)
            {
                for (int i = 0; i < connectorLabel.size(); i++)
                {
                    connectorLabel[i]->setPos(connectorLabel[i]->parentItem()->mapFromScene(QPointF(labelShapes[i]->getCentreX(), labelShapes[i]->getCentreY())));
                }
            }

            ******************/
        }
        for (int i = 0; i < allShapes.size(); ++i)
        {
            delete allShapes[i];
            allShapes[i] = NULL;
        }
        for (int i = 0; i < rs.size(); ++i)
        {
            delete rs[i];
            rs[i] = NULL;
        }
        for (int i = 0; i < dummyNodesOnMidConnectors.size(); ++i)
        {
            delete dummyNodesOnMidConnectors[i];
            dummyNodesOnMidConnectors[i] = NULL;
        }
        for (int i = 0; i < labelShapes.size(); ++i)
        {
            delete labelShapes[i];
            labelShapes[i] = NULL;
        }
    }
#endif
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
