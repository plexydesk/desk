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
#ifndef MEMORYSYNCENGINE_H
#define MEMORYSYNCENGINE_H

#include <ck_sync_engine_interface.h>
#include <QuetzalDataKitQt_export.h>

namespace cherry_kit {

class QuetzalDataKitQt_EXPORT memory_sync_engine : public sync_engine_interface {
  Q_OBJECT

public:
  explicit memory_sync_engine(QObject *a_parent_ptr = 0);

  virtual QString data(const QString &storeName);

  virtual void sync(const QString &datastoreName, const QString &data);

  virtual bool hasLock();

private:
  QString mData;
};
}
#endif // MEMORYSYNCENGINE_H
