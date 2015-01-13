#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include "ui_searchwidget.h"
#include "libdunnartcanvas/canvasview.h"
#include <QDockWidget>

namespace dunnart {

class Canvas;
class ShapeObj;

class SearchWidget : public QDockWidget, private Ui::SearchWidget
{
    Q_OBJECT

    public:
        SearchWidget(CanvasView* canvasView, QWidget *parent = NULL);
        ~SearchWidget();

    private slots:
        void changeCanvasView(CanvasView *canvasview);
        void search(void);
        void locateResult(QListWidgetItem *widgetItem);

    private:
        CanvasView *m_canvasview;
        Canvas *m_canvas;

        bool searchWithinEndShapes(ShapeObj* shape, QString searchText);
};

}

#endif // SEARCHWIDGET_H
