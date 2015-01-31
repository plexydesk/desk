#include "iconwidgetview.h"

#include <QGraphicsProxyWidget>
#include <QFileSystemModel>
#include <QPropertyAnimation>

#include <tableview.h>
#include <desktopwidget.h>

#include "folderitem.h"
#include "folderprovider.h"
#include "fileinforview.h"

class IconWidgetView::PrivateIconWidgetView {
public:
  PrivateIconWidgetView() {}
  ~PrivateIconWidgetView() { delete mInfoView; }

  FileInforView *mInfoView;
  PlexyDesk::TableView *mTableView;
  FolderProvider *mFolderViewSource;
};

IconWidgetView::IconWidgetView(QGraphicsObject *parent)
    : PlexyDesk::UIWidget(parent), d(new PrivateIconWidgetView) {
  this->setFlag(QGraphicsItem::ItemIsMovable, false);
  this->setWindowFlag(PlexyDesk::UIWidget::kRenderBackground, false);
  this->setWindowFlag(PlexyDesk::UIWidget::kRenderDropShadow, false);

  QRectF rect(0.0, 0.0, 200.0, 200.0);
  QRectF iconViewRect =
      QRectF(12.0, 32.0, rect.width() - 24.0, rect.height() - 64.0);
  QRectF iconViewRectTable =
      QRectF(12.0, 32.0, rect.width() - 24.0, rect.height() - 64.0);
  QRectF infoViewRect = QRectF(0.0, 0.0, rect.width(), 40);

  d->mInfoView = new FileInforView(this);

  d->mTableView = new PlexyDesk::TableView(this);

  d->mFolderViewSource = new FolderProvider(iconViewRectTable, this);
  d->mTableView->setModel(d->mFolderViewSource);

  d->mInfoView->hide();
  d->mInfoView->setZValue(d->mTableView->zValue() + 1);

  d->mInfoView->setSliderPos(
      QPointF(0.0, rect.height() + 124),
      QPointF(0.0, d->mTableView->boundingRect().height() - (105)));

  connect(d->mFolderViewSource, SIGNAL(itemClicked(FolderItem *)), this,
          SLOT(onClicked(FolderItem *)));
}

IconWidgetView::~IconWidgetView() { delete d; }

void IconWidgetView::setDirectoryPath(const QString &path) {
  d->mFolderViewSource->setDirectoryPath(path);
}

void IconWidgetView::onClicked(FolderItem *item) {
  d->mTableView->clearSelection();

  if (item) {
    d->mInfoView->setIcon(item->icon());
    d->mInfoView->setFileInfo(item->fileInfo());
    item->setSelected();
  }

  d->mInfoView->pop();
}

void IconWidgetView::infoViewClicked() { d->mInfoView->push(); }
