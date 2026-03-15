#include <GL/glut.h>
#include<bits/stdc++.h>
#include "QUAD.h"
using namespace std;

// Estructura of the balls
struct Ball {
    float x, y;        // Posición
    float vx, vy;      // Velocidad
    float radius;
    int id;
    
    Ball(float _x, float _y, float _vx, float _vy, float _r, int _id) 
        : x(_x), y(_y), vx(_vx), vy(_vy), radius(_r), id(_id) {}

    // Integración de movimiento
    void update(float deltaTime) {
        x += vx * deltaTime;
        y += vy * deltaTime;
    }

    // Colisión con paredes
    void handleBorderCollision(int minX, int minY, int maxX, int maxY) {
        if (x - radius < minX) {
            x = minX + radius;
            vx = -vx;
        }
        if (x + radius > maxX) {
            x = maxX - radius;
            vx = -vx;
        }
        if (y - radius < minY) {
            y = minY + radius;
            vy = -vy;
        }
        if (y + radius > maxY) {
            y = maxY - radius;
            vy = -vy;
        }
    }
};

int windowWidth = 800;
int windowHeight = 600;
string inputText = "";
bool inputActive = false;
vector<Ball> balls;
const float BALL_RADIUS = 2.0f;
const int BORDER_MARGIN = 50;
int nextBallId = 0;
const float INITIAL_SPEED = 100.0f;
float lastTime = 0.0f;

// QuadTree para detección de colisiones O(n log n)
QuadTree* quadTree = nullptr;

// Tiempo actual en segundos
float getCurrentTime() {
    return glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
}

// Ángulo aleatorio en radianes
float getRandomAngle() {
    static random_device rd;
    static mt19937 gen(rd());
    static uniform_real_distribution<> dis(0, 2 * M_PI);
    return dis(gen);
}

// Reconstruir QuadTree con posiciones actuales
void updateQuadTree() {
    delete quadTree;
    Point topLeft(BORDER_MARGIN, BORDER_MARGIN);
    Point botRight(windowWidth - BORDER_MARGIN, windowHeight - BORDER_MARGIN);
    quadTree = new QuadTree(topLeft, botRight);
    
    // Insertar todas las entidades
    for (const Ball& ball : balls) {
        Point pos(static_cast<int>(ball.x), static_cast<int>(ball.y));
        Node* node = new Node(pos, ball.id);
        quadTree->insert(node);
    }
}

// Generador de números aleatorios
random_device rd;
mt19937 gen(rd());

// Verificar colisión de nueva posición con entidades existentes
bool checkCollision(float x, float y, const vector<Ball>& balls) {
    if (quadTree == nullptr){
        return false;
    } 

    // Query espacial en QuadTree
    Point checkPoint(static_cast<int>(x), static_cast<int>(y));
    
    // Buscar en el QuadTree si hay alguna bola cercana
    Node* nearby = quadTree->search(checkPoint);
    
    if (nearby != nullptr) {
        // Verificar la distancia real con la bola encontrada
        for (const Ball& ball : balls) {
            if (ball.id == nearby->data) {
                float dx = x - ball.x;
                float dy = y - ball.y;
                float distanceSquared = dx * dx + dy * dy;
                float minDistance = (BALL_RADIUS * 2) * (BALL_RADIUS * 2);

                if (distanceSquared < minDistance) {
                    return true;  // Hay colisión
                }
            }
        }
    }
    return false;
}

// Generador de bolas aleatorias con verificación de colisiones
void generateBalls(int n) {
    balls.clear();
    nextBallId = 0;
    
    // Crear un nuevo QuadTree
    Point topLeft(BORDER_MARGIN, BORDER_MARGIN);
    Point botRight(windowWidth - BORDER_MARGIN, windowHeight - BORDER_MARGIN);
    delete quadTree;
    quadTree = new QuadTree(topLeft, botRight);
    
    // Generador de números aleatorios
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> disX(BORDER_MARGIN + BALL_RADIUS, 
                                        windowWidth - BORDER_MARGIN - BALL_RADIUS);
    uniform_real_distribution<> disY(BORDER_MARGIN + BALL_RADIUS, 
                                        windowHeight - BORDER_MARGIN - BALL_RADIUS);
    
    int attempts = 0;
    const int MAX_ATTEMPTS = 1000;
    
    while (balls.size() < n && attempts < MAX_ATTEMPTS * n) {
        float x = disX(gen);
        float y = disY(gen);
        
        if (!checkCollision(x, y, balls)) {
            // Velocidad inicial aleatoria
            float angle = getRandomAngle();
            float vx = INITIAL_SPEED * cos(angle);
            float vy = INITIAL_SPEED * sin(angle);

            
            // Nueva entidad
            Ball newBall(x, y, vx, vy, BALL_RADIUS, nextBallId++);
            balls.push_back(newBall);
            
            // Insertar en estructura espacial
            Point pos(static_cast<int>(x), static_cast<int>(y));
            Node* node = new Node(pos, newBall.id);
            quadTree->insert(node);
        }
        attempts++;
    }
    
    lastTime = getCurrentTime();
}

