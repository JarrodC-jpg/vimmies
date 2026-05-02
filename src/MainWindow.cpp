#include "MainWindow.h"
#include "CommandModule.h"
#include "ModeIndicator.h"
#include "ProjectLabelModule.h"
#include <QAction>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QStandardPaths>
#include <QTimer>
#include <QVBoxLayout>
#include <qcoreevent.h>
#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qsettings.h>
#include <set>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  loadAppSettings();
  qDebug() << "Welcome to vimies, notes with vim-like motions!";
  setWindowTitle("Stickies");
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
  setStyleSheet("QMainWindow { border 3px solid #232378;}");
  resize(420, 500);

  createMainContainer();
  createTextEditor();
  createProjectList();
  createCommandBar();
  createTabBar();
  setupCommandHandlers();
  updateModeIndicator();

  m_defaultSaveDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(m_defaultSaveDir);

  loadLastProjectOnOpen();

  applyColor(m_currentColor);

  new QShortcut(QKeySequence("Ctrl+/"), this, [this]() {
    if (!m_commandModule || !m_textEdit) {
      return;
    }

    if (m_commandModule->isActive()) {
      exitCommandLine();
      return;
    }
    enterCommandLine();
  });

  auto *openProjectShortcut = new QShortcut(QKeySequence("Ctrl+o"), this);
  connect(openProjectShortcut, &QShortcut::activated, this,
          &MainWindow::showProjectBrowser);

  auto *newProjectShortcut = new QShortcut(QKeySequence("Ctrl+n"), this);
  connect(newProjectShortcut, &QShortcut::activated, this,
          &MainWindow::newProjectInsertCmd);
}

MainWindow::~MainWindow() {
  saveCurrentText();
  saveToDisk();
}

void MainWindow::loadAppSettings() {
  QSettings settings("stickies.conf", QSettings::IniFormat);
  m_fontFamily = settings.value("fontFamily", m_fontFamily).toString();
  m_fontSize = settings.value("fontSize", m_fontSize).toInt();

  settings.beginGroup("mode_colors");
  m_theme.mode.normalBg =
      settings.value("modeModuleNormalBg", m_theme.mode.normalBg).toString();
  m_theme.mode.insertBg =
      settings.value("modeModuleInsertBg", m_theme.mode.insertBg).toString();
  m_theme.mode.textColor =
      settings.value("modeModuleTextColor", m_theme.mode.textColor).toString();
  settings.endGroup();

  settings.beginGroup("editor_colors");
  m_theme.editor.bg = settings.value("editorBg", m_theme.editor.bg).toString();
  m_theme.editor.fg = settings.value("editorFg", m_theme.editor.fg).toString();
  m_theme.editor.currLine =
      settings.value("editorCurrentLineColor", m_theme.editor.currLine)
          .toString();
  m_theme.editor.currLineNum =
      settings.value("editorCurrentLineNumColor", m_theme.editor.currLineNum)
          .toString();
  m_theme.editor.lineNum =
      settings.value("editorLineNumColor", m_theme.editor.lineNum).toString();
  settings.endGroup();

  settings.beginGroup("note_colors");
  m_theme.note.one =
      settings.value("noteOneColor", m_theme.note.one).toString();
  m_theme.note.two =
      settings.value("noteTwoColor", m_theme.note.two).toString();
  m_theme.note.three =
      settings.value("noteThreeColor", m_theme.note.three).toString();
  m_theme.note.four =
      settings.value("noteFourColor", m_theme.note.four).toString();
  m_theme.note.five =
      settings.value("noteFiveColor", m_theme.note.five).toString();
  m_theme.note.six =
      settings.value("noteSixColor", m_theme.note.six).toString();
  settings.endGroup();

  settings.beginGroup("command_colors");
  m_theme.cmd.bg = settings.value("commandModuleBg", m_theme.cmd.bg).toString();
  m_theme.cmd.textColor =
      settings.value("commandModuleTextColor", m_theme.cmd.textColor)
          .toString();
  settings.endGroup();

  qDebug() << "Theme loaded - editorBg:" << m_theme.editor.bg;
}

