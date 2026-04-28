#pragma once

#include <QWidget>
#include <qevent.h>
#include <qobject.h>
#include <qsize.h>
#include <qtmetamacros.h>
#include <qwidget.h>

enum class Mode;

class ModeIndicator : public QWidget {
  Q_OBJECT

public:
  explicit ModeIndicator(QWidget *parent = nullptr);

  void setAppFont(const QString &family, int size);

  void setMode(Mode mode);

  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  Mode m_mode;
  QString m_fontFamily = "JetBrainsMono NL Nerd Font";
  int m_fontSize = 11;
  QColor modeColor() const;
};
