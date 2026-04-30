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
  // layout->setContentsMargins(k_promptWidth + 4, 0, k_arrowWidth + 4, 0);
  layout->setContentsMargins(k_promptWidth + 4, 0, 0, 0);
  layout->setSpacing(0);

  m_lineEdit = new QLineEdit(this);
  m_lineEdit->setContentsMargins(0, 0, 0, 0);
  m_lineEdit->setFrame(false);
  m_lineEdit->setStyleSheet("QLineEdit {"
                            " background: transparent;"
                            " border: none;"
                            " padding-left: 0;"
                            " color: #4fd6be;"
                            "}");
  layout->addWidget(m_lineEdit);
  resize(sizeHint());
  updateGeometry();
  connect(m_lineEdit, &QLineEdit::returnPressed, this,
          [this]() { emit commandSubmitted(); });

  m_lineEdit->installEventFilter(this);
  connect(m_lineEdit, &QLineEdit::textChanged, this, [this]() {
    int textWidth =
        QFontMetrics(m_lineEdit->font()).horizontalAdvance(m_lineEdit->text()) +
        4;
    if (m_lineEdit->hasFocus()) {
      m_lineEdit->setMinimumWidth(textWidth);
    } else {
      qDebug() << "Line edit does not have focus";
      m_lineEdit->setMinimumWidth(0);
    }
    resize(sizeHint());
    updateGeometry();
    update();
  });
}

void CommandModule::setAppFont(const QString &family, int size) {
  m_fontFamily = family;
  m_fontSize = size;

  QFont f(family);
  f.setPixelSize(size + 3);
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

  if (parentWidget() && parentWidget()->layout()) {
    parentWidget()->layout()->activate();
  }
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

  int textWidth =
      QFontMetrics(m_lineEdit->font()).horizontalAdvance(m_lineEdit->text()) +
      2;

  int total = 0;

  if (m_lineEdit->hasFocus()) {
    total = k_promptWidth + k_promptPadding + qMax(textWidth, k_minTextWidth) +
            k_arrowWidth;
  } else {
    total = k_promptWidth + k_promptPadding + k_arrowWidth;
  }
  // k_promptWidth + 8 + textWidth + k_arrowWidth;

  qDebug() << "Width in sizeEvent" << total;
  return QSize(total, 20);
}

void CommandModule::paintEvent(QPaintEvent *) {
  qDebug() << "Width in paintEvent" << width();
  QPainter p(this);
  p.setRenderHint(QPainter::TextAntialiasing);

  QColor bg("#3b4261");
  QColor dark("#1e2030");
  QColor promptColor("#4fd6be");
  p.fillRect(0, 0, width() - k_arrowWidth, height(), bg);

  QFont promptFont(m_fontFamily);
  promptFont.setPixelSize(m_fontSize + 3);
  p.setFont(promptFont);
  p.setPen(promptColor);
  QFontMetrics fm(promptFont);
  int y = (height() + fm.ascent() - fm.descent()) / 2;
  // p.drawText(4, y, "$");
  p.drawText(4, y, "");

  int rightArrowX = width() - k_arrowWidth;
  //  p.fillRect(rightArrowX, 0, k_arrowWidth, height(), dark);
  QFont arrowFont(m_fontFamily, m_fontSize + 3);
  //  arrowFont.setPixelSize(m_fontSize + 5);
  QFontMetrics afm(arrowFont);
  int arrowY = (height() + afm.ascent() - afm.descent()) / 2;
  p.setFont(arrowFont);
  p.setPen(bg);
  p.drawText(rightArrowX, arrowY, QString(QChar(0xe0b0)));
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
