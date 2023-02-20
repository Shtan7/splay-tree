#pragma once
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>
#include <ranges>
#include <queue>
#include <iterator>
#include <initializer_list>
#include <algorithm>

namespace internal
{
  template<class Allocator, class NewType>
  concept RebindPresence = requires
  {
    typename Allocator::template rebind<NewType>::other;
  };

  template<class Allocator, class NewType>
  struct node_allocator_impl
  {
    using type = typename Allocator::template rebind<NewType>::other;
  };

  template<template<class, class...> class Allocator, class NewType, class OldType, class... OtherArgs>
    requires (!RebindPresence<Allocator<OldType, OtherArgs...>, NewType>)
  struct node_allocator_impl<Allocator<OldType, OtherArgs...>, NewType>
  {
    using type = Allocator<NewType, OtherArgs...>;
  };

  template<class Allocator, class NewType>
  using node_allocator_t = typename node_allocator_impl<Allocator, NewType>::type;

  template<class Key, class DummyArg>
  const Key& extract_key(const Key& key, const DummyArg&) noexcept
  {
    return key;
  }

  template<class Key>
  const Key& extract_key(const Key& key) noexcept
  {
    return key;
  }
}

template<class Key, class Data, class Comparator = std::less<Key>, class Allocator = std::allocator<std::pair<const Key, Data>>>
class splay_tree
{
  friend class iterator;
  friend class const_iterator;
  class data_node;

public:
  using key_type = Key;
  using mapped_type = Data;
  using value_type = std::pair<const Key, Data>;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using key_compare = Comparator;
  using allocator_type = Allocator;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = typename std::allocator_traits<Allocator>::pointer;
  using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

private:
  class tree_node
  {
    friend class splay_tree<Key, Data, Comparator, Allocator>;

  private:
    tree_node* parent_;
    tree_node* left_;
    tree_node* right_;

  public:
    tree_node(const tree_node&) = default;
    tree_node(tree_node&&) noexcept = default;
    tree_node& operator=(const tree_node&) = default;
    tree_node& operator=(tree_node&&) noexcept = default;
    ~tree_node() noexcept = default;

    tree_node() noexcept : parent_{}, left_{}, right_{}
    {}

    value_type& get_pair() noexcept
    {
      return static_cast<data_node*>(this)->get_pair();
    }
  };

  class data_node : public tree_node
  {
    friend class splay_tree<Key, Data, Comparator, Allocator>;

  private:
    value_type key_data_pair_;

  public:
    data_node(const data_node&) = delete;
    data_node(data_node&&) noexcept = default;
    data_node& operator=(const data_node&) = delete;
    data_node& operator=(data_node&&) noexcept = default;
    ~data_node() noexcept = default;

    data_node() noexcept : key_data_pair_{}
    {}

    template<class Pair>
    explicit data_node(Pair&& data) noexcept : key_data_pair_{ std::forward<Pair>(data) }
    {}

    value_type& get_pair() noexcept
    {
      return key_data_pair_;
    }
  };

public:
  class iterator
  {
    friend class splay_tree<Key, Data, Comparator, Allocator>;

  public:
    using value_type = std::pair<const Key, Data>;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;

  protected:
    tree_node* node_;

  public:
    iterator() noexcept = default;
    iterator(const iterator&) noexcept = default;
    iterator(iterator&&) noexcept = default;
    iterator& operator=(const iterator&) noexcept = default;
    iterator& operator=(iterator&&) noexcept = default;
    ~iterator() noexcept = default;

    explicit iterator(tree_node* node) noexcept : node_{ node }
    {}

    reference operator*() const noexcept
    {
      return node_->get_pair();
    }

    pointer operator->() const noexcept
    {
      return &node_->get_pair();
    }

    iterator& operator++() noexcept
    {
      if (node_->right_ == nullptr)
      {
        tree_node* parent_node;

        while ((parent_node = node_->parent_) != nullptr && node_ == parent_node->right_)
        {
          node_ = parent_node;
        }

        node_ = parent_node;
      }
      else
      {
        node_ = find_sub_tree_min(node_->right_);
      }

      return *this;
    }

