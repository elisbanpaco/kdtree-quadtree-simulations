#ifndef KD_H
#define KD_H
/**
 * @file KD.h
 * @brief Implementación de KD-Tree para indexación espacial 3D
 *
 * Árbol k-dimensional que particiona el espacio por planos perpendiculares
 * a los ejes, rotando cíclicamente entre dimensiones (X → Y → Z → X...).
 * Inserción: O(log n) promedio | Búsqueda: O(log n) promedio
 */
#include <vector>
using namespace std;

const int k = 3;  // Dimensionalidad del espacio

/**
 * @class Node
 * @brief Nodo del KD-Tree conteniendo un punto k-dimensional
 */
class Node {
public:
    /// Punto k-dimensional (x, y, z)
    vector<int> point;
    /// Subárbol izquierdo (puntos menores en el eje actual)
    Node* left;
    /// Subárbol derecho (puntos mayores o iguales en el eje actual)
    Node* right;

    /**
     * @brief Constructor
     * @param arr Vector de k enteros representando el punto
     */
    Node(vector<int> arr) : point(arr), left(nullptr), right(nullptr) {}
};

/**
 * @class KDTree
 * @brief KD-Tree para indexación espacial 3D
 *
 * Implementa inserción y búsqueda en espacio tridimensional.
 * Complejidad teórica: O(log n) promedio para ambas operaciones.
 */
class KDTree {
private:
    Node* root;

    /**
     * @brief Inserción recursiva
     * @param root Nodo raíz del subárbol actual
     * @param point Punto a insertar
     * @param depth Profundidad actual (determina el eje de comparación)
     * @return Nodo insertado
     * @note Complejidad: O(log n) promedio
     */
    Node* insertRec(Node* root, vector<int> point, unsigned depth) {
        if (root == nullptr)
            return new Node(point);

        unsigned cd = depth % k;

        if (point[cd] < root->point[cd]) {
            root->left = insertRec(root->left, point, depth + 1);
        } else {
            root->right = insertRec(root->right, point, depth + 1);
        }

        return root;
    }

    /**
     * @brief Búsqueda recursiva
     * @param root Nodo raíz del subárbol actual
     * @param point Punto a buscar
     * @param depth Profundidad actual
     * @return true si el punto existe, false en caso contrario
     * @note Complejidad: O(log n) promedio
     */
    bool searchRec(Node* root, vector<int> point, unsigned depth) {
        if (root == nullptr)
            return false;

        if (root->point == point)
            return true;

        unsigned cd = depth % k;

        if (point[cd] < root->point[cd])
            return searchRec(root->left, point, depth + 1);
        
        return searchRec(root->right, point, depth + 1);
    }

public:
    /**
     * @brief Constructor por defecto
     */
    KDTree() : root(nullptr) {}

    /**
     * @brief Inserta un punto en el KD-Tree
     * @param point Vector de k enteros representando el punto 3D
     * @note Complejidad: O(log n) promedio
     */
    void insert(vector<int> point) {
        root = insertRec(root, point, 0);
    }

    /**
     * @brief Busca un punto en el KD-Tree
     * @param point Vector de k enteros representando el punto 3D
     * @return true si el punto existe, false en caso contrario
     * @note Complejidad: O(log n) promedio
     */
    bool search(vector<int> point) {
        return searchRec(root, point, 0);
    }
};
#endif // KD_H
