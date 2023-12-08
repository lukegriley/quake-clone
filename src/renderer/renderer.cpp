#include "Renderer.h"

#include <QCoreApplication>
#include <QMouseEvent>
#include <QKeyEvent>
#include <iostream>
#include "shaderloader.h"
#include "objects/Sphere.h"
#include "objects/Cube.h"
#include "objects/Cylinder.h"
#include "objects/Cone.h"
#include "scene/sceneparser.h"
#include "game_types.h"



// ================== Project 5: Lights, Camera

Renderer::Renderer(/*QWidget *parent*/)
    //: QOpenGLWidget(parent)
{

    default_render = this;
    m_prev_mouse_pos = glm::vec2(DSCREEN_WIDTH/2, DSCREEN_HEIGHT/2);
    //setMouseTracking(true);
    //setFocusPolicy(Qt::StrongFocus);

    m_keyMap[Qt::Key_W]       = false;
    m_keyMap[Qt::Key_A]       = false;
    m_keyMap[Qt::Key_S]       = false;
    m_keyMap[Qt::Key_D]       = false;
    m_keyMap[Qt::Key_Control] = false;
    m_keyMap[Qt::Key_Space]   = false;
    initializeGL();

    // If you must use this function, do not edit anything above this


}






void Renderer::finish() {


//    killTimer(m_timer);
//    this->makeCurrent();

    // Students: anything requiring OpenGL calls when the program exits should be done here

    // Delete VAO and VBOs, delete the program
//    glDeleteProgram()
    glDeleteProgram(m_shader);
    glDeleteProgram(m_texture_shader);

    glDeleteBuffers(1, &m_fbo);
    glDeleteBuffers(1, &m_defaultFBO);
    glDeleteRenderbuffers(1, &m_fbo_renderbuffer);

    glDeleteBuffers(5, &(m_vbos[0]));
    glDeleteVertexArrays(5, &(m_vaos[0]));

    glDeleteTextures(1, &m_fbo_texture);

//    this->doneCurrent();
}

void Renderer::bindBuff(std::vector<float>&& dat, int ind) {

    // Generate and bind VBO
    glGenBuffers(1, &(m_vbos[ind]));
    glBindBuffer(GL_ARRAY_BUFFER, m_vbos[ind]);
     //generateSphereData(10,20);

    m_data[ind] = dat;

    // Send data to VBO
    glBufferData(GL_ARRAY_BUFFER, m_data[ind].size() * sizeof(GLfloat),
                 m_data[ind].data(), GL_STATIC_DRAW);

    // Generate, and bind vao
    glGenVertexArrays(1, &(m_vaos[ind]));
    glBindVertexArray(m_vaos[ind]);


    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Clean-up bindings
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

}


void Renderer::makeFBO() {
    // Task 19: Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &m_fbo_texture);
    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screen_width, m_screen_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    // Task 20: Generate and bind a renderbuffer of the right size, set its format, then unbind
    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_screen_width, m_screen_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Task 18: Generate and bind an FBO
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    // Task 21: Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_fbo_texture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_fbo_renderbuffer);

    // Task 22: Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);

}

