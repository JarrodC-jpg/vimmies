#pragma once

#include <QCloseEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <map>
#include <qobject.h>

enum class StickieColor { Yellow, Cyan, Purple, Peach, Pink, Gray };
enum class Mode { Normal, Insert };

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  // Creation of window views and such
  void createMainContainer();
  void createTextEditor();
  void createTabBar();
  void createCommandBar();
  void createProjectList();

  // Action Functions
  void executeCommand();
  void updateTabBar();
  void applyColor(StickieColor color);
  void setCurrentTitle(const QString &title);
  void loadTextForColor(StickieColor color);
  void restoreFontSettings();
  void saveCurrentText();
  void saveStateOnClose();

  // Project Functions
  void showProjectBrowser();
  void hideProjectBrowser();
  void openProject(const QString &name); // TODO call from executeCommand
  void newProject(const QString &name);  // TODO call from executeCommand
  void newProjectInsertCmd();
  void loadLastProjectOnOpen();
  void loadSelectedProject(const QString &filename);

  // Disk Functions
  void loadFromDisk();
  void saveToDisk();

  // Reset data in an empty project
  void resetAllData();

  QWidget *m_tabBar = nullptr;
  QList<QPushButton *> m_tabs;

  QStackedWidget *m_stackedWidget = nullptr;
  QListWidget *m_projectList = nullptr;
  QWidget *m_editorView = nullptr;

  QTextEdit *m_textEdit = nullptr;
  StickieColor m_currentColor = StickieColor::Yellow;
  Mode m_mode = Mode::Normal;

  QLineEdit *m_commandLine = nullptr;
  QTimer *m_saveTimer = nullptr;
  std::map<StickieColor, QString> m_notes;
  std::map<StickieColor, QString> m_titles;
  std::map<StickieColor, int> m_cursorPositions;
  std::map<StickieColor, int> m_scrollPositions;
  QString m_projectFilePath = "empty.vmi";
  QString m_defaultSaveDir;
};
