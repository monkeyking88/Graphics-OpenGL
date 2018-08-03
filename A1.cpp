#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

GLfloat mmyaw   = -90.0f;	// Yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right (due to how Eular angles work) so we initially rotate a bit to the left.
GLfloat mmpitch =   0.0f;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
: current_col(0), active_x(0.0f), active_z(1.0f),slider_color(glm::vec3(0.0f, 0.0f, 0.0f)),changeColor(false),lastX(1024/2),horizontalAngle(0.0f),scale_vec(glm::vec3(1.0f,1.0f,1.0f))
{
	colour[0] = 0.0f;
	colour[1] = 0.0f;
	colour[2] = 0.0f;
    
    //including the ring area
    grid = new Grid(DIM + 2);
    
    //hold RGB value for 8 buttons
    buttonColors = new float[3*8];
    
    for(int k=0; k<3*8; k++)
        buttonColors[k] = 0;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
	// Set the background colour.
	glClearColor( 0.3, 0.5, 0.7, 1.0 );

	// Build the shader
	m_shader.generateProgramObject();
	m_shader.attachVertexShader(
		getAssetFilePath( "VertexShader.vs" ).c_str() );
	m_shader.attachFragmentShader(
		getAssetFilePath( "FragmentShader.fs" ).c_str() );
	m_shader.link();

	// Set up the uniforms
	P_uni = m_shader.getUniformLocation( "P" );
	V_uni = m_shader.getUniformLocation( "V" );
	M_uni = m_shader.getUniformLocation( "M" );
	col_uni = m_shader.getUniformLocation( "colour" );
   	initGrid();

	// Set up initial view and projection matrices (need to do this here,
	// since it depends on the GLFW window being set up correctly).
	view = glm::lookAt( 
		glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
		glm::vec3( 0.0f, 0.0f, 0.0f ),
		glm::vec3( 0.0f, 1.0f, 0.0f ) );
	proj = glm::perspective( 
		glm::radians( 45.0f ),
		float( m_framebufferWidth ) / float( m_framebufferHeight ),
		1.0f, 1000.0f );
}