void Renderer::initializeGL() {

    old_settings = settings;

    std::cout <<" HII" << std::endl;

//    m_timer = startTimer(1000/60);
    m_elapsedTimer.start();



    // Initializing GL.
    // GLEW (GL Extension Wrangler) provides access to OpenGL functions.
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "Error while initializing GL: " << glewGetErrorString(err) << std::endl;
    }
    std::cout << "Initialized GL: Version " << glewGetString(GLEW_VERSION) << std::endl;

    // Allows OpenGL to draw objects appropriately on top of one another
    glEnable(GL_DEPTH_TEST);
    // Tells OpenGL to only draw the front face
    glEnable(GL_CULL_FACE);



    // Students: anything requiring OpenGL calls when the program starts should be done here
    m_defaultFBO = 0;

    m_screen_width = DSCREEN_WIDTH;
    m_screen_height =  DSCREEN_HEIGHT;

    m_o_scrn_width = m_screen_width;
    m_o_scrn_height = m_screen_height;

    // Tells OpenGL how big the screen is
    glViewport(0, 0, m_screen_width, m_screen_height);


    m_shader = ShaderLoader::createShaderProgram("../../resources/shaders/default.vert", "../../resources/shaders/default.frag");
    m_texture_shader = ShaderLoader::createShaderProgram("../../resources/shaders/texture.vert", "../../resources/shaders/texture.frag");

    glUseProgram(m_texture_shader);
    GLint texLoc = glGetUniformLocation(m_texture_shader, "tex");
    if (texLoc == -1) {
        std::cout << "err " << m_texture_shader << std::endl;
    }
    glUniform1i(texLoc, 0);

    // FULLSCREEN QUAD CODE
    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS, THEN UV's    //
            -1.f,  1.f, 0.0f,
            0.f,  1.f, 0.0f,

            -1.f, -1.f, 0.0f,
            0.0f, 0.0f, 0.0f,

            1.f, -1.f, 0.0f,
            1.f, 0.0f, 0.0f,


            1.f, -1.f, 0.0f,
            1.f,  0.f, 0.0f,


            1.f,  1.f, 0.0f,
            1.f,  1.f, 0.0f,


            -1.f,  1.f, 0.0f,
            0.f, 1.f, 0.0f
        };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    // Task 14: modify the code below to add a second attribute to the vertex attribute array

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void*>(0 * sizeof(GLfloat)));
    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);




    std::cout << "shader: " << std::to_string(m_shader) << std::endl;

    generateShape();

    init_gen = true;

    makeFBO();

    sceneChanged();
}


// Task 31: Update the paintTexture function signature
void Renderer::paintTexture(GLuint texture, bool post_process){
    glUseProgram(m_texture_shader);
    // Task 32: Set your bool uniform on whether or not to filter the texture drawn
    glUniform1i(glGetUniformLocation(m_texture_shader, "filt"), settings.perPixelFilter);
    glUniform1i(glGetUniformLocation(m_texture_shader, "box_blur"), settings.kernelBasedFilter);

//    glUniform1i(glGetUniformLocation(m_texture_shader, "gray_scale"), settings.kernelBasedFilter);


    glm::vec2 uv_traverse = glm::vec2(1.f/m_o_scrn_width, 1.f/m_o_scrn_height);
    glUniform2fv(glGetUniformLocation(m_texture_shader, "uv_traverse"), 1, &uv_traverse[0]);


    glBindVertexArray(m_fullscreen_vao);
    // Task 10: Bind "texture" to slot 0
    // FIRST PART I"M UNSURE ABOUT
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}




void Renderer::generateShape() {

    int p_1 = 10;
    int p_2 = 10;

    Sphere sphere = Sphere();
    sphere.updateParams(p_1, p_2);
    bindBuff(sphere.generateShape(), sphere_in);

    Cube cube = Cube();
    cube.updateParams(p_1);
    bindBuff(cube.generateShape(), cube_in);

    Cone cone = Cone();
    cone.updateParams(p_1, p_2);
    bindBuff(cone.generateShape(), cone_in);

    Cylinder cyl = Cylinder();
    cyl.updateParams(p_1, p_2);
    bindBuff(cyl.generateShape(), cylinder_in);

}


