#pragma once
#include "AppTheme.h"
#include <QLineEdit>
#include <QWidget>
#include <qcoreevent.h>
#include <qevent.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtwidgetsexports.h>
#include <qwidget.h>

class CommandModule : public QWidget {
  Q_OBJECT
public:
  explicit CommandModule(const AppTheme &theme, QWidget *parent = nullptr);

  void setAppFont(const QString &family, int size);
  void activate();
  void deactivate();

  bool isActive() const;
  QString text() const;
  void setText(const QString &text);

  QLineEdit *lineEdit() const { return m_lineEdit; }
  QSize sizeHint() const override;

signals:
  void commandSubmitted();
  void deactivated();

protected:
  void resizeEvent(QResizeEvent *event) override;
  void paintEvent(QPaintEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  AppTheme m_theme;
  QLineEdit *m_lineEdit = nullptr;
  QString m_fontFamily = "JetBrainsMono NL Nerd Font";
  int m_fontSize = 11;

  static constexpr int k_textPixelSizeCompensation = 3;
  static constexpr int k_promptPadding = 6;
  static constexpr int k_arrowWidth = 12;
  static constexpr int k_promptWidth = 18;
  static constexpr int k_minTextWidth = 20;
  static constexpr int k_veritcalPadding = 0;
};
