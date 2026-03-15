#include <GL/glut.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "KD.h"

using namespace std;
// Definición de vértices del cubo
//float scale = 1.5f;

// Definir colores para los ejes
float axisColors[3][3] = {
    {0.0f, 0.0f, 1.0f},  // Azul para X
    {1.0f, 0.0f, 0.0f},  // Rojo para Y
    {0.0f, 1.0f, 0.0f}   // Verde para Z
};

// Variable para ajustar la longitud de los ejes
float axisLength = 100.0f;

// Estructura para representar un espacio 3D
struct Space {
    float minX, maxX, minY, maxY, minZ, maxZ;
};

// Función para dibujar una línea
void drawLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glEnd();
    glVertex3f(x2, y2, z2);
}

// Función para dibujar texto en 3D
void drawText3D(const char* text, float x, float y, float z) {
    glRasterPos3f(x, y, z);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }
}

// Función para dibujar un eje con cinta métrica
void drawAxis(int axis) {
    glColor3fv(axisColors[axis]);
    
    // Dibujar el eje principal
    drawLine(0, 0, 0, 
             axis == 0 ? axisLength : 0,
             axis == 1 ? axisLength : 0,
             axis == 2 ? axisLength : 0);
    
    // Dibujar marcas en la cinta métrica
    for (int i = 0; i <= axisLength; i += 10) {
        float markStart[3] = {0}, markEnd[3] = {0};
        markStart[axis] = i;
        markEnd[axis] = i;
        markEnd[(axis + 1) % 3] = 0.5f;  // Marca perpendicular al eje
        
        drawLine(markStart[0], markStart[1], markStart[2],
                 markEnd[0], markEnd[1], markEnd[2]);
        
        // Dibujar números en las marcas principales
        if (i % 20 == 0) {
            char number[10];
            sprintf(number, "%d", i);
            float textPos[3] = {0};
            textPos[axis] = i;
            textPos[(axis + 1) % 3] = 1.0f;  // Posición del texto
            drawText3D(number, textPos[0], textPos[1], textPos[2]);
        }
    }
}

// Eliminar la variable de escala global, ya que ahora usaremos axisLength
// float scale = 1.5f;

// Definición de vértices del cubo (ahora unitario)
float ver[8][3] = 
{
    {0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f},
    {1.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f},
    {0.0f, 1.0f, 0.0f},
    {1.0f, 1.0f, 0.0f},
    {1.0f, 0.0f, 0.0f},
};


// Variables de rotación del cubo
double rotate_y = 0; 
double rotate_x = 0;

// Variables para el manejo del mouse
int prev_x, prev_y;
bool mouse_dragging = false;

// Estructura para campos de texto
struct InputField {
    float x, y, width, height;
    std::string label;
    std::string text;
    bool active;
    KDTree* kdTree;
}inputX, inputY, inputZ, button;


KDTree kdTree;

// Estructura para almacenar nodos del KD-Tree con información de partición
struct TreeNode {
    vector<float> point;
    TreeNode* left;
    TreeNode* right;
    int level;
    Space space;
};

// Vector para almacenar los nodos del árbol
vector<TreeNode*> treeNodes;

// Función para insertar un nuevo nodo en el árbol
void insertNode(const vector<float>& point) {
    if (treeNodes.empty()) {
        TreeNode* root = new TreeNode{point, nullptr, nullptr, 0, {0, axisLength, 0, axisLength, 0, axisLength}};
        treeNodes.push_back(root);
    } else {
        TreeNode* current = treeNodes[0];
        int depth = 0;
        while (true) {
            int axis = depth % 3;
            if (point[axis] < current->point[axis]) {
                if (current->left == nullptr) {
                    Space newSpace = current->space;
                    if (axis == 0) newSpace.maxX = current->point[axis];
                    else if (axis == 1) newSpace.maxY = current->point[axis];
                    else newSpace.maxZ = current->point[axis];
                    
                    current->left = new TreeNode{point, nullptr, nullptr, depth + 1, newSpace};
                    treeNodes.push_back(current->left);
                    break;
                }
                current = current->left;
            } else {
                if (current->right == nullptr) {
                    Space newSpace = current->space;
                    if (axis == 0) newSpace.minX = current->point[axis];
                    else if (axis == 1) newSpace.minY = current->point[axis];
                    else newSpace.minZ = current->point[axis];
                    
                    current->right = new TreeNode{point, nullptr, nullptr, depth + 1, newSpace};
                    treeNodes.push_back(current->right);
                    break;
                }
                current = current->right;
            }
            depth++;
        }
    }
}

