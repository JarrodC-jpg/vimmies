#include "ModeIndicator.h"
#include "MainWindow.h"
#include <QFont>
#include <QPainter>
#include <qcolor.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpainter.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qtypes.h>
#include <qwidget.h>

ModeIndicator::ModeIndicator(QWidget *parent)
    : QWidget(parent), m_mode(Mode::Normal) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}
void ModeIndicator::setMode(Mode mode) {
  m_mode = mode;
  update();
}
void ModeIndicator::setAppFont(const QString &family, int size) {
  m_fontFamily = family;
  m_fontSize = size;
  update();
}
QSize ModeIndicator::sizeHint() const { return QSize(80, height()); }
QColor ModeIndicator::modeColor() const {
  switch (m_mode) {
  case Mode::Normal:
    return QColor("#82aaff");
  case Mode::Insert:
    return QColor("#c3e88d");
  }
  return QColor("#82aaff");
}
void ModeIndicator::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::TextAntialiasing);

  QColor bg = modeColor();
  QColor dark("#1a1b26");
  QColor arrowBg("#3b4261");

  int arrowWidth = 12;
  int labelWidth = width() - arrowWidth;

  p.fillRect(0, 0, labelWidth, height(), bg);

  p.fillRect(labelWidth, 0, arrowWidth, height(), arrowBg);

  QFont arrowFont(m_fontFamily, m_fontSize + 3);
  arrowFont.setPixelSize(m_fontSize);
  p.setFont(arrowFont);
  p.setPen(bg);

  p.drawText(labelWidth - 2, 0, arrowWidth + 2, height(),
             Qt::AlignVCenter | Qt::AlignLeft, QString(QChar(0xe0b0)));

  QFont labelFont(m_fontFamily, m_fontSize);
  labelFont.setWeight(QFont::Light);
  labelFont.setPixelSize(m_fontSize);
  p.setFont(labelFont);
  p.setPen(dark);

  QString label;
  switch (m_mode) {
  case Mode::Normal:
    label = "NORMAL";
    break;
  case Mode::Insert:
    label = "INSERT";
    break;
  }
  QFontMetrics fm(labelFont);
  int textY = (height() + fm.ascent() - fm.descent()) / 2;
  p.drawText(10, textY, label);
  // p.drawText(0, 0, labelWidth, height(), Qt::AlignVCenter | Qt::AlignHCenter,
  //            label);
}