void MainWindow::updateModeIndicator() {
  if (m_modeIndicator) {
    m_modeIndicator->setMode(m_mode);
  }
}

void MainWindow::createMainContainer() {
  m_stackedWidget = new QStackedWidget(this);
  auto *mainContainer = new QWidget(this);
  auto *mainLayout = new QVBoxLayout(mainContainer);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  mainLayout->addWidget(m_stackedWidget, 1);

  setCentralWidget(mainContainer);
}

void MainWindow::createTextEditor() {
  m_editorView = new QWidget();
  auto *layout = new QVBoxLayout(m_editorView);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_textEdit = new Editor(m_theme.editor, m_editorView);
  m_textEdit->setAppFont(m_fontFamily, m_fontSize);
  m_textEdit->setAcceptRichText(true);
  m_textEdit->installEventFilter(this);

  m_editorView->setFocusProxy(m_textEdit);

  m_saveTimer = new QTimer(this);
  m_saveTimer->setSingleShot(true);
  m_saveTimer->setInterval(400);

  connect(m_saveTimer, &QTimer::timeout, this, [this]() { saveToDisk(); });

  connect(m_textEdit, &QTextEdit::textChanged, this, [this]() {
    restoreFontSettings();
    saveCurrentText();
    m_saveTimer->start();
  });

  layout->addWidget(m_textEdit);
  m_stackedWidget->addWidget(m_editorView);
}

void MainWindow::createTabBar() {
  m_tabBar = new QWidget(this);
  auto *layout = new QHBoxLayout(m_tabBar);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_tabs.clear();

  for (int i = 0; i < 6; ++i) {
    QPushButton *tab = new QPushButton(m_tabBar);
    tab->setFixedHeight(32);
    tab->setFlat(true);

    connect(tab, &QPushButton::clicked, this, [this, i]() {
      StickieColor color = static_cast<StickieColor>(i);
      applyColor(color);
    });

    m_tabs.append(tab);
    layout->addWidget(tab);
  }
  updateTabBar();

  auto *mainContainer = qobject_cast<QWidget *>(centralWidget());
  if (mainContainer && mainContainer->layout()) {
    auto *vbox = qobject_cast<QVBoxLayout *>(mainContainer->layout());
    if (vbox) {
      vbox->insertWidget(0, m_tabBar);
    }
  }

  QTimer::singleShot(50, this, [this]() { updateTabBar(); });
}

void MainWindow::updateTabBar() {

  if (!m_tabBar || m_tabs.isEmpty())
    return;

  const int squareSize = 20;

  int totalSqauresWidth = 5 * squareSize;
  int availableWidth = m_tabBar->width() - totalSqauresWidth;

  for (int i = 0; i < 6; ++i) {
    StickieColor color = static_cast<StickieColor>(i);
    QPushButton *tab = m_tabs[i];

    QString bgColor;
    QString textColor = m_theme.note.tabTextColor;
    switch (color) {

    case StickieColor::Yellow:
      bgColor = m_theme.note.one;
      break;
    case StickieColor::Cyan:
      bgColor = m_theme.note.two;
      break;
    case StickieColor::Purple:
      bgColor = m_theme.note.three;
      break;
    case StickieColor::Peach:
      bgColor = m_theme.note.four;
      break;
    case StickieColor::Pink:
      bgColor = m_theme.note.five;
      break;
    case StickieColor::Gray: {
      bgColor = m_theme.note.six;
      break;
    }
    }

    if (color == m_currentColor) {
      tab->setFixedHeight(squareSize);
      tab->setMinimumWidth(squareSize);
      tab->setMaximumWidth(availableWidth);

      QString title = m_titles[color];
      if (title.isEmpty())
        title = "";

      tab->setText(QString("[%1]{\"%2\"}").arg(i + 1).arg(title));
      tab->setStyleSheet(QString("background-color: %1; color: %2; "
                                 "font-weight: bold; border:none;")
                             .arg(bgColor, textColor));
    } else {
      tab->setFixedHeight(squareSize);
      tab->setFixedWidth(squareSize);
      tab->setText(QString::number(i + 1));
      tab->setStyleSheet(
          QString("background-color: %1; color: %2; border: none;")
              .arg(bgColor, textColor));
    }
  }
}

