#include <cstdlib>
#include "Renderer.h"
#include "logging.h"

namespace phb::gui {

bool Renderer::on_init(AbstractEglGui &context) {
    char vShaderStr[] =
        "attribute vec4 vPosition;    \n"
        "void main()                  \n"
        "{                            \n"
        "   gl_Position = vPosition;  \n"
        "}                            \n";

    char fShaderStr[] =
        "precision mediump float;\n"\
      "void main()                                  \n"
        "{                                            \n"
        "  gl_FragColor = vec4 ( 1.0, 0.0, 0.0, 1.0 );\n"
        "}                                            \n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLint linked;

    // Load the vertex/fragment shaders
    vertexShader = load_shader ( GL_VERTEX_SHADER, vShaderStr );
    fragmentShader = load_shader ( GL_FRAGMENT_SHADER, fShaderStr );

    // Create the program object
    program_object_ = glCreateProgram ( );

    if ( program_object_ == 0 )
        return 0;

    glAttachShader ( program_object_, vertexShader );
    glAttachShader ( program_object_, fragmentShader );

    // Bind vPosition to attribute 0
    glBindAttribLocation ( program_object_, 0, "vPosition" );

    // Link the program
    glLinkProgram ( program_object_ );

    // Check the link status
    glGetProgramiv ( program_object_, GL_LINK_STATUS, &linked );

    if ( !linked )
    {
        GLint infoLen = 0;

        glGetProgramiv ( program_object_, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog = (char*) malloc (sizeof(char) * infoLen );

            glGetProgramInfoLog ( program_object_, infoLen, NULL, infoLog );
            ERR() <<  "Error linking program:" << infoLog;

            free ( infoLog );
        }

        glDeleteProgram ( program_object_ );
        return false;
    }

    glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );
    return true;
}

void Renderer::on_update(AbstractEglGui &context) {

}

void Renderer::on_draw(AbstractEglGui &context) {
    GLfloat vVertices[] = {  0.0f,  0.5f, 0.0f,
                             -0.5f, -0.5f, 0.0f,
                             0.5f, -0.5f, 0.0f };

    // Set the viewport
    glViewport ( 0, 0, context.get_width(), context.get_height() );

    // Clear the color buffer
    glClear ( GL_COLOR_BUFFER_BIT );

    // Use the program object
    glUseProgram ( program_object_ );

    // Load the vertex data
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vVertices );
    glEnableVertexAttribArray ( 0 );

    glDrawArrays ( GL_TRIANGLES, 0, 3 );
}

void Renderer::on_keyboard_event(AbstractEglGui &context, char key) {

}

GLuint Renderer::load_shader(GLenum type, const char *shader_src) {
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader ( type );

    if ( shader == 0 )
        return 0;

    // Load the shader source
    glShaderSource ( shader, 1, &shader_src, NULL );

    // Compile the shader
    glCompileShader ( shader );

    // Check the compile status
    glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

    if ( !compiled )
    {
        GLint infoLen = 0;

        glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char* infoLog = (char *) malloc (sizeof(char) * infoLen );

            glGetShaderInfoLog ( shader, infoLen, NULL, infoLog );
            ERR() << "Error compiling shader: " << infoLog;

            free ( infoLog );
        }

        glDeleteShader ( shader );
        return 0;
    }

    return shader;
}

}