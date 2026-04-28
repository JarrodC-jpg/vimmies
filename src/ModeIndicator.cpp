#include "ModeIndicator.h"
#include "MainWindow.h"
#include <QFont>
#include <QPainter>
#include <qcolor.h>
#include <qfont.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qpainter.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qwidget.h>

ModeIndicator::ModeIndicator(QWidget *parent)
    : QWidget(parent), m_mode(Mode::Normal) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
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
QSize ModeIndicator::sizeHint() const { return QSize(90, 24); }
QColor ModeIndicator::modeColor() const {
  switch (m_mode) {
  case Mode::Normal:
    return QColor("#7aa2f7");
  case Mode::Insert:
    return QColor("#9ece6a");
  }
  return QColor("#7aa2f7");
}
void ModeIndicator::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::TextAntialiasing);

  QColor bg = modeColor();
  QColor dark("#1a1b26");

  int arrowWidth = 12;
  int labelWidth = width() - arrowWidth;

  p.fillRect(0, 0, labelWidth, height(), bg);

  p.fillRect(labelWidth, 0, arrowWidth, height(), dark);

  QFont arrowFont(m_fontFamily, m_fontSize + 3);
  p.setFont(arrowFont);
  p.setPen(bg);
  p.drawText(labelWidth - 2, 0, arrowWidth + 2, height(),
             Qt::AlignVCenter | Qt::AlignLeft, QString(QChar(0xe0b0)));

  QFont labelFont(m_fontFamily, m_fontSize - 1);
  labelFont.setBold(true);
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
  p.drawText(0, 0, labelWidth - 4, height(),
             Qt::AlignVCenter | Qt::AlignHCenter, label);
}