void A1::initGrid()
{
	size_t sz = 3 * 2 * 2 * (DIM+3);

	float *verts = new float[ sz ];
	size_t ct = 0;
	for( int idx = 0; idx < DIM+3; ++idx ) {
		verts[ ct ] = -1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = idx-1;
		verts[ ct+3 ] = DIM+1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = idx-1;
		ct += 6;

		verts[ ct ] = idx-1;
		verts[ ct+1 ] = 0;
		verts[ ct+2 ] = -1;
		verts[ ct+3 ] = idx-1;
		verts[ ct+4 ] = 0;
		verts[ ct+5 ] = DIM+1;
		ct += 6;
	}

	// Create the vertex array to record buffer assignments.
	glGenVertexArrays( 1, &m_grid_vao );
	glBindVertexArray( m_grid_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_grid_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
	glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
		verts, GL_STATIC_DRAW );

	// Specify the means of extracting the position values properly.
	GLint posAttrib = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray( posAttrib );
	glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

	// Reset state to prevent rogue code from messing with *my* 
	// stuff!
	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	// OpenGL has the buffer now, there's no need for us to keep a copy.
	delete [] verts;

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
	// Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
	// We already know there's only going to be one window, so for 
	// simplicity we'll store button states in static local variables.
	// If there was ever a possibility of having multiple instances of
	// A1 running simultaneously, this would break; you'd want to make
	// this into instance fields of A1.
	static bool showTestWindow(false);
	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
    
    if( ImGui::Button( "Quit Application" )||glfwGetKey(m_window,GLFW_KEY_Q) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
    }
    
    if( ImGui::Button( "Reset" )||glfwGetKey(m_window,GLFW_KEY_R) ) {
            //glfwSetWindowShouldClose(m_window, GL_TRUE);
        colour[0] = 0.0f;
        colour[1] = 0.0f;
        colour[2] = 0.0f;
        
        current_col = 0;
        
        active_x = 0.0f;
        active_z = 1.0f;
        slider_color = glm::vec3(0.0f, 0.0f, 0.0f);
        changeColor =false;
        lastX= 1024/2;
        horizontalAngle = 0.0f;
        scale_vec = glm::vec3(1.0f,1.0f,1.0f);
        
        for(int i= 0; i< DIM+2; i++){
            for (int j =0;j< DIM +2; j++) {
                this->grid->setHeight(i,j,0);
                //setColour(x,y,0);
                
                glClear(GL_COLOR_BUFFER_BIT );
            }
        }
        
        for(int k=0; k<3*8; k++)
        buttonColors[k] = 0;

    }

		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.
    
    
    
    if(ImGui::SliderFloat("Red", &slider_color.r, 0.0f, 1.0f)||
       ImGui::SliderFloat("Green", &slider_color.g, 0.0f, 1.0f)||
       ImGui::SliderFloat("Blue", &slider_color.b, 0.0f, 1.0f)){
        changeColor = true;
    }

    ImGui::PushID( 0 );
    ImGui::ColorEdit3( "##Colour", getRadioButtonColor(0) );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
        if(changeColor) setRadioButtonColor(current_col,slider_color.r,slider_color.g,slider_color.b);
        this->grid->setColour(active_x,active_z,current_col);
        changeColor = false;
    }
    ImGui::PopID();
    
    ImGui::PushID( 1 );
    ImGui::ColorEdit3( "##Colour", getRadioButtonColor(1) );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 1 ) ) {
        if(changeColor) setRadioButtonColor(current_col,slider_color.r,slider_color.g,slider_color.b);
        this->grid->setColour(active_x,active_z,current_col);
        changeColor = false;
    }
    ImGui::PopID();
    ImGui::PushID( 2 );
    ImGui::ColorEdit3( "##Colour", getRadioButtonColor(2) );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 2 ) ) {
        if(changeColor) setRadioButtonColor(current_col,slider_color.r,slider_color.g,slider_color.b);
        this->grid->setColour(active_x,active_z,current_col);
        changeColor = false;
    }
    ImGui::PopID();
    ImGui::PushID( 3 );
    ImGui::ColorEdit3( "##Colour", getRadioButtonColor(3) );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 3 ) ) {
        if(changeColor) setRadioButtonColor(current_col,slider_color.r,slider_color.g,slider_color.b);
        this->grid->setColour(active_x,active_z,current_col);
        changeColor = false;

    }
    ImGui::PopID();
    ImGui::PushID( 4 );
    ImGui::ColorEdit3( "##Colour", getRadioButtonColor(4) );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 4 ) ) {
        if(changeColor) setRadioButtonColor(current_col,slider_color.r,slider_color.g,slider_color.b);
        this->grid->setColour(active_x,active_z,current_col);
        changeColor = false;

    }
    ImGui::PopID();
    
    ImGui::PushID( 5 );
    ImGui::ColorEdit3( "##Colour", getRadioButtonColor(5) );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 5 ) ) {
        if(changeColor) setRadioButtonColor(current_col,slider_color.r,slider_color.g,slider_color.b);
        this->grid->setColour(active_x,active_z,current_col);
        changeColor = false;
    }
    ImGui::PopID();
    
    ImGui::PushID( 6 );
    ImGui::ColorEdit3( "##Colour", getRadioButtonColor(6) );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 6 ) ) {
        if(changeColor) setRadioButtonColor(current_col,slider_color.r,slider_color.g,slider_color.b);
        this->grid->setColour(active_x,active_z,current_col);
        changeColor = false;
    }
    ImGui::PopID();
    
    ImGui::PushID( 7 );
    ImGui::ColorEdit3( "##Colour", getRadioButtonColor(7) );
    ImGui::SameLine();
    if( ImGui::RadioButton( "##Col", &current_col, 7 ) ) {
        if(changeColor) setRadioButtonColor(current_col,slider_color.r,slider_color.g,slider_color.b);
        this->grid->setColour(active_x,active_z,current_col);
        changeColor = false;
    }
    ImGui::PopID();

