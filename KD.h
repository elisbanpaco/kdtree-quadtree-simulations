#ifndef KD_H
#define KD_H
#include <vector>
using namespace std;

const int k = 3;

class Node {
public:
    vector<int> point;
    Node* left;
    Node* right;

    // Constructor
    Node(vector<int> arr) : point(arr), left(nullptr), right(nullptr) {}
};

// Clase para representar el KD-Tree
class KDTree {
private:
    Node* root;

    // Función recursiva para insertar un nodo en el árbol
    Node* insertRec(Node* root, vector<int> point, unsigned depth) {
        // Si el árbol está vacío, retorna un nuevo nodo
        if (root == nullptr)
            return new Node(point);

        // Calcular el eje actual de comparación
        unsigned cd = depth % k;

        // Decidir si se va al subárbol izquierdo o derecho
        if (point[cd] < root->point[cd]) {
            root->left = insertRec(root->left, point, depth + 1);
        } else { // incluir puntos iguales en el subárbol derecho
            root->right = insertRec(root->right, point, depth + 1);
        }

        return root;
    }

    // Función recursiva para buscar un punto en el KD-Tree
    bool searchRec(Node* root, vector<int> point, unsigned depth) {
        // Caso base: el nodo es nulo
        if (root == nullptr)
            return false;

        // Si el punto es el mismo
        if (root->point == point)
            return true;

        // Calcular el eje actual de comparación
        unsigned cd = depth % k;

        // Decidir si buscar en el subárbol izquierdo o derecho
        if (point[cd] < root->point[cd])
            return searchRec(root->left, point, depth + 1);
        
        return searchRec(root->right, point, depth + 1);
    }

public:
    // Constructor del KDTree
    KDTree() : root(nullptr) {}

    // Función para insertar un punto en el KD-Tree
    void insert(vector<int> point) {
        root = insertRec(root, point, 0);
    }

    // Función para buscar un punto en el KD-Tree
    bool search(vector<int> point) {
        return searchRec(root, point, 0);
    }
};
#endif // KD_H