void MainWindow::setupCommandHandlers() {
  m_commandHandlers["new"] = [this](const QString &args) { newProject(args); };
  m_commandHandlers["title"] = [this](const QString &args) {
    setCurrentTitle(args);
  };
  m_commandHandlers["open"] = [this](const QString &args) {
    if (args.isEmpty()) {
      showProjectBrowser();
    } else {
      openProject(args);
    }
  };
}

void MainWindow::createCommandBar() {
  auto *commandWidget = new QWidget(this);
  //  commandWidget->setFixedHeight(20);
  auto *layout = new QHBoxLayout(commandWidget);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  m_modeIndicator = new ModeIndicator(m_theme, commandWidget);
  m_modeIndicator->setAppFont(m_fontFamily, m_fontSize);
  layout->addWidget(m_modeIndicator);

  m_commandModule = new CommandModule(m_theme, commandWidget);
  m_commandModule->setAppFont(m_fontFamily, m_fontSize);
  layout->addWidget(m_commandModule);

  layout->addStretch(1);

  m_projectLableModule =
      new ProjectLabelModule(m_theme, QString("Project Name"), commandWidget);
  m_projectLableModule->setAppFont(m_fontFamily, m_fontSize);
  layout->addWidget(m_projectLableModule);

  auto *mainContainer = qobject_cast<QWidget *>(centralWidget());
  if (mainContainer && mainContainer->layout()) {
    auto *vbox = qobject_cast<QVBoxLayout *>(mainContainer->layout());
    if (vbox) {
      vbox->addWidget(commandWidget);
    }
  }
  connect(m_commandModule, &CommandModule::commandSubmitted, this,
          &MainWindow::executeCommand);

  connect(m_commandModule, &CommandModule::deactivated, this,
          [this]() { m_stackedWidget->currentWidget()->setFocus(); });
}

void MainWindow::executeCommand() {
  QString input = m_commandModule->text().trimmed();
  if (input.isEmpty())
    return;

  if (!input.startsWith(':')) {
    exitCommandLine();
    return;
  }

  input.remove(0, 1);

  const int firstSpace = input.indexOf(' ');

  QString cmd;
  QString args;

  if (firstSpace == -1) {
    cmd = input;
    args = "";
  } else {
    cmd = input.left(firstSpace);
    args = input.mid(firstSpace + 1).trimmed();
  }

  auto it = m_commandHandlers.find(cmd);
  if (it != m_commandHandlers.end()) {
    it.value()(args);
  }
  exitCommandLine();
}

void MainWindow::enterCommandLine() { m_commandModule->activate(); }

void MainWindow::exitCommandLine() { m_commandModule->deactivate(); }

void MainWindow::createProjectList() {

  m_projectList = new QListWidget();
  m_projectList->setAlternatingRowColors(false);
  QPalette palette = m_projectList->palette();
  palette.setColor(QPalette::Base, QColor("#222436"));
  palette.setColor(QPalette::Text, QColor("#CDD6F4"));
  m_projectList->setPalette(palette);
  connect(m_projectList, &QListWidget::itemDoubleClicked, this,
          [this](QListWidgetItem *item) {
            if (item)
              loadSelectedProject(item->data(Qt::UserRole).toString());
          });
  m_stackedWidget->addWidget(m_projectList);
}

void MainWindow::resetAllData() {
  m_titles.clear();
  m_notes.clear();
  m_cursorPositions.clear();
  m_scrollPositions.clear();

  m_currentColor = StickieColor::Yellow;

  if (m_textEdit) {
    m_textEdit->clear();

    QTextCursor cursor = m_textEdit->textCursor();
    cursor.setPosition(0);
    m_textEdit->setTextCursor(cursor);

    if (m_textEdit->verticalScrollBar()) {
      m_textEdit->verticalScrollBar()->setValue(0);
    }
  }
  applyColor(m_currentColor);
  qDebug() << "Empty project created";
}

