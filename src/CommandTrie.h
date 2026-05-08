#pragma once
#include <QMap>
#include <functional>
#include <qalgorithms.h>
#include <qlist.h>
#include <qmap.h>

struct TrieNode {
  QMap<int, TrieNode *> children;
  std::function<void()> handler;

  ~TrieNode() { qDeleteAll(children); }
};

enum class FeedResult { Executed, Pending, NoMatch };

class CommandTrie {
public:
  CommandTrie();
  ~CommandTrie();

  void insert(const QList<int> &keys, std::function<void()> handler);
  FeedResult feed(int key);
  void reset();

private:
  TrieNode *m_root = nullptr;
  TrieNode *m_current = nullptr;
};
