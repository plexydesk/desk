/*******************************************************************************
* This file is part of PlexyDesk.
*  Maintained by : Siraj Razick <siraj@plexydesk.com>
*  Authored By  :
*
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
#ifndef DOCK_DATA_H
#define DOCK_DATA_H

#include <QtCore>

#include <ck_data_source.h>
#include <ck_desktop_controller_interface.h>
#include <ck_widget.h>
#include <QtNetwork>
#include <ck_table_view_item.h>
#include <ck_fixed_layout.h>

using namespace cherry_kit;

class desktop_panel_controller_impl
    : public cherry_kit::desktop_controller_interface {
  Q_OBJECT

public:
  desktop_panel_controller_impl(QObject *object = 0);
  void init();

  virtual ~desktop_panel_controller_impl();
  void prepare_removal();

  void session_data_available(const cherry_kit::sync_object &a_sesion_root);
  virtual void submit_session_data(cherry_kit::sync_object *a_obj);

  void set_view_rect(const QRectF &rect);

  cherry_kit::ActionList actions() const;
  void request_action(const QString &actionName, const QVariantMap &args);

  QString icon() const;

  void switch_to_previous_space();
  void switch_to_next_space();
  void toggle_seamless();

  void create_dock_action(cherry_kit::fixed_layout *build, int row, int column,
                          const std::string &icon,
                          std::function<void()> a_button_action_func);

  void toggle_panel();

  void discover_actions_from_controller(const QString &name);

  void remove_space_request();
  void add_new_space();

  void exec_action(const QString &action);

  widget *create_task_action(const QString &aIcon, const QString &aLabel, const QString &aControllerName);
  void insert_action(const QString &a_name,
                     const QString &a_icon,
                     const QString &a_ctr);

protected:
  void udpate_desktop_preview();

private:
  class PrivateDock;
  PrivateDock *const priv;

  cherry_kit::desktop_dialog_ref createActivity(const QString &controller_name,
                                                const QString &activity,
                                                const QString &title,
                                                const QPoint &pos,
                                                const QVariantMap &dataItem);
  QAction *createAction(int id, const QString &action_name,
                        const QString &icon_name);
};

#endif