#include "themepackloader.h"
#include "window.h"

#include <QRectF>
#include <QPointF>
#include <QSizeF>

#include <button.h>
#include <windowbutton.h>
#include <space.h>

#include <QDebug>
#include <QTimer>
#include <QGraphicsDropShadowEffect>

namespace UIKit
{
class Window::PrivateWindow
{
public:
  PrivateWindow() :  m_window_content(0),
    mWindowBackgroundVisibility(true) {}
  ~PrivateWindow() {}

  QRectF m_window_geometry;
  Widget *m_window_content;
  Space *m_window_viewport;
  QString m_window_title;

  WindowType m_window_type;
  bool mWindowBackgroundVisibility;

  WindowButton *m_window_close_button;

  std::function<void (const QSizeF &size)> m_window_size_callback;
  std::function<void (const QPointF &size)> m_window_move_callback;
  std::function<void (Window *)> m_window_close_callback;
  std::function<void (Window *)> m_window_discard_callback;
};

Window::Window(QGraphicsObject *parent) : Widget(parent), m_priv_impl(new PrivateWindow)
{
  set_widget_flag(Widget::kRenderBackground, true);
  setFlag(QGraphicsItem::ItemIsMovable, true);
  setGeometry(QRectF(0, 0, 400, 400));
  set_window_title("");
  m_priv_impl->m_window_type = kApplicationWindow;

  m_priv_impl->m_window_close_button = new WindowButton(this);
  m_priv_impl->m_window_close_button->setPos(5, 5);
  m_priv_impl->m_window_close_button->show();
  m_priv_impl->m_window_close_button->setZValue(10000);

  setFocus(Qt::MouseFocusReason);

  on_input_event([this](Widget::InputEvent aEvent, const Widget * aWidget) {
    if (aEvent == Widget::kFocusOutEvent &&
        m_priv_impl->m_window_type == kPopupWindow) {
      hide();
    }
  });

  m_priv_impl->m_window_close_button->on_input_event([this](Widget::InputEvent aEvent,
  const Widget * aWidget) {
    if (aEvent == Widget::kMouseReleaseEvent) {
      if (m_priv_impl->m_window_close_callback) {
        m_priv_impl->m_window_close_callback(this);
      }

    }
  });
}

Window::~Window()
{
  qDebug() << Q_FUNC_INFO;
  delete m_priv_impl;
}

void Window::set_window_content(Widget *a_widget)
{
  if (m_priv_impl->m_window_content) {
    return;
  }

  m_priv_impl->m_window_content = a_widget;
  m_priv_impl->m_window_content->setParentItem(this);

  this->setGeometry(m_priv_impl->m_window_content->boundingRect());

  float sWindowTitleHeight = 0;

  if (UIKit::Theme::style()) {
    sWindowTitleHeight = UIKit::Theme::style()
                         ->attribute("frame", "window_title_height")
                         .toFloat();
  }

  if (m_priv_impl->m_window_type == kApplicationWindow) {
    m_priv_impl->m_window_content->setPos(0.0, sWindowTitleHeight);
  } else {
    m_priv_impl->m_window_close_button->hide();
  }

  if (m_priv_impl->m_window_type != kFramelessWindow) {
    QGraphicsDropShadowEffect *lEffect = new QGraphicsDropShadowEffect(this);
    lEffect->setColor(QColor("#111111"));
    lEffect->setBlurRadius(26);
    lEffect->setXOffset(0);
    lEffect->setYOffset(0);
    setGraphicsEffect(lEffect);
  }
}

void Window::set_window_viewport(Space *a_space)
{
  m_priv_impl->m_window_viewport = a_space;
}

void Window::set_window_title(const QString &a_window_title)
{
  m_priv_impl->m_window_title = a_window_title;
}

QString Window::window_title() const
{
  return m_priv_impl->m_window_title;
}

Window::WindowType Window::window_type()
{
  return m_priv_impl->m_window_type;
}

void Window::set_window_type(Window::WindowType a_window_type)
{
  m_priv_impl->m_window_type = a_window_type;

  if (a_window_type == kApplicationWindow &&
      m_priv_impl->m_window_content) {
    m_priv_impl->m_window_content->setPos(0.0, 72.0);
  } else {
    m_priv_impl->m_window_close_button->hide();
  }

  if (m_priv_impl->m_window_type == kPopupWindow) {
    setZValue(10000);
  }
}

void Window::on_window_resized(
  std::function<void (const QSizeF &)> handler)
{
  m_priv_impl->m_window_size_callback = handler;
}

void Window::on_window_moved(std::function<void (const QPointF &)> a_handler)
{
  m_priv_impl->m_window_move_callback = a_handler;
}

void Window::on_window_closed(std::function<void (Window *)> a_handler)
{
  m_priv_impl->m_window_close_callback = a_handler;
}

void Window::on_window_discarded(std::function<void (Window *)> a_handler)
{
  m_priv_impl->m_window_discard_callback = a_handler;
}

void Window::paint_view(QPainter *painter, const QRectF &rect)
{
  if (!m_priv_impl->mWindowBackgroundVisibility) {
    return;
  }

  StyleFeatures feature;
  feature.geometry = rect;
  feature.text_data = m_priv_impl->m_window_title;

  if (style()) {
    style()->draw("window_frame", feature, painter);
  }
}

void Window::show()
{
  setVisible(true);
  setFocus(Qt::MouseFocusReason);
}

void Window::hide()
{
  // Qt 5.4 -> QTimer::singleShot(250, []() { windowHide(); } );
  QTimer *lTimer = new QTimer(this);
  lTimer->setSingleShot(true);

  connect(lTimer, &QTimer::timeout, [ = ]() {
    setVisible(false);
    delete lTimer;
  });

  lTimer->start(250);
}

void Window::discard()
{
  if (m_priv_impl->m_window_discard_callback) {
    qDebug() << Q_FUNC_INFO << "Discard Requested: Notifiy";
    m_priv_impl->m_window_discard_callback(this);
  }
}

void Window::enable_window_background(bool a_visibility)
{
  m_priv_impl->mWindowBackgroundVisibility = a_visibility;
}

void Window::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsObject::mousePressEvent(event);
}

void Window::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
  QGraphicsObject::mouseReleaseEvent(event);
}

}
