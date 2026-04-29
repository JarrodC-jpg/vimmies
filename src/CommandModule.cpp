#include "CommandModule.h"
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QPainter>
#include <qboxlayout.h>
#include <qcolor.h>
#include <qevent.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qlineedit.h>
#include <qminmax.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qwidget.h>

CommandModule::CommandModule(QWidget *parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

  auto *layout = new QHBoxLayout(this);
  layout->setContentsMargins(k_promptWidth + 4, 0, k_arrowWidth + 4, 0);
  layout->setSpacing(0);

  m_lineEdit = new QLineEdit(this);
  m_lineEdit->setFrame(false);
  m_lineEdit->setStyleSheet("QLineEdit {"
                            " background: transparent;"
                            " border: none;"
                            " padding: 0;"
                            " color: #c0caf5;"
                            "}");

  layout->addWidget(m_lineEdit);
}

void CommandModule::setAppFont(const QString &family, int size) {
  m_fontFamily = family;
  m_fontSize = size;

  QFont f(family);
  f.setPixelSize(size);
  m_lineEdit->setFont(f);
}

void CommandModule::activate() {
  m_lineEdit->clear();
  m_lineEdit->setFocus();
  m_lineEdit->setCursorPosition(0);
  updateGeometry();
}

void CommandModule::deactivate() {
  m_lineEdit->clear();
  m_lineEdit->clearFocus();
  updateGeometry();
  emit deactivated();
}

bool CommandModule::isActive() const { return m_lineEdit->hasFocus(); }

QSize CommandModule::sizeHint() const {
  QFont f(m_fontFamily);
  f.setPixelSize(m_fontSize);
  QFontMetrics fm(f);

  int textWidth = fm.horizontalAdvance(m_lineEdit->text());

  int total =
      k_promptWidth + 8 + qMax(textWidth, k_minTextWidth) + k_arrowWidth;

  return QSize(total, height());
}

void CommandModule::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setRenderHint(QPainter::TextAntialiasing);

  QColor bg("#1e2030");
  QColor promptColor("#c3e88d");

  p.fillRect(rect(), bg);

  QFont promptFont(m_fontFamily);
  promptFont.setPixelSize(m_fontSize);
  p.setFont(promptFont);
  p.setPen(promptColor);

  QFontMetrics fm(promptFont);
  int y = (height() + fm.ascent() - fm.descent()) / 2;

  p.drawText(6, y, "$");
}