    iterator operator++(int) noexcept
    {
      iterator temp = *this;
      ++*this;
      return temp;
    }

    iterator& operator--() noexcept
    {
      if (node_->left_ == nullptr)
      {
        tree_node* parent_node;

        while ((parent_node = node_->parent_) != nullptr && node_ == parent_node->left_)
        {
          node_ = parent_node;
        }

        node_ = parent_node;
      }
      else
      {
        node_ = find_sub_tree_max(node_->left_);
      }

      return *this;
    }

    iterator operator--(int) noexcept
    {
      iterator temp = *this;
      --*this;
      return temp;
    }

    bool operator==(const iterator& obj) const noexcept
    {
      return node_ == obj.node_;
    }

    bool operator!=(const iterator& obj) const noexcept
    {
      return node_ != obj.node_;
    }
  };

  class const_iterator
  {
    friend class splay_tree<Key, Data, Comparator, Allocator>;

  private:
    iterator it_;

  public:
    using value_type = const std::pair<const Key, Data>;
    using reference = const value_type&;
    using pointer = const value_type*;
    using iterator_category = std::bidirectional_iterator_tag;
    using difference_type = std::ptrdiff_t;

    const_iterator() noexcept = default;
    const_iterator(const const_iterator&) noexcept = default;
    const_iterator(const_iterator&&) noexcept = default;
    const_iterator& operator=(const const_iterator&) noexcept = default;
    const_iterator& operator=(const_iterator&&) noexcept = default;
    ~const_iterator() noexcept = default;

    explicit const_iterator(tree_node* node) : it_{ node }
    {}

    reference operator*() const noexcept
    {
      return it_.node_->get_pair();
    }

    pointer operator->() const noexcept
    {
      return &it_.node_->get_pair();
    }

    const_iterator& operator++() noexcept
    {
      ++it_;
      return *this;
    }

    const_iterator operator++(int) noexcept
    {
      const_iterator temp = *this;
      ++it_;
      return temp;
    }

    const_iterator& operator--() noexcept
    {
      --it_;
      return *this;
    }

    const_iterator operator--(int) noexcept
    {
      const_iterator temp = *this;
      --it_;
      return temp;
    }

    bool operator==(const const_iterator& obj) const noexcept
    {
      return it_.node_ == obj.it_.node_;
    }

    bool operator!=(const const_iterator& obj) const noexcept
    {
      return it_.node_ != obj.it_.node_;
    }
  };

private:
  internal::node_allocator_t<Allocator, data_node> node_allocator_;
  Comparator comparator_;
  mutable tree_node end_ = {};
  tree_node* root_ = {};
  tree_node* begin_ = &end_;
  std::size_t tree_size_ = {};

private:
  template<class T>
  tree_node* allocate_and_construct_node(T&& data)
  {
    data_node* result = node_allocator_.allocate(1);
    std::construct_at(result, std::forward<T>(data));

    return result;
  }

  template<class... Args>
  tree_node* allocate_and_construct_node_emplace(const Key& key, Args&&... args)
  {
    data_node* result = node_allocator_.allocate(1);
    std::construct_at(std::addressof(result->get_pair().second), std::forward<Args>(args)...);
    std::construct_at(std::addressof(result->get_pair().first), key);

    result->right_ = {};
    result->left_ = {};
    result->parent_ = {};

    return result;
  }

