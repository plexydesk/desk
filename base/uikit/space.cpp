﻿#include "space.h"

#include <QAction>
#include <QDebug>
#include <QDropEvent>
#include <QGraphicsItem>
#include <disksyncengine.h>
#include <extensionmanager.h>
#include <syncobject.h>
#include <QGraphicsScene>
#include <QWeakPointer>
#include <widget.h>
#include <datasync.h>

#include "window.h"
#include "workspace.h"

namespace CherryKit {
#define kMaximumZOrder 10000
#define kMinimumZOrder 100
#define kMediumZOrder 5000

typedef std::function<void(Space::ViewportNotificationType, const QVariant &,
                           const Space *)> NotifyFunc;
class Space::PrivateSpace {
public:
  PrivateSpace() : m_surface(0) {}

  ~PrivateSpace();

  QString sessionNameForSpace();

  void initSessionStorage(Space *space);

  QString sessionNameForController(const QString &controllerName);

  void createActionsFromController(const Space *space, ViewControllerPtr ptr);

public:
  int mID;
  QString mName;
  QRectF m_geometry;
  WorkSpace *mWorkSpace;
  QGraphicsScene *mMainScene;
  QList<CherryKit::DesktopActivityPtr> mActivityList;
  std::vector<CherryKit::Window *> mWindowList;
  QMap<QString, ViewControllerPtr> mCurrentControllerMap;
  QList<ViewControllerPtr> mControllerList;

  QList<NotifyFunc> m_notify_chain;

