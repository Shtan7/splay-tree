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

private:
	class node
	{
		friend class splay_tree<Key, Data, Comparator, Allocator>;

	private:
		std::pair<const Key, Data> key_data_pair_;
		node* parent_;
		node* left_;
		node* right_;

	public:
		node(const node&) = delete;
		node(node&&) noexcept = delete;
		node& operator=(const node&) = delete;
		node& operator=(node&&) noexcept = delete;
		~node() noexcept = default;

		node() noexcept : key_data_pair_{}, parent_{}, left_{}, right_{}
		{}

		template<class Pair>
		explicit node(Pair&& data) noexcept : key_data_pair_{ std::forward<Pair>(data) }, parent_{}, left_{}, right_{}
		{}
	};

public:
	class const_iterator
	{
		friend class splay_tree<Key, Data, Comparator, Allocator>;
		using iter_value_type = std::pair<const Key, Data>;

	public:
		using value_type = iter_value_type;
		using reference = iter_value_type&;
		using pointer = iter_value_type*;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = ptrdiff_t;

	private:
		node* node_;

	public:
		const_iterator() noexcept = default;
		const_iterator(const const_iterator&) noexcept = default;
		const_iterator(const_iterator&&) noexcept = default;
		const_iterator& operator=(const const_iterator&) noexcept = default;
		const_iterator& operator=(const_iterator&&) noexcept = default;
		~const_iterator() noexcept = default;

		explicit const_iterator(node* node) noexcept : node_{ node }
		{}

		const iter_value_type& operator*() const noexcept
		{
			return node_->key_data_pair_;
		}

		const iter_value_type* operator->() const noexcept
		{
			return &node_->key_data_pair_;
		}

		const_iterator& operator++() noexcept
		{
			if (node_->right_ == nullptr)
			{
				node* parent_node;

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

		const_iterator operator++(int) noexcept
		{
      const_iterator temp = *this;
			++*this;
			return temp;
		}

		const_iterator& operator--() noexcept
		{
			if (node_->left_ == nullptr)
			{
				node* parent_node;

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

		const_iterator operator--(int) noexcept
		{
			const_iterator temp = *this;
			--*this;
			return temp;
		}

		bool operator==(const const_iterator& obj) const noexcept
		{
			return node_ == obj.node_;
		}

		bool operator!=(const const_iterator& obj) const noexcept
		{
			return node_ != obj.node_;
		}
	};

	class iterator
	{
		friend class splay_tree<Key, Data, Comparator, Allocator>;
		using iter_value_type = std::pair<const Key, Data>;

	public:
		using value_type = iter_value_type;
		using reference = iter_value_type&;
		using pointer = iter_value_type*;
		using iterator_category = std::bidirectional_iterator_tag;
		using difference_type = std::ptrdiff_t;

	private:
		node* node_;

	public:
		iterator() noexcept = default;
		iterator(const iterator&) noexcept = default;
		iterator(iterator&&) noexcept = default;
		iterator& operator=(const iterator&) noexcept = default;
		iterator& operator=(iterator&&) noexcept = default;
		~iterator() noexcept = default;

		explicit iterator(node* node) noexcept : node_{ node }
		{}

		iter_value_type& operator*() const noexcept
		{
			return node_->key_data_pair_;
		}

		iter_value_type* operator->() const noexcept
		{
			return &node_->key_data_pair_;
		}

		iterator& operator++() noexcept
		{
			if (node_->right_ == nullptr)
			{
				node* parent_node;

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
				node* parent_node;

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
	using node_type = node;

private:
	internal::node_allocator_t<Allocator, node> node_allocator_;
	Comparator comparator_;
	node* root_;
	node* begin_;
	node* end_;
	std::size_t tree_size_;

private:
	void default_initialization()
	{
		root_ = {};
		tree_size_ = {};
		end_ = node_allocator_.allocate(1);
		end_->right_ = {};
		end_->left_ = {};
		begin_ = end_;
	}

	template<class T>
	node* allocate_and_construct_node(T&& data)
	{
		node* result = node_allocator_.allocate(1);
		std::construct_at(result, std::forward<T>(data));

		return result;
	}

	template<class... Args>
	node* allocate_and_construct_node_emplace(const Key& key, Args&&... args)
	{
		node* result = node_allocator_.allocate(1);
		std::construct_at(std::addressof(result->key_data_pair_.second), std::forward<Args>(args)...);
		std::construct_at(std::addressof(result->key_data_pair_.first), key);

		result->right_ = {};
		result->left_ = {};
		result->parent_ = {};

		return result;
	}

	void insert_node(node* node_to_insert) noexcept
	{
		auto [target_node, prev_node] = find_internal(node_to_insert->key_data_pair_.first);

		if (target_node == nullptr && prev_node == nullptr)
		{
			root_ = node_to_insert;
			node_to_insert->right_ = end_;
			end_->parent_ = node_to_insert;
			begin_ = node_to_insert;
			++tree_size_;
		}
		else if (target_node && target_node != end_)
		{
			splay(target_node);
			std::destroy_at(node_to_insert);
			node_allocator_.deallocate(node_to_insert, 1);
		}
		else
		{
			node** where_to_place_ptr = comparator_(node_to_insert->key_data_pair_.first, prev_node->key_data_pair_.first) ?
				&prev_node->left_ : &prev_node->right_;

			if (*where_to_place_ptr == end_)
			{
				node_to_insert->right_ = end_;
				end_->parent_ = node_to_insert;
			}

			if (comparator_(node_to_insert->key_data_pair_.first, begin_->key_data_pair_.first))
			{
				begin_ = node_to_insert;
			}

			node_to_insert->parent_ = prev_node;
			*where_to_place_ptr = node_to_insert;
			splay(node_to_insert);
			++tree_size_;
		}
	}

	std::pair<node*, node*> find_internal(const Key& key) noexcept
	{
		node* current_node = root_, * prev_node = {};

		while (current_node && current_node != end_)
		{
			prev_node = current_node;

			if (comparator_(key, current_node->key_data_pair_.first))
			{
				current_node = current_node->left_;
			}
			else if (comparator_(current_node->key_data_pair_.first, key))
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

	void erase_internal(node* target_node) noexcept
	{
		splay(target_node);

		node* left_sub_tree = target_node->left_;
		node* right_sub_tree = target_node->right_;

		if (left_sub_tree == nullptr && right_sub_tree == nullptr)
		{
			tree_size_ = 0;
			begin_ = end_;
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
			node* new_root = find_sub_tree_max(left_sub_tree);
			splay(new_root);
			new_root->right_ = end_;
			end_->parent_ = new_root;
		}
		else
		{
			node* new_root = find_sub_tree_max(left_sub_tree);
			splay(new_root);
			new_root->right_ = right_sub_tree;
			right_sub_tree->parent_ = new_root;
		}

		std::destroy_at(target_node);
		node_allocator_.deallocate(target_node, 1);
		--tree_size_;
	}

	static node* find_sub_tree_min(node* obj) noexcept
	{
		node* current_node = obj;
		for (; current_node->left_ != nullptr; current_node = current_node->left_);

		return current_node;
	}

	static node* find_sub_tree_max(node* obj) noexcept
	{
		node* current_node = obj;
		for (; current_node->right_ != nullptr; current_node = current_node->right_);

		return current_node;
	}

	void zig(node* target_node) noexcept
	{
		root_ = target_node;
		node* target_node_parent = target_node->parent_;
		node* target_node_right_child = target_node->right_;

		target_node->parent_ = nullptr;
		target_node->right_ = target_node_parent;
		target_node_parent->parent_ = target_node;
		target_node_parent->left_ = target_node_right_child;

		if (target_node_right_child)
		{
			target_node_right_child->parent_ = target_node_parent;
		}
	}

	void zag(node* target_node) noexcept
	{
		root_ = target_node;
		node* target_node_parent = target_node->parent_;
		node* target_node_left_child = target_node->left_;

		target_node->parent_ = nullptr;
		target_node->left_ = target_node_parent;
		target_node_parent->parent_ = target_node;
		target_node_parent->right_ = target_node_left_child;

		if (target_node_left_child)
		{
			target_node_left_child->parent_ = target_node_parent;
		}
	}

	void zig_zig(node* target_node_parent) noexcept
	{
		node* target_node = target_node_parent->left_;
		node* target_node_parent_right_child = target_node_parent->right_;
		node* sub_tree_root = target_node_parent->parent_;
		node* grandparent = sub_tree_root->parent_;

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

	void zag_zag(node* target_node_parent) noexcept
	{
		node* target_node = target_node_parent->right_;
		node* target_node_parent_left_child = target_node_parent->left_;
		node* sub_tree_root = target_node_parent->parent_;
		node* grandparent = sub_tree_root->parent_;

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

	void zig_zag(node* target_node_parent) noexcept
	{
		node* target_node = target_node_parent->left_;
		node* sub_tree_root = target_node_parent->parent_;
		node* grandparent = sub_tree_root->parent_;

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

	void zag_zig(node* target_node_parent) noexcept
	{
		node* target_node = target_node_parent->right_;
		node* sub_tree_root = target_node_parent->parent_;
		node* grandparent = sub_tree_root->parent_;

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

	void splay(node* target_node) noexcept
	{
		while (target_node->parent_ != nullptr)
		{
			node* parent = target_node->parent_;
			node* grandparent = parent->parent_;

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
	{
		default_initialization();
	}

  splay_tree(std::initializer_list<value_type> list, const Comparator& comp = Comparator{}, const Allocator& alloc = Allocator{})
		: node_allocator_{ alloc }, comparator_{ comp }
	{
    default_initialization();
    insert(list);
	}

	template<std::input_iterator It>
	splay_tree(It begin, It end, const Comparator& comp = Comparator{}, const Allocator& alloc = Allocator{})
		: node_allocator_{ alloc }, comparator_{ comp }
	{
		default_initialization();
		insert(begin, end);
	}

	template<std::ranges::input_range Range>
	explicit splay_tree(Range&& range, const Comparator& comp = Comparator{}, const Allocator& alloc = Allocator{})
		: node_allocator_{ alloc }, comparator_{ comp }
	{
		default_initialization();
		insert(std::begin(range), std::end(range));
	}

	splay_tree(const splay_tree& obj, const Comparator& comp = Comparator{}, const Allocator& alloc = Allocator{})
		: node_allocator_{ alloc }, comparator_{ comp }
	{
		default_initialization();
		insert(obj.begin(), obj.end());
	}

	splay_tree(splay_tree&& obj, const Comparator& comp = Comparator{}, const Allocator& alloc = Allocator{})
		: node_allocator_{ alloc }, comparator_{ comp }
	{
		default_initialization();

		root_ = obj.root_;
		begin_ = obj.begin_;
		tree_size_ = obj.tree_size_;
		obj.end_->parent_->right_ = end_;
		end_->parent_ = obj.end_->parent_;

		obj.root_ = nullptr;
		obj.begin_ = obj.end_;
		obj.tree_size_ = 0;
		obj.end_->parent_ = nullptr;
	}

	splay_tree& operator=(const splay_tree& obj)
	{
		if (&obj == this)
		{
			return *this;
		}

		clear();
		insert(obj.begin(), obj.end());

		return *this;
	}

	splay_tree& operator=(splay_tree&& obj)
	{
		if (&obj == this)
		{
			return *this;
		}

		clear();

		root_ = obj.root_;
		begin_ = obj.begin_;
		tree_size_ = obj.tree_size_;
		obj.end_->parent_->right_ = end_;
		end_->parent_ = obj.end_->parent_;

		obj.root_ = nullptr;
		obj.begin_ = obj.end_;
		obj.tree_size_ = 0;
		obj.end_->parent_ = nullptr;

		return *this;
	}

	~splay_tree() noexcept
	{
		clear();
		node_allocator_.deallocate(end_, 1);
	}

	Data& at(const Key& key)
	{
		auto [target_node, prev_node] = find_internal(key);

		if (target_node == nullptr || target_node == end_)
		{
			if (prev_node != nullptr)
			{
				splay(prev_node);
			}

			throw std::out_of_range{ "splay_tree: key was out of range." };
		}

		splay(target_node);
		return target_node->key_data_pair_.second;
	}

	iterator find(const Key& key) noexcept
	{
		auto [target_node, prev_node] = find_internal(key);

		if (target_node == nullptr || target_node == end_)
		{
			target_node = end_;

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

		std::queue<node*> node_queue;
		node_queue.push(obj.root_);
		if (obj.end_->parent_)
		{
			obj.end_->parent_->right_ = nullptr;
			obj.end_->parent_ = nullptr;
		}

		while (!node_queue.empty())
		{
			node* current_node = node_queue.front();
			node_queue.pop();

			if (current_node->right_) node_queue.push(current_node->right_);
			if (current_node->left_) node_queue.push(current_node->left_);

			current_node->left_ = nullptr;
			current_node->right_ = nullptr;

			insert_node(current_node);
		}

		obj.begin_ = obj.end_;
		obj.root_ = nullptr;
		obj.tree_size_ = 0;
	}

	Data& operator[](const Key& key)
	{
		auto resulted_pair = emplace(key);
		return resulted_pair.first.node_->key_data_pair_.second;
	}

	template<class... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		node* new_node;
		const auto& key = internal::extract_key(std::forward<Args>(args)...);
		std::pair<iterator, bool> result = {};

		auto [target_node, prev_node] = find_internal(key);

		if (target_node == nullptr && prev_node == nullptr)
		{
			new_node = allocate_and_construct_node_emplace(std::forward<Args>(args)...);
			root_ = new_node;
			new_node->right_ = end_;
			end_->parent_ = new_node;
			begin_ = new_node;

			result.second = true;
			++tree_size_;
		}
		else if (target_node && target_node != end_)
		{
			new_node = target_node;
			result.second = false;
		}
		else
		{
			node** where_to_place_ptr = comparator_(key, prev_node->key_data_pair_.first)
				? &prev_node->left_ : &prev_node->right_;
			new_node = allocate_and_construct_node_emplace(std::forward<Args>(args)...);

			if (*where_to_place_ptr == end_)
			{
				new_node->right_ = end_;
				end_->parent_ = new_node;
			}

			if (comparator_(new_node->key_data_pair_.first, begin_->key_data_pair_.first))
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
		node* new_node;
		auto [target_node, prev_node] = find_internal(data.first);
		std::pair<iterator, bool> result = {};

		if (target_node == nullptr && prev_node == nullptr)
		{
			new_node = allocate_and_construct_node(std::forward<Pair>(data));
			root_ = new_node;
			new_node->right_ = end_;
			end_->parent_ = new_node;
			begin_ = new_node;

			result.second = true;
			++tree_size_;
		}
		else if (target_node && target_node != end_)
		{
			new_node = target_node;
			result.second = false;
		}
		else
		{
			node** where_to_place_ptr = comparator_(data.first, prev_node->key_data_pair_.first)
				? &prev_node->left_ : &prev_node->right_;
			new_node = allocate_and_construct_node(std::forward<Pair>(data));

			if (*where_to_place_ptr == end_)
			{
				new_node->right_ = end_;
				end_->parent_ = new_node;
			}

			if (comparator_(new_node->key_data_pair_.first, begin_->key_data_pair_.first))
			{
				begin_ = new_node;
			}

			*where_to_place_ptr = new_node;
			new_node->parent_ = prev_node;
			++tree_size_;
			result.second = true;
		}

		splay(new_node);
		result.first = iterator{ new_node };

		return result;
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
		auto [target_node, prev_node] = find_internal(key);

		if (target_node == nullptr || target_node == end_)
		{
			if (prev_node != nullptr)
			{
				splay(prev_node);
			}

      return false;
		}

		if (target_node->right_ == end_)
		{
			target_node->right_ = nullptr;
		}

		erase_internal(target_node);
		return true;
	}

	void clear()
	{
		if (root_ == nullptr)
		{
			return;
		}

		std::queue<node*> node_queue;
		node_queue.push(root_);
		end_->parent_->right_ = nullptr;

		while (!node_queue.empty())
		{
			node* current_node = node_queue.front();
			node_queue.pop();

			if (current_node->right_) node_queue.push(current_node->right_);
			if (current_node->left_) node_queue.push(current_node->left_);

			std::destroy_at(current_node);
			node_allocator_.deallocate(current_node, 1);
		}

		tree_size_ = 0;
		root_ = nullptr;
		begin_ = end_;
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
		return iterator{ end_ };
	}

	[[nodiscard]] const_iterator begin() const noexcept
	{
		return const_iterator{ begin_ };
	}

	[[nodiscard]] const_iterator end() const noexcept
	{
		return const_iterator{ end_ };
	}

	[[nodiscard]] const_iterator cbegin() const noexcept
	{
		return const_iterator{ begin_ };
	}

	[[nodiscard]] const_iterator cend() const noexcept
	{
		return const_iterator{ end_ };
	}
};