/*
		// For convenience, you can uncomment this to show ImGui's massive
		// demonstration window right in your application.  Very handy for
		// browsing around to get the widget you want.  Then look in 
		// shared/imgui/imgui_demo.cpp to see how it's done.
		if( ImGui::Button( "Test Window" ) ) {
			showTestWindow = !showTestWindow;
		}
*/

    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();

	if( showTestWindow ) {
		ImGui::ShowTestWindow( &showTestWindow );
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
	// Create a global transformation for the model (centre it).
	mat4 W;
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ));
    W *= glm::scale(mat4(), vec3(scale_vec));
    //W *= glm::scale(mat4(), vec3(-1.0f,1.0f,1.0f));

    W *= glm::translate(mat4(),vec3(float(DIM)/2.0f, 0, float(DIM)/2.0f));
    W *= glm::rotate(mat4(),horizontalAngle,vec3(0.0f, 1.0f, 0.0f));
    W *= glm::translate(mat4(),vec3(-float(DIM)/2.0f, 0, -float(DIM)/2.0f));

	m_shader.enable();
    
    glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
    glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
    glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

    // Just draw the grid for now.
    glBindVertexArray( m_grid_vao );
    glUniform3f( col_uni, 1, 1, 1 );
    glDrawArrays( GL_LINES, 0, (3+DIM)*4 );
    
    //glEnable( GL_DEPTH_TEST );

    // Draw the cubes
    for(float x = 0; x < DIM; x ++){
        for(float z = 1; z <= DIM; z++){
            float y_val = this->grid->getHeight(x,z);
            if( y_val > 0.0f)
                for(float y = 0; y < y_val; y++){
                    drawCube(x,y,z,this->grid->getColour(x,z));
                }
        }
    }
    
    setActiveCell();
    
    // Highlight the active square.
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
}



void A1::setRadioButtonColor(int index, float r, float g, float b){
    buttonColors[index*3+0]= r;
    buttonColors[index*3+1]= g;
    buttonColors[index*3+2]= b;
}

float* A1::getRadioButtonColor(int index){
    float ret[3];
    ret[0]= buttonColors[index*3+0];
    ret[1]= buttonColors[index*3+1];
    ret[2]= buttonColors[index*3+2];

    return ret;
}

//coloring top face to dark to mark as active grid
//do fragment shader
void A1::setActiveCell(){
    float h = this->grid->getHeight(active_x,active_z);
    
    //cout << "height is " << h << endl;
    GLuint vertexbuffer_top;

    glGenVertexArrays( 1, &vertexbuffer_top );
    glBindVertexArray( vertexbuffer_top );
    
    GLfloat cube_vertex_top[] = {
        active_x+1.0f,h,active_z,
        active_x,h,active_z,
        active_x,h,active_z-1.0f,
        
        active_x,h,active_z-1.0f,
        active_x+1.0f,h,active_z-1.0f,
        active_x+1.0f,h,active_z
    };
    
    glGenBuffers(1, &vertexbuffer_top);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_top);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_top), cube_vertex_top, GL_STATIC_DRAW);
    
    //top
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_top);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    //glUniform3f( col_uni, 255, 255, 255 );
    glUniform3f( col_uni, 1,1,1 );

    glDrawArrays(GL_TRIANGLES, 0, 3*2);
    glDisableVertexAttribArray(0);
}