  // experiment
  unsigned char *m_surface;
};

Space::Space() : o_space(new PrivateSpace) {
  o_space->mWorkSpace = 0;
  o_space->mMainScene = 0;
}

Space::~Space() {
  clear();
  delete o_space;
}

void Space::add_controller(const QString &a_name) {
  if (o_space->mCurrentControllerMap.keys().contains(a_name)) {
    return;
  }

  QSharedPointer<ViewController> controllerPtr =
      (CherryKit::ExtensionManager::instance()->controller(a_name));

  if (!controllerPtr) {
    qWarning() << Q_FUNC_INFO << "Error loading extension" << a_name;
    return;
  }

  o_space->mCurrentControllerMap[a_name] = controllerPtr;

  controllerPtr->set_viewport(this);
  controllerPtr->set_controller_name(a_name);

  controllerPtr->init();
  controllerPtr->set_view_rect(geometry());

  o_space->createActionsFromController(this, controllerPtr);

  register_controller(a_name);

  foreach(NotifyFunc l_notify_handler, o_space->m_notify_chain) {
    if (l_notify_handler)
      l_notify_handler(Space::kControllerAddedNotification, QVariant(a_name),
                       this);
  }
}

CherryKit::DesktopActivityPtr
Space::create_activity(const QString &a_activity, const QString &a_title,
                       const QPointF &a_pos, const QRectF &a_rect,
                       const QVariantMap &a_data_map) {
  CherryKit::DesktopActivityPtr intent =
      CherryKit::ExtensionManager::instance()->activity(a_activity);

  if (!intent) {
    qWarning() << Q_FUNC_INFO << "No such Activity: " << a_activity;
    return CherryKit::DesktopActivityPtr();
  }

  intent->set_activity_attribute("data", QVariant(a_data_map));

  intent->create_window(a_rect, a_title, QPointF(a_pos.x(), a_pos.y()));

  if (intent->window()) {
    intent->window()->set_window_title(a_title);
    intent->window()->set_window_viewport(this);
    intent->window()->setPos(a_pos);
  }

  add_activity(intent);

  return intent;
}

void Space::update_session_value(const QString &a_controller_name,
                                 const QString &a_key, const QString &a_value) {
  QuetzalKit::DataSync *sync = new QuetzalKit::DataSync(
      o_space->sessionNameForController(a_controller_name).toStdString());
  QuetzalKit::DiskSyncEngine *engine = new QuetzalKit::DiskSyncEngine();

  sync->set_sync_engine(engine);

  sync->on_object_found([&](QuetzalKit::SyncObject &a_object,
                            const std::string &a_app_name, bool a_found) {
    if (!a_found) {
      QuetzalKit::SyncObject obj;
      obj.setName("AppSession");
      obj.setObjectAttribute(
          "name", o_space->sessionNameForController(a_controller_name));

      sync->add_object(obj);
    }

    controller(a_controller_name)->submit_session_data(&a_object);
    a_object.sync();
  });

  sync->find("AppSession", "", "");

  delete sync;
}

void Space::add_activity(CherryKit::DesktopActivityPtr a_activity_ptr) {
  if (!a_activity_ptr || !a_activity_ptr->window()) {
    return;
  }

  a_activity_ptr->on_discarded([&](const DesktopActivity *a_activity) {
    on_activity_finished(a_activity);
  });

  a_activity_ptr->window()->on_window_discarded([=](Window *a_window) {
    a_activity_ptr->discard_activity();
  });

  if (a_activity_ptr->window() && o_space->mMainScene) {
    if (o_space->mActivityList.contains(a_activity_ptr)) {
      qWarning() << Q_FUNC_INFO << "Space already contains the activity";
      return;
    }

    o_space->mActivityList << a_activity_ptr;

    a_activity_ptr->set_viewport(this);

    insert_window_to_view(a_activity_ptr->window());
  }
}

void Space::insert_window_to_view(Window *a_window) {
  if (!a_window) {
    return;
  }

  if (!o_space->mMainScene) {
    qWarning() << Q_FUNC_INFO << "Scene not Set";
    return;
  }

  QPointF _center_of_space_location = this->geometry().center();
  QPoint _widget_location;

  _widget_location.setX(_center_of_space_location.x() -
                        a_window->boundingRect().width() / 2);
  _widget_location.setY(_center_of_space_location.y() -
                        a_window->boundingRect().height() / 2);

  o_space->mMainScene->addItem(a_window);
  if (a_window->window_type() == Window::kApplicationWindow)
    a_window->setZValue(kMaximumZOrder);

  if (a_window->controller()) {
    a_window->controller()->set_view_rect(o_space->m_geometry);
  }

  o_space->mWindowList.push_back(a_window);

  a_window->on_update([this](const Widget *window) {
    qDebug() << Q_FUNC_INFO << "Got Update Request";
  });

  a_window->on_window_closed([this](Window *window) {
    qDebug() << Q_FUNC_INFO << "Request Window Removal from Space";
    remove_window_from_view(window);
  });

  a_window->on_window_focused([this](Window *a_window) {
    if (!a_window) {
      return;
    }

    std::for_each(std::begin(o_space->mWindowList),
                  std::end(o_space->mWindowList), [&](Window *a_win) {
      if (a_win->window_type() == Window::kFramelessWindow) {
        a_win->setZValue(kMinimumZOrder);
      }

      if (a_win->window_type() == Window::kPanelWindow) {
        a_win->setZValue(kMaximumZOrder + 2);
      }

      if (a_win->window_type() == Window::kPopupWindow) {
        a_win->setZValue(kMaximumZOrder + 1);
        if (a_win != a_window)
          a_win->hide();
      }

      if (a_win->window_type() == Window::kApplicationWindow) {
        if (a_win->zValue() == kMaximumZOrder && a_win != a_window)
          a_win->setZValue((kMaximumZOrder - 1));
      }
    });

    if (a_window->window_type() == Window::kApplicationWindow)
      a_window->setZValue(kMaximumZOrder);
  });

  a_window->raise();
  a_window->show();
}

void Space::remove_window_from_view(Window *a_window) {
  if (!o_space->mMainScene) {
    return;
  }

  if (!a_window) {
    return;
  }

  if (a_window->controller()) {
    if (a_window->controller()->remove_widget(a_window)) {
      return;
    }
  }

  o_space->mMainScene->removeItem(a_window);

  qDebug() << Q_FUNC_INFO << "Before :" << o_space->mWindowList.size();
  o_space->mWindowList.erase(std::remove(o_space->mWindowList.begin(),
                                         o_space->mWindowList.end(), a_window),
                             o_space->mWindowList.end());
  qDebug() << Q_FUNC_INFO << "After:" << o_space->mWindowList.size();

  a_window->discard();
}

void Space::on_viewport_event_notify(
    std::function<void(ViewportNotificationType, const QVariant &,
                       const Space *)> a_notify_handler) {
  o_space->m_notify_chain.append(a_notify_handler);
}

void Space::on_activity_finished(const DesktopActivity *a_activity) {
  if (a_activity) {
    int i = 0;
    foreach(DesktopActivityPtr _activity, o_space->mActivityList) {
      // todo : enable runtime identification of activities.
      if (_activity.data() == a_activity) {
        _activity.clear();
        qDebug() << Q_FUNC_INFO << "Before :" << o_space->mActivityList.count();
        o_space->mActivityList.removeAt(i);
        o_space->mActivityList.removeAll(_activity);
        qDebug() << Q_FUNC_INFO << "After :" << o_space->mActivityList.count();
      }
      i++;
    }
  }
}

int qtz_read_color_value_at_pos(GraphicsSurface *surface, int width, int x,
                                int y, int comp) {

  if (!(*surface))
    return -1;

  return (*surface)[((y * width + x) * 4) + comp];
}

void qtz_set_color_value_at_pos(GraphicsSurface *surface, int width, int x,
                                int y, int red, int green, int blue,
                                int alpha) {

  if (!(*surface))
    return;

  (*surface)[((y * width + x) * 4)] = red;
  (*surface)[((y * width + x) * 4) + 1] = green;
  (*surface)[((y * width + x) * 4) + 2] = blue;
  (*surface)[((y * width + x) * 4) + 3] = alpha;
}

void Space::draw() {
  // todo: Move this expensive loop out of here.
  std::sort(
      std::begin(o_space->mWindowList), std::end(o_space->mWindowList),
      [&](Window *a_a, Window *a_b) { return a_a->zValue() < a_b->zValue(); });

  std::for_each(std::begin(o_space->mWindowList),
                std::end(o_space->mWindowList), [&](Window *a_win) {

    if (!a_win)
      return;

    if (!a_win->isVisible())
      return;

    a_win->draw();

    qDebug() << Q_FUNC_INFO << " Z Index" << a_win->zValue()
             << " TITLE : " << a_win->window_title()
             << " Geometry: " << a_win->geometry()
             << " Bounding Rect : " << a_win->geometry()
             << " (x : " << a_win->x() << " (y :)  " << a_win->y();

    GraphicsSurface *window_surface = a_win->surface();

    if (!(*window_surface))
      return;

    for (int w = 0; w < geometry().width(); w++) {
      for (int h = 0; h < geometry().height(); h++) {
        // qDebug() << Q_FUNC_INFO << "(" << w << "," << h << ")";
        if (w >= a_win->geometry().width())
          continue;
        if (h >= a_win->geometry().height())
          continue;

        int red = qtz_read_color_value_at_pos(
            window_surface, a_win->geometry().width(), w, h, 0);
        int green = qtz_read_color_value_at_pos(
            window_surface, a_win->geometry().width(), w, h, 1);
        int blue = qtz_read_color_value_at_pos(
            window_surface, a_win->geometry().width(), w, h, 2);
        int alpha = qtz_read_color_value_at_pos(
            window_surface, a_win->geometry().width(), w, h, 3);

        if (alpha < 255)
          continue;

        qtz_set_color_value_at_pos(&o_space->m_surface, geometry().width(),
                                   (a_win->x() + w), (a_win->y() + h), red,
                                   green, blue, alpha);
      }
    }
  });
}

GraphicsSurface *Space::surface() { return &o_space->m_surface; }

void Space::save_controller_to_session(const QString &a_controller_name) {
  QuetzalKit::DataSync *sync =
      new QuetzalKit::DataSync(o_space->sessionNameForSpace().toStdString());
  QuetzalKit::DiskSyncEngine *engine = new QuetzalKit::DiskSyncEngine();

  sync->set_sync_engine(engine);

  sync->on_object_found([&](QuetzalKit::SyncObject &a_object,
                            const std::string &a_app_name, bool a_found) {
    if (!a_found) {
      QuetzalKit::SyncObject obj;
      obj.setName("Controller");
      obj.setObjectAttribute("name", a_controller_name);

      sync->add_object(obj);
    }
  });

  sync->find("Controller", "name", a_controller_name.toStdString());

  delete sync;
}

void
Space::revoke_controller_session_attributes(const QString &a_controller_name) {
  QuetzalKit::DataSync *sync = new QuetzalKit::DataSync(
      o_space->sessionNameForController(a_controller_name).toStdString());
  QuetzalKit::DiskSyncEngine *engine = new QuetzalKit::DiskSyncEngine();

  sync->set_sync_engine(engine);

  sync->on_object_found([&](QuetzalKit::SyncObject &a_object,
                            const std::string &a_app_name, bool a_found) {
    qDebug() << Q_FUNC_INFO << "Restore Session For Controllers"
             << a_controller_name;

    a_object.setObjectAttribute(
        "name", o_space->sessionNameForController(a_controller_name));
    if (!a_found) {
      a_object.setName("AppSession");
      sync->add_object(a_object);
    }

    if (controller(a_controller_name)) {
      controller(a_controller_name)->session_data_available(a_object);
    }
  });

  sync->find("AppSession", "", "");

  delete sync;
}

void Space::register_controller(const QString &a_controller_name) {
  save_controller_to_session(a_controller_name);
  revoke_controller_session_attributes(a_controller_name);
}

void Space::PrivateSpace::createActionsFromController(const Space *space,
                                                      ViewControllerPtr ptr) {
  Q_FOREACH(const QAction * action, ptr->actions()) {
    // qDebug() << action->text();
    // qDebug() << action->icon();
  }
}

void Space::PrivateSpace::initSessionStorage(Space *space) {
  QuetzalKit::DataSync *sync =
      new QuetzalKit::DataSync(sessionNameForSpace().toStdString());
  QuetzalKit::DiskSyncEngine *engine = new QuetzalKit::DiskSyncEngine();
  sync->set_sync_engine(engine);

  sync->on_object_found([&](QuetzalKit::SyncObject &a_object,
                            const std::string &a_app_name, bool a_found) {
    if (a_found) {
      QString _current_controller_name =
          a_object.attributeValue("name").toString();

      ViewControllerPtr _controller_ptr =
          space->controller(_current_controller_name);

      if (!_controller_ptr) {
        space->add_controller(_current_controller_name);
      }
    }
  });

  sync->find("Controller", "", "");

  delete sync;
}

Space::PrivateSpace::~PrivateSpace() { mCurrentControllerMap.clear(); }

QString Space::PrivateSpace::sessionNameForSpace() {
  return QString("%1_%2_Space_%3")
      .arg(QString::fromStdString(mWorkSpace->workspace_instance_name()))
      .arg(mName)
      .arg(mID);
}

QString
Space::PrivateSpace::sessionNameForController(const QString &controllerName) {
  return QString("%1_Controller_%2").arg(sessionNameForSpace()).arg(
      controllerName);
}

QString Space::session_name() const { return o_space->sessionNameForSpace(); }

QString Space::session_name_for_controller(const QString &a_controller_name) {
  return o_space->sessionNameForController(a_controller_name);
}

void Space::clear() {
  int i = 0;
  foreach(DesktopActivityPtr _activity, o_space->mActivityList) {
    qDebug() << Q_FUNC_INFO << "Remove Activity: ";
    if (_activity) {
      o_space->mActivityList.removeAt(i);
    }
    i++;
  }

  // delete owner widgets
  for (Window *_widget : o_space->mWindowList) {
    if (_widget) {
      if (o_space->mMainScene->items().contains(_widget)) {
        o_space->mMainScene->removeItem(_widget);
        _widget->discard();
      }
      qDebug() << Q_FUNC_INFO << "Widget Deleted: OK";
    }
  }

  o_space->mWindowList.clear();
  // remove spaces which belongs to the space.
  foreach(const QString & _key, o_space->mCurrentControllerMap.keys()) {
    qDebug() << Q_FUNC_INFO << _key;
    ViewControllerPtr _controller_ptr = o_space->mCurrentControllerMap[_key];
    _controller_ptr->prepare_removal();
    qDebug() << Q_FUNC_INFO
             << "Before Removal:" << o_space->mCurrentControllerMap.count();
    o_space->mCurrentControllerMap.remove(_key);
    qDebug() << Q_FUNC_INFO
             << "After Removal:" << o_space->mCurrentControllerMap.count();
  }

  o_space->mCurrentControllerMap.clear();
}

QPointF Space::cursor_pos() const {
  if (!o_space->mWorkSpace) {
    return QCursor::pos();
  }

  QGraphicsView *_view_parent =
      qobject_cast<QGraphicsView *>(o_space->mWorkSpace);

  if (!_view_parent) {
    return QCursor::pos();
  }

  return _view_parent->mapToScene(QCursor::pos());
}

QPointF Space::center(const QRectF &a_view_geometry,
                      const QRectF a_window_geometry,
                      const ViewportLocation &a_location) const {
  QPointF _rv;
  float _x_location;
  float _y_location;

  switch (a_location) {
  case kCenterOnViewport:
    _y_location = (geometry().height() / 2) - (a_view_geometry.height() / 2);
    _x_location = (geometry().width() / 2) - (a_view_geometry.width() / 2);
    break;
  case kCenterOnViewportLeft:
    _y_location = (geometry().height() / 2) - (a_view_geometry.height() / 2);
    _x_location = (geometry().topLeft().x());
    break;
  case kCenterOnViewportRight:
    _y_location = (geometry().height() / 2) - (a_view_geometry.height() / 2);
    _x_location = (geometry().width() - (a_view_geometry.width()));
    break;
  case kCenterOnViewportTop:
    _y_location = (0.0);
    _x_location = ((geometry().width() / 2) - (a_view_geometry.width() / 2));
    break;
  case kCenterOnViewportBottom:
    _y_location = (geometry().height() - (a_view_geometry.height()));
    _x_location = ((geometry().width() / 2) - (a_view_geometry.width() / 2));
    break;
  case kCenterOnWindow:
    _y_location = (a_window_geometry.height() / 2) -
                  (a_view_geometry.height() / 2) + a_window_geometry.y();
    _x_location = (a_window_geometry.width() / 2) -
                  (a_view_geometry.width() / 2) + a_window_geometry.x();
    break;
  default:
    qWarning() << Q_FUNC_INFO << "Error : Unknown Viewprot Location Type:";
  }

  _rv.setY(geometry().y() + _y_location);
  _rv.setX(_x_location);

  return _rv;
}

ViewControllerPtr Space::controller(const QString &a_name) {
  if (!o_space->mCurrentControllerMap.keys().contains(a_name)) {
    return ViewControllerPtr();
  }

  return o_space->mCurrentControllerMap[a_name];
}

QStringList Space::current_controller_list() const {
  return o_space->mCurrentControllerMap.keys();
}

void Space::set_name(const QString &a_name) { o_space->mName = a_name; }

QString Space::name() const { return o_space->mName; }

void Space::set_id(int a_id) { o_space->mID = a_id; }

int Space::id() const { return o_space->mID; }

void Space::setGeometry(const QRectF &a_geometry) {
  o_space->m_geometry = a_geometry;

  if (!o_space->m_surface) {
    o_space->m_surface =
        (unsigned char *)malloc(4 * a_geometry.width() * a_geometry.height());
    memset(o_space->m_surface, 0, 4 * a_geometry.width() * a_geometry.height());
  } else {
    o_space->m_surface = (unsigned char *)realloc(
        o_space->m_surface, (4 * a_geometry.width() * a_geometry.height()));
  }

  foreach(DesktopActivityPtr _activity, o_space->mActivityList) {
    if (_activity && _activity->window()) {
      QPointF _activity_pos = _activity->window()->pos();

      if (_activity_pos.y() > a_geometry.height()) {
        _activity_pos.setY(_activity_pos.y() - a_geometry.height());
        _activity->window()->setPos(_activity_pos);
      }
    }
  }

  foreach(const QString & _key, o_space->mCurrentControllerMap.keys()) {
    ViewControllerPtr _controller_ptr = o_space->mCurrentControllerMap[_key];
    _controller_ptr->set_view_rect(o_space->m_geometry);
  }
}

QObject *Space::workspace() { return o_space->mWorkSpace; }

void Space::set_workspace(WorkSpace *a_workspace_ptr) {
  o_space->mWorkSpace = a_workspace_ptr;
}

void Space::restore_session() { o_space->initSessionStorage(this); }

void Space::set_qt_graphics_scene(QGraphicsScene *a_qt_graphics_scene_ptr) {
  o_space->mMainScene = a_qt_graphics_scene_ptr;
}

QRectF Space::geometry() const { return o_space->m_geometry; }

void Space::drop_event_handler(QDropEvent *event,
                               const QPointF &local_event_location) {
  if (o_space->mMainScene) {
    QList<QGraphicsItem *> items =
        o_space->mMainScene->items(local_event_location);

    Q_FOREACH(QGraphicsItem * item, items) {
      QGraphicsObject *itemObject = item->toGraphicsObject();

      if (!itemObject) {
        continue;
      }

      Widget *widget = qobject_cast<Widget *>(itemObject);

      if (!widget || !widget->controller()) {
        qDebug() << Q_FUNC_INFO << "Not a Valid Item";
        continue;
      }

      widget->controller()->handle_drop_event(widget, event);
      return;
    }
  }
}
}
