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
#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <plexydesk_ui_exports.h>

#include <ck_style.h>
#include <functional>

namespace cherry_kit {
class DECL_UI_KIT_EXPORT resource_manager {
public:
  typedef std::function<void()> resource_ready_callback_t;

  typedef enum {
    kDarkPrimaryColor = 1,
    kPrimaryColor,
    kLightPrimaryColor,
    kTextBackground,
    kTextColor,
    kSecondryTextColor,
    kDividerColor,
    kAccentColor
  } ColorName;

  resource_manager(const QString &a_theme_name);
  virtual ~resource_manager();
  virtual void init();

  static resource_manager *instance();

  virtual void set_theme_name(const QString &a_name);

  virtual void set_style(const std::string &a_name);
  static style_ref style();

  virtual QImage drawable(const QString &a_fileName, const QString &a_dpi);
  virtual QString drawable_file_name(const QString &a_dpi,
                                     const QString &a_fileName);
  static const char *color(ColorName a_name);
  virtual void set_color_scheme(const std::string &a_name);
  virtual std::string color_scheme() const;
  virtual const char *color_code(ColorName a_name);

  virtual void on_ready(resource_ready_callback_t a_callback);
private:
  void scane_resources();
  style_ref default_desktop_style();
  void load_default_color_values();

  class ThemepackLoaderPrivate;
  ThemepackLoaderPrivate *const priv;

  static resource_manager *s_theme_instance;
};
}
#endif
