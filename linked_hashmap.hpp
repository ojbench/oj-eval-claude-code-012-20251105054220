/**
 * implement a container like std::linked_hashmap
 */
#ifndef SJTU_LINKEDHASHMAP_HPP
#define SJTU_LINKEDHASHMAP_HPP

// only for std::equal_to<T> and std::hash<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    /**
     * In linked_hashmap, iteration ordering is differ from map,
     * which is the order in which keys were inserted into the map.
     * You should maintain a doubly-linked list running through all
     * of its entries to keep the correct iteration order.
     *
     * Note that insertion order is not affected if a key is re-inserted
     * into the map.
     */
    
template<
	class Key,
	class T,
	class Hash = std::hash<Key>,
	class Equal = std::equal_to<Key>
> class linked_hashmap {
private:
	// Node structure for hash table buckets and linked list
	struct Node {
		pair<const Key, T> data;
		Node* next; // next in hash bucket
		Node* prev; // prev in hash bucket
		Node* list_next; // next in insertion order
		Node* list_prev; // prev in insertion order

		Node(const pair<const Key, T>& val) : data(val), next(nullptr), prev(nullptr), list_next(nullptr), list_prev(nullptr) {}
		Node(pair<const Key, T>&& val) : data(std::move(val)), next(nullptr), prev(nullptr), list_next(nullptr), list_prev(nullptr) {}
	};

	// Hash table parameters
	static const size_t INITIAL_CAPACITY = 16;
	static const double LOAD_FACTOR;

	Node** buckets;
	size_t bucket_count;
	size_t element_count;

	// Doubly linked list for insertion order
	Node* head;
	Node* tail;

	Hash hasher;
	Equal key_equal;

	// Helper functions
	size_t hash_index(const Key& key) const {
		return hasher(key) % bucket_count;
	}

	void rehash(size_t new_capacity) {
		Node** new_buckets = new Node*[new_capacity]();

		for (size_t i = 0; i < bucket_count; ++i) {
			Node* current = buckets[i];
			while (current) {
				Node* next = current->next;
				size_t new_index = hasher(current->data.first) % new_capacity;

				// Insert at head of new bucket
				current->next = new_buckets[new_index];
				if (new_buckets[new_index]) {
					new_buckets[new_index]->prev = current;
				}
				current->prev = nullptr;
				new_buckets[new_index] = current;

				current = next;
			}
		}

		delete[] buckets;
		buckets = new_buckets;
		bucket_count = new_capacity;
	}

	void ensure_capacity() {
		if (element_count >= bucket_count * LOAD_FACTOR) {
			rehash(bucket_count * 2);
		}
	}

public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::linked_hashmap as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;
 
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = linked_hashmap.begin(); --it;
	 *       or it = linked_hashmap.end(); ++end();
	 */
	friend class iterator;
	friend class const_iterator;

	class const_iterator;
	class iterator {
	public:
		Node* current;
		const linked_hashmap* container;

		iterator(Node* node, const linked_hashmap* cont) : current(node), container(cont) {}
		// The following code is written for the C++ type_traits library.
		// Type traits is a C++ feature for describing certain properties of a type.
		// For instance, for an iterator, iterator::value_type is the type that the
		// iterator points to.
		// STL algorithms and containers may use these type_traits (e.g. the following
		// typedef) to work properly.
		// See these websites for more information:
		// https://en.cppreference.com/w/cpp/header/type_traits
		// About value_type: https://blog.csdn.net/u014299153/article/details/72419713
		// About iterator_category: https://en.cppreference.com/w/cpp/iterator
		using difference_type = std::ptrdiff_t;
		using value_type = typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::bidirectional_iterator_tag;

		iterator() : current(nullptr), container(nullptr) {}
		iterator(const iterator &other) : current(other.current), container(other.container) {}

		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			iterator temp = *this;
			if (current) {
				current = current->list_next;
			}
			return temp;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
			if (current) {
				current = current->list_next;
			}
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			iterator temp = *this;
			if (current) {
				current = current->list_prev;
			}
			return temp;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
			if (current) {
				current = current->list_prev;
			}
			return *this;
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
			return current->data;
		}
		bool operator==(const iterator &rhs) const {
			return current == rhs.current;
		}
		bool operator==(const const_iterator &rhs) const {
			return current == rhs.current;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const {
			return current != rhs.current;
		}
		bool operator!=(const const_iterator &rhs) const {
			return current != rhs.current;
		}

		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {
			return &(current->data);
		}
	};
 
	class const_iterator {
	public:
		const Node* current;
		const linked_hashmap* container;

		const_iterator(const Node* node, const linked_hashmap* cont) : current(node), container(cont) {}
		using difference_type = std::ptrdiff_t;
		using value_type = const typename linked_hashmap::value_type;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::bidirectional_iterator_tag;

		const_iterator() : current(nullptr), container(nullptr) {}
		const_iterator(const const_iterator &other) : current(other.current), container(other.container) {}
		const_iterator(const iterator &other) : current(other.current), container(other.container) {}

		const_iterator operator++(int) {
			const_iterator temp = *this;
			if (current) {
				current = current->list_next;
			}
			return temp;
		}

		const_iterator & operator++() {
			if (current) {
				current = current->list_next;
			}
			return *this;
		}

		const_iterator operator--(int) {
			const_iterator temp = *this;
			if (current) {
				current = current->list_prev;
			}
			return temp;
		}

		const_iterator & operator--() {
			if (current) {
				current = current->list_prev;
			}
			return *this;
		}

		const value_type & operator*() const {
			return current->data;
		}

		bool operator==(const const_iterator &rhs) const {
			return current == rhs.current;
		}

		bool operator==(const iterator &rhs) const {
			return current == rhs.current;
		}

		bool operator!=(const const_iterator &rhs) const {
			return current != rhs.current;
		}

		bool operator!=(const iterator &rhs) const {
			return current != rhs.current;
		}

		const value_type* operator->() const noexcept {
			return &(current->data);
		}
	};
 
	/**
	 * TODO two constructors
	 */
	linked_hashmap() : bucket_count(INITIAL_CAPACITY), element_count(0), head(nullptr), tail(nullptr) {
		buckets = new Node*[bucket_count]();
	}

	linked_hashmap(const linked_hashmap &other) : bucket_count(other.bucket_count), element_count(0), head(nullptr), tail(nullptr) {
		buckets = new Node*[bucket_count]();
		try {
			for (const_iterator it = other.cbegin(); it != other.cend(); ++it) {
				insert(*it);
			}
		} catch (...) {
			clear();
			delete[] buckets;
			throw;
		}
	}

	/**
	 * TODO assignment operator
	 */
	linked_hashmap & operator=(const linked_hashmap &other) {
		if (this != &other) {
			linked_hashmap temp(other);
			swap(temp);
		}
		return *this;
	}

	void swap(linked_hashmap &other) {
		std::swap(buckets, other.buckets);
		std::swap(bucket_count, other.bucket_count);
		std::swap(element_count, other.element_count);
		std::swap(head, other.head);
		std::swap(tail, other.tail);
	}

	/**
	 * TODO Destructors
	 */
	~linked_hashmap() {
		clear();
		delete[] buckets;
	}
 
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
		iterator it = find(key);
		if (it == end()) {
			throw index_out_of_bound();
		}
		return it->second;
	}

	const T & at(const Key &key) const {
		const_iterator it = find(key);
		if (it == cend()) {
			throw index_out_of_bound();
		}
		return it->second;
	}

	/**
	 * TODO
	 * access specified element
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
		iterator it = find(key);
		if (it != end()) {
			return it->second;
		}
		pair<iterator, bool> result = insert(value_type(key, T()));
		return result.first->second;
	}

	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
		return at(key);
	}

	/**
	 * return a iterator to the beginning
	 */
	iterator begin() {
		return iterator(head, this);
	}

	const_iterator cbegin() const {
		return const_iterator(head, this);
	}

	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() {
		return iterator(nullptr, this);
	}

	const_iterator cend() const {
		return const_iterator(nullptr, this);
	}

	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const {
		return element_count == 0;
	}

	/**
	 * returns the number of elements.
	 */
	size_t size() const {
		return element_count;
	}

	/**
	 * clears the contents
	 */
	void clear() {
		Node* current = head;
		while (current) {
			Node* next = current->list_next;
			delete current;
			current = next;
		}
		head = tail = nullptr;
		element_count = 0;

		// Clear buckets
		for (size_t i = 0; i < bucket_count; ++i) {
			buckets[i] = nullptr;
		}
	}
 
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion),
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		ensure_capacity();

		size_t index = hash_index(value.first);
		Node* current = buckets[index];

		// Check if key already exists
		while (current) {
			if (key_equal(current->data.first, value.first)) {
				return pair<iterator, bool>(iterator(current, this), false);
			}
			current = current->next;
		}

		// Create new node
		Node* new_node = new Node(value);

		// Insert into hash bucket
		new_node->next = buckets[index];
		if (buckets[index]) {
			buckets[index]->prev = new_node;
		}
		buckets[index] = new_node;

		// Insert into linked list
		if (!head) {
			head = tail = new_node;
		} else {
			tail->list_next = new_node;
			new_node->list_prev = tail;
			tail = new_node;
		}

		++element_count;
		return pair<iterator, bool>(iterator(new_node, this), true);
	}

	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if (pos == end() || pos.container != this) {
			throw invalid_iterator();
		}

		Node* node = pos.current;

		// Remove from hash bucket
		size_t index = hash_index(node->data.first);
		if (node->prev) {
			node->prev->next = node->next;
		} else {
			buckets[index] = node->next;
		}
		if (node->next) {
			node->next->prev = node->prev;
		}

		// Remove from linked list
		if (node->list_prev) {
			node->list_prev->list_next = node->list_next;
		} else {
			head = node->list_next;
		}
		if (node->list_next) {
			node->list_next->list_prev = node->list_prev;
		} else {
			tail = node->list_prev;
		}

		delete node;
		--element_count;
	}

	/**
	 * Returns the number of elements with key
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0
	 *     since this container does not allow duplicates.
	 */
	size_t count(const Key &key) const {
		return find(key) != cend() ? 1 : 0;
	}

	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) {
		size_t index = hash_index(key);
		Node* current = buckets[index];

		while (current) {
			if (key_equal(current->data.first, key)) {
				return iterator(current, this);
			}
			current = current->next;
		}

		return end();
	}

	const_iterator find(const Key &key) const {
		size_t index = hash_index(key);
		const Node* current = buckets[index];

		while (current) {
			if (key_equal(current->data.first, key)) {
				return const_iterator(current, this);
			}
			current = current->next;
		}

		return cend();
	}
};

// Static member definition
template<class Key, class T, class Hash, class Equal>
const double linked_hashmap<Key, T, Hash, Equal>::LOAD_FACTOR = 0.75;

}

#endif
