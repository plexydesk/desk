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

#include <config.h>

#include <QCoreApplication>
#include <QDir>
#include <QHash>
#include <QtDebug>
#include "ck_config.h"
#include <ck_widget.h>
#include <QDesktopWidget>

#ifdef Q_OS_MAC
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <ck_sandbox.h>

namespace cherry_kit {

class config::Private {
public:
  Private() {}
  ~Private() {}
};

config *config::g_config = 0;

config *config::instance() {
  if (g_config == 0) {
    g_config = new config();
    return g_config;
  } else {
    return g_config;
  }
}

config::~config() { delete d; }
QString config::prefix() {
#ifdef Q_OS_WIN
  QDir binaryPath(QCoreApplication::applicationDirPath());
  return QDir::toNativeSeparators(binaryPath.canonicalPath() + "/lib/");
#endif

#ifdef Q_OS_LINUX
  QString basePath(qgetenv("PLEXYDESK_DIR"));
  if (basePath.isEmpty() || basePath.isNull()) {
    return CK_PLUGIN_PREFIX;
  }

  return basePath;
#endif

#ifdef __APPLE__
  CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  CFStringRef macPath =
      CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
  const char *pathPtr =
      CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
  qDebug() << Q_FUNC_INFO << QString(pathPtr);

  QLatin1String _prefix = QLatin1String(pathPtr);
  CFRelease(appUrlRef);
  CFRelease(macPath);

  return _prefix + QString("/Contents/");
#endif

  return QString();
}

QString config::cache_dir(const QString &a_folder) {
  QString rv = QDir::toNativeSeparators(ck_sandbox_root() + "/" +
                                        ".socialkit/cache/" + a_folder);
  QDir(ck_sandbox_root()).mkpath(rv);
  return rv;
}

std::string config::icon_resource_prefix_path()
{
  QString prefix_path = PLEXYPREFIX;
  QString rv = prefix_path;

#ifdef __APPLE__
  CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
  CFStringRef macPath =
      CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
  const char *pathPtr =
      CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
  qDebug() << Q_FUNC_INFO << QString(pathPtr);

  QLatin1String _prefix = QLatin1String(pathPtr);
  CFRelease(appUrlRef);
  CFRelease(macPath);

  rv = _prefix + "/Contents/Resources/icons/";
  qDebug() << Q_FUNC_INFO << "App Prefix : " << _prefix;
  qDebug() << Q_FUNC_INFO << rv;
#endif

#ifdef Q_OS_LINUX
  rv = prefix_path + "/share/icons/plexydesk/resources/icons/";
#endif

#ifdef Q_OS_WIN
  //rv = QDir::toNativeSeparators(prefix_path + "/resources/icons/");
  QDir binaryPath(QCoreApplication::applicationDirPath());
  rv = QDir::toNativeSeparators(binaryPath.canonicalPath() + "/resources/icons");
#endif
  return rv.toStdString();
}

config::config() : d(new Private) {}
}
