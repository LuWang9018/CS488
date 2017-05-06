#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;
float cur_x = 0;
float cur_y = 0;
int cur_col = 0;
float turn = 0.0f;
bool mouse_L_hold;
bool mouse_R_hold;
float angle1 = 0;
float y_change = 0;
double pos_x = 0;
double pos_y = 0;
double scroll_y = 0;
float distance1 = 1;
bool if_loaded = false;
//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
	: current_col( 0 )
	
{
	colour[0][0] = 1.0f;
	colour[0][1] = 1.0f;
	colour[0][2] = 1.0f;

	colour[1][0] = 0.5f;
	colour[1][1] = 0.5f;
	colour[1][2] = 0.5f;

	colour[2][0] = 0.0f;
	colour[2][1] = 0.0f;
	colour[2][2] = 0.0f;

	colour[3][0] = 1.0f;
	colour[3][1] = 0.0f;
	colour[3][2] = 0.0f;

	colour[4][0] = 0.0f;
	colour[4][1] = 1.0f;
	colour[4][2] = 0.0f;

	colour[5][0] = 1.0f;
	colour[5][1] = 1.0f;
	colour[5][2] = 0.0f;

	colour[6][0] = 1.0f;
	colour[6][1] = 0.0f;
	colour[6][2] = 1.0f;

	colour[7][0] = 0.0f;
	colour[7][1] = 1.0f;
	colour[7][2] = 1.0f;
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
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}
		if( ImGui::Button( "SAVE" ) ) {
			save();
		}
		if( ImGui::Button( "LOAD" ) ) {
			load();
		}
		if( ImGui::Button( "Reset" ) ) {
				cur_x = 0.0f;
				cur_y = 0.0f;
				for(int i = 0; i < 16; i++){
					for(int j = 0; j < 16; j++){
						(arr[i][j]).hight = 0.0f;
					}
				}
				angle1 = 2*M_PI;
				distance1 = 1;
				y_change = 0;
				view = glm::lookAt( 
				glm::vec3( float(DIM)*2.0*M_SQRT1_2*sin(angle1)*distance1,
						   float(DIM)*2.0*M_SQRT1_2*distance1,
						   float(DIM)*2.0*M_SQRT1_2*cos(angle1)*distance1),
				glm::vec3( 0.0f, 0.0f, 0.0f ),
				glm::vec3( 0.0f, 1.0f, 0.0f ) );


				colour[0][0] = 1.0f;
				colour[0][1] = 1.0f;
				colour[0][2] = 1.0f;

				colour[1][0] = 0.5f;
				colour[1][1] = 0.5f;
				colour[1][2] = 0.5f;

				colour[2][0] = 0.0f;
				colour[2][1] = 0.0f;
				colour[2][2] = 0.0f;

				colour[3][0] = 1.0f;
				colour[3][1] = 0.0f;
				colour[3][2] = 0.0f;

				colour[4][0] = 0.0f;
				colour[4][1] = 1.0f;
				colour[4][2] = 0.0f;

				colour[5][0] = 1.0f;
				colour[5][1] = 1.0f;
				colour[5][2] = 0.0f;

				colour[6][0] = 1.0f;
				colour[6][1] = 0.0f;
				colour[6][2] = 1.0f;

				colour[7][0] = 0.0f;
				colour[7][1] = 1.0f;
				colour[7][2] = 1.0f;

		}
		// Eventually you'll create multiple colour widgets with
		// radio buttons.  If you use PushID/PopID to give them all
		// unique IDs, then ImGui will be able to keep them separate.
		// This is unnecessary with a single colour selector and
		// radio button, but I'm leaving it in as an example.

		// Prefixing a widget name with "##" keeps it from being
		// displayed.

		ImGui::PushID( 0 );
		ImGui::ColorEdit3( "##Colour", colour[0] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col", &current_col, 0 ) ) {
			// Select this colour.
			change_color(0);
		}
		ImGui::PopID();

		ImGui::PushID( 1 );
		ImGui::ColorEdit3( "##Colour", colour[1] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col2", &current_col, 1 ) ) {
			// Select this colour.
			change_color(1);
		}
		ImGui::PopID();

		ImGui::PushID( 2 );
		ImGui::ColorEdit3( "##Colour", colour[2] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col2", &current_col, 2 ) ) {
			// Select this colour.
			change_color(2);
		}
		ImGui::PopID();

		ImGui::PushID( 3 );
		ImGui::ColorEdit3( "##Colour", colour[3] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col2", &current_col, 3 ) ) {
			// Select this colour.
			change_color(3);
		}
		ImGui::PopID();

		ImGui::PushID( 4 );
		ImGui::ColorEdit3( "##Colour", colour[4] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col2", &current_col, 4 ) ) {
			// Select this colour.
			change_color(4);
		}
		ImGui::PopID();

		ImGui::PushID( 5 );
		ImGui::ColorEdit3( "##Colour", colour[5] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col2", &current_col, 5 ) ) {
			// Select this colour.
			change_color(5);
		}
		ImGui::PopID();

		ImGui::PushID( 6 );
		ImGui::ColorEdit3( "##Colour", colour[6] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col2", &current_col, 6 ) ) {
			// Select this colour.
			change_color(6);
		}
		ImGui::PopID();

				ImGui::PushID( 7 );
		ImGui::ColorEdit3( "##Colour", colour[7] );
		ImGui::SameLine();
		if( ImGui::RadioButton( "##Col2", &current_col, 7 ) ) {
			// Select this colour.
			change_color(7);
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
	W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

	m_shader.enable();
		glEnable( GL_DEPTH_TEST );

		glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
		glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
		glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

		// Just draw the grid for now.
		glBindVertexArray( m_grid_vao );
		glUniform3f( col_uni, 1, 1, 1 );
		glDrawArrays( GL_LINES, 0, (3+DIM)*4 );

		// Draw the cubes
		draw_arror(cur_x, cur_y);
		//draw_a_cube(0, 1, 2);
		if(!if_loaded){
			load();
			if_loaded = true;
		}
		//draw_a_cube(3, 4, 1);
		for(int i = 0 ; i < 16.0f; i++){
			for (int j = 0; j < 16.0f; j++){
				draw_a_cube((float)i, (float)j, (arr[i][j]).hight);
			}
		}

		// Highlight the active square.
	m_shader.disable();

	// Restore defaults
	glBindVertexArray( 0 );

	CHECK_GL_ERRORS;
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

	if (!ImGui::IsMouseHoveringAnyWindow() && mouse_L_hold) {

		if(pos_x == 0){
			pos_x = xPos;
		}
		if(pos_y == 0){
			pos_y = yPos;
		}

		if(pos_x > xPos){
			if(angle1 > -2 * M_PI){
				angle1 -= 0.015*(pos_x - xPos)/2;
			}
			pos_x = xPos;
		}
		else if (pos_x < xPos){
			if(angle1 < 2 * M_PI){
				angle1 += 0.015*( xPos - pos_x)/2;
			}
			pos_x = xPos;
		}

		if(pos_y > yPos){
			if(y_change < 1){
				//cout<< "here1" <<endl;
				y_change += 0.015*(pos_y - yPos)/2;
			}
			pos_y = yPos;
		}
		else if (pos_y < yPos){
			if(y_change > -2){
				//cout<< "here2" <<endl;
				y_change -= 0.015*( yPos - pos_y)/2;
			}
			pos_y = yPos;
		}
//cout<< y_change <<endl;
		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		view = glm::lookAt( 
			glm::vec3( float(DIM)*2.0*M_SQRT1_2*sin(angle1)*distance1,
			           float(DIM)*2.0*M_SQRT1_2*distance1+y_change*10*distance1,
			           float(DIM)*2.0*M_SQRT1_2*cos(angle1)*distance1),
			glm::vec3( 0.0f, 0.0f, 0.0f ),
			glm::vec3( 0.0f, 1.0f, 0.0f ) );
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
		if (actions == GLFW_PRESS){
			if(mods == GLFW_MOUSE_BUTTON_1) {
		        mouse_L_hold = 1;
		    }
		}
	    else if(actions == GLFW_RELEASE){
			if(mods == GLFW_MOUSE_BUTTON_1) {
				pos_x =0;
				pos_y = 0;
		        mouse_L_hold = 0;
		    }
	    }
	}

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
	bool eventHandled(false);

	if (!ImGui::IsMouseHoveringAnyWindow()) {
		if(0 < yOffSet && distance1 >=0.72f){

			distance1 -= 0.02;
		}
		else if (0 > yOffSet && distance1 <= 1.4){

			distance1 += 0.02;

		}

		// Put some code here to handle rotations.  Probably need to
		// check whether we're *dragging*, not just moving the mouse.
		// Probably need some instance variables to track the current
		// rotation amount, and maybe the previous X position (so 
		// that you can rotate relative to the *change* in X.
		view = glm::lookAt( 
			glm::vec3( float(DIM)*2.0*M_SQRT1_2*sin(angle1)*distance1,
					   float(DIM)*2.0*M_SQRT1_2*distance1+y_change*10*distance1,
					   float(DIM)*2.0*M_SQRT1_2*cos(angle1)*distance1),
			glm::vec3( 0.0f, 0.0f, 0.0f ),
			glm::vec3( 0.0f, 1.0f, 0.0f ) );
	}
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
		if (mods == GLFW_MOD_SHIFT) {
			if (key == GLFW_KEY_UP) {
				if(cur_y > 0){
					cur_y--;
					arr[(int)cur_x][(int)cur_y].color = arr[(int)cur_x][(int)cur_y+1].color;
					arr[(int)cur_x][(int)cur_y].hight = arr[(int)cur_x][(int)cur_y+1].hight;
					

				}
			}
			if (key == GLFW_KEY_DOWN) {
				if(cur_y < 15){
					cur_y++;
					arr[(int)cur_x][(int)cur_y].color = arr[(int)cur_x][(int)cur_y-1].color;
					arr[(int)cur_x][(int)cur_y].hight = arr[(int)cur_x][(int)cur_y-1].hight;
					
				}
			}	
			if (key == GLFW_KEY_LEFT) {
				if(cur_x > 0){
					cur_x--;
					arr[(int)cur_x][(int)cur_y].color = arr[(int)cur_x+1][(int)cur_y].color;
					arr[(int)cur_x][(int)cur_y].hight = arr[(int)cur_x+1][(int)cur_y].hight;
					
				}
			}	
			if (key == GLFW_KEY_RIGHT) {
				if(cur_x < 15){
					cur_x++;
					arr[(int)cur_x][(int)cur_y].color = arr[(int)cur_x-1][(int)cur_y].color;
					arr[(int)cur_x][(int)cur_y].hight = arr[(int)cur_x-1][(int)cur_y].hight;
					
				}
			}
		}	
		else{
			if (key == GLFW_KEY_Q) {
				glfwSetWindowShouldClose(m_window, GL_TRUE);
			}
			if (key == GLFW_KEY_R) {
				cur_x = 0.0f;
				cur_y = 0.0f;
				for(int i = 0; i < 16; i++){
					for(int j = 0; j < 16; j++){
						(arr[i][j]).hight = 0.0f;
					}
				}
				angle1 = 2*M_PI;
				distance1 = 1;
				y_change = 0;
				view = glm::lookAt( 
				glm::vec3( float(DIM)*2.0*M_SQRT1_2*sin(angle1)*distance1,
						   float(DIM)*2.0*M_SQRT1_2*distance1,
						   float(DIM)*2.0*M_SQRT1_2*cos(angle1)*distance1),
				glm::vec3( 0.0f, 0.0f, 0.0f ),
				glm::vec3( 0.0f, 1.0f, 0.0f ) );


				colour[0][0] = 1.0f;
				colour[0][1] = 1.0f;
				colour[0][2] = 1.0f;

				colour[1][0] = 0.5f;
				colour[1][1] = 0.5f;
				colour[1][2] = 0.5f;

				colour[2][0] = 0.0f;
				colour[2][1] = 0.0f;
				colour[2][2] = 0.0f;

				colour[3][0] = 1.0f;
				colour[3][1] = 0.0f;
				colour[3][2] = 0.0f;

				colour[4][0] = 0.0f;
				colour[4][1] = 1.0f;
				colour[4][2] = 0.0f;

				colour[5][0] = 1.0f;
				colour[5][1] = 1.0f;
				colour[5][2] = 0.0f;

				colour[6][0] = 1.0f;
				colour[6][1] = 0.0f;
				colour[6][2] = 1.0f;

				colour[7][0] = 0.0f;
				colour[7][1] = 1.0f;
				colour[7][2] = 1.0f;
			}
			if (key == GLFW_KEY_UP) {
				if(cur_y > 0){
					cur_y--;
				}
			}
			if (key == GLFW_KEY_DOWN) {
				if(cur_y < 15){
					cur_y++;
				}
			}	
			if (key == GLFW_KEY_LEFT) {
				if(cur_x > 0){
					cur_x--;
				}
			}	
			if (key == GLFW_KEY_RIGHT) {
				if(cur_x < 15){
					cur_x++;
				}
			}
			if (key == GLFW_KEY_SPACE) {
				if(arr[(int)cur_x][(int)cur_y].hight == 0){
					arr[(int)cur_x][(int)cur_y].color = cur_col;
				}
				arr[(int)cur_x][(int)cur_y].hight++;
			}
			if (key == GLFW_KEY_BACKSPACE) {
				if(arr[(int)cur_x][(int)cur_y].hight > 0){
					arr[(int)cur_x][(int)cur_y].hight--;
				}
			}
			if (key == GLFW_KEY_S) {
				save();
			}
			if (key == GLFW_KEY_L) {
				load();
			}
		}
					
	}

	return eventHandled;
}