// Función para dibujar un punto en el espacio 3D
void drawPoint(const vector<float>& point) {
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 1.0f);  // Color magenta para los puntos
    glVertex3f(point[0], point[1], point[2]);
    glEnd();
}

// Función para renderizar texto usando GLUT
void renderBitmapString(float x, float y, void *font, const char *string)
{
    glRasterPos2f(x, y);
    for (const char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Función para dibujar un plano de partición
void drawPartitionPlane(const TreeNode* node) {
    int axis = node->level % 3;
    glColor4f(axisColors[axis][0], axisColors[axis][1], axisColors[axis][2], 0.3f);  // Semi-transparente
    glBegin(GL_QUADS);
    if (axis == 0) {  // Plano YZ
        glVertex3f(node->point[0], node->space.minY, node->space.minZ);
        glVertex3f(node->point[0], node->space.maxY, node->space.minZ);
        glVertex3f(node->point[0], node->space.maxY, node->space.maxZ);
        glVertex3f(node->point[0], node->space.minY, node->space.maxZ);
    } else if (axis == 1) {  // Plano XZ
        glVertex3f(node->space.minX, node->point[1], node->space.minZ);
        glVertex3f(node->space.maxX, node->point[1], node->space.minZ);
        glVertex3f(node->space.maxX, node->point[1], node->space.maxZ);
        glVertex3f(node->space.minX, node->point[1], node->space.maxZ);
    } else {  // Plano XY
        glVertex3f(node->space.minX, node->space.minY, node->point[2]);
        glVertex3f(node->space.maxX, node->space.minY, node->point[2]);
        glVertex3f(node->space.maxX, node->space.maxY, node->point[2]);
        glVertex3f(node->space.minX, node->space.maxY, node->point[2]);
    }
    glEnd();
}

// Función para dibujar todos los planos de partición y puntos
void drawKDTreePartitions() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& node : treeNodes) {
        drawPartitionPlane(node);
        drawPoint(node->point);
    }
    glDisable(GL_BLEND);
}

