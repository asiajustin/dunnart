/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2010  Monash University
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

#include <QtWidgets>
#include <QObject>

#include "libdunnartcanvas/shapeplugininterface.h"
#include "libdunnartcanvas/applicationplugininterface.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/canvasapplication.h"
#include "libdunnartcanvas/canvas.h"

using namespace dunnart;

#include "editumlclassinfodialog.h"
#include "umlclass.h"
#include "umlnote.h"
#include "umlpackage.h"

class UMLShapesPlugin : public QObject, public ShapePluginInterface, public ApplicationPluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::ShapePluginInterface)
        Q_INTERFACES (dunnart::ApplicationPluginInterface)
        Q_PLUGIN_METADATA (IID "org.dunnart.UMLShapesPlugin")

    public:
        UMLShapesPlugin()
        {
        }

        void applicationMainWindowInitialised(CanvasApplication *canvasApplication)
        {
            qDebug("WE ARE AN APPLICATION PLUGIN");

            m_canvas_application = canvasApplication;

            QAction *m_action_show_edit_uml_class_info_dialog = new QAction(
                    tr("Edit UML Class Information"), this);
            m_action_show_edit_uml_class_info_dialog->setCheckable(true);

            QStringList path;
            path << "View" << "Show Dialogs";
            QMenu *dialogMenu = searchMenuItem(path)->menu();
            dialogMenu->addSeparator();
            dialogMenu->addAction(m_action_show_edit_uml_class_info_dialog);

            m_dialog_uml_class_info = new EditUmlClassInfoDialog(m_canvas_application->currentCanvas(), m_canvas_application->mainWindow());
            connect(m_canvas_application, SIGNAL(currentCanvasChanged(Canvas*)),
                    m_dialog_uml_class_info, SLOT(changeCanvas(Canvas*)));
            connect(m_action_show_edit_uml_class_info_dialog,  SIGNAL(triggered(bool)),
                    m_dialog_uml_class_info, SLOT(setVisible(bool)));
            connect(m_dialog_uml_class_info, SIGNAL(visibilityChanged(bool)),
                    m_action_show_edit_uml_class_info_dialog,  SLOT(setChecked(bool)));
            m_canvas_application->mainWindow()->addDockWidget(Qt::LeftDockWidgetArea, m_dialog_uml_class_info);
            m_dialog_uml_class_info->hide();
        }

        void applicationWillClose(CanvasApplication *canvasApplication)
        {

        }

        QString shapesClassLabel(void) const
        {
            return "UML";
        }
        QStringList producableShapeTypes() const
        {
            QStringList shapes;
            shapes << "org.dunnart.shapes.umlClass";
            shapes << "org.dunnart.shapes.umlNote";
            shapes << "org.dunnart.shapes.umlPackage";

            return shapes;
        }
        ShapeObj *generateShapeOfType(QString shapeType)
        {
            if (shapeType == "org.dunnart.shapes.umlClass")
            {
                ClassShape *newClassShape = new ClassShape();
                newClassShape->setEditDialog(m_dialog_uml_class_info);
                return newClassShape;
            }
            else if (shapeType == "org.dunnart.shapes.umlNote")
            {
                return new NoteShape();
            }
            else if (shapeType == "org.dunnart.shapes.umlPackage")
            {
                return new PackageShape();
            }
            
            return NULL;
        }

private:
        CanvasApplication *m_canvas_application;
        EditUmlClassInfoDialog *m_dialog_uml_class_info;

        /*From: http://living-tomorrow.blogspot.com.au/2009/06/code-to-get-menu-item-in-qt.html; First Access on 28/06/2014 */
        QAction* searchMenuItem(QAction* submenu, QStringList& path, int layer)
        {
           QAction* target = NULL;

           if(submenu->text() == path[layer]){
               target = submenu;
               if(path.count() > layer+1){
                   target = searchMenuItem(target, path, layer+1);
               }
           }else{
               QMenu* menu = submenu->menu();
               QList<QAction*> actions = menu->actions();
               foreach(QAction* act, actions){
                   if(act->text() == path[layer]){
                       target = act;
                       if(path.count() > layer+1){ //need to go deep
                           target = searchMenuItem(act, path, layer+1);
                       }
                       break;
                   }else{
                       // "ignore action ";
                   }
               }
           }

           return target;
        }

        /*From: http://living-tomorrow.blogspot.com.au/2009/06/code-to-get-menu-item-in-qt.html; First Access on 28/06/2014 */
        QAction* searchMenuItem(QStringList path)
        {
           if(!path.count()){
               return NULL;
           }

           QAction* target = NULL;

           QMenuBar* menus = m_canvas_application->mainWindow()->menuBar();
           Q_ASSERT(menus);
           if(menus){
               QList<QAction*> actions = menus->actions();
               int i=0;
               int layer = 0;

               QString mainMenuLabel = path[0];

               foreach(QAction* act, actions){
                   if(act->text() == mainMenuLabel){ //found 1st level menu item
                       target = act;
                       if(path.count() > layer+1){ //need to go deep
                           target = searchMenuItem(act, path, layer+1);
                       }
                       break;
                   }else{
                       // "ignore action ";
                   }
               }
           }
           return target;
        }

};


// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
