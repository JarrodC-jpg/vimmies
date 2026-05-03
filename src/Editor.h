#pragma once

#include "AppTheme.h"
#include <QTextEdit>
#include <QWidget>
#include <qevent.h>
#include <qobject.h>
#include <qsizepolicy.h>
#include <qtextedit.h>
#include <qtmetamacros.h>
#include <qwidget.h>

class LineNumberArea;

class Editor : public QTextEdit {
  Q_OBJECT

public:
  explicit Editor(const EditorColors &c, QWidget *parent = nullptr);

  int lineNumberAreaWidth() const;

  void lineNumberAreaPaintEvent(QPaintEvent *event);

  void setAppFont(const QString &family, int size);

  bool handleNormalModeKey(int key);

  void setCursorBlock(bool block);

protected:
  void resizeEvent(QResizeEvent *event) override;

private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void updateLineNumberArea(const QRect &rect, int dy);
  void highlightCurrentLine();

private:
  EditorColors m_colors;
  LineNumberArea *m_lineNumberArea = nullptr;
  QString m_fontFamily = "JetBrainsMono NL Nerd Font";
  int m_fontSize = 11;
};

class LineNumberArea : public QWidget {
public:
  explicit LineNumberArea(Editor *editor, QWidget *parent = nullptr)
      : QWidget(parent), m_editor(editor) {}

  QSize sizeHint() const override {
    return QSize(m_editor->lineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) override {
    m_editor->lineNumberAreaPaintEvent(event);
  }

private:
  Editor *m_editor = nullptr;
};
