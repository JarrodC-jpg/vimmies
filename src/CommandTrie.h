#pragma once
#include <QMap>
#include <functional>
#include <qalgorithms.h>
#include <qhash.h>
#include <qlist.h>

struct TrieNode {
  QHash<QKeyCombination, TrieNode *> children;
  std::function<void()> handler;

  ~TrieNode() { qDeleteAll(children); }
};

enum class FeedResult { Executed, Pending, NoMatch };

class CommandTrie {
public:
  CommandTrie();
  ~CommandTrie();

  void insert(const QList<QKeyCombination> &keys,
              std::function<void()> handler);
  FeedResult feed(QKeyCombination key);
  void reset();

private:
  TrieNode *m_root = nullptr;
  TrieNode *m_current = nullptr;
};
