#include "CommandTrie.h"
#include <qlist.h>

CommandTrie::CommandTrie() {
  m_root = new TrieNode();
  m_current = m_root;
}

CommandTrie::~CommandTrie() { delete m_root; }

void CommandTrie::insert(const QList<QKeyCombination> &keys,
                         std::function<void()> handler) {
  TrieNode *node = m_root;
  for (QKeyCombination key : keys) {
    if (!node->children.contains(key)) {
      node->children[key] = new TrieNode();
    }
    node = node->children[key];
  }
  node->handler = handler;
}

void CommandTrie::reset() { m_current = m_root; }

FeedResult CommandTrie::feed(QKeyCombination key) {
  if (!m_current->children.contains(key)) {
    reset();
    return FeedResult::NoMatch;
  }
  m_current = m_current->children[key];

  if (m_current->handler) {
    m_current->handler();
    reset();
    return FeedResult::Executed;
  }

  return FeedResult::Pending;
}
