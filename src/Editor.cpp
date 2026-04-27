#include "Editor.h"
#include <QChar>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <qcolor.h>
#include <qevent.h>
#include <qminmax.h>
#include <qobject.h>
#include <qscrollbar.h>
#include <qsizepolicy.h>
#include <qtextedit.h>
#include <qvectornd.h>
#include <qwidget.h>

Editor::Editor(QWidget *parent) : QTextEdit(parent) {
  m_lineNumberArea = new LineNumberArea(this, this);

  connect(document(), &QTextDocument::blockCountChanged, this,
          &Editor::updateLineNumberAreaWidth);

  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
          [this](int) { m_lineNumberArea->update(); });

  connect(this, &QTextEdit::textChanged, this,
          [this]() { m_lineNumberArea->update(); });

  updateLineNumberAreaWidth(0);
}

int Editor::lineNumberAreaWidth() const {
  int digits = 1;
  int max = qMax(1, document()->blockCount());

  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int charWidth = fontMetrics().horizontalAdvance(QLatin1Char('9'));
  return charWidth * digits + 8;
}

void Editor::updateLineNumberAreaWidth(int) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy) {
    m_lineNumberArea->scroll(0, dy);
  } else {
    m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(),
                             rect.height());
  }
}

void Editor::resizeEvent(QResizeEvent *event) {
  QTextEdit::resizeEvent(event);
  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(m_lineNumberArea);
  painter.fillRect(event->rect(), QColor("#1e1e2e"));
}
