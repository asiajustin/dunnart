#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "ui_searchwidget.h"
#include <QDockWidget>

namespace dunnart {

class Canvas;

class SearchWidget : public QDockWidget, private Ui::SearchWidget
{
    Q_OBJECT

    public:
        SearchWidget(Canvas* canvas, QWidget *parent = NULL);
        ~SearchWidget();

    private slots:
        void changeCanvas(Canvas* canvas);

    private:
        Canvas *m_canvas;
};

}

#endif // SEARCHWIDGET_H