void A1::draw_a_cube(float ori_x, float ori_y, float hight){

	size_t sz = 3 * 2 * 6 * 3;
	float *verts = new float[ sz ];
		verts[0] = ori_x;
		verts[1] = 0.0f;
		verts[2] = ori_y;

		verts[3] = ori_x;
		verts[4] = hight;
		verts[5] = ori_y;

		verts[6] = ori_x + 1;
		verts[7] = 0.0f;
		verts[8] = ori_y;



		verts[9] = ori_x;
		verts[10] = hight;
		verts[11] = ori_y;

		verts[12] = ori_x + 1;
		verts[13] = 0.0f;
		verts[14] = ori_y; 

		verts[15] = ori_x + 1;
		verts[16] = hight;
		verts[17] = ori_y;



		verts[18] = ori_x;
		verts[19] = 0.0f;
		verts[20] = ori_y;

		verts[21] = ori_x;
		verts[22] = hight;
		verts[23] = ori_y;

		verts[24] = ori_x;
		verts[25] = 0.0f;
		verts[26] = ori_y + 1;



		verts[27] = ori_x;
		verts[28] = hight;
		verts[29] = ori_y;

		verts[30] = ori_x;
		verts[31] = 0.0f;
		verts[32] = ori_y + 1; 

		verts[33] = ori_x;
		verts[34] = hight;
		verts[35] = ori_y + 1;



		verts[36] = ori_x;
		verts[37] = 0.0f;
		verts[38] = ori_y;

		verts[39] = ori_x + 1;
		verts[40] = 0.0f;
		verts[41] = ori_y;

		verts[42] = ori_x;
		verts[43] = 0.0f;
		verts[44] = ori_y + 1;



		verts[45] = ori_x + 1;
		verts[46] = 0.0f;
		verts[47] = ori_y + 1;

		verts[48] = ori_x;
		verts[49] = 0.0f;
		verts[50] = ori_y + 1; 

		verts[51] = ori_x + 1;
		verts[52] = 0.0f;
		verts[53] = ori_y;






		verts[54] = ori_x;
		verts[55] = 0.0f;
		verts[56] = ori_y + 1;

		verts[57] = ori_x;
		verts[58] = hight;
		verts[59] = ori_y + 1;

		verts[60] = ori_x + 1;
		verts[61] = 0.0f;
		verts[62] = ori_y + 1;



		verts[63] = ori_x;
		verts[64] = hight;
		verts[65] = ori_y + 1;

		verts[66] = ori_x + 1;
		verts[67] = 0.0f;
		verts[68] = ori_y + 1; 

		verts[69] = ori_x + 1;
		verts[70] = hight;
		verts[71] = ori_y + 1;



		verts[72] = ori_x + 1;
		verts[73] = 0.0f;
		verts[74] = ori_y;

		verts[75] = ori_x + 1;
		verts[76] = hight;
		verts[77] = ori_y;

		verts[78] = ori_x + 1;
		verts[79] = 0.0f;
		verts[80] = ori_y + 1;



		verts[81] = ori_x + 1;
		verts[82] = hight;
		verts[83] = ori_y;

		verts[84] = ori_x + 1;
		verts[85] = 0.0f;
		verts[86] = ori_y + 1; 

		verts[87] = ori_x + 1;
		verts[88] = hight;
		verts[89] = ori_y + 1;



		verts[90] = ori_x;
		verts[91] = 0.0f + hight;
		verts[92] = ori_y;

		verts[93] = ori_x + 1;
		verts[94] = 0.0f + hight;
		verts[95] = ori_y;

		verts[96] = ori_x;
		verts[97] = 0.0f + hight;
		verts[98] = ori_y + 1;



		verts[99] = ori_x + 1;
		verts[100] = 0.0f + hight;
		verts[101] = ori_y + 1;

		verts[102] = ori_x;
		verts[103] = 0.0f + hight;
		verts[104] = ori_y + 1; 

		verts[105] = ori_x + 1;
		verts[106] = 0.0f + hight;
		verts[107] = ori_y;


	glGenVertexArrays( 1, &m_cube_vao );
	glBindVertexArray( m_cube_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_cube_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_cube_vbo );
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

	int colour_cube = arr[(int)ori_x][(int)ori_y].color;
	glBindVertexArray( m_cube_vao );
	glUniform3f( col_uni, colour[colour_cube][0], colour[colour_cube][1], colour[colour_cube][2] );

	if (hight != 0){
		glDrawArrays(GL_TRIANGLES, 0, 3*12);
	}
}

