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
#ifndef CONTROLLER_ACTION_H
#define CONTROLLER_ACTION_H

#include <cstring>
#include <string>

namespace cherry_kit {

class ui_action {
public:
  ui_action();
  ~ui_action();

  virtual std::string name() const;
  virtual void set_name(const std::string &a_name);

  virtual unsigned int id() const;
  virtual void set_id(unsigned int a_id);

  virtual void set_visible(bool a_visibility = true);
  virtual bool is_visibile() const;

  virtual void set_icon(const std::string a_icon);
  virtual std::string icon() const;

private:
  class PrivateControllerAction;
  PrivateControllerAction *const o_ui_action;
};
}

#endif // CONTROLLER_ACTION_H