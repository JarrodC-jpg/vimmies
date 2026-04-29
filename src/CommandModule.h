#pragma once

#include <QLineEdit>
#include <QWidget>
#include <qevent.h>
#include <qlineedit.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <qtwidgetsexports.h>
#include <qwidget.h>

class CommandModule : public QWidget {
  Q_OBJECT
public:
  explicit CommandModule(QWidget *parent = nullptr);

  void setAppFont(const QString &family, int size);
  void activate();
  void deactivate();

  bool isActive() const;

  QLineEdit *lineEdit() const { return m_lineEdit; }
  QSize sizeHint() const override;

signals:
  void commandSubmitted(const QString &text);
  void deactivated();

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  QLineEdit *m_lineEdit = nullptr;
  QString m_fontFamily = "JetBrainsMono NL Nerd Font";
  int m_fontSize = 11;

  static constexpr int k_arrowWidth = 12;
  static constexpr int k_promptWidth = 20;
  static constexpr int k_minTextWidth = 40;
};
