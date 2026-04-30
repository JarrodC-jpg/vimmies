#include "CommandModule.h"
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QPainter>
#include <qboxlayout.h>
#include <qcolor.h>
#include <qcoreevent.h>
#include <qevent.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qlineedit.h>
#include <qlogging.h>
#include <qminmax.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qsize.h>
#include <qsizepolicy.h>
#include <qwidget.h>

CommandModule::CommandModule(QWidget *parent) : QWidget(parent) {
  setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

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
  connect(m_lineEdit, &QLineEdit::returnPressed, this,
          [this]() { emit commandSubmitted(); });

  m_lineEdit->installEventFilter(this);
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

QString CommandModule::text() const { return m_lineEdit->text(); }

void CommandModule::setText(const QString &text) {
  m_lineEdit->setText(text);
  m_lineEdit->setCursorPosition(text.length());
  updateGeometry();
}

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

  QColor bg("#3b4261");
  QColor dark("#1e2030");
  QColor promptColor("#c3e88d");

  p.fillRect(0, 0, width() - k_arrowWidth, height(), bg);
  qDebug() << width();

  //  QFont promptFont(m_fontFamily);
  //  promptFont.setPixelSize(m_fontSize);
  //  p.setFont(promptFont);
  // p.setPen(promptColor);

  //  QFontMetrics fm(promptFont);
  //  int y = (height() + fm.ascent() - fm.descent()) / 2;

  //  p.drawText(6, y, "$");
}

bool CommandModule::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_lineEdit && event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      deactivate();
      return true;
    }
  }
  return QWidget::eventFilter(obj, event);
}