// Función para dibujar un nodo del árbol
void drawNode(const TreeNode* node, float x, float y, float width) {
    if (node == nullptr) return;

    // Dibujar el rectángulo del nodo
    glColor3f(0.0f, 0.0f, 0.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(x - width/2, y + 0.05f);
    glVertex2f(x + width/2, y + 0.05f);
    glVertex2f(x + width/2, y - 0.05f);
    glVertex2f(x - width/2, y - 0.05f);
    glEnd();

    // Dibujar el texto del nodo
    stringstream ss;
    ss << fixed << setprecision(2) << "(" << node->point[0] << ", " << node->point[1] << ", " << node->point[2] << ")";
    string nodeText = ss.str();
    renderBitmapString(x - width/2 + 0.01f, y, GLUT_BITMAP_HELVETICA_10, nodeText.c_str());

    // Dibujar líneas a los hijos
    float childY = y - 0.15f;
    float separationFactor = 1.0f;  
    float childWidth = width * 0.8f * separationFactor;

    
    if (node->left) {
        glBegin(GL_LINES);
        glVertex2f(x, y - 0.05f);
        glVertex2f(x - childWidth, childY + 0.05f);
        glEnd();
        drawNode(node->left, x - childWidth, childY, childWidth);
    }

    if (node->right) {
        glBegin(GL_LINES);
        glVertex2f(x, y - 0.05f);
        glVertex2f(x + childWidth, childY + 0.05f);
        glEnd();
        drawNode(node->right, x + childWidth, childY, childWidth);
    }

}

// Función para dibujar el KD-Tree
void drawKDTree() {
    if (!treeNodes.empty()) {
        drawNode(treeNodes[0], 0.0f, 0.6f, 0.3f);
    }
}

// Vector para almacenar las coordenadas agregadas
std::vector<std::vector<float>> coordenadas;

void drawLineCube() {
    glPushMatrix();
    glScalef(axisLength, axisLength, axisLength);  // Escalar el cubo según la longitud de los ejes
    
    // Dibujar el cubo
    glColor3f(0.5f, 0.5f, 0.5f);  // Color gris para el cubo
    glBegin(GL_LINE_LOOP);
    glVertex3fv(ver[0]);
    glVertex3fv(ver[1]);
    glVertex3fv(ver[2]);
    glVertex3fv(ver[3]);
    glEnd();

    glBegin(GL_LINE_LOOP);
    glVertex3fv(ver[4]);
    glVertex3fv(ver[5]);
    glVertex3fv(ver[6]);
    glVertex3fv(ver[7]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(ver[0]);
    glVertex3fv(ver[4]);
    glVertex3fv(ver[1]);
    glVertex3fv(ver[5]);
    glVertex3fv(ver[2]);
    glVertex3fv(ver[6]);
    glVertex3fv(ver[3]);
    glVertex3fv(ver[7]);
    glEnd();

    glPopMatrix();

    // Dibujar los ejes
    drawAxis(0);  // Eje X
    drawAxis(1);  // Eje Y
    drawAxis(2);  // Eje Z
}

// Inicializar campos de entrada y botón
void initUI() {
    // Posicionar los campos en la parte superior de la ventana
    float startX = -0.9f; // Comienza un poco más a la izquierda
    float startY = 0.8f;  // Parte superior

    inputX.x = startX;
    inputX.y = startY;
    inputX.width = 0.15f; // Ancho ajustado
    inputX.height = 0.1f;
    inputX.label = "X:";
    inputX.text = "";
    inputX.active = false;

    inputY.x = startX + 0.2f; // Espacio para el campo X
    inputY.y = startY;
    inputY.width = 0.15f; // Ancho ajustado
    inputY.height = 0.1f;
    inputY.label = "Y:";
    inputY.text = "";
    inputY.active = false;

    inputZ.x = startX + 0.4f; // Espacio para el campo Y
    inputZ.y = startY;
    inputZ.width = 0.15f; // Ancho ajustado
    inputZ.height = 0.1f;
    inputZ.label = "Z:";
    inputZ.text = "";
    inputZ.active = false;

    button.x = startX + 0.6f; // Espacio para el campo Z
    button.y = startY; // Alineado en la misma línea
    button.width = 0.3f; // Ancho ajustado para el botón
    button.height = 0.1f; // Ajustado para que coincida con los campos
    button.label = "Agregar";
    button.text = "";
    button.active = false;
}

// Función para dibujar la interfaz de entrada en la mitad derecha
void drawUI()
{
    // Configurar proyección ortográfica para la interfaz
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Dibujar fondo de la interfaz (opcional, aquí blanco ya está)
    
    // Configurar color negro para texto y rectángulos
    glColor3f(0.0f, 0.0f, 0.0f);

    // Dibujar campos de entrada
    auto drawField = [](InputField &field) {
        // Dibujar el label
        renderBitmapString(field.x, field.y + 0.05f, GLUT_BITMAP_HELVETICA_18, field.label.c_str());

        // Dibujar el rectángulo del campo de texto
        glBegin(GL_LINE_LOOP);
        glVertex2f(field.x, field.y);
        glVertex2f(field.x + field.width, field.y);
        glVertex2f(field.x + field.width, field.y - field.height);
        glVertex2f(field.x, field.y - field.height);
        glEnd();

        // Dibujar el texto dentro del campo
        renderBitmapString(field.x + 0.02f, field.y - 0.08f, GLUT_BITMAP_HELVETICA_18, field.text.c_str());
    };

    drawField(inputX);
    drawField(inputY);
    drawField(inputZ);

    // Dibujar el botón
    // Dibujar rectángulo del botón
    glBegin(GL_LINE_LOOP);
    glVertex2f(button.x, button.y);
    glVertex2f(button.x + button.width, button.y);
    glVertex2f(button.x + button.width, button.y - button.height);
    glVertex2f(button.x, button.y - button.height);
    glEnd();

    // Dibujar el texto del botón centrado
    // Calcular posición del texto
    float textX = button.x + 0.02f;
    float textY = button.y - 0.1f;
    renderBitmapString(textX, textY, GLUT_BITMAP_HELVETICA_18, button.label.c_str());

    // Opcional: Mostrar las coordenadas agregadas
    float coordY = -0.5f;
    for(auto &coord : coordenadas){
        std::string coordStr = "(" + std::to_string(coord[0]) + ", " + 
                                    std::to_string(coord[1]) + ", " + 
                                    std::to_string(coord[2]) + ")";
        renderBitmapString(-0.9f, coordY, GLUT_BITMAP_HELVETICA_18, coordStr.c_str());
        coordY -= 0.2f;
    }
}

// Función para dibujar la línea divisoria
void drawDividerLine()
{
    glColor3f(0.0, 0.0, 0.0); // Color negro para la línea
    glBegin(GL_LINES);
    glVertex2f(0.0f, 1.0f);  // Parte superior
    glVertex2f(0.0f, -1.0f); // Parte inferior
    glEnd();
}

// Función de visualización
void display()
{
    // Limpiar la pantalla con fondo blanco
    glClearColor(1, 1, 1, 1); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Obtener el tamaño de la ventana
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // ----------- Mitad Izquierda: Cubo 3D -----------
    glViewport(0, 0, w / 2, h); // Mitad izquierda
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)(w / 2) / h, 0.1, axisLength * 3);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Ajustar la posición de la cámara para ver todo el cubo
    gluLookAt(axisLength * 1.5, axisLength * 1.5, axisLength * 1.5,
              axisLength / 2, axisLength / 2, axisLength / 2,
              0, 0, 1);


    // Rotar la escena según los valores actuales
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 1.0, 0.0);

    // Dibujar el cubo con ejes
    drawLineCube();
    drawKDTreePartitions();

    // ----------- Mitad Derecha: Interfaz de Entrada -----------
        // Dibujar el KD-Tree en la mitad derecha
    glViewport(w / 2, 0, w / 2, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawUI();
    drawKDTree();

    // ----------- Dibujar la Línea Divisoria -----------
    // Configurar la vista para toda la pantalla para dibujar la línea divisoria
    glViewport(0, 0, w, h); // Toda la ventana
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawDividerLine();

    // Intercambiar buffers
    glutSwapBuffers();
}