//center point xyz,left bottom cornor
void A1::drawCube(float x, float y, float z, int current_col){
    GLuint vertexbuffer_front;
    GLuint vertexbuffer_back;
    GLuint vertexbuffer_left;
    GLuint vertexbuffer_right;
    GLuint vertexbuffer_top;
    GLuint vertexbuffer_bottom;
    
    float* color = getRadioButtonColor(current_col);
    colour[0] =color[0];
    colour[1] =color[1];
    colour[2] =color[2];
    
    glGenVertexArrays( 1, &vertexbuffer_front );
    glBindVertexArray( vertexbuffer_front );
    
    glGenVertexArrays( 1, &vertexbuffer_back );
    glBindVertexArray( vertexbuffer_back );
    
    glGenVertexArrays( 1, &vertexbuffer_left );
    glBindVertexArray( vertexbuffer_left );
    
    glGenVertexArrays( 1, &vertexbuffer_right );
    glBindVertexArray( vertexbuffer_right );
    
    glGenVertexArrays( 1, &vertexbuffer_top );
    glBindVertexArray( vertexbuffer_top );
    
    glGenVertexArrays( 1, &vertexbuffer_bottom );
    glBindVertexArray( vertexbuffer_bottom );
    
    GLfloat cube_vertex_front[] = {
        x+1.0f,y,z,
        x,y,z,
        x,y+1.0f,z,
        
        x+1.0f,y,z,
        x+1.0f,y+1.0f,z,
        x,y+1.0f,z
    };
    
    GLfloat cube_vertex_back[] = {
        x+1.0f,y,z-1.0f,
        x,y,z-1.0f,
        x,y+1.0f,z-1.0f,
        
        x,y+1.0f,z-1.0f,
        x+1.0f,y+1.0f,z-1.0f,
        x+1.0f,y,z-1.0f
    };

    GLfloat cube_vertex_left[] = {
        x,y,z,
        x,y+1.0f,z,
        x,y,z-1.0f,
        
        x,y,z-1.0f,
        x,y+1.0f,z,
        x,y+1.0f,z-1.0f
    };
    
    GLfloat cube_vertex_right[] = {
        x+1.0f,y,z,
        x+1.0f,y+1.0f,z,
        x+1.0f,y,z-1.0f,
        
        x+1.0f,y,z-1.0f,
        x+1.0f,y+1.0f,z,
        x+1.0f,y+1.0f,z-1.0f
    };
    
    GLfloat cube_vertex_top[] = {
        x+1.0f,y+1.0f,z,
        x,y+1.0f,z,
        x,y+1.0f,z-1.0f,
        
        x,y+1.0f,z-1.0f,
        x+1.0f,y+1.0f,z-1.0f,
        x+1.0f,y+1.0f,z
    };
    
    GLfloat cube_vertex_bottom[] = {
        x+1.0f,y,z,
        x,y,z,
        x,y,z-1.0f,
        
        x,y,z-1.0f,
        x+1.0f,y,z-1.0f,
        x+1.0f,y,z
    };
    
    glGenBuffers(1, &vertexbuffer_front);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_front);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_front), cube_vertex_front, GL_STATIC_DRAW);
    
    glGenBuffers(1, &vertexbuffer_back);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_back);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_back), cube_vertex_back, GL_STATIC_DRAW);
    
    glGenBuffers(1, &vertexbuffer_left);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_left);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_left), cube_vertex_left, GL_STATIC_DRAW);

    glGenBuffers(1, &vertexbuffer_right);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_right);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_right), cube_vertex_right, GL_STATIC_DRAW);

    glGenBuffers(1, &vertexbuffer_top);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_top);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_top), cube_vertex_top, GL_STATIC_DRAW);

    glGenBuffers(1, &vertexbuffer_bottom);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_bottom);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertex_bottom), cube_vertex_bottom, GL_STATIC_DRAW);

    //top
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_top);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glUniform3f( col_uni, colour[0], colour[1], colour[2]);
    glDrawArrays(GL_TRIANGLES, 0, 3*2);
    glDisableVertexAttribArray(0);
    
    //right
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_right);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glUniform3f( col_uni, colour[0], colour[1], colour[2]);
    glDrawArrays(GL_TRIANGLES, 0, 3*2);
    glDisableVertexAttribArray(0);
    
    //bottom
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_bottom);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glUniform3f( col_uni, colour[0], colour[1], colour[2]);
    glDrawArrays(GL_TRIANGLES, 0, 3*2);
    glDisableVertexAttribArray(0);
    
    
    //left
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_left);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glUniform3f( col_uni, colour[0], colour[1], colour[2]);
    glDrawArrays(GL_TRIANGLES, 0, 3*2);
    glDisableVertexAttribArray(0);

    //back
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_back);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glUniform3f( col_uni, colour[0], colour[1], colour[2]);
    glDrawArrays(GL_TRIANGLES, 0, 3*2);
    glDisableVertexAttribArray(0);
    
    //front
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer_front);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glUniform3f( col_uni, colour[0], colour[1], colour[2]);
    glDrawArrays(GL_TRIANGLES, 0, 3*2); //2 triangles
    glDisableVertexAttribArray(0);
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A1::mouseMoveEvent(double xPos, double yPos) 
{
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
        
        //window size 1024*768, lastx/y are the centre
        if(glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS){
            
            horizontalAngle += (xPos -lastX);
            
            horizontalAngle *=0.05;

            lastX = xPos;
            
        }
    }

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

    //cout <<"xOffSet :" <<xOffSet<<endl;
    //cout << "yOffSet :"<<yOffSet<<endl;
	// Zoom in or out.
    
    if(xOffSet< 0.0f && xOffSet >=-2.0f){
        scale_vec.x -= 1/(-(float)xOffSet*10);
    }
    
    if(xOffSet <= 2.0f && xOffSet >0.0f)
        scale_vec.x += (float)xOffSet;
    
    if(yOffSet< 0.0f && yOffSet >=-2.0f){
        scale_vec.z -= 1/(-(float)yOffSet*10);
    }
        
    if(yOffSet <= 2.0f && yOffSet >0.0f)
        scale_vec.z += (float)yOffSet;
    
    if(scale_vec.z >20.0f) scale_vec.z=20.0f;
    
    if(scale_vec.z < 0.1f) scale_vec.z = 0.1f;
    
    if(scale_vec.x >20.0f) scale_vec.x=20.0f;
    
    if(scale_vec.x < 0.1f) scale_vec.x = 0.1f;
    
   	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if( action == GLFW_PRESS ) {
		// Respond to some key events.
        
        //space
        if (key == GLFW_KEY_SPACE ) {
            
            this->grid->setHeight(active_x,active_z, this->grid->getHeight(active_x,active_z)+1.0f);
            
            this->grid->setColour(active_x,active_z,current_col);
            eventHandled = true;
        }
        //backspace
        if (key == GLFW_KEY_BACKSPACE ) {
             this->grid->setHeight(active_x,active_z, this->grid->getHeight(active_x,active_z)-1.0f);
            eventHandled = true;
        }
        
        
        //move active grid
        if(mods != GLFW_MOD_SHIFT){
            //down key arrow
            if (key == GLFW_KEY_DOWN ) {
                if(active_z >= DIM ){
                    active_z = DIM ;
                }else{
                    active_z ++;
                }
                eventHandled = true;
            }
            
            //up key arrow
            if (key == GLFW_KEY_UP ) {
                if(active_z <= 1){
                    active_z = 1;
                }else{
                    active_z --;
                }
                eventHandled = true;
            }

            //right key arrow
            if (key == GLFW_KEY_RIGHT ) {
                if(active_x >= DIM -1 ){
                    active_x = DIM -1;
                }else{
                    active_x ++;
                }
                eventHandled = true;
            }
        
            //left key arrow
            if (key == GLFW_KEY_LEFT ) {
                if(active_x <= 0){
                    active_x = 0;
                }else{
                    active_x --;
                }
                eventHandled = true;
            }
        }else{
            //shift key hold
            //down key arrow
            if (key == GLFW_KEY_DOWN ) {
                if(active_z >= DIM ){
                    active_z = DIM ;
                }else{
                    this->grid->setHeight(active_x,active_z+1, this->grid->getHeight(active_x,active_z));
                    this->grid->setColour(active_x,active_z+1,this->grid->getColour(active_x,active_z));
                    active_z ++;
                }
                eventHandled = true;
            }
            
            //up key arrow
            if (key == GLFW_KEY_UP ) {
                if(active_z <= 1){
                    active_z = 1;
                }else{
                    this->grid->setHeight(active_x,active_z-1, this->grid->getHeight(active_x,active_z));
                    this->grid->setColour(active_x,active_z-1,this->grid->getColour(active_x,active_z));
                    active_z --;
                }
                eventHandled = true;
            }
            if (key == GLFW_KEY_RIGHT ) {
                if(active_x >= DIM -1 ){
                    active_x = DIM -1;
                }else{
                    this->grid->setHeight(active_x+1,active_z, this->grid->getHeight(active_x,active_z));
                    this->grid->setColour(active_x+1,active_z,this->grid->getColour(active_x,active_z));
                    active_x ++;
                }
                eventHandled = true;
            }
            //left key arrow
            if (key == GLFW_KEY_LEFT ) {
                if(active_x <= 0){
                    active_x = 0;
                }else{
                    this->grid->setHeight(active_x-1,active_z, this->grid->getHeight(active_x,active_z));
                    this->grid->setColour(active_x-1,active_z,this->grid->getColour(active_x,active_z));

                    active_x --;
                }
                eventHandled = true;
            }
        }
    }

	return eventHandled;
}