// Detección y resolución de colisiones bola-bola via QuadTree
void handleBallCollisions() {
    // Actualizar QuadTree con posiciones actuales
    updateQuadTree();
    
    // Para cada bola, buscar colisiones en su vecindad
    for (size_t i = 0; i < balls.size(); i++) {
        Ball& ball1 = balls[i];
        
        // Punto de query
        Point checkPoint(static_cast<int>(ball1.x), static_cast<int>(ball1.y));
        
        // Query espacial
        Node* nearby = quadTree->search(checkPoint);
        
        if (nearby != nullptr) {
            // Verificar colisiones con entidades en rango
            for (size_t j = i + 1; j < balls.size(); j++) {
                Ball& ball2 = balls[j];
                
                // Distancia entre centros
                float dx = ball2.x - ball1.x;
                float dy = ball2.y - ball1.y;
                float distanceSquared = dx * dx + dy * dy;
                float minDistance = (ball1.radius + ball2.radius);
                
                // Colisión detectada
                if (distanceSquared <= minDistance * minDistance) {
                    // Normal de colisión
                    float distance = sqrt(distanceSquared);
                    float nx = dx / distance;
                    float ny = dy / distance;
                    
                    // Velocidad relativa
                    float relativeVx = ball2.vx - ball1.vx;
                    float relativeVy = ball2.vy - ball1.vy;
                    
                    // Velocidad relativa en dirección normal
                    float velocityAlongNormal = relativeVx * nx + relativeVy * ny;
                    
                    
                    // Separación: skip si se alejan
                    if (velocityAlongNormal > 0){
                        continue;
                    }
                    
                    // Coeficiente de restitución
                    const float restitution = 0.8f;
                    
                    // Impulso escalar
                    float j = -(1.0f + restitution) * velocityAlongNormal;
                    j /= 2.0f; // Masa unitaria
                    
                    // Aplicar impulso
                    ball1.vx -= j * nx;
                    ball1.vy -= j * ny;
                    ball2.vx += j * nx;
                    ball2.vy += j * ny;
                    
                    // Corrección de posición para evitar overlap
                    float overlap = minDistance - distance;
                    float moveX = (overlap * nx) / 2.0f;
                    float moveY = (overlap * ny) / 2.0f;
                    
                    ball1.x -= moveX;
                    ball1.y -= moveY;
                    ball2.x += moveX;
                    ball2.y += moveY;
                }
            }
        }
    }
}

// Actualizar física de todas las entidades
void updateBalls() {
    // Delta time
    float currentTime = getCurrentTime();
    float deltaTime = currentTime - lastTime;
    lastTime = currentTime;

    // Integración de movimiento
    for (Ball& ball : balls) {
        ball.update(deltaTime);
        ball.handleBorderCollision(BORDER_MARGIN, BORDER_MARGIN, 
                                 windowWidth - BORDER_MARGIN, 
                                 windowHeight - BORDER_MARGIN);
    }

    // Resolver colisiones
    handleBallCollisions();

    // Reconstruir estructura espacial
    updateQuadTree();
    
    glutPostRedisplay();
}

// Timer: ~60 FPS
void timer(int value) {
    updateBalls();
    glutTimerFunc(16, timer, 0);
}

