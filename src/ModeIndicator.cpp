#include "ModeIndicator.h"
#include "AppTheme.h"
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

ModeIndicator::ModeIndicator(const AppTheme &c, QWidget *parent)
    : QWidget(parent), m_colors(c), m_mode(Mode::Normal) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}
void ModeIndicator::setMode(Mode mode) {
  m_mode = mode;
  update();
}
void ModeIndicator::setAppFont(const QString &family, int size) {
  m_fontFamily = family;
  m_fontSize = size + k_textPixelSizeCompensation;
  QFont f(m_fontFamily);
  f.setPixelSize(m_fontSize);
  QFontMetrics fm(f);
  m_labelPadding = fm.horizontalAdvance(QString(" "));
  int nerdFontBuffer = qMax(2, m_fontSize / 8);
  m_arrowWidth = QFontMetrics(f).tightBoundingRect(QString("")).width();
  update();
}
QSize ModeIndicator::sizeHint() const {
  QFont f(m_fontFamily);
  f.setPixelSize(m_fontSize);
  QFontMetrics fm(f);
  int h = fm.height();
  int w = m_labelPadding + fm.horizontalAdvance(m_modeText.current) +
          m_labelPadding + m_arrowWidth;
  return QSize(w, h);
}
QColor ModeIndicator::modeColor() const {
  switch (m_mode) {
  case Mode::Normal:
    return QColor(m_colors.mode.normalBg);
  case Mode::Insert:
    return QColor(m_colors.mode.insertBg);
  }
  return QColor(m_colors.mode.insertBg);
}
void ModeIndicator::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::TextAntialiasing);

  QColor bg = modeColor();
  QColor dark(m_colors.mode.textColor);
  QColor arrowBg(m_colors.cmd.bg);

  int labelWidth = width() - m_arrowWidth;

  p.fillRect(0, 0, labelWidth, height(), bg);

  p.fillRect(labelWidth, 0, m_arrowWidth, height(), arrowBg);

  QFont arrowFont(m_fontFamily, m_fontSize);
  arrowFont.setPixelSize(m_fontSize);
  p.setFont(arrowFont);
  p.setPen(bg);

  p.drawText(labelWidth, 0, m_arrowWidth, height(),
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
  p.drawText(m_labelPadding, textY, label);
  // p.drawText(0, 0, labelWidth, height(), Qt::AlignVCenter | Qt::AlignHCenter,
  //            label);
}
