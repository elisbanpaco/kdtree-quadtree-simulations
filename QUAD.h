#ifndef QUAD_H
#define QUAD_H
// ************ Quad Tree *************************
//************************************************ */
#include <cmath>
#include <iostream>
using namespace std;

struct Point {
	int x;
	int y;
	Point(int _x, int _y)
	{
		x = _x;
		y = _y;
	}
	Point()
	{
		x = 0;
		y = 0;
	}
};

struct Node {
	Point pos;
	int data;
	Node(Point _pos, int _data)
	{
		pos = _pos;
		data = _data;
	}
	Node() { data = 0; }
};

class QuadTree {

public:
	// Hold details of the boundary of this node
	Point topLeft;
	Point botRight;

	Node* n;

	QuadTree* topLeftTree;
	QuadTree* topRightTree;
	QuadTree* botLeftTree;
	QuadTree* botRightTree;
	QuadTree()
	{
		topLeft = Point(0, 0);
		botRight = Point(0, 0);
		n = NULL;
		topLeftTree = NULL;
		topRightTree = NULL;
		botLeftTree = NULL;
		botRightTree = NULL;
	}
	QuadTree(Point topL, Point botR)
	{
		n = NULL;
		topLeftTree = NULL;
		topRightTree = NULL;
		botLeftTree = NULL;
		botRightTree = NULL;
		topLeft = topL;
		botRight = botR;
	}
	void insert(Node*);
	Node* search(Point);
	bool inBoundary(Point);
};

// Insert a node into the quadtree
void QuadTree::insert(Node* node)
{
	if (node == NULL)
		return;

	if (!inBoundary(node->pos))
		return;

	if (abs(topLeft.x - botRight.x) <= 1
		&& abs(topLeft.y - botRight.y) <= 1) {
		if (n == NULL)
			n = node;
		return;
	}

	if ((topLeft.x + botRight.x) / 2 >= node->pos.x) {
		if ((topLeft.y + botRight.y) / 2 >= node->pos.y) {
			if (topLeftTree == NULL)
				topLeftTree = new QuadTree(
					Point(topLeft.x, topLeft.y),
					Point((topLeft.x + botRight.x) / 2,
						(topLeft.y + botRight.y) / 2));
			topLeftTree->insert(node);
		}
		else {
			if (botLeftTree == NULL)
				botLeftTree = new QuadTree(
					Point(topLeft.x,
						(topLeft.y + botRight.y) / 2),
					Point((topLeft.x + botRight.x) / 2,
						botRight.y));
			botLeftTree->insert(node);
		}
	}
	else {
		if ((topLeft.y + botRight.y) / 2 >= node->pos.y) {
			if (topRightTree == NULL)
				topRightTree = new QuadTree(
					Point((topLeft.x + botRight.x) / 2,
						topLeft.y),
					Point(botRight.x,
						(topLeft.y + botRight.y) / 2));
			topRightTree->insert(node);
		}

		else {
			if (botRightTree == NULL)
				botRightTree = new QuadTree(
					Point((topLeft.x + botRight.x) / 2,
						(topLeft.y + botRight.y) / 2),
					Point(botRight.x, botRight.y));
			botRightTree->insert(node);
		}
	}
}

// Find a node in a quadtree
Node* QuadTree::search(Point p)
{
	if (!inBoundary(p))
		return NULL;

	if (n != NULL)
		return n;

	if ((topLeft.x + botRight.x) / 2 >= p.x) {
		if ((topLeft.y + botRight.y) / 2 >= p.y) {
			if (topLeftTree == NULL)
				return NULL;
			return topLeftTree->search(p);
		}
		else {
			if (botLeftTree == NULL)
				return NULL;
			return botLeftTree->search(p);
		}
	}
	else {
		if ((topLeft.y + botRight.y) / 2 >= p.y) {
			if (topRightTree == NULL)
				return NULL;
			return topRightTree->search(p);
		}
		else {
			if (botRightTree == NULL)
				return NULL;
			return botRightTree->search(p);
		}
	}
};

bool QuadTree::inBoundary(Point p)
{
	return (p.x >= topLeft.x && p.x <= botRight.x
			&& p.y >= topLeft.y && p.y <= botRight.y);
}

#endif // QUAD_H