void A1::draw_arror(float cur_x1, float cur_y1){

	size_t sz = 3 * 3 * 6 + 3 * 3 * 4;
	float *verts = new float[ sz ];
		float hight_hi = 1.25f + arr[(int)cur_x][(int)cur_y].hight;
		float hight_low = 0.25f + arr[(int)cur_x][(int)cur_y].hight;
		verts[0] = cur_x1 + 0.25f;
		verts[1] = hight_hi;
		verts[2] = cur_y1 + 0.25f;

		verts[3] = cur_x1 + 0.75f;
		verts[4] = hight_hi;
		verts[5] = cur_y1 + 0.25f;

		verts[6] = cur_x1 + 0.25f;
		verts[7] = hight_hi;
		verts[8] = cur_y1 + 0.75f;



		verts[9] = cur_x1 + 0.75f;
		verts[10] = hight_hi;
		verts[11] = cur_y1 + 0.75f;

		verts[12] = cur_x1 + 0.25f;
		verts[13] = hight_hi;
		verts[14] = cur_y1 + 0.75f;

		verts[15] = cur_x1 + 0.75f;
		verts[16] = hight_hi;
		verts[17] = cur_y1 + 0.25f;





		verts[18] = cur_x1 + 0.25f;
		verts[19] = hight_hi;
		verts[20] = cur_y1 + 0.25f;

		verts[21] = cur_x1 + 0.75f;
		verts[22] = hight_hi;
		verts[23] = cur_y1 + 0.25f;

		verts[24] = cur_x1 + 0.5f;
		verts[25] = hight_low;
		verts[26] = cur_y1 + 0.5f;



		verts[27] = cur_x1 + 0.25f;
		verts[28] = hight_hi;
		verts[29] = cur_y1 + 0.25f;

		verts[30] = cur_x1 + 0.25f;
		verts[31] = hight_hi;
		verts[32] = cur_y1 + 0.75f;

		verts[33] = cur_x1 + 0.5f;
		verts[34] = hight_low;
		verts[35] = cur_y1 + 0.5f;



		verts[36] = cur_x1 + 0.75f;
		verts[37] = hight_hi;
		verts[38] = cur_y1 + 0.75f;

		verts[39] = cur_x1 + 0.75f;
		verts[40] = hight_hi;
		verts[41] = cur_y1 + 0.25f;

		verts[42] = cur_x1 + 0.5f;
		verts[43] = hight_low;
		verts[44] = cur_y1 + 0.5f;



		verts[45] = cur_x1 + 0.75f;
		verts[46] = hight_hi;
		verts[47] = cur_y1 + 0.75f;

		verts[48] = cur_x1 + 0.25f;
		verts[49] = hight_hi;
		verts[50] = cur_y1 + 0.75f;

		verts[51] = cur_x1 + 0.5f;
		verts[52] = hight_low;
		verts[53] = cur_y1 + 0.5f;


		verts[54] = cur_x1;
		verts[55] = 0.0f;
		verts[56] = -1.0f;

		verts[57] = cur_x1 + 0.5;
		verts[58] = 0.0f;
		verts[59] = 0.0f;

		verts[60] = cur_x1 + 1;
		verts[61] = 0.0f;
		verts[62] = -1.0f;



		verts[63] = cur_x1;
		verts[64] = 0.0f;
		verts[65] = 17;

		verts[66] = cur_x1 + 0.5;
		verts[67] = 0.0f;
		verts[68] = 16; 

		verts[69] = cur_x1 + 1;
		verts[70] = 0.0f;
		verts[71] = 17;



		verts[72] = -1;
		verts[73] = 0.0f;
		verts[74] = cur_y1;

		verts[75] = -1;
		verts[76] = 0.0f;
		verts[77] = cur_y1 + 1;

		verts[78] = 0;
		verts[79] = 0.0f;
		verts[80] = cur_y1 + 0.5;



		verts[81] = 17;
		verts[82] = 0.0f;
		verts[83] = cur_y1;

		verts[84] = 16;
		verts[85] = 0.0f;
		verts[86] = cur_y1 + 0.5; 

		verts[87] = 17;
		verts[88] = 0.0f;
		verts[89] = cur_y1 + 1;




	glGenVertexArrays( 1, &m_arror_vao );
	glBindVertexArray( m_arror_vao );

	// Create the cube vertex buffer
	glGenBuffers( 1, &m_arror_vbo );
	glBindBuffer( GL_ARRAY_BUFFER, m_arror_vbo );
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


	glBindVertexArray( m_arror_vao );
	glUniform3f( col_uni, colour[cur_col][0], colour[cur_col][1], colour[cur_col][2] );

		glDrawArrays(GL_TRIANGLES, 0, 3*6 + 3*4);

}

