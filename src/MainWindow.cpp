#include "MainWindow.h"
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
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
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

  m_defaultSaveDir =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir().mkpath(m_defaultSaveDir);

  loadLastProjectOnOpen();

  applyColor(m_currentColor);

  new QShortcut(QKeySequence("Ctrl+/"), this, [this]() {
    if (!m_commandLine || !m_textEdit) {
      return;
    }

    if (m_commandLine->hasFocus()) {
      exitCommadLine();
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
  layout->setContentsMargins(3, 0, 3, 0);
  layout->setSpacing(0);

  m_textEdit = new QTextEdit(m_editorView);
  m_textEdit->setAcceptRichText(true);
  m_textEdit->setFontPointSize(12);
  m_textEdit->installEventFilter(this);

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
  layout->setContentsMargins(3, 3, 3, 0);
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
    QString textColor = "#000000";
    switch (color) {

    case StickieColor::Yellow:
      bgColor = "#F9E2AF";
      textColor = "#D4B46F";
      break;
    case StickieColor::Cyan:
      bgColor = "#74C7EC";
      textColor = "#4A9BC4";
      break;
    case StickieColor::Purple:
      bgColor = "#B4BEFE";
      textColor = "#7E8BDB";
      break;
    case StickieColor::Peach:
      bgColor = "#FAB387";
      textColor = "#E08F5F";
      break;
    case StickieColor::Pink:
      bgColor = "#EB6690";
      textColor = "#C14A72";
      break;
    case StickieColor::Gray: {
      bgColor = "#cdd6f4";
      textColor = "#2d2d2d";
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
  auto *layout = new QHBoxLayout(commandWidget);
  layout->setContentsMargins(3, 0, 3, 3);
  layout->setSpacing(0);

  // QLabel *label = new QLabel("Command:", commandWidget);
  m_commandLine = new QLineEdit(commandWidget);
  m_commandLine->installEventFilter(this);
  m_commandLine->setPlaceholderText("Ctrl+/ to focus");

  m_commandLine->setStyleSheet(
      "QLineEdit {"
      " background-color: #1e1e2e;"
      " color: #CDD6F4;"
      " border: none;"
      " padding: 4px 6px;"
      " font-family: 'JetBrainsMono Nerd Font', monospace;"
      "}");

  layout->addWidget(m_commandLine, 1);

  auto *mainContainer = qobject_cast<QWidget *>(centralWidget());
  if (mainContainer && mainContainer->layout()) {
    auto *vbox = qobject_cast<QVBoxLayout *>(mainContainer->layout());
    if (vbox) {
      vbox->addWidget(commandWidget);
    }
  }

  connect(m_commandLine, &QLineEdit::returnPressed, this,
          &MainWindow::executeCommand);
}

void MainWindow::executeCommand() {
  QString input = m_commandLine->text().trimmed();
  if (input.isEmpty())
    return;

  if (!input.startsWith(':')) {
    m_commandLine->clear();
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
  m_commandLine->clear();
}

void MainWindow::enterCommandLine() {
  m_commandLine->setFocus();
  m_commandLine->setText(":");
  m_commandLine->setCursorPosition(1);
}

void MainWindow::exitCommadLine() {
  m_commandLine->clear();
  m_commandLine->setPlaceholderText("Ctrl+/ to focus");
  m_textEdit->setFocus();
}

void MainWindow::createProjectList() {

  m_projectList = new QListWidget();
  m_projectList->setAlternatingRowColors(true);
  QPalette palette = m_projectList->palette();
  palette.setColor(QPalette::Base, QColor("#1e1e2e"));
  palette.setColor(QPalette::AlternateBase, QColor("#181825"));
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
  if (font.pointSize() != 12) {
    font.setPointSize(12);
    m_textEdit->setFont(font);
  }
}

void MainWindow::applyColor(StickieColor color) {
  saveCurrentText();

  m_currentColor = color;
  loadTextForColor(color);

  QString bgColor = "#1E1E2E";
  QString textColor = "#1E1E2E";
  QString textColorDark = "#1E1E2E";

  switch (color) {

  case StickieColor::Yellow:
    textColor = "#F9E2AF";
    textColorDark = "#D4B46F";
    break;
  case StickieColor::Cyan:
    textColor = "#74C7EC";
    textColorDark = "#4A9BC4";
    break;
  case StickieColor::Purple:
    textColor = "#B4BEFE";
    textColorDark = "#7E8BDB";
    break;
  case StickieColor::Peach:
    textColor = "#FAB387";
    textColorDark = "#E08F5F";
    break;
  case StickieColor::Pink:
    textColor = "#EB6690";
    textColorDark = "#C14A72";
    break;
  case StickieColor::Gray: {
    textColor = "#cdd6f4";
    textColorDark = "#2d2d2d";
    break;
  }
  }
  updateTabBar();
  QString style =
      QString("QTextEdit {background-color: %1; color: %2; border: "
              "none; border-bottom: 1px solid %2; border-top: 3px solid %2;}")
          .arg(bgColor, textColor);
  m_textEdit->setStyleSheet(style);
  // setStyleSheet(style);

  QString cmd_style =
      QString("QLineEdit {"
              " background-color: %1;"
              " color: %2;"
              " border: none;"
              " padding: 4px 6px;"
              " font-family: 'JetBrainsMono Nerd Font', monospace;"
              "}")
          .arg(bgColor, textColorDark);
  m_commandLine->setStyleSheet(cmd_style);

  QString windowStyle =
      QString("QMainWindow {background-color: %1; border: 3px solid %1;}")
          .arg(textColor);
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
  if (m_commandLine) {
    m_commandLine->setText("new: ");
    m_commandLine->setFocus();
    m_commandLine->setCursorPosition(5);
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
  m_projectList->setFocus();

  m_projectList->setStyleSheet(R"(
    QListWidget {
      background-color: #1e1e2e;
      color: #CDD6F4;
      border: none;
      padding: 3px;
      font-family: 'JetBrainsMono Nerd Font', monospace;
    }
    QListWidget::item {
      padding: 4px 6px;
    }
    QListWidget::item:selected {
      background-color: #3a3a3a !important;
      color: #ffffff !important;
      font-weight: bold;
    }
    QListWidget::item:hover:!selected {
            background-color: #2a2a3a !important;
    }
  )");

  m_projectList->clear();
  QDir dir(m_defaultSaveDir);
  QFileInfoList files =
      dir.entryInfoList(QStringList() << "*.vmi", QDir::Files);

  for (const auto &fi : files) {
    QListWidgetItem *item = new QListWidgetItem(fi.fileName());
    item->setData(Qt::UserRole, fi.absoluteFilePath());
    m_projectList->addItem(item);
  }

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
  }

  if (obj == m_projectList && event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    int key = keyEvent->key();

    if (key == Qt::Key_Escape) {
      hideProjectBrowser();
      return true;
    }

    if (key == Qt::Key_J) {
      m_projectList->setCurrentRow(m_projectList->currentRow() + 1);
    }
    if (key == Qt::Key_K) {
      m_projectList->setCurrentRow(m_projectList->currentRow() - 1);
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
      QListWidgetItem *item = m_projectList->currentItem();
      if (item) {
        loadSelectedProject(item->data(Qt::UserRole).toString());
      }
    }
  }

  if (obj == m_commandLine && event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    if (keyEvent->key() == Qt::Key_Escape) {
      exitCommadLine();
      return true;
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
