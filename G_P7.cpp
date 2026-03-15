#include <GL/glut.h>
#include <cstring>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "KD.h"

using namespace std;
//float scale = 1.5f;

// Definir colores para los ejes (X=Azul, Y=Rojo, Z=Verde)
float axisColors[3][3] = {
    {0.0f, 0.0f, 1.0f},  // Azul para X
    {1.0f, 0.0f, 0.0f},  // Rojo para Y
    {0.0f, 1.0f, 0.0f}   // Verde para Z
};

// longitudes de los ejes
float axisLength = 100.0f;

struct Space {
    float minX, maxX, minY, maxY, minZ, maxZ;
};

// Dibujar línea
void drawLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    glBegin(GL_LINES);
    glVertex3f(x1, y1, z1);
    glEnd();
    glVertex3f(x2, y2, z2);
}

// Renderizar texto en viewport 3D
void drawText3D(const char* text, float x, float y, float z) {
    glRasterPos3f(x, y, z);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
    }
}

// Eje con marcas de escala
void drawAxis(int axis) {
    glColor3fv(axisColors[axis]);
    
    // Eje principal
    drawLine(0, 0, 0, 
             axis == 0 ? axisLength : 0,
             axis == 1 ? axisLength : 0,
             axis == 2 ? axisLength : 0);
    
    // Marcas de escala
    for (int i = 0; i <= axisLength; i += 10) {
        float markStart[3] = {0}, markEnd[3] = {0};
        markStart[axis] = i;
        markEnd[axis] = i;
        markEnd[(axis + 1) % 3] = 0.5f;  // Marca perpendicular al eje
        
        drawLine(markStart[0], markStart[1], markStart[2],
                 markEnd[0], markEnd[1], markEnd[2]);
        
        // Números en marcas principales
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

// float scale = 1.5f;

// Vértices del cubo unitario
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


// Rotación del cubo
double rotate_y = 0; 
double rotate_x = 0;

// Estado del mouse
int prev_x, prev_y;
bool mouse_dragging = false;

// Campos de texto para UI
struct InputField {
    float x, y, width, height;
    std::string label;
    std::string text;
    bool active;
    KDTree* kdTree;
}inputX, inputY, inputZ, button;


KDTree kdTree;

// Nodo del KD-Tree con metadatos de partición espacial
struct TreeNode {
    vector<float> point;
    TreeNode* left;
    TreeNode* right;
    int level;
    Space space;
};

// Vector para almacenar los nodos del árbol
vector<TreeNode*> treeNodes;

// Inserción de punto en el KD-Tree con actualización del bounding box
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

// Renderizar punto en el espacio 3D
void drawPoint(const vector<float>& point) {
    glPointSize(5.0f);
    glBegin(GL_POINTS);
    glColor3f(1.0f, 0.0f, 1.0f);  // Color magenta para los puntos
    glVertex3f(point[0], point[1], point[2]);
    glEnd();
}

// Renderizar texto usando GLUT
void renderBitmapString(float x, float y, void *font, const char *string)
{
    glRasterPos2f(x, y);
    for (const char* c = string; *c != '\0'; c++) {
        glutBitmapCharacter(font, *c);
    }
}

// Plano de partición del KD-Tree (plano YZ, XZ o XY según el nivel)
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

// Visualización de planos de partición y puntos del KD-Tree
void drawKDTreePartitions() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    for (const auto& node : treeNodes) {
        drawPartitionPlane(node);
        drawPoint(node->point);
    }
    glDisable(GL_BLEND);
}

// Renderizar nodo del árbol con conexiones a hijos
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

// Visualización jerárquica del KD-Tree
void drawKDTree() {
    if (!treeNodes.empty()) {
        drawNode(treeNodes[0], 0.0f, 0.6f, 0.3f);
    }
}

// Almacenamiento de coordenadas
std::vector<std::vector<float>> coordenadas;

void drawLineCube() {
    glPushMatrix();
    glScalef(axisLength, axisLength, axisLength);
    
    // Wireframe del cubo unitario
    glColor3f(0.5f, 0.5f, 0.5f);
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

    // Ejes coordenados
    drawAxis(0);  // Eje X
    drawAxis(1);  // Eje Y
    drawAxis(2);  // Eje Z
}

// Configurar UI
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

// UI: panel de entrada en viewport derecho
void drawUI()
{
    // Configurar proyección ortográfica para UI
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Color para texto y rectángulos
    glColor3f(0.0f, 0.0f, 0.0f);

    // Campos de entrada
    auto drawField = [](InputField &field) {
        // Label
        renderBitmapString(field.x, field.y + 0.05f, GLUT_BITMAP_HELVETICA_18, field.label.c_str());

        // Rectángulo del campo
        glBegin(GL_LINE_LOOP);
        glVertex2f(field.x, field.y);
        glVertex2f(field.x + field.width, field.y);
        glVertex2f(field.x + field.width, field.y - field.height);
        glVertex2f(field.x, field.y - field.height);
        glEnd();

        // Texto del campo
        renderBitmapString(field.x + 0.02f, field.y - 0.08f, GLUT_BITMAP_HELVETICA_18, field.text.c_str());
    };

    drawField(inputX);
    drawField(inputY);
    drawField(inputZ);

    // Botón
    glBegin(GL_LINE_LOOP);
    glVertex2f(button.x, button.y);
    glVertex2f(button.x + button.width, button.y);
    glVertex2f(button.x + button.width, button.y - button.height);
    glVertex2f(button.x, button.y - button.height);
    glEnd();

    // Texto del botón
    float textX = button.x + 0.02f;
    float textY = button.y - 0.1f;
    renderBitmapString(textX, textY, GLUT_BITMAP_HELVETICA_18, button.label.c_str());

    // Coordenadas ingresadas
    float coordY = -0.5f;
    for(auto &coord : coordenadas){
        std::string coordStr = "(" + std::to_string(coord[0]) + ", " + 
                                    std::to_string(coord[1]) + ", " + 
                                    std::to_string(coord[2]) + ")";
        renderBitmapString(-0.9f, coordY, GLUT_BITMAP_HELVETICA_18, coordStr.c_str());
        coordY -= 0.2f;
    }
}

// Línea divisoria entre viewports
void drawDividerLine()
{
    glColor3f(0.0, 0.0, 0.0); // Color negro para la línea
    glBegin(GL_LINES);
    glVertex2f(0.0f, 1.0f);  // Parte superior
    glVertex2f(0.0f, -1.0f); // Parte inferior
    glEnd();
}

// Render loop principal
void display()
{
    // Fondo blanco
    glClearColor(1, 1, 1, 1); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Obtener el tamaño de la ventana
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    // ============ Viewport izquierdo: Cubo 3D ============
    glViewport(0, 0, w / 2, h); // Mitad izquierda
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (float)(w / 2) / h, 0.1, axisLength * 3);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Cámara en posición isométrica
    gluLookAt(axisLength * 1.5, axisLength * 1.5, axisLength * 1.5,
              axisLength / 2, axisLength / 2, axisLength / 2,
              0, 0, 1);


    // Rotación mediante mouse
    glRotatef(rotate_x, 1.0, 0.0, 0.0);
    glRotatef(rotate_y, 0.0, 1.0, 0.0);

    // Escena 3D
    drawLineCube();
    drawKDTreePartitions();

    // ============ Viewport derecho: UI ============
    // KD-Tree en vista 2D
    glViewport(w / 2, 0, w / 2, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawUI();
    drawKDTree();

    // ============ Línea divisoria ============
    glViewport(0, 0, w, h); // Toda la ventana
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawDividerLine();

    glutSwapBuffers();
}

// Procesar coordenadas ingresadas por el usuario
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

            // Limpiar campos
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

// Ajustar escala de los ejes
void adjustAxisLength(float delta) {
    axisLength += delta;
    if (axisLength < 10) axisLength = 10;
    if (axisLength > 500) axisLength = 500;
    glutPostRedisplay();
}

// Rotación del modelo mediante drag
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

// Evento de botones del mouse (rotación + interacción UI)
void mouseButton(int buttonType, int state, int x, int y)
{
    if (buttonType == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        // Dimensiones del viewport
        int w = glutGet(GLUT_WINDOW_WIDTH);
        int h = glutGet(GLUT_WINDOW_HEIGHT);

        // Coordenadas normalizadas del mouse (-1 a 1)
        float normX = (float)x / w * 2 - 1;
        float normY = -((float)y / h * 2 - 1);

        if(x < w / 2){
            // Viewport izquierdo: iniciar rotación
            mouse_dragging = true;
            prev_x = x;
            prev_y = y;
        }
        else{
            // Viewport derecho: interactuar con UI
            // Ajustar coordenadas para viewport derecho
            float uiX = (float)(x - w / 2) / (w / 2) * 2 - 1;
            float uiY = (float)(h - y) / h * 2 - 1;

            // Verificar clic en campos de entrada
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
                // Clic en botón: procesar entrada
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

                            // Limpiar campos
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

// Input handler para campos de texto
void keyboard(unsigned char key, int x, int y)
{
    // Determinar campo activo
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

// Entry point
int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(1000, 600);
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