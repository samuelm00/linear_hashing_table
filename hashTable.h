#pragma once
#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <functional>
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <cmath>

template <typename Key, size_t N = 4>
class HashTable {
public:
	class Iterator;
	using value_type = Key;
	using key_type = Key;
	using reference = key_type&;
	using const_reference = const key_type&;
	using size_type = size_t;
	using difference_type = std::ptrdiff_t;
	using iterator = Iterator;
	using const_iterator = Iterator;
	using key_equal = std::equal_to<key_type>; // Hashing
	using hasher = std::hash<key_type>;        // Hashing

private:

	struct block {
		key_type block_table[N];
		block* overflow{ nullptr };
		int curr_size{ 0 };
	};

	block* table;
	size_type curr_size{ 0 };
	size_type table_size{ 8 };
	size_type block_size{ N };
	size_type next_to_split{ 0 };
	size_type d{ 2 };

	size_type h(const key_type& key) const;
	size_type h2(const key_type& key) const;
	void split(block* overf);
	block* find_(const key_type& key) const;
	void insert_(const key_type& key, bool sp = false);

public:

	HashTable();
	HashTable(std::initializer_list<key_type> ilist);
	template<typename InputIt> HashTable(InputIt first, InputIt last);
	HashTable(const HashTable& other);
	~HashTable();
	HashTable& operator=(const HashTable& other);
	HashTable& operator=(std::initializer_list<key_type> ilist);
	size_type size() const;
	bool empty() const;
	size_type count(const key_type& key) const;
	iterator find(const key_type& key) const;
	void clear();
	void swap(HashTable& other);
	void insert(std::initializer_list<key_type> ilist);
	std::pair<iterator, bool> insert(const key_type& key);
	template<typename InputIt> void insert(InputIt first, InputIt last);
	size_type erase(const key_type& key);
	const_iterator begin() const;
	const_iterator end() const;
	void dump(std::ostream& o = std::cerr) const;

	friend bool operator==(const HashTable<Key, N>& lhs, const HashTable<Key, N>& rhs) {
		if (lhs.curr_size != rhs.curr_size) return false;
		for (const auto& it : rhs) {
			if (!(lhs.count(it))) return false;
		}
		return true;
	}
	friend bool operator!=(const HashTable<Key, N>& lhs, const HashTable<Key, N>& rhs) { return !(lhs == rhs); }
};

template <typename Key, size_t N>
class HashTable<Key, N>::Iterator {
public:
	using value_type = Key;
	using difference_type = std::ptrdiff_t;
	using reference = const value_type&;
	using pointer = const value_type*;
	using iterator_category = std::forward_iterator_tag;
private:
	block* pos;
	block* root{ nullptr }; 
	int count{ 0 };
	void skip();

public:
	explicit Iterator(block* pos = nullptr);
	reference operator*() const;
	pointer operator->() const;
	Iterator& operator++();
	Iterator operator++(int);
	friend bool operator==(const Iterator& lhs, const Iterator& rhs) { return lhs.pos->block_table + lhs.count == rhs.pos->block_table + rhs.count; }
	friend bool operator!=(const Iterator& lhs, const Iterator& rhs) { return !(lhs.pos->block_table + lhs.count == rhs.pos->block_table + rhs.count); }
};



#endif 