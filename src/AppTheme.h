#pragma once

#include <QString>
#include <random>

struct EditorColors {
  QString bg = "#222436";
  QString fg = "#cdd6f4";
  QString currLine = "#2f334d";
  QString currLineNum = "#e88864";
  QString lineNum = "#6c7086";
};

struct NoteColors {
  QString one = "#65bcff";
  QString two = "#4fd6be";
  QString three = "#c099ff";
  QString four = "#fcc576";
  QString five = "#c3e88d";
  QString six = "#0db9d7";
  QString tabTextColor = "#222436";
};

struct ModeColors {
  QString insertBg = "#c3e88d";
  QString normalBg = "#82aaff";
  QString textColor = "#222436";
};

struct CommandColors {
  QString bg = "#3b4261";
  QString textColor = "#4fd6be";
};

struct AppTheme {

  EditorColors editor;
  NoteColors note;
  ModeColors mode;
  CommandColors cmd;
};
