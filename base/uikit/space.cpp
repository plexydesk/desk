﻿#include "space.h"

#include <QAction>
#include <QDebug>
#include <QDropEvent>
#include <QGraphicsItem>
#include <datastore.h>
#include <disksyncengine.h>
#include <extensionmanager.h>
#include <syncobject.h>
#include <desktopactivitymenu.h>
#include <QGraphicsScene>
#include <QWeakPointer>
#include <desktopwidget.h>

#include "workspace.h"

namespace PlexyDesk {

typedef QWeakPointer<DesktopActivityMenu> ActivityPoupWeekPtr;

class Space::PrivateSpace {
public:
  PrivateSpace() {}

  ~PrivateSpace();

  QString sessionNameForSpace();

  void initSessionStorage(Space *space);

  QString sessionNameForController(const QString &controllerName);

  void createActionsFromController(const Space *space, ControllerPtr ptr);

public:
  int mID;
  QString mName;
  QRectF m_desktop_rect;
  WorkSpace *mWorkSpace;
  QGraphicsScene *m_main_scene;
  QList<DesktopActivityPtr> m_activity_list;
  std::list<PlexyDesk::Widget *> m_window_list;
  QMap<QString, ControllerPtr> m_current_controller_map;
  QList<ActivityPoupWeekPtr> m_activity_popup_list;
  QList<ControllerPtr> m_controller_list;
};

Space::Space(QObject *parent) : DesktopViewport(parent), d(new PrivateSpace) {
  d->mWorkSpace = 0;
  d->m_main_scene = 0;
  connect(this, SIGNAL(controllerAdded(QString)), this,
          SLOT(onControllerAdded(QString)));
}

Space::~Space() {
  clear();
  delete d;
}

void Space::addController(const QString &controllerName) {
  qDebug() << Q_FUNC_INFO << controllerName << "start";
  if (d->m_current_controller_map.keys().contains(controllerName)) {
    qDebug() << Q_FUNC_INFO << "Has Controller :" << controllerName;
    return;
  }

  QSharedPointer<ViewController> _controller =
      (PlexyDesk::ExtensionManager::instance()->controller(controllerName));

  if (!_controller.data()) {
    qWarning() << Q_FUNC_INFO << "Error loading extension" << controllerName;
    return;
  }

  d->m_current_controller_map[controllerName] = _controller;

  _controller->setViewport(this);
  _controller->setControllerName(controllerName);
  _controller->init();

  d->createActionsFromController(this, _controller);

  Q_EMIT controllerAdded(controllerName);
}

DesktopActivityPtr Space::createActivity(const QString &activity,
                                         const QString &title,
                                         const QPointF &pos, const QRectF &rect,
                                         const QVariantMap &dataItem) {
  PlexyDesk::DesktopActivityPtr intent =
      PlexyDesk::ExtensionManager::instance()->activity(activity);

  if (!intent) {
    qWarning() << Q_FUNC_INFO << "No such Activity: " << activity;
    return DesktopActivityPtr();
  }

  intent->setActivityAttribute("data", QVariant(dataItem));

  intent->createWindow(rect, title, QPointF(pos.x(), pos.y()));

  addActivity(intent);

  /// d->m_activity_map[activity] = intent;
  d->m_activity_list << intent;

  return intent;
}

void Space::updateSessionValue(const QString &controllerName,
                               const QString &key, const QString &value) {
  if (key.isEmpty())
    return;

  QuetzalKit::DataStore *_data_store = new QuetzalKit::DataStore(
      d->sessionNameForController(controllerName), this);
  QuetzalKit::DiskSyncEngine *engine =
      new QuetzalKit::DiskSyncEngine(_data_store);

  _data_store->setSyncEngine(engine);

  QuetzalKit::SyncObject *_session_list_ptr = _data_store->begin("SessionData");

  if (!_session_list_ptr) {
    return;
  }

  if (!_session_list_ptr->hasChildren()) {
    QuetzalKit::SyncObject *_session_object_ptr =
        _session_list_ptr->createNewObject("Session");

    _session_object_ptr->setObjectAttribute(key, value);
    _data_store->insert(_session_object_ptr);
    delete _data_store;
  } else {
    Q_FOREACH(QuetzalKit::SyncObject * _child_session_object,
              _session_list_ptr->childObjects()) {
      if (!_child_session_object)
        continue;
      _child_session_object->setObjectAttribute(key, value);
      _data_store->updateNode(_child_session_object);
    }

    delete _data_store;
  }
}

void Space::addActivity(DesktopActivityPtr activity) {
  if (!activity)
    return;

  connect(activity.data(), SIGNAL(finished()), this,
          SLOT(onActivityFinished()));

  if (activity->window() && d->m_main_scene) {
    if (d->m_activity_list.contains(activity)) {
      // qWarning() << Q_FUNC_INFO << "Space already contains the activity";
      return;
    }
    qDebug() << Q_FUNC_INFO << "Before Adding: " << d->m_activity_list.count();
    d->m_activity_list << activity;

    qDebug() << Q_FUNC_INFO << "After Adding: " << d->m_activity_list.count();
    activity->setViewport(this);
    d->m_main_scene->addItem(activity->window());
  }
}

void Space::addActivityPoupToView(QSharedPointer<DesktopActivityMenu> menu) {
  d->m_activity_popup_list.append(menu.toWeakRef());
}

void Space::addWidgetToView(Widget *widget) {
  if (!widget)
    return;

  if (!d->m_main_scene) {
    qWarning() << Q_FUNC_INFO << "Scene not Set";
    return;
  }

  QPointF _center_of_space_location = this->geometry().center();
  QPoint _widget_location;

  _widget_location.setX(_center_of_space_location.x() -
                        widget->boundingRect().width() / 2);
  _widget_location.setY(_center_of_space_location.y() -
                        widget->boundingRect().height() / 2);

  d->m_main_scene->addItem(widget);
  widget->setPos(_widget_location);

  connect(widget, SIGNAL(closed(PlexyDesk::Widget *)), this,
          SLOT(onWidgetClosed(PlexyDesk::Widget *)));

  widget->show();

  if (widget->controller()) {
    widget->controller()->setViewRect(d->m_desktop_rect);
  }

  d->m_window_list.push_front(widget);
}

void Space::onWidgetClosed(Widget *widget) {
  qDebug() << Q_FUNC_INFO << __LINE__;
  if (!d->m_main_scene)
    return;

  if (!widget)
    return;

  if (widget->controller()) {
    if (widget->controller()->removeWidget(widget))
      return;
  }

  d->m_main_scene->removeItem(widget);

  qDebug() << Q_FUNC_INFO << "Before :" << d->m_window_list.size();
  d->m_window_list.remove(widget);
  qDebug() << Q_FUNC_INFO << "After:" << d->m_window_list.size();

  if (widget)
    delete widget;
}

void Space::onActivityFinished() {
  qDebug() << Q_FUNC_INFO;
  DesktopActivity *activity = qobject_cast<DesktopActivity *>(sender());

  if (activity) {

    if (activity->controller()) {
      activity->controller()->requestAction(
          activity->result()["action"].toString(), activity->result());
    }

    int i = 0;
    foreach(DesktopActivityPtr _activity, d->m_activity_list) {
      if (_activity.data() == activity) {
        _activity.clear();
        qDebug() << Q_FUNC_INFO << "Before :" << d->m_activity_list.count();
        d->m_activity_list.removeAt(i);
        d->m_activity_list.removeAll(_activity);
        qDebug() << Q_FUNC_INFO << "After :" << d->m_activity_list.count();
      }
      i++;
    }
  }
}

void Space::onControllerAdded(const QString &controllerName) {
  qDebug() << Q_FUNC_INFO << "Controller :" << controllerName;
  QuetzalKit::DataStore *_data_store =
      new QuetzalKit::DataStore(d->sessionNameForSpace(), this);
  QuetzalKit::DiskSyncEngine *_engine =
      new QuetzalKit::DiskSyncEngine(_data_store);

  _data_store->setSyncEngine(_engine);

  QuetzalKit::SyncObject *_session_list_ptr =
      _data_store->begin("ControllerList");

  bool _has_controller = false;

  Q_FOREACH(QuetzalKit::SyncObject * _child_object_ptr,
            _session_list_ptr->childObjects()) {
    if (!_child_object_ptr)
      continue;

    if (_child_object_ptr->attributeValue("name").toString() == controllerName)
      _has_controller = true;
  }

  if (!_has_controller) {
    QuetzalKit::SyncObject *_controller_ptr =
        _session_list_ptr->createNewObject("Controller");

    _controller_ptr->setObjectAttribute("name", controllerName);
    _data_store->insert(_controller_ptr);
  }

  delete _data_store;

  // create a new data  Store to link it to this store.
  QuetzalKit::DataStore *_linked_sub_store = new QuetzalKit::DataStore(
      d->sessionNameForController(controllerName), this);
  _engine = new QuetzalKit::DiskSyncEngine(_linked_sub_store);
  _linked_sub_store->setSyncEngine(_engine);

  if (!_linked_sub_store->beginsWith("SessionData")) {
    // create blank root
    QuetzalKit::SyncObject *_attribute_list =
        _linked_sub_store->begin("SessionData");
    delete _linked_sub_store;
    return;
  }

  QVariantMap _session_data;
  QuetzalKit::SyncObject *_session_attribute_list =
      _linked_sub_store->begin("SessionData");

  qDebug() << Q_FUNC_INFO
           << "Child Count:" << _session_attribute_list->childCount();

  Q_FOREACH(QuetzalKit::SyncObject * _session_ptr,
            _session_attribute_list->childObjects()) {
    if (!_session_ptr)
      continue;

    Q_FOREACH(const QString & attribute, _session_ptr->attributes()) {
      _session_data[attribute] =
          _session_ptr->attributeValue(attribute).toString();
    }
  }

  if (controller(controllerName))
    controller(controllerName)->revokeSession(_session_data);

  delete _linked_sub_store;
}

void Space::PrivateSpace::createActionsFromController(const Space *space,
                                                      ControllerPtr ptr) {
  Q_FOREACH(const QAction * action, ptr->actions()) {
    qDebug() << action->text();
    qDebug() << action->icon();
  }
}

void Space::PrivateSpace::initSessionStorage(Space *space) {
  QuetzalKit::DataStore *_data_store =
      new QuetzalKit::DataStore(sessionNameForSpace(), space);
  QuetzalKit::DiskSyncEngine *_engine =
      new QuetzalKit::DiskSyncEngine(_data_store);

  _data_store->setSyncEngine(_engine);

  QuetzalKit::SyncObject *_session_list_ptr =
      _data_store->begin("ControllerList");

  if (_session_list_ptr) {
    Q_FOREACH(const QuetzalKit::SyncObject * _object,
              _session_list_ptr->childObjects()) {
      if (!_object)
        continue;

      QString _current_controller_name =
          _object->attributeValue("name").toString();

      ControllerPtr _controller_ptr =
          space->controller(_current_controller_name);

      if (!_controller_ptr) {
        space->addController(_current_controller_name);
        continue;
      }
    }
  }

  delete _data_store;
}

Space::PrivateSpace::~PrivateSpace() { m_current_controller_map.clear(); }

QString Space::PrivateSpace::sessionNameForSpace() {
  return QString("%1_Space_%2").arg(mName).arg(mID);
}

QString Space::PrivateSpace::sessionNameForController(
    const QString &controllerName) {
  return QString("%1_Controller_%2").arg(sessionNameForSpace()).arg(
      controllerName);
}

QString Space::sessionName() const { return d->sessionNameForSpace(); }

void Space::clear() {
  int i = 0;
  foreach(DesktopActivityPtr _activity, d->m_activity_list) {
    qDebug() << Q_FUNC_INFO << "Remove Activity: ";
    if (_activity) {
      Widget *_widget = _activity->window();
      if (_widget) {
        if (d->m_main_scene->items().contains(_widget)) {
          d->m_main_scene->removeItem(_widget);
          delete _widget;
        }
      }

      d->m_activity_list.removeAt(i);
    }
    i++;
  }

  // delete owner widgets
  for (Widget *_widget : d->m_window_list) {
    if (_widget) {
      if (d->m_main_scene->items().contains(_widget)) {
        d->m_main_scene->removeItem(_widget);
      }
      delete _widget;
      qDebug() << Q_FUNC_INFO << "Widget Deleted: OK";
    }
  }

  d->m_window_list.clear();
  // remove spaces which belongs to the space.
  foreach(const QString & _key, d->m_current_controller_map.keys()) {
    qDebug() << Q_FUNC_INFO << _key;
    ControllerPtr _controller_ptr = d->m_current_controller_map[_key];
    _controller_ptr->prepareRemoval();
    qDebug() << Q_FUNC_INFO
             << "Before Removal:" << d->m_current_controller_map.count();
    d->m_current_controller_map.remove(_key);
    qDebug() << Q_FUNC_INFO
             << "After Removal:" << d->m_current_controller_map.count();
  }

  d->m_current_controller_map.clear();
}

void Space::closeMenu() {
  Q_FOREACH(ActivityPoupWeekPtr popup, d->m_activity_popup_list) {
    QSharedPointer<DesktopActivityMenu> _strong_ptr = popup.toStrongRef();
    if (_strong_ptr) {
      _strong_ptr->hide();
    } else {
      // activity is deleted so we remove from the list
      qDebug() << Q_FUNC_INFO << "Error no ref";
      d->m_activity_popup_list.removeAll(popup);
    }
  }
}

QPointF Space::clickLocation() const {
  if (!d->mWorkSpace)
    return QCursor::pos();

  QGraphicsView *_view_parent = qobject_cast<QGraphicsView *>(d->mWorkSpace);

  if (!_view_parent)
    return QCursor::pos();

  return _view_parent->mapToScene(QCursor::pos());
}

ControllerPtr Space::controller(const QString &name) {
  if (!d->m_current_controller_map.keys().contains(name))
    return ControllerPtr();

  return d->m_current_controller_map[name];
}

QStringList Space::currentDesktopControllers() const {
  return d->m_current_controller_map.keys();
}

void Space::setName(const QString &name) { d->mName = name; }

QString Space::name() const { return d->mName; }

void Space::setId(int id) { d->mID = id; }

int Space::id() const { return d->mID; }

void Space::setSpaceGeometry(const QRectF &rectf) {
  d->m_desktop_rect = rectf;

  foreach(DesktopActivityPtr _activity, d->m_activity_list) {
    if (_activity && _activity->window()) {
      QPointF _activity_pos = _activity->window()->pos();

      if (_activity_pos.y() > rectf.height()) {
        _activity_pos.setY(_activity_pos.y() - rectf.height());
        _activity->window()->setPos(_activity_pos);
      }
    }
  }

  foreach(const QString & _key, d->m_current_controller_map.keys()) {
    ControllerPtr _controller_ptr = d->m_current_controller_map[_key];
    _controller_ptr->setViewRect(d->m_desktop_rect);
  }
}

QObject *Space::workspace() { return d->mWorkSpace; }

void Space::setWorkspace(WorkSpace *workspace) { d->mWorkSpace = workspace; }

void Space::restoreSession() { d->initSessionStorage(this); }

void Space::setScene(PlatformGraphicsScene *scene) {
  d->m_main_scene = (QGraphicsScene *)scene;
}

QRectF Space::geometry() const { return d->m_desktop_rect; }

void Space::handleDropEvent(QDropEvent *event,
                            const QPointF &local_event_location) {
  if (d->m_main_scene) {
    QList<QGraphicsItem *> items = d->m_main_scene->items(local_event_location);

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

      widget->controller()->handleDropEvent(widget, event);
      return;
    }
  }
}
}
