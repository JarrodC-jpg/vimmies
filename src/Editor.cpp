#include "Editor.h"
#include "AppTheme.h"
#include <QAbstractTextDocumentLayout>
#include <QChar>
#include <QGuiApplication>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <qcolor.h>
#include <qevent.h>
#include <qfloat16.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qguiapplication.h>
#include <qlist.h>
#include <qlogging.h>
#include <qminmax.h>
#include <qnamespace.h>
#include <qnumeric.h>
#include <qobject.h>
#include <qscrollbar.h>
#include <qsizepolicy.h>
#include <qtextcursor.h>
#include <qtextedit.h>
#include <qtextformat.h>
#include <qvectornd.h>
#include <qwidget.h>

Editor::Editor(const EditorColors &c, QWidget *parent)
    : QTextEdit(parent), m_colors(c) {
  m_lineNumberArea = new LineNumberArea(this, this);

  QFont editorFont("JetBrainsMonoNL Nerd Font", 10);
  editorFont.setStyleHint(QFont::Monospace);
  setFont(editorFont);

  connect(document(), &QTextDocument::blockCountChanged, this,
          &Editor::updateLineNumberAreaWidth);

  connect(verticalScrollBar(), &QScrollBar::valueChanged, this,
          [this](int) { m_lineNumberArea->update(); });

  connect(this, &QTextEdit::textChanged, this,
          [this]() { m_lineNumberArea->update(); });

  connect(this, &QTextEdit::cursorPositionChanged, this,
          [this]() { m_lineNumberArea->update(); });

  connect(this, &QTextEdit::cursorPositionChanged, this,
          &Editor::highlightCurrentLine);

  updateLineNumberAreaWidth(0);
}

void Editor::setAppFont(const QString &family, int size) {
  m_fontFamily = family;
  m_fontSize = size;

  QFont f(family, size);
  f.setStyleHint(QFont::Monospace);
  setFont(f);

  m_lineNumberArea->update();
}

bool Editor::handleNormalModeKey(int key) {
  QTextCursor cursor = textCursor();
  QFontMetrics fm(font());

  if (key == Qt::Key_I) {
    emit requestInsertMode();
  } else if (key == Qt::Key_A) {
    if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) {
      cursor.movePosition(QTextCursor::EndOfLine);
    } else {
      cursor.movePosition(QTextCursor::NextCharacter);
    }
    emit requestInsertMode();
  } else if (key == Qt::Key_O) {
    if (QGuiApplication::keyboardModifiers() & Qt::ShiftModifier) {
      cursor.movePosition(QTextCursor::StartOfLine);
      cursor.insertText("\n");
      cursor.movePosition(QTextCursor::Up);
    } else {
      cursor.movePosition(QTextCursor::EndOfLine);
      cursor.insertText("\n");
    }
    emit requestInsertMode();
  } else if (key == Qt::Key_H) {
    if (cursor.positionInBlock() > 0) {
      cursor.movePosition(QTextCursor::PreviousCharacter);
      currCursorPos = cursor.positionInBlock();
      qDebug() << currCursorPos;
    }
  } else if (key == Qt::Key_L) {
    if (cursor.positionInBlock() < cursor.block().length() - 2) {
      cursor.movePosition(QTextCursor::NextCharacter);
      currCursorPos = cursor.positionInBlock();
      qDebug() << currCursorPos;
    }
  } else if (key == Qt::Key_J) {
    cursor.movePosition(QTextCursor::Down);
    if (cursor.positionInBlock() > cursor.block().length() - 2 &&
        cursor.positionInBlock() > 0) {
      cursor.movePosition(QTextCursor::PreviousCharacter);
    }
    if (currCursorPos > cursor.positionInBlock()) {
      if (cursor.block().length() - 2 <= currCursorPos) {
        int len = cursor.block().length() - 2;
        if (len < 0) {
          len = 0;
        }
        cursor.setPosition(len + cursor.block().position());
      } else {
        auto pos = cursor.block().position() + currCursorPos;
        qDebug() << pos;
        cursor.setPosition(pos);
      }
    }
  } else if (key == Qt::Key_K) {
    cursor.movePosition(QTextCursor::Up);
    if (currCursorPos > cursor.positionInBlock()) {
      if (cursor.block().length() - 2 <= currCursorPos) {
        int len = cursor.block().length() - 2;
        if (len < 0) {
          len = 0;
        }
        cursor.setPosition(len + cursor.block().position());
      } else {
        auto pos = cursor.block().position() + currCursorPos;
        qDebug() << pos;
        cursor.setPosition(pos);
      }
    }
  } else if (key == Qt::Key_0) {
    cursor.movePosition(QTextCursor::StartOfLine);
  } else if (key == Qt::Key_Dollar) {
    cursor.movePosition(QTextCursor::EndOfLine);
    if (cursor.positionInBlock() > 0) {
      cursor.movePosition(QTextCursor::PreviousCharacter);
    }
  } else if (key == Qt::Key_X) {
    if (cursor.positionInBlock() < cursor.block().length() - 1) {
      cursor.deleteChar();
    }
  } else {
    return false;
  }

  setTextCursor(cursor);

  return true;
}

void Editor::exitInsertMode() {
  QTextCursor c = textCursor();
  if (c.positionInBlock() > 0) {
    c.movePosition(QTextCursor::PreviousCharacter);
  }
  setTextCursor(c);
}

void Editor::setCursorBlock(bool block) {
  if (block) {
    QFontMetrics fm(font());
    setCursorWidth(fm.averageCharWidth());
  } else {
    setCursorWidth(2);
  }
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

void Editor::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> selections;

  QTextEdit::ExtraSelection selection;

  selection.format.setBackground(QColor(m_colors.currLine));
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);

  selection.cursor = textCursor();
  selection.cursor.clearSelection();

  selections.append(selection);
  setExtraSelections(selections);
}

void Editor::resizeEvent(QResizeEvent *event) {
  QTextEdit::resizeEvent(event);
  QRect cr = contentsRect();
  m_lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(m_lineNumberArea);
  painter.fillRect(event->rect(), QColor(m_colors.bg));

  QFont gutterFont(m_fontFamily, m_fontSize);
  gutterFont.setStyleHint(QFont::Monospace);
  painter.setFont(gutterFont);

  int currentLine = textCursor().blockNumber();
  QTextBlock block = document()->begin();
  int blockNumber = 0;
  int top, bottom;

  while (block.isValid()) {
    // Get pixel positoin of this block on screen
    QRectF blockRect = document()->documentLayout()->blockBoundingRect(block);
    top = qRound(blockRect.translated(0, -verticalScrollBar()->value()).top());
    bottom = top + qRound(blockRect.height());

    if (top <= event->rect().bottom() && bottom >= event->rect().top()) {
      QString number;

      if (blockNumber == currentLine) {
        number = QString::number(blockNumber + 1);
        painter.setPen(QColor(m_colors.currLineNum));
      } else {
        number = QString::number(qAbs(blockNumber - currentLine));
        painter.setPen(QColor(m_colors.lineNum));
      }

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
