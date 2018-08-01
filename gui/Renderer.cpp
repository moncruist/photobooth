#include <cstdlib>
#include "Renderer.h"
#include "logging.h"

namespace phb::gui {

bool Renderer::on_init(AbstractEglGui &context) {
    glClearColor ( 0.0f, 0.0f, 0.0f, 0.0f );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &frame_texture_);
    glBindTexture(GL_TEXTURE_2D, frame_texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

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