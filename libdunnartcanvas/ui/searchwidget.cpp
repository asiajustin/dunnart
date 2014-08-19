#include "searchwidget.h"

namespace dunnart {

SearchWidget::SearchWidget(Canvas *canvas, QWidget *parent) :
    QDockWidget(parent),
    m_canvas(NULL)
{
    this->setupUi(this);

    changeCanvas(canvas);
}

SearchWidget::~SearchWidget()
{
}

void SearchWidget::changeCanvas(Canvas* canvas)
{
    if (m_canvas)
    {
        //disconnect(m_canvas, 0, this, 0);
        //disconnect(this, 0, m_canvas, 0);
    }
    m_canvas = canvas;

    //connect(m_canvas, SIGNAL(selectionChanged()), this, SLOT(selection_changed()));
}

}
