#include "hashTable.h"

template<typename Key, size_t N>
typename HashTable<Key, N>::size_type HashTable<Key, N>::h(const key_type& key) const {
	return std::hash<key_type>{}(key) % (1 << d);
}

template<typename Key, size_t N>
typename HashTable<Key, N>::size_type HashTable<Key, N>::h2(const key_type& key) const {
	return std::hash<key_type>{}(key) % (1 << (d + 1));
}

template <typename Key, size_t N>
HashTable<Key, N>::~HashTable() {
	for (size_type i{ 0 }; i < table_size; ++i) {
		if (table[i].overflow) {
			block *head{ table[i].overflow };
			block *help;
			while (head) {
				help = head;
				head = head->overflow;
				delete help;
			}
		}
	}
	delete[] table;
}

template<typename Key, size_t N>
void HashTable<Key, N>::split(block* overf) {
	if (next_to_split == static_cast<size_type>((1 << d))) {
		block* old_table{ table };
		size_type old_table_size{ table_size };
		table_size *= 2;
		table = new block[table_size + 1];
		table[table_size].curr_size = -1;
		for (size_type i{ 0 }; i < old_table_size; ++i) {
			if (i != next_to_split) {
				table[i] = old_table[i];
			}
			else {
				for (int j{ 0 }; j < old_table[i].curr_size; ++j) {
					--curr_size;
					insert_(old_table[i].block_table[j], true);
				}
			}
		}

		while (overf) {
			size_type help_size{ static_cast<size_type>(overf->curr_size) };
			overf->curr_size = 0;
			for (size_type i{ 0 }; i < help_size; ++i) {
				--curr_size;
				insert_(overf->block_table[i], true);
			}
			block* help{ overf };
			overf = overf->overflow;
			delete help;
		}
		delete[] old_table;
		next_to_split = 0;
		++d;
	}
	else {
		size_type curr_block_size{ static_cast<size_type>(table[next_to_split].curr_size) };
		table[next_to_split].curr_size = 0;
		for (size_type i{ 0 }; i < curr_block_size; ++i) {
			--curr_size;
			insert_(table[next_to_split].block_table[i], true);
		}
		while (overf) {
			size_type help_size{ static_cast<size_type>(overf->curr_size) };
			overf->curr_size = 0;
			for (size_type i{ 0 }; i < help_size; ++i) {
				--curr_size;
				insert_(overf->block_table[i], true);
			}
			overf = overf->overflow;
		}
		++next_to_split;
	}
}

template <typename Key, size_t N>
typename HashTable<Key, N>::block* HashTable<Key, N>::find_(const key_type& key) const {
	size_type idx{ h(key) };
	if (idx < next_to_split) idx = h2(key);
	int count{ 0 };
	while (count < table[idx].curr_size) {
		if (std::equal_to<key_type>{}(table[idx].block_table[count], key)) return table+idx;
		++count;
	}
	if (!table[idx].overflow) return nullptr;
	block* help{ table[idx].overflow };
	while (help) {
		for (int i{ 0 }; i < help->curr_size; ++i) {
			if (std::equal_to<key_type>{}(help->block_table[i], key)) return help;
		}
		help = help->overflow;
	}
	return nullptr;
}

template <typename Key, size_t N>
void HashTable<Key, N>::insert_(const key_type& key, bool sp /* = false */) {
	size_type idx;
	if (!sp) idx = h(key);
	else idx = h2(key);
	if (idx < next_to_split) idx = h2(key);

	if (table[idx].curr_size == static_cast<int>(block_size)) {
		if (!table[idx].overflow) {
			table[idx].overflow = new block;
			table[idx].overflow->block_table[0] = key;
			++table[idx].overflow->curr_size;
			if (!sp) split(table[next_to_split].overflow);
			++curr_size;
			if (table[idx].overflow->curr_size == 0) {
				delete table[idx].overflow;
				table[idx].overflow = nullptr;
			}
			return;
		}
		block* help{ table[idx].overflow };
		while (help) {
			if (static_cast<int>(block_size) > help->curr_size) {
				help->block_table[help->curr_size++] = key;
				if (!sp) split(table[next_to_split].overflow);
				++curr_size;
				return;
			}
			if (!help->overflow) {
				help->overflow = new block;
				help->overflow->block_table[0] = key;
				++help->overflow->curr_size;
				if (!sp) split(table[next_to_split].overflow);
				++curr_size;
				return;
			}
			help = help->overflow;
		}
	}
	else {
		++curr_size;
		table[idx].block_table[table[idx].curr_size++] = key;
		return;
	}
}

