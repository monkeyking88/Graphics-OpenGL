#pragma once

#include <glm/glm.hpp>

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include "grid.hpp"

class A1 : public CS488Window {
public:
	A1();
	virtual ~A1();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

private:
	void initGrid();
    
    //helpers
    void drawCube(float x, float y, float z, int current_col);
    void setActiveCell();
    //float* colourSwitch(int index);
    
    void setRadioButtonColor(int index, float r, float g, float b);
    float* getRadioButtonColor(int index);

	// Fields related to the shader and uniforms.
	ShaderProgram m_shader;
	GLint P_uni; // Uniform location for Projection matrix.
	GLint V_uni; // Uniform location for View matrix.
	GLint M_uni; // Uniform location for Model matrix.
	GLint col_uni;   // Uniform location for cube colour.
    
    GLfloat lastX;
    GLfloat lastY;
    float horizontalAngle;
    
    GLfloat m_yaw;// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
    GLfloat m_pitch;


	// Fields related to grid geometry.
	GLuint m_grid_vao; // Vertex Array Object
	GLuint m_grid_vbo; // Vertex Buffer Object

	// Matrices controlling the camera and projection.
	glm::mat4 proj;
	glm::mat4 view;

	float colour[3];
	int current_col;
    glm::vec3 slider_color;
    glm::vec3 scale_vec;
    
    //extra param
    float active_x;
    float active_z;
    static const size_t DIM = 16;
    //recoding height for each grid point in case that we need height info

    Grid* grid;
    
    float* buttonColors;
    bool changeColor;
};
