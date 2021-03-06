#ifndef EVENT_BROWSER_UI_H
#define EVENT_BROWSER_UI_H

#include <ck_session_sync.h>

class date_controller;

class event_browser_ui {
public:
  event_browser_ui(cherry_kit::session_sync *a_sessoin,
                   date_controller *a_ctr);
  ~event_browser_ui();

  void insert_event();

  int event_id() const;

  std::function<void ()> update_time_line();

private:
  class evb_context;
  evb_context *const ctx;
};

#endif // EVENT_BROWSER_UI_H
