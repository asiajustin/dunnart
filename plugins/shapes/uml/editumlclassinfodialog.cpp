#include <QSignalMapper>

#include "editumlclassinfodialog.h"
#include "umlclass.h"

namespace dunnart {

EditUmlClassInfoDialog::EditUmlClassInfoDialog(Canvas *canvas, QWidget *parent) :
    QDockWidget(parent),
    m_canvas(NULL)
{
    this->setupUi(this);

    classNameArea->setAlignment(Qt::AlignHCenter);

    classMethodsArea->setEnabled(false);
    classAttributesArea->setEnabled(false);
    classNameArea->setEnabled(false);

    changeCanvas(canvas);
}

EditUmlClassInfoDialog::~EditUmlClassInfoDialog()
{

}

QTextEdit* EditUmlClassInfoDialog::getClassNameArea()
{
    return classNameArea;
}

QTextEdit* EditUmlClassInfoDialog::getClassAttributesArea()
{
    return classAttributesArea;
}

QTextEdit* EditUmlClassInfoDialog::getClassMethodsArea()
{
    return classMethodsArea;
}

void EditUmlClassInfoDialog::selection_changed()
{
    std::cout << "Inside selection_changed()" << std::endl;

    disconnect(classMethodsArea, 0, 0, 0);
    disconnect(classAttributesArea, 0, 0, 0);
    disconnect(classNameArea, 0, 0, 0);

    QList<CanvasItem *> list = m_canvas->selectedItems();

    if (list.count() == 1)
    {
        ClassShape *classShapeItem = dynamic_cast<ClassShape*>(list.front());

        if (classShapeItem) {
            classMethodsArea->setEnabled(true);
            classAttributesArea->setEnabled(true);
            classNameArea->setEnabled(true);

            classNameArea->clear();
            classNameArea->setAlignment(Qt::AlignHCenter);
            classNameArea->append(classShapeItem->getClassNameAreaText());
            classNameArea->setAlignment(Qt::AlignHCenter);

            classAttributesArea->setText(classShapeItem->getClassAttributesAreaText());
            classMethodsArea->setText(classShapeItem->getClassMethodsAreaText());

            connect( classMethodsArea, SIGNAL( textChanged() ), classShapeItem, SLOT( classMethodsAreaChanged() ));
            connect( classAttributesArea, SIGNAL( textChanged() ), classShapeItem, SLOT( classAttributesAreaChanged() ));
            connect( classNameArea, SIGNAL( textChanged() ), classShapeItem, SLOT( classNameAreaChanged() ));

            classShapeItem->classNameAreaChanged(false);
            classShapeItem->classAttributesAreaChanged(false);
            classShapeItem->classMethodsAreaChanged(false);

            return;
        }

    }
    else {
        classMethodsArea->clear();
        classAttributesArea->clear();
        classNameArea->clear();

        classMethodsArea->setEnabled(false);
        classAttributesArea->setEnabled(false);
        classNameArea->setEnabled(false);
    }
}


void EditUmlClassInfoDialog::changeCanvas(Canvas* canvas)
{
    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
    }
    m_canvas = canvas;

    connect(m_canvas, SIGNAL(selectionChanged()), this, SLOT(selection_changed()));
}

}