  void insert_node(tree_node* node_to_insert) noexcept
  {
    auto [target_node, prev_node] = find_internal(node_to_insert->get_pair().first);

    if (target_node == nullptr && prev_node == nullptr)
    {
      root_ = node_to_insert;
      node_to_insert->right_ = &end_;
      end_.parent_ = node_to_insert;
      begin_ = node_to_insert;
      ++tree_size_;
    }
    else if (target_node && target_node != &end_)
    {
      splay(target_node);
      std::destroy_at(static_cast<data_node*>(node_to_insert));
      node_allocator_.deallocate(static_cast<data_node*>(node_to_insert), 1);
    }
    else
    {
      tree_node** where_to_place_ptr = comparator_(node_to_insert->get_pair().first, prev_node->get_pair().first) ?
        &prev_node->left_ : &prev_node->right_;

      if (*where_to_place_ptr == &end_)
      {
        node_to_insert->right_ = &end_;
        end_.parent_ = node_to_insert;
      }

      if (comparator_(node_to_insert->get_pair().first, begin_->get_pair().first))
      {
        begin_ = node_to_insert;
      }

      node_to_insert->parent_ = prev_node;
      *where_to_place_ptr = node_to_insert;
      splay(node_to_insert);
      ++tree_size_;
    }
  }

  std::pair<tree_node*, tree_node*> find_internal(const Key& key) noexcept
  {
    tree_node* current_node = root_, * prev_node = {};

    while (current_node && current_node != &end_)
    {
      prev_node = current_node;

      if (comparator_(key, current_node->get_pair().first))
      {
        current_node = current_node->left_;
      }
      else if (comparator_(current_node->get_pair().first, key))
      {
        current_node = current_node->right_;
      }
      else
      {
        break;
      }
    }

    return { current_node, prev_node };
  }

  void erase_internal(tree_node* target_node) noexcept
  {
    splay(target_node);

    tree_node* left_sub_tree = target_node->left_;
    tree_node* right_sub_tree = target_node->right_;

    if (left_sub_tree == nullptr && right_sub_tree == nullptr)
    {
      tree_size_ = 0;
      begin_ = &end_;
      root_ = nullptr;
    }
    else if (left_sub_tree == nullptr)
    {
      begin_ = find_sub_tree_min(right_sub_tree);
      splay(begin_);
      begin_->left_ = nullptr;
    }
    else if (right_sub_tree == nullptr)
    {
      tree_node* new_root = find_sub_tree_max(left_sub_tree);
      splay(new_root);
      new_root->right_ = &end_;
      end_.parent_ = new_root;
    }
    else
    {
      tree_node* new_root = find_sub_tree_max(left_sub_tree);
      splay(new_root);
      new_root->right_ = right_sub_tree;
      right_sub_tree->parent_ = new_root;
    }

    std::destroy_at(static_cast<data_node*>(target_node));
    node_allocator_.deallocate(static_cast<data_node*>(target_node), 1);
    --tree_size_;
  }

  static tree_node* find_sub_tree_min(tree_node* obj) noexcept
  {
    tree_node* current_node = obj;
    for (; current_node->left_ != nullptr; current_node = current_node->left_);

    return current_node;
  }

  static tree_node* find_sub_tree_max(tree_node* obj) noexcept
  {
    tree_node* current_node = obj;
    for (; current_node->right_ != nullptr; current_node = current_node->right_);

    return current_node;
  }

  void zig(tree_node* target_node) noexcept
  {
    root_ = target_node;
    tree_node* target_node_parent = target_node->parent_;
    tree_node* target_node_right_child = target_node->right_;

    target_node->parent_ = nullptr;
    target_node->right_ = target_node_parent;
    target_node_parent->parent_ = target_node;
    target_node_parent->left_ = target_node_right_child;

    if (target_node_right_child)
    {
      target_node_right_child->parent_ = target_node_parent;
    }
  }

  void zag(tree_node* target_node) noexcept
  {
    root_ = target_node;
    tree_node* target_node_parent = target_node->parent_;
    tree_node* target_node_left_child = target_node->left_;

    target_node->parent_ = nullptr;
    target_node->left_ = target_node_parent;
    target_node_parent->parent_ = target_node;
    target_node_parent->right_ = target_node_left_child;

    if (target_node_left_child)
    {
      target_node_left_child->parent_ = target_node_parent;
    }
  }

