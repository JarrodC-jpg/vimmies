#pragma once

#include "AppTheme.h"
#include <QWidget>
#include <qevent.h>
#include <qobject.h>
#include <qsize.h>
#include <qtmetamacros.h>
#include <qwidget.h>

enum class Mode;

class ProjectLabelModule : public QWidget {
  Q_OBJECT

public:
  explicit ProjectLabelModule(const AppTheme &c, const QString &p,
                              QWidget *parent = nullptr);

  void setAppFont(const QString &family, int size);

  void setProjectString(const QString &s);
  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  AppTheme m_colors;

  QString m_projectName;

  QString m_fontFamily = "JetBrainsMono NL Nerd Font";
  int m_fontSize = 11;

  int m_labelPadding = 6;
  int m_arrowWidth = 0;
  static constexpr int k_textPixelSizeCompensation = 3;
};