void Renderer::setUniforms(RenderObject& sp) {

    glm::vec4 cam_pos = glm::inverse(camera.getViewMatrix()) * glm::vec4(0, 0, 0, 1);//inverse(m_view) * glm::vec4(0, 0, 0, 1);

    GLint matrixLoc = glGetUniformLocation(m_shader, "model_matrix");
    GLint viewLoc = glGetUniformLocation(m_shader, "view_matrix");
    GLint projLoc = glGetUniformLocation(m_shader, "proj_matrix");
    if (viewLoc == -1 || projLoc == -1 || matrixLoc == -1) {

        std::cout << "matrix name wrong" << std::endl;
        exit(1);
    }


    glUniform4f(glGetUniformLocation(m_shader, "ambient"),
                sp.primitive.material.cAmbient.x,
                sp.primitive.material.cAmbient.y,
                sp.primitive.material.cAmbient.z,
                sp.primitive.material.cAmbient[3]);

    glUniform4f(glGetUniformLocation(m_shader, "diffuse"),
                sp.primitive.material.cDiffuse.x,
                sp.primitive.material.cDiffuse.y,
                sp.primitive.material.cDiffuse.z,
                sp.primitive.material.cDiffuse[3]);

    glUniform4f(glGetUniformLocation(m_shader, "specular"),
                sp.primitive.material.cSpecular.x,
                sp.primitive.material.cSpecular.y,
                sp.primitive.material.cSpecular.z,
                sp.primitive.material.cSpecular[3]);


    // OBJECT SPACE NORMAL SOMEWHERE
    auto view_mat = camera.getViewMatrix();
    auto proj_mat = camera.getProjectionMatrix();

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &(view_mat[0][0]));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &(proj_mat[0][0]));
    glUniformMatrix4fv(matrixLoc, 1, GL_FALSE, &(sp.ctm[0][0]));

    // Phong lighting constants
    GLuint ka_in = glGetUniformLocation(m_shader, "kd");
    if (ka_in == -1) {
        std::cout << "ka err" << std::endl;
    }

    glUniform1f(glGetUniformLocation(m_shader, "ka"), data.globalData.ka);
    glUniform1f(ka_in, data.globalData.kd);
    glUniform1f(glGetUniformLocation(m_shader, "ks"), data.globalData.ks);


    // PASS IN MATERIAL PARAMETERS, such as ambient, diffuse, specular, shininess from material

    glUniform1f(glGetUniformLocation(m_shader, "shininess"), sp.primitive.material.shininess);

    glUniform4f(glGetUniformLocation(m_shader, "cam_pos"),
                cam_pos.x, cam_pos.y, cam_pos.z, cam_pos[3]);


}



