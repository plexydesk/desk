#ifndef SCREEN_H
#define SCREEN_H

#include <functional>
#include <vector>
#include <plexydesk_ui_exports.h>

namespace cherry_kit {
typedef enum {
  kScreenCountChange,
  kScreenGeomentyChange,
  kScreenStructureChange,
  kScreenErrorChange
} display_change_notify_t;

typedef std::function<void(display_change_notify_t)>
    display_change_notify_callback_t;

class DECL_UI_KIT_EXPORT screen {
public:
  virtual ~screen();

  static screen *get();

  int screen_count() const;

  float scale_factor(int a_id) const;

  int x_resolution(int a_id) const;
  int y_resolution(int a_id) const;

  void change_notifications(display_change_notify_callback_t a_callback);

  float desktop_height(int a_display_id) const;
  float desktop_width(int a_display_id) const;

protected:
  screen();

private:
  class platform_screen;
  platform_screen* const priv;

  std::vector<display_change_notify_callback_t> m_notify_chain;

  static screen *__self;
};
}

#endif // SCREEN_H