// par ala modificacion de las coordenadas
// Modificar la función para manejar la entrada de coordenadas
// Modificar la función para manejar la entrada de coordenadas
void handleCoordinateInput() {
    if (!inputX.text.empty() && !inputY.text.empty() && !inputZ.text.empty()) {
        try {
            float xCoord = stof(inputX.text);
            float yCoord = stof(inputY.text);
            float zCoord = stof(inputZ.text);
            
            // Verificar que las coordenadas estén dentro del rango del cubo
            if (xCoord < 0 || xCoord > axisLength || yCoord < 0 || yCoord > axisLength || zCoord < 0 || zCoord > axisLength) {
                cout << "Las coordenadas deben estar dentro del rango 0 a " << axisLength << endl;
                return;
            }
            
            vector<float> point = {xCoord, yCoord, zCoord};
            insertNode(point);
            cout << "Coordenadas agregadas: (" << xCoord << ", " << yCoord << ", " << zCoord << ")" << endl;

            // Limpiar los campos de entrada
            inputX.text.clear();
            inputY.text.clear();
            inputZ.text.clear();

            glutPostRedisplay();
        } catch (const invalid_argument&) {
            cout << "Entrada inválida. Por favor, ingrese números reales." << endl;
        }
    } else {
        cout << "Por favor, ingrese todas las coordenadas." << endl;
    }
}

// Función para ajustar la longitud de los ejes (puedes vincularla a una tecla o UI)
void adjustAxisLength(float delta) {
    axisLength += delta;
    if (axisLength < 10) axisLength = 10;
    if (axisLength > 500) axisLength = 500;
    glutPostRedisplay();
}

// Manejo del movimiento del mouse para rotar el cubo
void mouseMove(int x, int y)
{
    if (mouse_dragging)
    {
        rotate_y += (x - prev_x) * 0.5;  // Sensibilidad al movimiento en X
        rotate_x += (y - prev_y) * 0.5;  // Sensibilidad al movimiento en Y
        prev_x = x;
        prev_y = y;
        glutPostRedisplay();
    }
}

