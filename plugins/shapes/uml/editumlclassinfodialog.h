#ifndef EDITUMLCLASSINFODIALOG_H
#define EDITUMLCLASSINFODIALOG_H

#include "ui_editumlclassinfodialog.h"
#include <QDockWidget>

class QSignalMapper;

namespace dunnart {

class Canvas;

class EditUmlClassInfoDialog : public QDockWidget,
        private Ui::EditUmlClassInfoDialog
{
    Q_OBJECT

    Q_PROPERTY(QTextEdit* getClassNameArea READ getClassNameArea)
    Q_PROPERTY(QTextEdit* getClassAttributesArea READ getClassAttributesArea)
    Q_PROPERTY(QTextEdit* getClassMethodsArea READ getClassMethodsArea)

    public:
        EditUmlClassInfoDialog(Canvas* canvas, QWidget *parent = NULL);
        ~EditUmlClassInfoDialog();

        QTextEdit* getClassNameArea();
        QTextEdit* getClassAttributesArea();
        QTextEdit* getClassMethodsArea();

    private:
        Canvas *m_canvas;


    private slots:
        void changeCanvas(Canvas* canvas);
        void selection_changed();
};

}
#endif // EDITUMLCLASSINFODIALOG_H
