#pragma once
#include "CommandModule.h"
#include "Editor.h"
#include "ModeIndicator.h"
#include <QCloseEvent>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QStackedWidget>
#include <QTextEdit>
#include <functional>
#include <map>
#include <qmap.h>
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
  void setupCommandHandlers();

  // Action Functions
  void loadAppSettings();
  void updateModeIndicator();
  void executeCommand();
  void enterCommandLine();
  void exitCommandLine();
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

  Mode m_mode = Mode::Normal;

  QWidget *m_tabBar = nullptr;
  QList<QPushButton *> m_tabs;

  QStackedWidget *m_stackedWidget = nullptr;
  QListWidget *m_projectList = nullptr;
  QWidget *m_editorView = nullptr;

  Editor *m_textEdit = nullptr;
  StickieColor m_currentColor = StickieColor::Yellow;

  ModeIndicator *m_modeIndicator = nullptr;
  CommandModule *m_commandModule = nullptr;
  using CommandHandler = std::function<void(const QString &)>;
  QMap<QString, CommandHandler> m_commandHandlers;

  QTimer *m_saveTimer = nullptr;
  std::map<StickieColor, QString> m_notes;
  std::map<StickieColor, QString> m_titles;
  std::map<StickieColor, int> m_cursorPositions;
  std::map<StickieColor, int> m_scrollPositions;
  QString m_projectFilePath = "empty.vmi";
  QString m_defaultSaveDir;
  QString m_fontFamily = "JetBrainsMono Nl Nerd Font";
  int m_fontSize = 11;
};