void MainWindow::restoreFontSettings() {
  QFont font = m_textEdit->font();
  if (font.pointSize() != m_fontSize) {
    font.setPointSize(m_fontSize);
    m_textEdit->setFont(font);
  }
}

void MainWindow::applyColor(StickieColor color) {
  saveCurrentText();

  m_currentColor = color;
  loadTextForColor(color);

  QString bgColor = m_theme.editor.bg;
  QString textColor = m_theme.editor.bg;

  switch (color) {

  case StickieColor::Yellow:
    textColor = m_theme.note.one;
    break;
  case StickieColor::Cyan:
    textColor = m_theme.note.two;
    break;
  case StickieColor::Purple:
    textColor = m_theme.note.three;
    break;
  case StickieColor::Peach:
    textColor = m_theme.note.four;
    break;
  case StickieColor::Pink:
    textColor = m_theme.note.five;
    break;
  case StickieColor::Gray: {
    textColor = m_theme.note.six;
    break;
  }
  }
  updateTabBar();
  QString style = QString("QTextEdit {background-color: %1; color: %2; border: "
                          "none; border-top: 0px solid %2;}")
                      .arg(bgColor, textColor);
  m_textEdit->setStyleSheet(style);
  // setStyleSheet(style);

  QString windowStyle =
      QString("QMainWindow {background-color: %1;}").arg(textColor);
  setStyleSheet(windowStyle);
}

void MainWindow::setCurrentTitle(const QString &title) {
  QString trimmedTitle = title.trimmed();

  if (trimmedTitle.isEmpty()) {
    return;
  }
  m_titles[m_currentColor] = trimmedTitle;
  updateTabBar();
  qDebug() << "Title set for color" << static_cast<int>(m_currentColor) + 1
           << "to" << trimmedTitle;
}

void MainWindow::saveCurrentText() {
  if (!m_textEdit)
    return;
  m_notes[m_currentColor] = m_textEdit->toPlainText();
  m_cursorPositions[m_currentColor] = m_textEdit->textCursor().position();
  m_scrollPositions[m_currentColor] = m_textEdit->verticalScrollBar()->value();
}

void MainWindow::loadTextForColor(StickieColor color) {
  auto it = m_notes.find(color);
  if (it != m_notes.end()) {
    m_textEdit->setPlainText(it->second);
  } else {
    m_textEdit->clear();
  }

  auto cursorIt = m_cursorPositions.find(color);
  if (cursorIt != m_cursorPositions.end()) {
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.setPosition(cursorIt->second);
    m_textEdit->setTextCursor(cursor);
  }

  auto scrollIt = m_scrollPositions.find(color);
  if (scrollIt != m_scrollPositions.end()) {
    m_textEdit->verticalScrollBar()->setValue(scrollIt->second);
  }
}

void MainWindow::saveToDisk() {
  QJsonObject root;
  for (const auto &[color, text] : m_notes) {
    QString key = QString::number(static_cast<int>(color));
    root[key] = text;
  }
  QJsonObject cursorObj;
  for (const auto &[color, pos] : m_cursorPositions) {
    cursorObj[QString::number(static_cast<int>(color))] = pos;
  }
  root["cursorPos"] = cursorObj;

  QJsonObject titlesObj;
  for (const auto &[color, title] : m_titles) {
    QString key = QString::number(static_cast<int>(color));
    titlesObj[key] = title;
  }
  root["titles"] = titlesObj;

  QJsonObject scrollObj;
  for (const auto &[color, pos] : m_scrollPositions) {
    scrollObj[QString::number(static_cast<int>(color))] = pos;
  }
  root["scrollPos"] = scrollObj;

  root["lastColor"] = static_cast<int>(m_currentColor);
  QJsonDocument doc(root);
  QFile file(m_projectFilePath);
  if (file.open(QIODevice::WriteOnly)) {
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    qDebug() << "Saved to " << m_projectFilePath;
  } else {
    qDebug() << "Failed to save " << m_projectFilePath;
  }
}