// Visualización de la estructura QuadTree (debug)
void drawQuadTreeGrid(QuadTree* tree, Point topLeft, Point botRight) {
    if (tree == nullptr) return;
    
    // Rectángulo del quad
    glColor3f(0.8f, 0.8f, 0.8f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(topLeft.x, topLeft.y);
    glVertex2i(botRight.x, topLeft.y);
    glVertex2i(botRight.x, botRight.y);
    glVertex2i(topLeft.x, botRight.y);
    glEnd();
    
    // Calcular punto medio
    int midX = (topLeft.x + botRight.x) / 2;
    int midY = (topLeft.y + botRight.y) / 2;
    
    // Recursivamente dibujar los subárboles
    if (abs(topLeft.x - botRight.x) > 1 && abs(topLeft.y - botRight.y) > 1) {
        Point midPoint(midX, midY);
        drawQuadTreeGrid(tree->topLeftTree, topLeft, midPoint);
        drawQuadTreeGrid(tree->topRightTree, Point(midX, topLeft.y), Point(botRight.x, midY));
        drawQuadTreeGrid(tree->botLeftTree, Point(topLeft.x, midY), Point(midX, botRight.y));
        drawQuadTreeGrid(tree->botRightTree, midPoint, botRight);
    }
}


// Renderizar bola con vector de velocidad
void drawBall(const Ball& ball) {
    // Cuerpo de la bola
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(ball.x, ball.y);
    for (int i = 0; i <= 360; i += 10) {
        float angle = i * M_PI / 180.0f;
        float dx = ball.radius * cos(angle);
        float dy = ball.radius * sin(angle);
        glVertex2f(ball.x + dx, ball.y + dy);
    }
    glEnd();

    // Vector de velocidad
    glBegin(GL_LINES);
    glVertex2f(ball.x, ball.y);
    float speedScale = 0.5f;
    glVertex2f(ball.x + ball.vx * speedScale, ball.y + ball.vy * speedScale);
    glEnd();
}

// Renderizar texto
void drawText(const char* text, int x, int y) {
    glRasterPos2i(x, y);
    for (const char* c = text; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_9_BY_15, *c);
    }
}

// Renderizar rectángulo
void drawRectangle(int x, int y, int width, int height, bool filled) {
    if (filled) {
        glBegin(GL_QUADS);
    } else {
        glBegin(GL_LINE_LOOP);
    }
    glVertex2i(x, y);
    glVertex2i(x + width, y);
    glVertex2i(x + width, y + height);
    glVertex2i(x, y + height);
    glEnd();
}

void display() {
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Visualizar estructura QuadTree (debug)
    if (quadTree != nullptr) {
        Point topLeft(BORDER_MARGIN, BORDER_MARGIN);
        Point botRight(windowWidth - BORDER_MARGIN, windowHeight - BORDER_MARGIN);
        drawQuadTreeGrid(quadTree, topLeft, botRight);
    }
    
    // Marco perimetral
    glColor3f(0.0f, 0.0f, 0.0f);
    drawRectangle(BORDER_MARGIN, BORDER_MARGIN, 
                 windowWidth - 2*BORDER_MARGIN, 
                 windowHeight - 2*BORDER_MARGIN, false);
    
    // Entidades
    glColor3f(0.0f, 0.0f, 1.0f);
    for (const Ball& ball : balls) {
        drawBall(ball);
    }
    
    // UI de entrada
    glColor3f(0.0f, 0.0f, 0.0f);
    if (inputActive) {
        glColor3f(0.9f, 0.9f, 0.9f);
        drawRectangle(200, windowHeight - 80, 100, 30, true);
    }
    glColor3f(0.0f, 0.0f, 0.0f);
    drawRectangle(200, windowHeight - 80, 100, 30, false);
    drawText("Ingrese la cant. de balls:", 20, windowHeight - 65);
    drawText(inputText.c_str(), 210, windowHeight - 65);
    
    glutSwapBuffers();
}

// Callback: redimensionamiento de ventana
void reshape(int w, int h) {
    windowWidth = w;
    windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
}

// Callback: keyboard input
void keyboard(unsigned char key, int x, int y) {
    if (key == 13) { // Enter
        inputActive = false;
        if (!inputText.empty()) {
            int n = stoi(inputText);
            generateBalls(n);
            inputText = "";
        }
    } else if (key == 8 && !inputText.empty()) { // Backspace
        inputText.pop_back();
    } else if (key >= '0' && key <= '9' && inputText.length() < 5) {
        inputText += key;
    }
    glutPostRedisplay();
}

// Callback: mouse click
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Transformar coordenadas (GLUT usa origen abajo-izquierda)
        y = windowHeight - y;
        
        // Verificar clic en área de input
        if (x >= 200 && x <= 300 && y >= windowHeight - 80 && y <= windowHeight - 50) {
            inputActive = true;
        } else {
            inputActive = false;
        }
        glutPostRedisplay();
    }
}

void cleanup() {
    delete quadTree;
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Balls Simulation with QuadTree");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutTimerFunc(0, timer, 0);
    
    atexit(cleanup);
    
    glutMainLoop();
    return 0;
}