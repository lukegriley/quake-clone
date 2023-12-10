#pragma once

// Defined before including GLEW to suppress deprecation messages on macOS
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <unordered_map>
#include <QElapsedTimer>
#include <QOpenGLWidget>
#include <QTime>
#include <QTimer>
#include "scene/scenedata.h"
#include "camera.h"
#include "objects/player.h"
#include "scene/level.h"
#include "scene/sceneparser.h"
#include "scene/settings.h"
#include "game_types.h"

class Renderer //: public QOpenGLWidget

{
public:
    Renderer(/*QWidget *parent = nullptr*/);
    void finish();                                      // Called on program exit
    void sceneChanged();
    void settingsChanged();
    void saveViewportImage(std::string filePath);
    void initializeGL();                       // Called once at the start of the program

    void drawStaticObs();
    static void drawDynamicOb(struct ECS*, entity_t entity_id, float delta_seconds);
    static std::map<u_int8_t,Model> generateModelsMap();

    void drawScreen();

    void resizeGL(int width, int height);      // Called when window size changes

    void startDraw();
    std::map<u_int8_t,Model> m_models;

    inline void setRatio(float x, float y) {
//        m_devicePixelRatio = ratio;
        ratio_x = x;
        ratio_y = y;
        resizeGL(DSCREEN_WIDTH * ratio_x, DSCREEN_HEIGHT * ratio_y);
    }

    static inline Renderer* default_render;
public slots:
    void tick(QTimerEvent* event);                      // Called once per tick of m_timer

protected:



private:

    void drawRenderOb(RenderObject& to_draw);
    void drawDynamicModel(struct ECS* e, entity_t ent, float delta_seconds);

//    void keyPressEvent(QKeyEvent *event) override;
//    void keyReleaseEvent(QKeyEvent *event) override;
//    void mousePressEvent(QMouseEvent *event) override;
//    void mouseReleaseEvent(QMouseEvent *event) override;
//    void mouseMoveEvent(QMouseEvent *event) override;
//    void timerEvent(QTimerEvent *event) override;

    // Tick Related Variables
    int m_timer;                                        // Stores timer which attempts to run ~60 times per second
    QElapsedTimer m_elapsedTimer;                       // Stores timer which keeps track of actual time between frames

    SceneData data = SceneData();

    void bindBuff(std::vector<float>&& dat, int ind);

    void generateShape();


    Settings old_settings = {};

    int m_screen_width = 0;
    int m_screen_height = 0;

    int m_o_scrn_width = 0;
    int m_o_scrn_height = 0;

    void paintTexture(GLuint texture, bool post_process);


    // Input Related Variables
    bool m_mouseDown = false;                           // Stores state of left mouse button
    glm::vec2 m_prev_mouse_pos;                         // Stores mouse position
    std::unordered_map<Qt::Key, bool> m_keyMap;         // Stores whether keys are pressed or not

    // Device Correction Variables
//    int m_devicePixelRatio;
    float ratio_x;
    float ratio_y;


    GLuint m_shader;

    Camera camera = Camera();

    GLuint m_vbos[5] = {};
    GLuint m_vaos[5] = {};
    std::vector<float> m_data[5] = {};

    SceneMaterial m_level_mat;



    int sphere_in = static_cast<int>(PrimitiveType::PRIMITIVE_SPHERE);
    int cube_in = static_cast<int>(PrimitiveType::PRIMITIVE_CUBE);
    int cone_in = static_cast<int>(PrimitiveType::PRIMITIVE_CONE);
    int cylinder_in = static_cast<int>(PrimitiveType::PRIMITIVE_CYLINDER);

    bool init_gen = false;

    Level m_level;


    void makeFBO();
    GLuint m_fbo_renderbuffer;
    GLuint m_fbo;
    GLuint m_defaultFBO;
    GLuint m_fbo_texture;
    GLuint m_texture_shader;

    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    void setUniforms(RenderObject& sp);





//    GLuint m_shader;     // Stores id of shader program
//    GLuint m_sphere_vbo; // Stores id of vbo
//    GLuint m_sphere_vao; // Stores id of vao
//    std::vector<float> m_sphereData;
};