// Manejo de los botones del mouse para rotar o interactuar con la UI
void mouseButton(int buttonType, int state, int x, int y)
{
    if (buttonType == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        // Obtener el tamaño de la ventana
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);

        // Convertir las coordenadas del mouse a normalizadas (-1 a 1)
        float normX = (float)x / w * 2 - 1;
        float normY = -((float)y / h * 2 - 1);

        if(x < w / 2){
            // Click en la mitad izquierda: iniciar rotación
            mouse_dragging = true;
            prev_x = x;
            prev_y = y;
        }
        else{
            // Click en la mitad derecha: interactuar con la UI
            // Ajustar las coordenadas para la mitad derecha
            float uiX = (float)(x - w / 2) / (w / 2) * 2 - 1;
            float uiY = (float)(h - y) / h * 2 - 1;

            // Verificar si se hizo clic en algún campo de entrada
            auto checkActive = [&](InputField &field) -> bool {
                return (uiX >= field.x && uiX <= field.x + field.width) &&
                       (uiY <= field.y && uiY >= field.y - field.height);
            };

            // Desactivar todos los campos
            inputX.active = inputY.active = inputZ.active = false;

            if(checkActive(inputX)){
                inputX.active = true;
            }
            else if(checkActive(inputY)){
                inputY.active = true;
            }
            else if(checkActive(inputZ)){
                inputZ.active = true;
            }
            else{
                // Verificar si se hizo clic en el botón
                // Modificar la acción del botón
                if ((uiX >= button.x) && (uiX <= button.x + button.width) &&
                    (uiY <= button.y) && (uiY >= button.y - button.height)) {
                    if (inputX.text.empty() || inputY.text.empty() || inputZ.text.empty()) {
                        cout << "Por favor, ingrese todas las coordenadas." << endl;
                    } else {
                        try {
                            float xCoord = stof(inputX.text);
                            float yCoord = stof(inputY.text);
                            float zCoord = stof(inputZ.text);
                            vector<float> point = {xCoord, yCoord, zCoord};
                            insertNode(point);
                            kdTree.insert({static_cast<int>(xCoord), static_cast<int>(yCoord), static_cast<int>(zCoord)});
                            cout << "Coordenadas agregadas: (" << xCoord << ", " << yCoord << ", " << zCoord << ")" << endl;

                            // Limpiar los campos de entrada
                            inputX.text.clear();
                            inputY.text.clear();
                            inputZ.text.clear();
                        } catch (const invalid_argument&) {
                            cout << "Entrada inválida. Por favor, ingrese números reales." << endl;
                        }
                    }

                    glutPostRedisplay();
                    handleCoordinateInput();
                }
            }
        }
    }
    else if(buttonType == GLUT_LEFT_BUTTON && state == GLUT_UP){
        mouse_dragging = false;
    }
}

// Manejo de la entrada del teclado para los campos de texto
void keyboard(unsigned char key, int x, int y)
{
    // Determinar qué campo está activo
    InputField* activeField = nullptr;
    if(inputX.active) activeField = &inputX;
    else if(inputY.active) activeField = &inputY;
    else if(inputZ.active) activeField = &inputZ;

    if(activeField){
        if(key == 8 || key == 127){ // Backspace
            if(!activeField->text.empty()){
                activeField->text.pop_back();
            }
        }
        else if((key >= '0' && key <= '9') || key == '.' || key == '-' ){
            activeField->text += key;
        }
        glutPostRedisplay();
    }
    switch(key) {
        case '+':
            adjustAxisLength(10);
            break;
        case '-':
            adjustAxisLength(-10);
            break;
    }

    glutPostRedisplay();
}

// Inicializar campos de entrada y botón
void initFields(){
    initUI();
}

// Función principal
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(1000, 600); // Aumentar el ancho de la ventana
    glutCreateWindow("KD-Tree Visualization");
    glutDisplayFunc(display);
    glutMotionFunc(mouseMove);
    glutMouseFunc(mouseButton);
    glutKeyboardFunc(keyboard);
    glEnable(GL_DEPTH_TEST);
    initFields();
    glutMainLoop();
    return 0;
}