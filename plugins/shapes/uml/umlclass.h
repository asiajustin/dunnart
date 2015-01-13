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
*/

/*
** UMLClass
** A shape, inheriting from Rect, for UML classes in Dunnart diagrams.
** Supports most (but not all) features of classes in UML.
**
** UML classes are shown with three sections, separated by horizontal lines:
** class name, attributes, and methods. If any section is ommitted that
** does not necessarily mean it is empty. UMLClass does not currently
** support ommitting the attributes section, it either displays all
** three sections or just the class name.
**
** The level of detail displayed can be adjusted using the scroll wheel,
** with "class name only" at one extreme and "all methods, return types,
** paramaters and parameter types" at the the other. The class size will
** adjust to fit the new level of detail. Likewise, if the class size is
** adjusted the level of detail will change to ensure the content fits.
** if the class is made too short to display all attributes/methods,
** some will be ommitted and a "..." displayed in the appropriate section
** (I think the code for doing that has a slight bug in it however)
**
** To edit the contents of the class, middle click in the appropriate
** section. Simply input text and it will be parsed, the patterns
** accepted are as follows...
** Class name section:
** [<<MY_STEREOTYPE>>]MY_CLASS_NAME
** Attribute section:
** [<<MY_STEREOTYPE>>][+|-]MY_ATTRIBUTE:MY_TYPE
** Method section:
** [+|-]MY_METHOD_NAME(one or more params):MY_RETURN_TYPE
** param:
** [in|out]PARAM_NAME:PARAM_TYPE
**
** Attribute stereotypes were only put in because one sample diagram 
** used <<property>> to represent properties in C#. When a stereotype
** is given the visibility (+/-) doesn't display. Stereotypes aren't
** currently supported for methods. 
** Tokenising and parsing is done in a single step in the implementation.
**
** When saved as svg, information is added both so that the class will
** render when viewing the svg diagram the same as it does within dunnart,
** and so that dunnart can load the attribute/method information when
** reading the svg file.
*/

#ifndef UMLCLASS_H
#define UMLCLASS_H

#include <string>
#include <vector> 

#include "libdunnartcanvas/shape.h"
#include "editumlclassinfodialog.h"
using namespace dunnart;


enum UML_Param_Mode { UNSPECIFIED, PARAM_IN, PARAM_OUT };
enum ACCESS_MODIFIER { DEFAULT, PRIVATE, PROTECTED, PUBLIC, PACKAGE};

struct Attribute {
    bool is_public;
    ACCESS_MODIFIER accessModifier;
    QString name;
    QString type;
    QString stereotype;
};

struct Parameter {
    QString name;
    QString type;
    UML_Param_Mode mode;
};

struct Method {
    bool is_public;
    ACCESS_MODIFIER accessModifier;
    QString name;
    QString return_type;
    std::vector<Parameter> params;
};

enum UML_Class_Abbrev_Mode { NO_ABBREV = 1, NO_RETURN_TYPES, NO_PARAM_TYPES, NO_PARAM_MODES, NO_PARAMS,
                             NO_METHODS, NO_ATTR_TYPES, NO_ATTR_STEREOTYPES, NO_ATTRS, NO_CLASS_STEREOTYPES};
enum UML_Class_Edit_Type { EDIT_CLASS_NAME, EDIT_ATTRIBUTES, EDIT_METHODS};


class ClassShape: public RectangleShape {
    Q_OBJECT

    /*Q_PROPERTY (QString classNameAreaText READ getClassNameAreaText)
    Q_PROPERTY (QString classAttributesAreaText READ getClassAttributesAreaText)
    Q_PROPERTY (QString classMethodsAreaText READ getClassMethodsAreaText)*/

    public:
        ClassShape();
        virtual ~ClassShape() { }

        QString getClassNameAreaText();
        QString getClassAttributesAreaText();
        QString getClassMethodsAreaText();

        void setEditDialog(EditUmlClassInfoDialog *editDialog);

        virtual void initWithXMLProperties(Canvas *canvas, const QDomElement& node, const QString& ns);
        virtual QDomElement to_QDomElement(const unsigned int subset, QDomDocument& doc);
#if 0
        void setMode(UML_Class_Abbrev_Mode newMode);
        void middle_click(const int& mouse_x, const int& mouse_y);

        const char* get_class_name(void) { return class_name.toUtf8().constData(); }

        virtual int get_min_width(void);
        virtual void change_label(void);//QT
        void update_contents(UML_Class_Edit_Type,
                const std::vector<QString>& lines,
                const bool store_undo = true);
#endif

        virtual QPainterPath buildPainterPath(void);
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        /**
         * Overrided, to specify how many levels of detail your shape has.
         */
        virtual uint levelsOfDetail(void) const;

        /**
         * Overrided, to specify the size that your shape should be
         * expanded to at each detail level.  The base level is 1.  Any
         * subsequent levels will be 2, 3, ...
         */
        virtual QSizeF sizeForDetailLevel(uint level);

        /**
         * @brief Override this to set up any label text, colours, etc for the
         *        instance of the shape to be shown in the shape picker.
         */
        virtual void setupForShapePickerPreview(void);

        /**
         *  Overrided, to set up any label text, colours, etc for the
         *  instance of the shape created on the canvas when the users
         *  drags shapes from the shape picker.
         */
        virtual void setupForShapePickerDropOnCanvas(void);

        virtual void setIdealPos(QPointF pos);
        virtual QPointF idealPos();

        void classNameAreaChanged(bool textChanged);
        void classAttributesAreaChanged(bool textChanged);
        void classMethodsAreaChanged(bool textChanged);

        QString classNameAreaText;
        QString classAttributesAreaText;
        QString classMethodsAreaText;

    public slots:
        void classNameAreaChanged();
        void classAttributesAreaChanged();
        void classMethodsAreaChanged();
    protected:
        virtual QAction *buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu);
        //virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
    private:

        EditUmlClassInfoDialog *m_editDialog;

        QString class_name;
        QString class_stereotype;
        std::vector<Attribute> attributes;
        std::vector<Method> methods;

        int classWidth;
        int classNameSectionHeight;
        int classAttributesSectionHeight;
        int classMethodsSectionHeight;

        QPointF ideal_pos;

        void do_init(void);
        void refresh();

        void updateAllClassAreas();

        QSizeF recalculateSize();
        int getMaxWidth();
        int getClassNameSectionHeight();
        int getClassAttributesSectionHeight();
        int getClassMethodsSectionHeight();

#if 0
        bool mode_changed_manually;
        int get_longest_text_width(UML_Class_Abbrev_Mode mode);
        void determine_best_mode(void);
        void determine_small_dimensions(int *w, int *h);
        void determine_best_dimensions(int *inner_width, int *h);
        
        void do_edit(UML_Class_Edit_Type);

        unsigned int get_attr_section_height(void);
        unsigned int get_method_section_height(void);
		unsigned int get_class_name_section_height(void);

        QString class_name_to_string(bool one_line);
        QString attribute_to_string(int index, UML_Class_Abbrev_Mode mode = NOABBREV);
        QString method_to_string(int index, UML_Class_Abbrev_Mode mode = NOABBREV);

        int class_name_section_size;
        int attr_section_size;
        int method_section_size;
        void detailLevelChanged(void);
        void draw(QPixmap *surface, const int x, const int y, const int type, const int w, const int h);
#endif

};


extern unsigned int umlFontSize;


#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

