#pragma once

#include <string>

template<class T>
class Node
{
public:
	T data;
	Node<T> * next;
	Node<T>(const T& d) : data(d), next() {}
	Node<T>(const Node<T>& copyNode) : data(copyNode.data), next() {}
	Node<T>(const Node<T>& copyNode, Node<T>* next)
		: data(copyNode.data)
		, next(next)
	{}

private:
	Node<T>& operator=(const Node<T>&) = delete;
};

template<class T>
class LinkedList
{
public:

	Node<T> * head;
	Node<T> * tail;

	LinkedList(const LinkedList& LL);

	LinkedList& operator=(LinkedList byValList) {
		// BUT this line is not enough
		//     Assignment should make a copy of all the elements.
		std::swap(head, byValList.head);

		// Usually this is implemented as:
		// Now you need to write a version of swap for this class
		// (both member and free standing)
		byValList.swap(*this);

		return *this;
	}
	LinkedList() : head(NULL), tail(NULL) {}
	LinkedList(Node<T> * newNode) : head(newNode), tail(newNode) {}
	~LinkedList();

	static LinkedList<int> sumLists(const LinkedList<int>& LL1, LinkedList<int>& LL2);

	void insertToTail(T val);
	void insertToHead(T val);
	void print();
	void printBackwards();
};


class CConfigVar
{
protected:
public:
	CConfigVar() {

	}
	~CConfigVar() {

	}
};

class CConfig
{
protected:
	bool FindConfig;

public:
	CConfig();
	virtual ~CConfig();

	std::string GetString() {

	}
};