  void zig_zig(tree_node* target_node_parent) noexcept
  {
    tree_node* target_node = target_node_parent->left_;
    tree_node* target_node_parent_right_child = target_node_parent->right_;
    tree_node* sub_tree_root = target_node_parent->parent_;
    tree_node* grandparent = sub_tree_root->parent_;

    target_node_parent->parent_ = target_node;

    if (target_node_parent_right_child)
    {
      target_node_parent_right_child->parent_ = sub_tree_root;
    }

    target_node_parent->left_ = target_node->right_;

    if (target_node->right_)
    {
      target_node->right_->parent_ = target_node_parent;
    }

    target_node->right_ = target_node_parent;

    target_node->parent_ = sub_tree_root->parent_;
    sub_tree_root->parent_ = target_node_parent;

    sub_tree_root->left_ = target_node_parent_right_child;
    target_node_parent->right_ = sub_tree_root;

    if (grandparent)
    {
      if (grandparent->left_ == sub_tree_root) grandparent->left_ = target_node;
      else grandparent->right_ = target_node;
    }
    else
    {
      root_ = target_node;
    }
  }

  void zag_zag(tree_node* target_node_parent) noexcept
  {
    tree_node* target_node = target_node_parent->right_;
    tree_node* target_node_parent_left_child = target_node_parent->left_;
    tree_node* sub_tree_root = target_node_parent->parent_;
    tree_node* grandparent = sub_tree_root->parent_;

    target_node_parent->parent_ = target_node;

    if (target_node_parent_left_child)
    {
      target_node_parent_left_child->parent_ = sub_tree_root;
    }

    target_node_parent->right_ = target_node->left_;

    if (target_node->left_)
    {
      target_node->left_->parent_ = target_node_parent;
    }

    target_node->left_ = target_node_parent;

    target_node->parent_ = sub_tree_root->parent_;
    sub_tree_root->parent_ = target_node_parent;

    sub_tree_root->right_ = target_node_parent_left_child;
    target_node_parent->left_ = sub_tree_root;

    if (grandparent)
    {
      if (grandparent->left_ == sub_tree_root) grandparent->left_ = target_node;
      else grandparent->right_ = target_node;
    }
    else
    {
      root_ = target_node;
    }
  }

  void zig_zag(tree_node* target_node_parent) noexcept
  {
    tree_node* target_node = target_node_parent->left_;
    tree_node* sub_tree_root = target_node_parent->parent_;
    tree_node* grandparent = sub_tree_root->parent_;

    target_node->parent_ = sub_tree_root->parent_;

    if (target_node->left_)
    {
      target_node->left_->parent_ = sub_tree_root;
    }

    sub_tree_root->right_ = target_node->left_;
    target_node->left_ = sub_tree_root;

    if (target_node->right_)
    {
      target_node->right_->parent_ = target_node_parent;
    }

    target_node_parent->left_ = target_node->right_;
    target_node->right_ = target_node_parent;
    target_node_parent->parent_ = target_node;
    sub_tree_root->parent_ = target_node;

    if (grandparent)
    {
      if (grandparent->left_ == sub_tree_root) grandparent->left_ = target_node;
      else grandparent->right_ = target_node;
    }
    else
    {
      root_ = target_node;
    }
  }

  void zag_zig(tree_node* target_node_parent) noexcept
  {
    tree_node* target_node = target_node_parent->right_;
    tree_node* sub_tree_root = target_node_parent->parent_;
    tree_node* grandparent = sub_tree_root->parent_;

    target_node->parent_ = sub_tree_root->parent_;

    if (target_node->right_)
    {
      target_node->right_->parent_ = sub_tree_root;
    }

    sub_tree_root->left_ = target_node->right_;
    target_node->right_ = sub_tree_root;

    if (target_node->left_)
    {
      target_node->left_->parent_ = target_node_parent;
    }

    target_node_parent->right_ = target_node->left_;
    target_node->left_ = target_node_parent;
    target_node_parent->parent_ = target_node;
    sub_tree_root->parent_ = target_node;

    if (grandparent)
    {
      if (grandparent->left_ == sub_tree_root) grandparent->left_ = target_node;
      else grandparent->right_ = target_node;
    }
    else
    {
      root_ = target_node;
    }
  }

