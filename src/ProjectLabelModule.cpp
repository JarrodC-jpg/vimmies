#include "ProjectLabelModule.h"
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

ProjectLabelModule::ProjectLabelModule(const AppTheme &c, const QString &p,
                                       QWidget *parent)
    : QWidget(parent), m_colors(c), m_projectName(p) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}
void ProjectLabelModule::setAppFont(const QString &family, int size) {
  m_fontFamily = family;
  m_fontSize = size + k_textPixelSizeCompensation;
  QFont f(m_fontFamily);
  f.setPixelSize(m_fontSize);
  QFontMetrics fm(f);
  m_labelPadding = fm.horizontalAdvance(QString(" "));
  int nerdFontBuffer = qMax(2, m_fontSize / 8);
  m_arrowWidth = QFontMetrics(f).tightBoundingRect(QString("")).width();
  update();
}
QSize ProjectLabelModule::sizeHint() const {
  QFont f(m_fontFamily);
  f.setPixelSize(m_fontSize);
  QFontMetrics fm(f);
  int h = fm.height();
  int w = m_labelPadding + fm.horizontalAdvance(m_projectName) +
          m_labelPadding + m_arrowWidth;
  return QSize(w, h);
}
void ProjectLabelModule::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::TextAntialiasing);

  QColor bg = m_colors.mode.normalBg;
  QColor dark(m_colors.mode.textColor);
  QColor arrowBg(Qt::transparent);

  int labelWidth = width() - m_arrowWidth;

  p.fillRect(0, 0, m_arrowWidth, height(), arrowBg);
  p.fillRect(m_arrowWidth, 0, labelWidth, height(), bg);

  QFont arrowFont(m_fontFamily, m_fontSize);
  arrowFont.setPixelSize(m_fontSize);
  p.setFont(arrowFont);
  p.setPen(bg);

  p.drawText(0, 0, m_arrowWidth, height(), Qt::AlignVCenter | Qt::AlignLeft,
             QString(""));

  QFont labelFont(m_fontFamily, m_fontSize);
  labelFont.setWeight(QFont::Light);
  labelFont.setPixelSize(m_fontSize);
  p.setFont(labelFont);
  p.setPen(dark);

  QFontMetrics fm(labelFont);
  int textY = (height() + fm.ascent() - fm.descent()) / 2;
  p.drawText(m_arrowWidth + m_labelPadding, textY, m_projectName);
}
