/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Siraj Razick <siraj@plexydesk.com>
*  Authored By  : *
*  PlexyDesk is free software: you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  PlexyDesk is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with PlexyDesk. If not, see <http://www.gnu.org/licenses/lgpl.html>
*******************************************************************************/
#include "date_dialog.h"
#include <ck_widget.h>
#include <ck_config.h>
#include <QTimer>
#include <ck_desktop_controller_interface.h>
#include <ck_TableView.h>
#include <ck_DefaultTableModel.h>
#include <ck_resource_manager.h>
#include <ck_icon_button.h>
#include <ck_button.h>
#include <ck_calendar_view.h>

class date_dialog::PrivateDatePicker {
public:
  PrivateDatePicker() {}
  ~PrivateDatePicker() {}

  cherry_kit::window *m_activity_window;
  cherry_kit::widget *m_window_content;
  cherry_kit::button *m_done_button;
  cherry_kit::calendar_view *mCalendarWidget;

  QVariantMap m_result_data;
};

date_dialog::date_dialog(QGraphicsObject *object)
    : cherry_kit::desktop_dialog(object),
      priv(new PrivateDatePicker) {}

date_dialog::~date_dialog() { delete priv; }

void date_dialog::create_window(const QRectF &window_geometry,
                                       const QString &window_title,
                                       const QPointF &window_pos) {
  priv->m_activity_window = new cherry_kit::window();
  priv->m_activity_window->setGeometry(window_geometry);
  priv->m_activity_window->set_window_title(window_title);

  priv->m_window_content =
      new cherry_kit::widget(priv->m_activity_window);
  priv->m_window_content->setGeometry(window_geometry);

  priv->mCalendarWidget =
      new cherry_kit::calendar_view(priv->m_window_content);
  priv->mCalendarWidget->setGeometry(window_geometry);
  priv->mCalendarWidget->setPos(0, 0);

  priv->m_done_button =
      new cherry_kit::button(priv->m_window_content);
  priv->m_done_button->set_label(tr("Done"));
  priv->m_done_button->show();

  priv->m_done_button->setPos(
      priv->mCalendarWidget->geometry().width() / 2 -
          (priv->m_done_button->boundingRect().width() + 10) / 2,
      310);

  priv->m_done_button->on_input_event([this](
      cherry_kit::widget::InputEvent a_event,
      const cherry_kit::widget *a_widget) {
    if (a_event == cherry_kit::widget::kMouseReleaseEvent) {
      qDebug() << Q_FUNC_INFO << "Activity complete";
      end_calendar();
      notify_done();
    }
  });

  priv->m_activity_window->set_window_content(
      priv->m_window_content);

  QRectF view_geometry = window_geometry;
  view_geometry.setHeight(window_geometry.height() + 64);

  set_geometry(view_geometry);

  exec(window_pos);
}

QVariantMap date_dialog::result() const {
  return priv->m_result_data;
}

cherry_kit::window *date_dialog::dialog_window() const {
  return priv->m_activity_window;
}

void date_dialog::cleanup() {
  if (priv->m_activity_window) {
    delete priv->m_activity_window;
  }

  priv->m_activity_window = 0;
}

void date_dialog::onImageReady(const QImage &img) {}

void date_dialog::end_calendar() {
  if (!priv->mCalendarWidget) {
    priv->m_result_data["date"] =
        QVariant(QDate::currentDate().toString());
  } else {
    priv->m_result_data["date"] =
        QVariant(priv->mCalendarWidget->a_date());
  }
}