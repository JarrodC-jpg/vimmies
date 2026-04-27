#include "Editor.h"
#include <QAbstractTextDocumentLayout>
#include <QChar>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <qcolor.h>
#include <qevent.h>
#include <qfloat16.h>
#include <qfont.h>
#include <qminmax.h>
#include <qnumeric.h>
#include <qobject.h>
#include <qscrollbar.h>
#include <qsizepolicy.h>
#include <qtextedit.h>
#include <qvectornd.h>
#include <qwidget.h>

Editor::Editor(QWidget *parent) : QTextEdit(parent) {
  m_lineNumberArea = new LineNumberArea(this, this);

  QFont editorFont("JetBrainsMonoNL Nerd Font", 12);
  editorFont.setStyleHint(QFont::Monospace);
  setFont(editorFont);

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

  QFont gutterFont("JetBrainsMonoNL Nerd Font", 12);
  gutterFont.setStyleHint(QFont::Monospace);
  painter.setFont(gutterFont);

  QTextBlock block = document()->begin();
  int blockNumber = 0;
  int top, bottom;

  while (block.isValid()) {
    // Get pixel positoin of this block on screen
    QRectF blockRect = document()->documentLayout()->blockBoundingRect(block);
    top = qRound(blockRect.translated(0, -verticalScrollBar()->value()).top());
    bottom = top + qRound(blockRect.height());

    if (top <= event->rect().bottom() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);

      painter.setPen(QColor("#6c7086"));
      painter.drawText(0, top, m_lineNumberArea->width() - 4,
                       fontMetrics().height(), Qt::AlignRight, number);
    }
    if (top > event->rect().bottom()) {
      break;
    }
    block = block.next();
    ++blockNumber;
  }
}