void MainWindow::loadFromDisk() {
  QFile file(m_projectFilePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "No " << m_projectFilePath << " file found - starting fresh";
    return;
  }

  QByteArray data = file.readAll();
  file.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (doc.isNull()) {
    qDebug() << "Invalid JSON in " << m_projectFilePath;
    resetAllData();
    return;
  }

  QJsonObject root = doc.object();

  for (int i = 0; i < 6; ++i) {
    QString key = QString::number(i);
    StickieColor color = static_cast<StickieColor>(i);
    if (root.contains(key)) {
      m_notes[color] = root[key].toString();
    } else {
      m_notes[color] = "";
      qDebug() << "No key for" << key;
    }
  }

  if (root.contains("titles")) {
    QJsonObject titlesObj = root["titles"].toObject();
    for (int i = 0; i < 6; ++i) {
      QString key = QString::number(i);
      if (titlesObj.contains(key)) {
        m_titles[static_cast<StickieColor>(i)] = titlesObj[key].toString();
      }
    }
  }

  if (root.contains("cursorPos")) {
    QJsonObject cursorObj = root["cursorPos"].toObject();
    for (int i = 0; i < 6; ++i) {
      QString key = QString::number(i);
      if (cursorObj.contains(key)) {
        m_cursorPositions[static_cast<StickieColor>(i)] =
            cursorObj[key].toInt();
      }
    }
  }

  if (root.contains("scrollPos")) {
    QJsonObject scrollObj = root["scrollPos"].toObject();
    for (int i = 0; i < 6; ++i) {
      QString key = QString::number(i);
      if (scrollObj.contains(key)) {
        m_scrollPositions[static_cast<StickieColor>(i)] =
            scrollObj[key].toInt();
      }
    }
  }

  if (root.contains("lastColor")) {
    int last = root["lastColor"].toInt();
    if (last >= 0 && last < 6) {
      m_currentColor = static_cast<StickieColor>(last);
      // m_textEdit->setPlainText(m_notes[m_currentColor]);
    }
  }

  loadTextForColor(m_currentColor);
}

// TODO Call newProject from executeCommand
void MainWindow::newProject(const QString &name) {
  QString projName = name;

  if (projName.isEmpty()) {
    qDebug() << "Empty Project Name";
    return;
  }
  if (!projName.endsWith(".vmi", Qt::CaseSensitive)) {
    projName += ".vmi";
  }
  QString fullpath = m_defaultSaveDir + "/" + projName;

  m_projectFilePath = fullpath;
  setWindowTitle("Stickies - " + projName);

  resetAllData();

  if (m_textEdit) {
    m_textEdit->setFocus();
  }

  qDebug() << "Created new Project" << fullpath;
}

void MainWindow::newProjectInsertCmd() {
  if (m_commandModule) {
    m_commandModule->activate();
    m_commandModule->setText(":new ");
  }
}

void MainWindow::openProject(const QString &name) {
  QString projectName = name.trimmed();

  if (projectName.isEmpty()) {
    return;
  }

  if (!projectName.endsWith(".vmi")) {
    projectName += ".vmi";
  }

  QString fullPath = m_defaultSaveDir + "/" + projectName;

  if (!QFile::exists(fullPath)) {
    qDebug() << "Project does not exist:" << fullPath;
    return;
  }

  loadSelectedProject(fullPath);
}

void MainWindow::saveStateOnClose() {
  saveCurrentText();
  saveToDisk();
  QSettings settings("stickies.conf", QSettings::IniFormat);
  settings.setValue("lastProject", m_projectFilePath);
}