template<typename Key, size_t N>
HashTable<Key, N>::HashTable() {
	table = new block[table_size + 1];
	table[table_size].curr_size = -1;
}

template <typename Key, size_t N>
HashTable<Key, N>::HashTable(const HashTable& other) {
	table_size = other.table_size;
	table = new block[table_size + 1];
	table[table_size].curr_size = -1;
	d = other.d;
	next_to_split = other.next_to_split;
	for (const auto& a : other) insert_(a);
}

template <typename Key, size_t N>
HashTable<Key, N>::HashTable(std::initializer_list<key_type> ilist) : HashTable{} { insert(ilist); }

template <typename Key, size_t N>
template<typename InputIt>
HashTable<Key,N>::HashTable(InputIt first, InputIt last) : HashTable{} { insert(first, last); }

template <typename Key,size_t N>
typename HashTable<Key,N>::HashTable& HashTable<Key,N>::operator=(const HashTable& other) {
	if (this == &other) return *this;
	HashTable temp{ other };
	swap(temp);
	return *this;
}

template <typename Key, size_t N>
typename HashTable<Key, N>::HashTable& HashTable<Key, N>::operator=(std::initializer_list<key_type> ilist) {
	HashTable temp{ ilist };
	swap(temp);
	return *this;
}

template <typename Key, size_t N>
typename HashTable<Key,N>::size_type  HashTable<Key,N>::size() const { return curr_size; }

template <typename Key, size_t N>
bool HashTable<Key,N>::empty() const { return 0 == curr_size; }

template <typename Key, size_t N>
typename HashTable<Key,N>::size_type HashTable<Key,N>::count(const key_type& key) const { return !!find_(key); }

template <typename Key, size_t N>
typename HashTable<Key,N>::iterator HashTable<Key,N>::find(const key_type& key) const {
	size_type idx{ h(key) };
	if (idx < next_to_split) idx = h2(key);
	iterator it{ table + idx };
	if (find_(key)) {
		while (true) {
			if (std::equal_to<key_type>{}(*it, key)) return it;
			++it;
		}
	}
	return end();
}

template <typename Key, size_t N>
void HashTable<Key,N>::clear() { HashTable temp{}; swap(temp); }

template <typename Key, size_t N>
void HashTable<Key,N>::swap(HashTable& other) {
	std::swap(table, other.table);
	std::swap(table_size, other.table_size);
	std::swap(curr_size, other.curr_size);
	std::swap(d, other.d);
	std::swap(next_to_split, other.next_to_split);
	std::swap(block_size, other.block_size);
}

template <typename Key, size_t N>
void HashTable<Key,N>::insert(std::initializer_list<key_type> ilist) { insert(std::begin(ilist), std::end(ilist)); }

template <typename Key, size_t N>
std::pair<typename HashTable<Key,N>::iterator, bool> HashTable<Key,N>::insert(const key_type& key) {
	size_type idx{ h(key) };
	if (idx < next_to_split) idx = h2(key);
	iterator it{ table + idx };
	if (find_(key)) {
		while (true) {
			if (std::equal_to<key_type>{}(*it, key)) return { it, false };
			++it;
		}
	}
	insert_(key);
	return { find(key), true };
}

template <typename Key, size_t N>
template<typename InputIt> 
void HashTable<Key,N>::insert(InputIt first, InputIt last) {
	for (auto it{ first }; it != last; ++it) {
		if (!count(*it)) insert_(*it);
	}
}

