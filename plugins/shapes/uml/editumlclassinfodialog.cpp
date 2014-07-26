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
        ClassShape *item = dynamic_cast<ClassShape*>(list.front());

        if (item) {
            classMethodsArea->setEnabled(true);
            classAttributesArea->setEnabled(true);
            classNameArea->setEnabled(true);

            classNameArea->clear();
            classNameArea->setAlignment(Qt::AlignHCenter);
            classNameArea->append(item->getClassNameAreaText());
            classNameArea->setAlignment(Qt::AlignHCenter);

            classAttributesArea->setText(item->getClassAttributesAreaText());
            classMethodsArea->setText(item->getClassMethodsAreaText());

            connect( classMethodsArea, SIGNAL( textChanged() ), item, SLOT( classMethodsAreaChanged() ));
            connect( classAttributesArea, SIGNAL( textChanged() ), item, SLOT( classAttributesAreaChanged() ));
            connect( classNameArea, SIGNAL( textChanged() ), item, SLOT( classNameAreaChanged() ));

            item->classNameAreaChanged();
            item->classAttributesAreaChanged();
            item->classMethodsAreaChanged();
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
    std::cout << "Inside changeCanvas()" << std::endl;
    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
    }
    m_canvas = canvas;

    connect(m_canvas, SIGNAL(selectionChanged()), this, SLOT(selection_changed()));
}

}