void MainWindow::loadLastProjectOnOpen() {
  QSettings settings("stickies.conf", QSettings::IniFormat);

  QString lastProject = settings.value("lastProject", "").toString();

  if (!lastProject.isEmpty() && QFile::exists(lastProject)) {
    m_projectFilePath = lastProject;
    setWindowTitle("Stickies - " + QFileInfo(lastProject).fileName());
    loadFromDisk();
    qDebug() << "Loaded last project:" << lastProject;
  } else {
    qDebug() << "No previous project found - starting new project";
  }
}

void MainWindow::showProjectBrowser() {
  saveCurrentText();
  saveToDisk();

  if (m_stackedWidget) {
    m_stackedWidget->setCurrentWidget(m_projectList);
  }
  // m_projectList->setFocus();

  m_projectList->setStyleSheet(QString("QListWidget {"
                                       " background-color: #222436;"
                                       " color: #CDD6F4;"
                                       " border: none;"
                                       " outline: none;"
                                       " padding: 0px;"
                                       " font-family: '%1', monospace;"
                                       "}"
                                       "QListWidget::item {"
                                       "  padding: 4px 6px;"
                                       "  border: none;"
                                       "}"
                                       "QListWidget::item:selected {"
                                       "  background-color: #2f334d;"
                                       "  border: none;"
                                       " outline: none;"
                                       "  color: #CDD6F4;"
                                       "  font-weight: bold;"
                                       "}")
                                   .arg(m_fontFamily));

  m_projectList->clear();
  QDir dir(m_defaultSaveDir);
  QFileInfoList files =
      dir.entryInfoList(QStringList() << "*.vmi", QDir::Files);

  for (const auto &fi : files) {
    QListWidgetItem *item = new QListWidgetItem(fi.baseName());
    item->setData(Qt::UserRole, fi.absoluteFilePath());
    m_projectList->addItem(item);
  }
  m_projectList->setCurrentRow(0);
  m_projectList->installEventFilter(this);
}

void MainWindow::hideProjectBrowser() {
  if (m_stackedWidget) {
    m_stackedWidget->setCurrentWidget(m_editorView);
  }
  if (m_textEdit) {
    m_textEdit->setFocus();
  }
}

void MainWindow::loadSelectedProject(const QString &filename) {
  if (filename.isEmpty())
    return;

  m_projectFilePath = filename;
  setWindowTitle("Stickies - " + QFileInfo(filename).fileName());

  loadFromDisk();
  applyColor(m_currentColor);

  hideProjectBrowser();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (obj == m_textEdit && event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    int key = keyEvent->key();
    if ((keyEvent->modifiers() & Qt::AltModifier) && key >= Qt::Key_1 &&
        key <= Qt::Key_6) {
      StickieColor newColor = static_cast<StickieColor>(key - Qt::Key_1);
      applyColor(newColor);
      return true;
    }

    if (m_mode == Mode::Normal) {
      if (key == Qt::Key_I) {
        m_mode = Mode::Insert;
        updateModeIndicator();
        return true;
      }
      return true;
    }

    if (m_mode == Mode::Insert) {
      if (key == Qt::Key_Escape) {
        m_mode = Mode::Normal;
        updateModeIndicator();
        return true;
      }
      // All other keys in Insert mode: QTextEdit handles that.
    }
  }

  if (obj == m_projectList && event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    int key = keyEvent->key();
    int currRow = m_projectList->currentRow();
    int rowCount = m_projectList->count();

    if (key == Qt::Key_Escape) {
      hideProjectBrowser();
      return true;
    }

    if (key == Qt::Key_J) {
      if (++currRow >= rowCount) {
        currRow = 0;
      }
      m_projectList->setCurrentRow(currRow);
    }
    if (key == Qt::Key_K) {
      if (--currRow < 0) {
        currRow = rowCount - 1;
      }
      m_projectList->setCurrentRow(currRow);
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
      QListWidgetItem *item = m_projectList->currentItem();
      if (item) {
        loadSelectedProject(item->data(Qt::UserRole).toString());
      }
    }
  }

  return QMainWindow::eventFilter(obj, event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  qDebug() << "Saved on closeEvent";
  saveStateOnClose();
  event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  QMainWindow::resizeEvent(event);
  updateTabBar();
}
