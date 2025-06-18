#include "stippleviewer.h"

#include <QCoreApplication>
#include <QPrinter>
#include <QSvgGenerator>
#include <QGraphicsItem>

class StippleItem : public QGraphicsItem {
public:
    std::vector<Stipple> stipples;  // Stipple points to render
    double imageWidth;
    double imageHeight;

    // Set the image dimensions, which will be used for scaling
    void setImageDimensions(double width, double height) {
        imageWidth = width;
        imageHeight = height;
    }

    QRectF boundingRect() const override {
        // Calculate bounding rect if needed, for better clipping and optimization
        return QRectF(0, 0, imageWidth, imageHeight);  // Adjust as necessary
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override {
        painter->setPen(Qt::NoPen);

        // Loop over the stipples and render each one
        for (const Stipple& s : stipples) {
            // Apply the coordinate correction based on the image size
            double x = static_cast<double>(s.pos.x() * imageWidth - s.size / 2.0);
            double y = static_cast<double>(s.pos.y() * imageHeight - s.size / 2.0);
            double size = static_cast<double>(s.size);

            painter->setBrush(s.color);
            painter->drawRect(QRectF(x, y, size, size));
        }
    }
};


StippleViewer::StippleViewer(const QImage &img, QWidget *parent)
    : QGraphicsView(parent), m_image(img) {
  setInteractive(false);
  setRenderHint(QPainter::Antialiasing, true);
  setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  setOptimizationFlag(QGraphicsView::DontSavePainterState);
  setFrameStyle(0);
  setAttribute(Qt::WA_TranslucentBackground, false);
  setCacheMode(QGraphicsView::CacheBackground);

  setScene(new QGraphicsScene(this));
  this->scene()->setSceneRect(m_image.rect());
  this->scene()->setItemIndexMethod(QGraphicsScene::NoIndex);
  this->scene()->addPixmap(QPixmap::fromImage(m_image));

  m_stippling = LBGStippling();
  m_stippling.setStatusCallback([this](const auto &status) {
    emit iterationStatus(status.iteration + 1, status.size, status.splits,
                         status.merges, status.hysteresis);
  });

  m_stippling.setStippleCallback(
      [this](const auto &stipples) { displayPoints(stipples); });
}

void StippleViewer::displayPoints(const std::vector<Stipple> &stipples) {
  if(this->draw())
  {
    this->scene()->clear();
    auto item = new StippleItem();
    item->stipples = stipples;
    item->setImageDimensions(m_image.width(), m_image.height());  // Set the image dimensions

    scene()->addItem(item);
    //for (const auto &s : stipples) {
    //  double x = static_cast<double>(s.pos.x() * m_image.width() - s.size / 2.0f);
    //  double y =
    //      static_cast<double>(s.pos.y() * m_image.height() - s.size / 2.0f);
    //  double size = static_cast<double>(s.size);
    //  this->scene()->addEllipse(x, y, size, size, Qt::NoPen, s.color);
    //}
  }
  // TODO: Fix event handling
  QCoreApplication::processEvents();
}

QPixmap StippleViewer::getImage() {
  QPixmap pixmap(this->scene()->sceneRect().size().toSize());
  pixmap.fill(Qt::white);

  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);
  this->scene()->render(&painter);
  return pixmap;
}

void StippleViewer::saveImageSVG(const QString &path) {
  QSvgGenerator generator;
  generator.setFileName(path);
  generator.setSize(m_image.size());
  generator.setViewBox(this->scene()->sceneRect());
  generator.setTitle("Stippling Result");
  generator.setDescription(
      "SVG File created by Weighted Linde-Buzo-Gray Stippling");

  QPainter painter;
  painter.begin(&generator);
  this->scene()->render(&painter);
  painter.end();
}

void StippleViewer::saveImagePDF(const QString &path) {
  adjustSize();

  QPrinter printer(QPrinter::HighResolution);
  printer.setOutputFormat(QPrinter::PdfFormat);
  printer.setCreator("Weighted Linde-Buzo-Gray Stippling");
  printer.setOutputFileName(path);
  QPageSize pageSize(QSizeF(m_image.size()), QPageSize::Point);
  printer.setPageSize(pageSize);
  printer.setFullPage(true);

  QPainter painter(&printer);
  this->render(&painter);
}

void StippleViewer::setInputImage(const QImage &img) {
  m_image = img;
  this->scene()->clear();
  this->scene()->addPixmap(QPixmap::fromImage(m_image));
  this->scene()->setSceneRect(m_image.rect());

  auto w = m_image.width();
  auto h = m_image.height();
  setMinimumSize(w, h);
  adjustSize();

  emit inputImageChanged();
}

void StippleViewer::stipple(const LBGStippling::Params params) {
  // TODO: Handle return value
  m_stippling.stipple(m_image, params);
  emit finished();
}

bool StippleViewer::draw() {
  bool doDraw(false);
  if(m_stippling.draw())
    doDraw = true;
  return(doDraw);
}

void StippleViewer::invert() {
  // TODO: Handle return value
  m_image.invertPixels();
  this->setInputImage(m_image);
}