void Renderer::paintGL()
{

    if (m_mouseDown) {
        // rotate cam based on delta x
    }


    // Clear screen color and depth before painting
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // TODO
    // Paramter support (just regenerate meshes on setting changes)
    // Multiple objects (generate VBO/VOA for each object, just a lot of code)
    // Multiple lights (may need to also pass in light color data, and type)

    // Draw to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
//    std::cout << m_fbo << std::endl;

    // Task 28: Call glViewport
    glViewport(0, 0, m_screen_width, m_screen_height);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Task 2: activate the shader program by calling glUseProgram with `m_shader`
    glUseProgram(m_shader);


    for (RenderObject sp : data.shapes) {

        int in = 0;

        switch (sp.primitive.type) {
            case PrimitiveType::PRIMITIVE_CUBE:
                in = cube_in;
                break;
            case PrimitiveType::PRIMITIVE_SPHERE:
                in = sphere_in;
                break;
            case PrimitiveType::PRIMITIVE_CYLINDER:
                in = cylinder_in;
                break;
            case PrimitiveType::PRIMITIVE_CONE:
                in = cone_in;
                break;
            default:
                in = 0;
            break;
        }

        // Bind Sphere Vertex Data
        glBindVertexArray(m_vaos[in]);

        setUniforms(sp);


        // NOT PROPERLY INTEGRATED INTO LIGHTING CODE YET

        int used = 0;
        for (int i = 0; i < data.lights.size(); i++) {

            SceneLightData lt = data.lights[i];

//            if (lt.type != LightType::LIGHT_DIRECTIONAL)
//                continue;

//            float fa = std::min(1.f, 1.f/(lt->function.x + (dist * lt->function.y) + (sqr_dist * light->function.z)));
            // PASS IN all light data
            // direction, color, etc.
            GLint loc = glGetUniformLocation(m_shader, ("lights[" + std::to_string(used) + "]").c_str());
//            std::cout << "pos " << lt.pos[0] << " " << lt.pos[1] << " " << lt.pos[2] << std::endl;
            glUniform4fv(loc, 1, &lt.pos[0]);


//            GLint loc = glGetUniformLocation(m_shader, ("light_dist[" + std::to_string(used) + "]").c_str());
//            glUniform1f(loc, glm::distance(lt.pos, sp.ctm * glm::vec4(0, 0, 0, 1)));

            GLint dir = glGetUniformLocation(m_shader, ("light_dir[" + std::to_string(used) + "]").c_str());
            glm::vec4 dir_vec = glm::normalize(lt.dir);
            glUniform4fv(dir, 1, &dir_vec[0]);

            GLint loc_fun = glGetUniformLocation(m_shader, ("light_fun[" + std::to_string(used) + "]").c_str());
//            std::cout << "func: " << lt.function.x << " " << lt.function.y << " " << lt.function.z << std::endl;
            glUniform3fv(loc_fun, 1, &lt.function[0]);

            GLint loc_in = glGetUniformLocation(m_shader, ("light_in[" + std::to_string(used) + "]").c_str());
            glm::vec3 col_vec = lt.color;
            glUniform3fv(loc_in, 1, &col_vec[0]);

            GLint loc_type = glGetUniformLocation(m_shader, ("type[" + std::to_string(used) + "]").c_str());
            glUniform1i(loc_type, static_cast<std::underlying_type_t<LightType>>(lt.type));
//            std::cout << "type " << static_cast<std::underlying_type_t<LightType>>(lt.type) << std::endl;
            if (loc_type == -1) {
                std::cout << "type not found" << std::endl;
            }

            GLint loc_angle = glGetUniformLocation(m_shader, ("angle[" + std::to_string(used) + "]").c_str());
            if (loc_angle == -1) {
                std::cout << "angle not found" << std::endl;
            }
            glUniform1f(loc_angle, lt.angle);

            GLint loc_pnum = glGetUniformLocation(m_shader, ("penumbra[" + std::to_string(used) + "]").c_str());
            if (loc_pnum == -1) {
                std::cout << "penum not found" << std::endl;
            }
            glUniform1f(loc_pnum, lt.penumbra);

            used++;

        }


        GLint loc_in = glGetUniformLocation(m_shader, "light_num");
        glUniform1i(loc_in, used);

        if (loc_in == -1) {
            std::cout << "light num err" << std::endl;
        }

        // Actually draw geo
        glBindVertexArray(m_vaos[in]);
        glDrawArrays(GL_TRIANGLES, 0, m_data[in].size() / 6);

    }
    glBindVertexArray(0);

    // Draw to screen
    glBindFramebuffer(GL_FRAMEBUFFER, m_defaultFBO);
    glViewport(0, 0, m_screen_width, m_screen_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintTexture(m_fbo_texture, true);

    glUseProgram(0);
}




void Renderer::resizeGL(int w, int h) {
    // Tells OpenGL how big the screen is
//    glViewport(0, 0, size().width() * m_devicePixelRatio, size().height() * m_devicePixelRatio);

//    m_screen_width = size().width() * m_devicePixelRatio;
//    m_screen_height = size().height() * m_devicePixelRatio;
    // Students: anything requiring OpenGL calls when the program starts should be done here

    m_screen_width = w;
    m_screen_height = h;

    glViewport(0, 0, m_screen_width, m_screen_height);

//    glGenRenderbuffers(1, &m_fbo_renderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_screen_width, m_screen_height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, m_fbo_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screen_width, m_screen_height,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);



}

void Renderer::sceneChanged() {

//    SceneParser parser = SceneParser();
//    parser.parse(settings.sceneFilePath);
//    parser.parse("../../resources/scenes/phong_total.json");

    if (!SceneParser::hasParsed()) {
        std::cout << "SCENE PARSER RENDERER ERROR" << std::endl;
        return;
    }
    data = SceneParser::getSceneData();

    camera = Camera(DSCREEN_WIDTH, DSCREEN_HEIGHT, data.cameraData);
//    camera = Camera(data.cameraData.up, data.cameraData.pos,
//                    data.cameraData.look,
//                    (float)size().width() / (float)size().height(),
//                    data.cameraData.aperture,
//                    data.cameraData.focalLength,
//                    data.cameraData.heightAngle,
//                    settings.nearPlane, settings.farPlane);

//    for (SceneData sp : data.shapes) {
//        std::cout << "SHAPE" << std::endl;
//    }

//    update(); // asks for a PaintGL() call to occur
}

void Renderer::settingsChanged() {

//    if (old_settings.nearPlane != settings.nearPlane || old_settings.farPlane != settings.farPlane)
//        camera.setNearFar(settings.nearPlane, settings.farPlane);

    if (init_gen && (old_settings.shapeParameter1 != settings.shapeParameter1 || old_settings.shapeParameter2 != settings.shapeParameter2))
        generateShape();

    old_settings = settings;

//    update(); // asks for a PaintGL() call to occur
}

// ================== Project 6: Action!
/*
void Renderer::keyPressEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = true;
}

void Renderer::keyReleaseEvent(QKeyEvent *event) {
    m_keyMap[Qt::Key(event->key())] = false;
}

void Renderer::mousePressEvent(QMouseEvent *event) {
    if (event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = true;
        m_prev_mouse_pos = glm::vec2(event->position().x(), event->position().y());
    }
}

void Renderer::mouseReleaseEvent(QMouseEvent *event) {
    if (!event->buttons().testFlag(Qt::LeftButton)) {
        m_mouseDown = false;
    }
}

void Renderer::mouseMoveEvent(QMouseEvent *event) {
    if (m_mouseDown) {
        int posX = event->position().x();
        int posY = event->position().y();
        int deltaX = posX - m_prev_mouse_pos.x;
        int deltaY = posY - m_prev_mouse_pos.y;
        m_prev_mouse_pos = glm::vec2(posX, posY);

        // Use deltaX and deltaY here to rotate
//        glm::rotate()
        camera.rotate(-deltaX / 500.f, deltaY / 500.f);

        update(); // asks for a PaintGL() call to occur
    }
}

void Renderer::timerEvent(QTimerEvent *event) {
    int elapsedms   = m_elapsedTimer.elapsed();
    float deltaTime = elapsedms * 0.001f;
    m_elapsedTimer.restart();

    // Use deltaTime and m_keyMap here to move around

    if (m_keyMap[Qt::Key_W]) {
        camera.move_forward(5 * deltaTime);
    }
    if (m_keyMap[Qt::Key_S]) {
        camera.move_forward(-5 * deltaTime);
    }

    if (m_keyMap[Qt::Key_D]) {
        camera.move_right(5 * deltaTime);
    }
    if (m_keyMap[Qt::Key_A]) {
        camera.move_right(-5 * deltaTime);
    }

    if (m_keyMap[Qt::Key_Space]) {
        camera.move_up(5 * deltaTime);
    }
    if (m_keyMap[Qt::Key_Control]) {
        camera.move_up(-5 * deltaTime);
    }

    update(); // asks for a PaintGL() call to occur
}*/

// DO NOT EDIT
void Renderer::saveViewportImage(std::string filePath) {
    // Make sure we have the right context and everything has been drawn
//    makeCurrent();

    int fixedWidth = 1024;
    int fixedHeight = 768;

    // Create Frame Buffer
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create a color attachment texture
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fixedWidth, fixedHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

    // Optional: Create a depth buffer if your rendering uses depth testing
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, fixedWidth, fixedHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Framebuffer is not complete!" << std::endl;
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    // Render to the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, fixedWidth, fixedHeight);

    // Clear and render your scene here
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    paintGL();

    // Read pixels from framebuffer
    std::vector<unsigned char> pixels(fixedWidth * fixedHeight * 3);
    glReadPixels(0, 0, fixedWidth, fixedHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

    // Unbind the framebuffer to return to default rendering to the screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Convert to QImage
    QImage image(pixels.data(), fixedWidth, fixedHeight, QImage::Format_RGB888);
    QImage flippedImage = image.mirrored(); // Flip the image vertically

    // Save to file using Qt
    QString qFilePath = QString::fromStdString(filePath);
    if (!flippedImage.save(qFilePath)) {
        std::cerr << "Failed to save image to " << filePath << std::endl;
    }

    // Clean up
    glDeleteTextures(1, &texture);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
}
