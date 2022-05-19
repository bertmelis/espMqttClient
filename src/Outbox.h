
/*
Copyright (c) 2022 Bert Melis. All rights reserved.

This work is licensed under the terms of the MIT license.  
For a copy, see <https://opensource.org/licenses/MIT> or
the LICENSE file.
*/

#pragma once

namespace espMqttClientInternals {

/**
 * @brief Simple outbox item to hold raw data
 */

template <typename T>
struct Node {
 public:
  explicit Node(T item)
  : item(item)
  , prev(nullptr)
  , next(nullptr) {
    // empty
  }

  T item;
  Node* prev;
  Node* next;
};

/**
 * @brief Doubly linked queue
 * 
 */

template <typename T>
class Outbox {
 public:
  Outbox()
  : _first(nullptr)
  , _last(nullptr)
  , _current(nullptr) {}
  ~Outbox() {
    clear();
  }

  class Iterator {
    friend class Outbox;
   public:
    void operator++() {
      if (_it) _it = _it->next;
    }

    void operator--() {
      if (_it) _it = _it->prev;
    }

    T* data() const {
      if (_it) return &(_it->item);
      return nullptr;
    }

   private:
    Node<T>* _it = nullptr;
  };

  // add node to back, advance current to new if applicable
  void add(T item) {
    Node<T>* node = new Node<T>(item);
    if (!_first) {
      // queue is empty
      _first = _last = node;
    } else {
      // queue has at least one item
      node->prev = _last;
      _last->next = node;
      _last = node;
    }
    // advance current to newly created if applicable
    if (!_current) {
      _current = _last;
    }
  }

  // add item to front, current points to newly created front
  void addFront(T item) {
    Node<T>* node = new Node<T>(item);
    if (!_first) {
      // queue is empty
      _last = node;
    } else {
      // queue has at least one
      node->next = _first;
      _first->prev = node;
    }
    _current = _first = node;
  }

  // remove node at iterator, iterator points to next
  void remove(Iterator& it) {  // NOLINT(runtime/references)
    Node<T>* n = it._it;
    ++it;
    _remove(n);
  }

  // remove current node, current points to next
  void removeCurrent() {
    _remove(_current);
  }

  // Get current item or return nullptr when done
  T* getCurrent() const {
    if (_current) return &(_current->item);
    return nullptr;
  }

  Iterator front() const {
    Iterator it;
    it._it = _first;
    return it;
  }

  // Advance current item
  void next() {
    if (_current) _current = _current->next;
  }

  // Remove all items
  void clear() {
    while (_first) {
      Node<T>* n = _first->next;
      delete _first;
      _first = n;
    }
    _last = _current = nullptr;
  }

  // Outbox is empty
  bool empty() {
    if (!_first) return true;
    return false;
  }

 private:
  Node<T>* _first;
  Node<T>* _last;
  Node<T>* _current;

  void _remove(Node<T>* node) {
    if (!node) return;

    // set current to next, node->next may be nullptr
    if (_current == node) _current = node->next;

    // only one element in outbox
    if (_first == _last) {
      _first = _last = nullptr;

    // delete first el in longer outbox
    } else if (_first == node) {
      _first = node->next;
      _first->prev = nullptr;

    // delete last in longer outbox
    } else if (_last == node) {
      _last = _last->prev;
      _last->next = nullptr;

    // delete somewhere in the middle
    } else {
      node->prev->next = node->next;
      node->next->prev = node->prev;
    }

    // finally, delete the node
    delete node;
  }
};

}  // end namespace espMqttClientInternals
