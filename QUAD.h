#ifndef QUAD_H
#define QUAD_H
/**
 * @file QUAD.h
 * @brief Implementación de QuadTree para indexación espacial 2D
 *
 * Estructura de datos que particiona el espacio 2D en cuatro regiones cuadrantes,
 * permitiendo búsqueda y inserción en O(log n) promedio para distribuciones uniforme.
 */
#include <cmath>
#include <iostream>
using namespace std;

/**
 * @struct Point
 * @brief Punto 2D con coordenadas enteras
 */
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

/**
 * @struct Node
 * @brief Nodo conteniendo posición e identificador de entidad
 */
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

/**
 * @class QuadTree
 * @brief QuadTree para indexación espacial 2D
 *
 * Particiona recursivamente el espacio en cuatro cuadrantes.
 * Inserción: O(log n) promedio | Búsqueda: O(log n) promedio
 */
class QuadTree {

public:
	/// Boundary del nodo (esquina superior izquierda)
	Point topLeft;
	/// Boundary del nodo (esquina inferior derecha)
	Point botRight;

	/// Puntero al nodo contenido (hoja)
	Node* n;

	/// Subárboles: cuadrantes NW, NE, SW, SE
	QuadTree* topLeftTree;
	QuadTree* topRightTree;
	QuadTree* botLeftTree;
	QuadTree* botRightTree;
	
	/**
	 * @brief Constructor por defecto
	 */
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
	
	/**
	 * @brief Constructor con boundary
	 * @param topL Esquina superior izquierda
	 * @param botR Esquina inferior derecha
	 */
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
	
	/**
	 * @brief Inserta un nodo en el QuadTree
	 * @param node Puntero al nodo a insertar
	 * @note Complejidad: O(log n) promedio
	 */
	void insert(Node*);
	
	/**
	 * @brief Busca un punto en el QuadTree
	 * @param p Punto a buscar
	 * @return Puntero al nodo encontrado, o nullptr si no existe
	 * @note Complejidad: O(log n) promedio
	 */
	Node* search(Point);
	
	/**
	 * @brief Verifica si un punto está dentro del boundary
	 * @param p Punto a verificar
	 * @return true si está dentro, false en caso contrario
	 */
	bool inBoundary(Point);
};

/**
 * @brief Inserta un nodo en el QuadTree
 * @details Si el nodo actual es hoja (sin datos), almacena el nodo.
 *          De lo contrario, determina el cuadrante y lo inserta recursivamente.
 * @param node Puntero al nodo a insertar
 * @pre El nodo debe estar dentro del boundary actual
 */
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

/**
 * @brief Busca un punto en el QuadTree
 * @details Realiza una búsqueda descendente por el cuadrante correspondiente
 * @param p Punto a buscar
 * @return Nodo encontrado o nullptr
 */
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

/**
 * @brief Verifica si un punto está dentro del boundary
 * @param p Punto a verificar
 * @return true si el punto está dentro del rectángulo definido por topLeft y botRight
 */
bool QuadTree::inBoundary(Point p)
{
	return (p.x >= topLeft.x && p.x <= botRight.x
			&& p.y >= topLeft.y && p.y <= botRight.y);
}

#endif // QUAD_H