  void splay(tree_node* target_node) noexcept
  {
    while (target_node->parent_ != nullptr)
    {
      tree_node* parent = target_node->parent_;
      tree_node* grandparent = parent->parent_;

      if (grandparent == nullptr)
      {
        if (parent->left_ == target_node)
        {
          zig(target_node);
        }
        else
        {
          zag(target_node);
        }
      }
      else if (grandparent->left_ == parent && parent->left_ == target_node)
      {
        zig_zig(parent);
      }
      else if (grandparent->right_ == parent && parent->right_ == target_node)
      {
        zag_zag(parent);
      }
      else if (grandparent->right_ == parent && parent->left_ == target_node)
      {
        zig_zag(parent);
      }
      else
      {
        zag_zig(parent);
      }
    }
  }

public:
  splay_tree() : node_allocator_{}, comparator_{}
  {}

  splay_tree(std::initializer_list<value_type> list, const Comparator& comp = Comparator{}, const Allocator& alloc = Allocator{})
    : node_allocator_{ alloc }, comparator_{ comp }
  {
    insert(list);
  }

  template<std::input_iterator It>
  splay_tree(It begin, It end, const Comparator& comp = Comparator{}, const Allocator& alloc = Allocator{})
    : node_allocator_{ alloc }, comparator_{ comp }
  {
    insert(begin, end);
  }

  splay_tree(const splay_tree& obj)
    : node_allocator_{ obj.node_allocator_ }, comparator_{ obj.comparator_ }
  {
    insert(obj.begin(), obj.end());
  }

  splay_tree(splay_tree&& obj) noexcept
  {
    swap(obj);
  }

  splay_tree& operator=(const splay_tree& obj)
  {
    if (&obj == this)
    {
      return *this;
    }

    splay_tree{ obj }.swap(*this);

    return *this;
  }

  splay_tree& operator=(splay_tree&& obj)
  {
    if (&obj == this)
    {
      return *this;
    }

    swap(obj);

    return *this;
  }

  ~splay_tree() noexcept
  {
    clear();
  }

  Data& at(const Key& key)
  {
    auto result = find(key);

    if (result.node_ == &end_)
    {
      throw std::out_of_range{ "splay_tree: key was out of range." };
    }

    return result->second;
  }

  iterator find(const Key& key) noexcept
  {
    auto [target_node, prev_node] = find_internal(key);

    if (target_node == nullptr || target_node == &end_)
    {
      target_node = &end_;

      if (prev_node)
      {
        splay(prev_node);
      }
    }
    else
    {
      splay(target_node);
    }

    return iterator{ target_node };
  }

  template<class SplayTree>
  void merge(SplayTree&& obj)
  {
    if (obj.root_ == nullptr)
    {
      return;
    }

    std::queue<tree_node*> node_queue;
    node_queue.push(obj.root_);
    if (obj.end_.parent_)
    {
      obj.end_.parent_->right_ = nullptr;
      obj.end_.parent_ = nullptr;
    }

    while (!node_queue.empty())
    {
      tree_node* current_node = node_queue.front();
      node_queue.pop();

      if (current_node->right_) node_queue.push(current_node->right_);
      if (current_node->left_) node_queue.push(current_node->left_);

      current_node->left_ = nullptr;
      current_node->right_ = nullptr;

      insert_node(current_node);
    }

    obj.begin_ = &obj.end_;
    obj.root_ = nullptr;
    obj.tree_size_ = 0;
  }

  void swap(splay_tree& obj) noexcept
  {
    if (begin_ == &end_)
    {
      begin_ = &obj.end_;
    }

    if (obj.begin_ == &obj.end_)
    {
      obj.begin_ = &end_;
    }

    std::swap(node_allocator_, obj.node_allocator_);
    std::swap(comparator_, obj.comparator_);
    std::swap(end_, obj.end_);
    std::swap(root_, obj.root_);
    std::swap(begin_, obj.begin_);
    std::swap(tree_size_, obj.tree_size_);
  }