void A1::change_color(int th_color){
	cur_col = th_color;
	arr[(int)cur_x][(int)cur_y].color = th_color;
	//cout << cur_col <<" " << (int)colour[cur_col][0] <<endl;

}

void A1::save(){
	float array_hight[256] = {0};
	int array_color[256] = {0};

	for(int i = 0; i < 16 ; i++){
		for(int j = 0; j < 16 ; j++){
			array_hight[i*16+j] = arr[i][j].hight;
			//cout << array_hight[i*16+j] ;
			array_color[i*16+j] = arr[i][j].color;
		}
	} 
	ofstream fout("hight.txt"); //opening an output stream for file test.txt

	if(fout.is_open()){
		for(int i = 0; i < sizeof(array_hight)/sizeof(array_hight[0]); i++){
	    	fout << array_hight[i] << " "; //writing ith character of array in the file
		}
	}
	ofstream fout2("color.txt"); //opening an output stream for file test.txt

	if(fout2.is_open()){
		for(int i = 0; i < sizeof(array_color)/sizeof(array_color[0]); i++){
	    	fout2 << array_color[i] << " "; //writing ith character of array in the file
		}
	}
	else{
		cout << "File could not be opened." << endl;
	}
}

void A1::load(){
	string array_hight;
	string array_color;
	ifstream hight123;
	ifstream color123;

    hight123.open("hight.txt");
	getline (hight123, array_hight);
    color123.open("color.txt");
	getline (color123, array_color);
	//cout << array_color <<endl;
	//cout << array_hight <<endl;	


	std::stringstream stream(array_hight);


	for(int i = 0; i < 16 ; i++){
		for(int j = 0; j < 16 ; j++){
			float height_get;
			stream >> height_get;
			arr[i][j].hight = height_get;
		}
	} 

	std::stringstream stream2(array_color);
	for(int i = 0; i < 16 ; i++){
		for(int j = 0; j < 16 ; j++){
			int color_get;
			stream2 >> color_get;
			arr[i][j].color = color_get;
		}
	} 
}