template <typename Key, size_t N>
typename HashTable<Key,N>::size_type HashTable<Key,N>::erase(const key_type& key) {
	if (block * del{ find(key) }) {
		--curr_size;
		if (del->curr_size == 1) {
			--del->curr_size;
			size_type idx{ h(key) };
			if (idx < next_to_split) idx = h2(key);
			if (table[idx].curr_size == 0 && table[idx].overflow) {
				if (table[idx].overflow->curr_size > 0) {
					std::swap(table[idx].block_table, table[idx].overflow->block_table);
					table[idx].curr_size = table[idx].overflow->curr_size;
					block* help{ table[idx].overflow->overflow };
					delete table[idx].overflow;
					table[idx].overflow = help;
					return 1;
				}
			}
			del = table + idx;
			while (del) {
				if (del->overflow) {
					if (del->overflow->curr_size == 0) {
						if (del->overflow->overflow) {
							block* help = del->overflow;
							del->overflow = del->overflow->overflow;
							delete help;
							return 1;
						}
						else {
							delete del->overflow;
							del->overflow = nullptr;
							return 1;
						}
					}
				}
				del = del->overflow;
			}
			return 1;
		}
		for (int i{ 0 }; i < del->curr_size; ++i) {
			if (std::equal_to<key_type>{}(del->block_table[i], key)) {
				std::swap(del->block_table[i], del->block_table[--del->curr_size]);
			}
		}
		return 1;
	}
	return 0;
}


template <typename Key, size_t N>
typename HashTable<Key,N>::const_iterator HashTable<Key,N>::begin() const { return const_iterator{ table }; }

template <typename Key, size_t N>
typename HashTable<Key, N>::const_iterator HashTable<Key, N>::end() const { return const_iterator{ table + table_size }; }
 
template <typename Key, size_t N>
void HashTable<Key,N>::dump(std::ostream& o) const {
	o << "Table size: " << table_size << ", Current size: " << curr_size << ", Next_to_split: " << next_to_split << ", Round: " << d << "\n";
	for (size_type i{ 0 }; i < table_size; ++i) {
		if (table[i].curr_size != 0 || table[i].overflow) {
			o << i << ": ";
			for (int j{ 0 }; j < table[i].curr_size; ++j) {
				o << table[i].block_table[j] << " ";
			}
			if (table[i].overflow) {
				block* help{ table[i].overflow };
				while (help) {
					o << ", overflow(" << help->curr_size << "): ";
					for (int i{ 0 }; i < help->curr_size; ++i) {
						o << help->block_table[i] << " ";
					}
					help = help->overflow;
				}
			}
			o << ", size: " << table[i].curr_size << "\n";
		}
	}
}


//______________________________________________Iterator____________________________________________

template <typename Key, size_t N>
void HashTable<Key, N>::Iterator::skip() {
	while (pos->curr_size == 0 && pos->curr_size != -1) {
		if (pos->overflow) {
			if (pos->overflow->curr_size > 0) break;
		}
		++pos;
	}
}

template <typename Key, size_t N>
HashTable<Key, N>::Iterator::Iterator(block* pos) : pos{ pos } { if (pos) skip(); }

template <typename Key, size_t N>
typename HashTable<Key, N>::Iterator::reference HashTable<Key, N>::Iterator::operator*() const { return pos->block_table[count]; }

template <typename Key, size_t N>
typename HashTable<Key, N>::Iterator::pointer HashTable<Key, N>::Iterator::operator->() const { return &pos->block_table[count]; }

template <typename Key, size_t N>
typename HashTable<Key, N>::Iterator& HashTable<Key, N>::Iterator::operator++() {
	if (++count < pos->curr_size) return *this;
	else {
		count = 0;
		if (pos->overflow) {
			if (pos->overflow->curr_size > 0) {
				if (!root) root = pos;
				pos = pos->overflow;
			}
			else {
				if (root) {
					pos = root;
					++pos;
					root = nullptr;
					skip();
				}
				else {
					++pos;
					skip();
				}
			}
		}
		else {
			if (root) {
				pos = root;
				++pos;
				root = nullptr;
				skip();
			}
			else {
				++pos;
				skip();
			}
		}
		return *this;
	}
}

template <typename Key, size_t N>
typename HashTable<Key, N>::Iterator HashTable<Key, N>::Iterator::operator++(int) {
	auto help{ *this };
	if (++count < pos->curr_size) return help;
	else {
		count = 0;
		if (pos->overflow) {
			if (pos->overflow->curr_size > 0) {
				if (!root) root = pos;
				pos = pos->overflow;
			}
			else {
				if (root) {
					pos = root;
					++pos;
					root = nullptr;
					skip();
				}
				else {
					++pos;
					skip();
				}
			}
		}
		else {
			if (root) {
				pos = root;
				++pos;
				root = nullptr;
				skip();
			}
			else {
				++pos;
				skip();
			}
		}
		return help;
	}
}

template <typename Key, size_t N> void swap(HashTable<Key, N>& lhs, HashTable<Key, N>& rhs) { lhs.swap(rhs); }