  Data& operator[](const Key& key)
  {
    return emplace(key).first.node_->get_pair().second;
  }

  template<class... Args>
  std::pair<iterator, bool> emplace(Args&&... args)
  {
    tree_node* new_node;
    const auto& key = internal::extract_key(std::forward<Args>(args)...);
    std::pair<iterator, bool> result = {};

    auto [target_node, prev_node] = find_internal(key);

    if (target_node == nullptr && prev_node == nullptr)
    {
      new_node = allocate_and_construct_node_emplace(std::forward<Args>(args)...);
      root_ = new_node;
      new_node->right_ = &end_;
      end_.parent_ = new_node;
      begin_ = new_node;

      result.second = true;
      ++tree_size_;
    }
    else if (target_node && target_node != &end_)
    {
      new_node = target_node;
      result.second = false;
    }
    else
    {
      tree_node** where_to_place_ptr = comparator_(key, prev_node->get_pair().first)
        ? &prev_node->left_ : &prev_node->right_;
      new_node = allocate_and_construct_node_emplace(std::forward<Args>(args)...);

      if (*where_to_place_ptr == &end_)
      {
        new_node->right_ = &end_;
        end_.parent_ = new_node;
      }

      if (comparator_(new_node->get_pair().first, begin_->get_pair().first))
      {
        begin_ = new_node;
      }

      new_node->parent_ = prev_node;
      *where_to_place_ptr = new_node;
      result.second = true;
      ++tree_size_;
    }

    splay(new_node);
    result.first = iterator{ new_node };

    return result;
  }

  template<std::input_iterator It>
  void insert(It begin, It end)
  {
    for (; begin != end; ++begin)
    {
      insert(*begin);
    }
  }

  template<std::ranges::input_range Range>
  void insert(Range&& range)
  {
    insert(std::begin(range), std::end(range));
  }

  template<class Pair>
  std::pair<iterator, bool> insert(Pair&& data)
  {
    return emplace(data.first, data.second);
  }

  iterator erase(iterator begin, iterator end) noexcept
  {
    while (begin != end)
    {
      iterator temp = begin++;
      erase_internal(temp.node_);
    }

    return end;
  }

  iterator erase(iterator it) noexcept
  {
    iterator temp_value = it++;
    erase_internal(temp_value.node_);

    return it;
  }

  bool erase(const Key& key) noexcept
  {
    auto resulted_pair = find(key);

    if (resulted_pair.node_ == &end_)
    {
      return false;
    }

    erase_internal(resulted_pair.node_);
    return true;
  }

  void clear()
  {
    if (root_ == nullptr)
    {
      return;
    }

    std::queue<tree_node*> node_queue;
    node_queue.push(root_);
    end_.parent_->right_ = nullptr;

    while (!node_queue.empty())
    {
      tree_node* current_node = node_queue.front();
      node_queue.pop();

      if (current_node->right_) node_queue.push(current_node->right_);
      if (current_node->left_) node_queue.push(current_node->left_);

      std::destroy_at(static_cast<data_node*>(current_node));
      node_allocator_.deallocate(static_cast<data_node*>(current_node), 1);
    }

    tree_size_ = 0;
    root_ = nullptr;
    begin_ = &end_;
  }

  [[nodiscard]] bool empty() const noexcept
  {
    return tree_size_ == 0;
  }

  [[nodiscard]] std::size_t size() const noexcept
  {
    return tree_size_;
  }

  iterator begin() noexcept
  {
    return iterator{ begin_ };
  }

  iterator end() noexcept
  {
    return iterator{ &end_ };
  }

  [[nodiscard]] const_iterator begin() const noexcept
  {
    return const_iterator{ begin_ };
  }

  [[nodiscard]] const_iterator end() const noexcept
  {
    return const_iterator{ &end_ };
  }

  [[nodiscard]] const_iterator cbegin() const noexcept
  {
    return const_iterator{ begin_ };
  }

  [[nodiscard]] const_iterator cend() const noexcept
  {
    return const_iterator{ &end_ };
  }
};
