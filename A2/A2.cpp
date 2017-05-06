#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;


mat3 box_points_after[8];
int cur_mode = 3;
bool mouse_L_hold = 0;
bool mouse_M_hold = 0;
bool mouse_R_hold = 0;
double pos_x = 0;
double pos_y = 0;
double pos_z = 0;
float cur_x = 0;
float cur_y = 0;
float cur_z = 0 ;
float window_width = 768;
float window_height = 768;	
float window_width_viewport = window_width * 0.9f;
float window_height_viewport = window_height * 0.9f;	
bool lock_xy_start = true;
bool lock_xy_end = false;
float tempx_pos_begin = -window_width * 0.9f;
float tempy_pos_begin = -window_height * 0.9f;
float tempx_pos_end = window_width * 0.9f;
float tempy_pos_end = window_height * 0.9f;
mat4 eye_rot_sum = mat4(1);
mat4 box_rot_sum = mat4(1);

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
	: numVertices(0),
	  index(0)
{
	positions.reserve(kMaxVertices);
	colours.reserve(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
	: m_currentLineColour(vec3(0.0f))
{
	box_points.reserve(8);
	box_points_new.reserve(8);
	box_points_after_clip.reserve(24);
	box_coord_after_clip.reserve(24);
	world_coord_after_clip.reserve(24);
	box_points[0] = vec4(1,1,1,1);
	box_points[1] = vec4(1,1,-1,1);
	box_points[2] = vec4(1,-1,1,1);
	box_points[3] = vec4(1,-1,-1,1);
	box_points[4] = vec4(-1,1,1,1);
	box_points[5] = vec4(-1,1,-1,1);
	box_points[6] = vec4(-1,-1,1,1);
	box_points[7] = vec4(-1,-1,-1,1);
	box_points_new[0] = vec4(1,1,1,1);
	box_points_new[1] = vec4(1,1,-1,1);
	box_points_new[2] = vec4(1,-1,1,1);
	box_points_new[3] = vec4(1,-1,-1,1);
	box_points_new[4] = vec4(-1,1,1,1);
	box_points_new[5] = vec4(-1,1,-1,1);
	box_points_new[6] = vec4(-1,-1,1,1);
	box_points_new[7] = vec4(-1,-1,-1,1);

	box_coord.reserve(4);
	box_coord_new.reserve(4);

	box_coord[0] = vec4(0,0,0,1);
	box_coord[1] = vec4(-0.3,0,0,1);
	box_coord[2] = vec4(0,-0.3,0,1);
	box_coord[3] = vec4(0,0,-0.3,1);
	box_coord_new[0] = vec4(0,0,0,1);
	box_coord_new[1] = vec4(-0.3,0,0,1);
	box_coord_new[2] = vec4(0,-0.3,0,1);
	box_coord_new[3] = vec4(0,0,-0.3,1);


	world_coord.reserve(4);
	world_coord_new.reserve(4);

	world_coord[0] = vec4(0,0,0,1);
	world_coord[1] = vec4(-0.3,0,0,1);
	world_coord[2] = vec4(0,-0.3,0,1);
	world_coord[3] = vec4(0,0,-0.3,1);
	world_coord_new[0] = vec4(0,0,0,1);
	world_coord_new[1] = vec4(-0.3,0,0,1);
	world_coord_new[2] = vec4(0,-0.3,0,1);
	world_coord_new[3] = vec4(0,0,-0.3,1);
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);


	// Set the background colour.
	glClearColor(0.3, 0.5, 0.7, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao);

	enableVertexAttribIndices();

	generateVertexBuffers();

	mapVboDataToVertexAttributeLocation();

}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
	glBindVertexArray(m_vao);

	// Enable the attribute index location for "position" when rendering.
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glEnableVertexAttribArray(positionAttribLocation);

	// Enable the attribute index location for "colour" when rendering.
	GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
	glEnableVertexAttribArray(colourAttribLocation);

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
	// Generate a vertex buffer to store line vertex positions
	{
		glGenBuffers(1, &m_vbo_positions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	// Generate a vertex buffer to store line colors
	{
		glGenBuffers(1, &m_vbo_colours);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

		// Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
				GL_DYNAMIC_DRAW);


		// Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao);

	// Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
	// "position" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
	GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
	glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
	// "colour" vertex attribute index for any bound shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
	GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
	glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//---------------------------------------------------------------------------------------
void A2::initLineData()
{
	m_vertexData.numVertices = 0;
	m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
		const glm::vec3 & colour
) {
	m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
		const glm::vec2 & v0,   // Line Start (NDC coordinate)
		const glm::vec2 & v1    // Line End (NDC coordinate)
) {

	m_vertexData.positions[m_vertexData.index] = v0;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;
	m_vertexData.positions[m_vertexData.index] = v1;
	m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
	++m_vertexData.index;

	m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
	// Place per frame, application logic here ...

	// Call at the beginning of frame, before drawing lines:
	initLineData();


	eye_view_sum = mat4(1)/(eye_trans * eye_rot_sum);



	mat4 final = eye_view_sum *
				 box_view  * 
			     box_trans *
				 box_rot_sum *
				 box_sca;

	//mat4 eye_trans = transpose(	eye_view_sum );
	//print_mat(final);
//matrix trans part
	//box matrix

	mat4 box_view_coord = eye_view_sum *
						  box_trans * 
						  box_rot_sum;

	mat4 world_view_coord = eye_view_sum;

	for(int i = 0; i < 8; i++){
		box_points_new[i] = final * box_points[i];
	}

	for(int i = 0; i < 4;  i++){
		box_coord_new[i] = box_view_coord * box_coord[i];
		//cout << box_coord_new[i][0] << endl;
	}	

	for(int i = 0; i < 4;  i++){
		world_coord_new[i] = world_view_coord * world_coord[i];
		//cout << box_coord_new[i][0] << endl;
	}

	vec4 normal_n4 = {0,0,1,0};
	vec4 plane_n_P = {0,0,n,1};
	vec4 normal_f4 = {0,0,-1,0};
	vec4 plane_f_P = {0,0,f,1};

	//cout<< box_rot_x[1][1] <<endl;
   	int	n_counter = 0;
	//box
	setLineColour(vec3(0.2f, 1.0f, 1.0f));
	for(int i = 0; i < 12; i++){
		drawline_n[i]= true;
	}

	for(int i = 0; i < 12; i++){
//cout << P_A_x << " " << P_A_y  << " " << P_B_x << " " << P_B_y<<endl;
		vec4 pointA = box_points_new[(link_arr[i][0])-1];
		vec4 pointB = box_points_new[(link_arr[i][1])-1];
		float tempA = dot((pointA - plane_n_P),
					       normal_n4);
		float tempB = dot((pointB - plane_n_P),
					       normal_n4);
		pointA = box_with_z * pointA;
		pointB = box_with_z * pointB;

		if(tempA < 0 && tempB < 0){
			drawline_n[i] = false;
			n_counter++;
			n_counter++;
		}
		else if(tempA >= 0 && tempB >= 0){
			box_points_after_clip[n_counter] = pointA;
			n_counter++;
			box_points_after_clip[n_counter] = pointB;
			n_counter++;
		}
		else{
			float t = tempA / (tempA - tempB);
			vec4 new_point = pointA + t * (pointB - pointA);
			//new_point = box_with_z * new_point;
			if(tempA < 0){
				pointA = new_point;		
			}
			else{
				pointB = new_point;	
			}
			box_points_after_clip[n_counter] = pointA;
			n_counter++;
			box_points_after_clip[n_counter] = pointB;
			n_counter++;

		}

	}
	n_counter = 0;



   	int	n_counter2 = 0;
	//box
	for(int i = 0; i < 3; i++){
		drawline_n2[i]= true;
	}

	for(int i = 0; i < 3; i++){
//cout << P_A_x << " " << P_A_y  << " " << P_B_x << " " << P_B_y<<endl;
		vec4 pointA = box_coord_new[(link_arr2[i][0])];
		vec4 pointB = box_coord_new[(link_arr2[i][1])];
		float tempA = dot((pointA - plane_n_P),
					       normal_n4);
		float tempB = dot((pointB - plane_n_P),
					       normal_n4);
		pointA = box_with_z * pointA;
		pointB = box_with_z * pointB;

		if(tempA < 0 && tempB < 0){
			drawline_n2[i] = false;
			n_counter2++;
			n_counter2++;
		}
		else if(tempA >= 0 && tempB >= 0){
			box_coord_after_clip[n_counter2] = pointA;
			n_counter2++;
			box_coord_after_clip[n_counter2] = pointB;
			n_counter2++;
		}
		else{
			float t = tempA / (tempA - tempB);
			vec4 new_point = pointA + t * (pointB - pointA);
			//new_point = box_with_z * new_point;
			if(tempA < 0){
				pointA = new_point;		
			}
			else{
				pointB = new_point;	
			}
			box_coord_after_clip[n_counter2] = pointA;
			n_counter2++;
			box_coord_after_clip[n_counter2] = pointB;
			n_counter2++;

		}

	}
	n_counter2 = 0;


   	int	n_counter3 = 0;
	for(int i = 0; i < 3; i++){
		drawline_n3[i]= true;
	}

	for(int i = 0; i < 3; i++){
//cout  << " a: " << link_arr3[i][0]  << " b: " << link_arr3[i][1] <<endl;
		vec4 pointA = world_coord_new[(link_arr3[i][0])];
		vec4 pointB = world_coord_new[(link_arr3[i][1])];
		float tempA = dot((pointA - plane_n_P),
					       normal_n4);
		float tempB = dot((pointB - plane_n_P),
					       normal_n4);
		pointA = box_with_z * pointA;
		pointB = box_with_z * pointB;

		if(tempA < 0 && tempB < 0){
			drawline_n3[i] = false;
			n_counter3++;
			n_counter3++;
		}
		else if(tempA >= 0 && tempB >= 0){
			world_coord_after_clip[n_counter3] = pointA;
			n_counter3++;
			world_coord_after_clip[n_counter3] = pointB;
			n_counter3++;
		}
		else{
			float t = tempA / (tempA - tempB);
			vec4 new_point = pointA + t * (pointB - pointA);
			//new_point = box_with_z * new_point;
			if(tempA < 0){
				pointA = new_point;		
			}
			else{
				pointB = new_point;	
			}
			world_coord_after_clip[n_counter3] = pointA;
			n_counter3++;
			world_coord_after_clip[n_counter3] = pointB;
			n_counter3++;

		}

	}
	n_counter3 = 0;

	clip_side(plane_f_P, normal_f4);
	clip_box_coord(plane_f_P, normal_f4);
	clip_world_coord(plane_f_P, normal_f4);


	for(int i = 0; i < 24; i++){
			box_points_after_clip[i][0] = box_points_after_clip[i][0]/box_points_after_clip[i][3];
			box_points_after_clip[i][1] = box_points_after_clip[i][1]/box_points_after_clip[i][3];
			box_points_after_clip[i][2] = box_points_after_clip[i][2]/box_points_after_clip[i][3];
			box_points_after_clip[i][3] = box_points_after_clip[i][3]/box_points_after_clip[i][3];
	}
	for(int i = 0; i < 6; i++){
			box_coord_after_clip[i][0] = box_coord_after_clip[i][0]/box_coord_after_clip[i][3];
			box_coord_after_clip[i][1] = box_coord_after_clip[i][1]/box_coord_after_clip[i][3];
			box_coord_after_clip[i][2] = box_coord_after_clip[i][2]/box_coord_after_clip[i][3];
			box_coord_after_clip[i][3] = box_coord_after_clip[i][3]/box_coord_after_clip[i][3];
	}
	for(int i = 0; i < 6; i++){
			world_coord_after_clip[i][0] = world_coord_after_clip[i][0]/world_coord_after_clip[i][3];
			world_coord_after_clip[i][1] = world_coord_after_clip[i][1]/world_coord_after_clip[i][3];
			world_coord_after_clip[i][2] = world_coord_after_clip[i][2]/world_coord_after_clip[i][3];
			world_coord_after_clip[i][3] = world_coord_after_clip[i][3]/world_coord_after_clip[i][3];
	}
	//cout << box_points_after_clip[0][0] << " " << box_points_after_clip[0][1] << " "<< box_points_after_clip[0][2] << endl;

	vec4 normal_nu4 = {0,1,0,0};
	vec4 plane_nu_P = {0,-1,0,1};
	clip_side(plane_nu_P, normal_nu4);
	clip_box_coord(plane_nu_P, normal_nu4);
	clip_world_coord(plane_nu_P, normal_nu4);

	vec4 normal_nd4 = {0,-1,0,0};
	vec4 plane_nd_P = {0,1,0,1};
	clip_side(plane_nd_P, normal_nd4);
	clip_box_coord(plane_nd_P, normal_nd4);
	clip_world_coord(plane_nd_P, normal_nd4);

	vec4 normal_nl4 = {1,0,0,0};
	vec4 plane_nl_P = {-1,0,0,1};
	clip_side(plane_nl_P, normal_nl4);
	clip_box_coord(plane_nl_P, normal_nl4);
	clip_world_coord(plane_nl_P, normal_nl4);

	vec4 normal_nr4 = {-1,0,0,0};
	vec4 plane_nr_P = {1,0,0,1};
	clip_side(plane_nr_P, normal_nr4);
	clip_box_coord(plane_nr_P, normal_nr4);
	clip_world_coord(plane_nr_P, normal_nr4);


	for(int i = 0; i < 24; i++){
		box_points_after_clip[i][0] = box_points_after_clip[i][0] * 
									  window_width_viewport / window_width + 
									  ( tempx_pos_end + tempx_pos_begin) / window_width / 2
									  ;
		box_points_after_clip[i][1] = box_points_after_clip[i][1] * 
									  window_height_viewport / window_height +
									  ( tempy_pos_end + tempy_pos_begin) / window_height /2; 									  									  									  
	}	
	for(int i = 0; i < 6; i++){
		box_coord_after_clip[i][0] = box_coord_after_clip[i][0] * 
									  window_width_viewport / window_width + 
									  ( tempx_pos_end + tempx_pos_begin) / window_width / 2
									  ;
		box_coord_after_clip[i][1] = box_coord_after_clip[i][1] * 
									  window_height_viewport / window_height +
									  ( tempy_pos_end + tempy_pos_begin) / window_height /2; 									  									  									  
	}
	for(int i = 0; i < 6; i++){
		world_coord_after_clip[i][0] = world_coord_after_clip[i][0] * 
									  window_width_viewport / window_width + 
									  ( tempx_pos_end + tempx_pos_begin) / window_width / 2
									  ;
		world_coord_after_clip[i][1] = world_coord_after_clip[i][1] * 
									  window_height_viewport / window_height +
									  ( tempy_pos_end + tempy_pos_begin) / window_height /2; 									  									  									  
	}		
	//cout << "Wx: " << tempx_pos_begin << " Hx: " << tempy_pos_begin <<endl;;

	//cout << "W: " << window_width_viewport << " H: " << window_height_viewport <<endl;;


	for(int i = 0; i < 24;){
		if(drawline_n[i/2] == true){
			drawLine(vec2(box_points_after_clip[i][0],
						  box_points_after_clip[i][1]),
					 vec2(box_points_after_clip[i+1][0],
					 	  box_points_after_clip[i+1][1]));
		}
		i++;
		i++;
	}

	setLineColour(vec3(1.0f, 0.0f, 0.0f));

	if(drawline_n2[0] == true){
		drawLine(vec2(box_coord_after_clip[0][0],
					  box_coord_after_clip[0][1]),
				 vec2(box_coord_after_clip[1][0],
				 	  box_coord_after_clip[1][1]));
	}
	if(drawline_n3[0] == true){	
		drawLine(vec2(world_coord_after_clip[0][0],
					  world_coord_after_clip[0][1]),
				 vec2(world_coord_after_clip[1][0],
				 	  world_coord_after_clip[1][1]));
	}

	setLineColour(vec3(0.0f, 1.0f, 0.0f));
	if(drawline_n2[1] == true){
		drawLine(vec2(box_coord_after_clip[2][0],
					  box_coord_after_clip[2][1]),
				 vec2(box_coord_after_clip[3][0],
				 	  box_coord_after_clip[3][1]));
	}
	if(drawline_n3[1] == true){	
		drawLine(vec2(world_coord_after_clip[2][0],
					  world_coord_after_clip[2][1]),
				 vec2(world_coord_after_clip[3][0],
				 	  world_coord_after_clip[3][1]));
	}

	setLineColour(vec3(0.0f, 0.0f, 1.0f));
	if(drawline_n2[2] == true){
		drawLine(vec2(box_coord_after_clip[4][0],
					  box_coord_after_clip[4][1]),
				 vec2(box_coord_after_clip[5][0],
				 	  box_coord_after_clip[5][1]));
	}
	if(drawline_n3[2] == true){	
		drawLine(vec2(world_coord_after_clip[4][0],
					  world_coord_after_clip[4][1]),
				 vec2(world_coord_after_clip[5][0],
				 	  world_coord_after_clip[5][1]));
	}

	setLineColour(vec3(0, 0, 0));


	drawLine(vec2(tempx_pos_begin/window_width, tempy_pos_begin/window_height),
			 vec2(tempx_pos_begin/window_width, tempy_pos_end/window_height));
	drawLine(vec2(tempx_pos_begin/window_width, tempy_pos_begin/window_height),
			 vec2(tempx_pos_end/window_width, tempy_pos_begin/window_height));
	drawLine(vec2(tempx_pos_end/window_width, tempy_pos_begin/window_height),
			 vec2(tempx_pos_end/window_width, tempy_pos_end/window_height));
	drawLine(vec2(tempx_pos_begin/window_width, tempy_pos_end/window_height),
			 vec2(tempx_pos_end/window_width, tempy_pos_end/window_height));

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}


		ImGui::PushID( 0 );
		if( ImGui::RadioButton( "Rotate View", &cur_mode, 0 ) ) {

		}
		ImGui::PopID();
		ImGui::PushID( 1 );				
		if( ImGui::RadioButton( "Translate View", &cur_mode, 1 ) ) {

		}
		ImGui::PopID();
		ImGui::PushID( 2 );			
		if( ImGui::RadioButton( "Perspective", &cur_mode, 2 ) ) {

		}
		ImGui::PopID();
		ImGui::PushID( 3 );			
		if( ImGui::RadioButton( "Rotate Model", &cur_mode, 3 ) ) {

		}
		ImGui::PopID();
		ImGui::PushID( 4 );			
		if( ImGui::RadioButton( "Translate Model", &cur_mode, 4 ) ) {

		}
		ImGui::PopID();
		ImGui::PushID( 5 );			
		if( ImGui::RadioButton( "Scale Model", &cur_mode, 5 ) ) {

		}
		ImGui::PopID();
		ImGui::PushID( 6 );			
		if( ImGui::RadioButton( "Viewport", &cur_mode, 6 ) ) {

		}			
		ImGui::PopID();
		if( ImGui::Button( "Reset" ) ) {
			reset();
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );
		ImGui::Text( "Near Plane: %.1f", n );
		ImGui::Text( "Far Plane: %.1f", f );


	ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

	//-- Copy vertex position data into VBO, m_vbo_positions:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
				m_vertexData.positions.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}

	//-- Copy vertex colour data into VBO, m_vbo_colours:
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
				m_vertexData.colours.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
	uploadVertexDataToVbos();

	glBindVertexArray(m_vao);

	m_shader.enable();
		glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
	m_shader.disable();

	// Restore defaults
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if(cur_mode == R_Rotate_Model){
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_L_hold) {

			if(pos_x == 0){
				pos_x = xPos;
			}

			if(pos_x > xPos){
					angle_box_y -= 0.015*(pos_x - xPos)/2;
				pos_x = xPos;
			}
			else if (pos_x < xPos){
					angle_box_y += 0.015*( xPos - pos_x)/2;
				pos_x = xPos;
			}

			mat4 tamp1 = {glm::vec4( cos(angle_box_y), 0, sin(angle_box_y), 0),
						       glm::vec4(0, 1, 0, 0),
						       glm::vec4(-sin(angle_box_y), 0, cos(angle_box_y), 0),
						       glm::vec4(0,0,0,1)};
			angle_box_y = 0;			   
			box_rot_sum = box_rot_sum * tamp1;			       
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_M_hold) {

			if(pos_y == 0){
				pos_y = yPos;
			}

			if(pos_y > yPos){
					angle_box_x -= 0.015*(pos_y - yPos)/2;
				pos_y = yPos;
			}
			else if (pos_y < yPos){
					angle_box_x += 0.015*( yPos - pos_y)/2;
				pos_y = yPos;
			}
		
			mat4 tamp2 = {glm::vec4(1,0,0,0),
						 glm::vec4(0,cos(angle_box_x),-sin(angle_box_x),0),
						 glm::vec4(0,sin(angle_box_x), cos(angle_box_x),0),
						 glm::vec4(0,0,0,1)};
			angle_box_x = 0;			 
			box_rot_sum = box_rot_sum * tamp2;			 
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_R_hold) {

			if(pos_z == 0){
				pos_z = xPos;
			}

			if(pos_z > xPos){
					angle_box_z -= 0.015*(pos_z - xPos)/2;
				pos_z = xPos;
			}
			else if (pos_z < xPos){
					angle_box_z += 0.015*( xPos - pos_z)/2;
				pos_z = xPos;
			}

		    mat4 tamp3 = {glm::vec4(cos(angle_box_z), -sin(angle_box_z),0,0),
						       glm::vec4(sin(angle_box_z),  cos(angle_box_z),0,0),
						       glm::vec4(0,0,1,0),
						       glm::vec4(0,0,0,1)};
			angle_box_z = 0;			       
			box_rot_sum = box_rot_sum * tamp3;			       
		}
	}
	if(cur_mode == T_Translate_Model){
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_L_hold) {

			if(pos_x == 0){
				pos_x = xPos;
			}

			if(pos_x > xPos){
				if(trans_box_x > -20){
					trans_box_x += 0.015*(pos_x - xPos)/2;
				}
				pos_x = xPos;
			}
			else if (pos_x < xPos){
				if(trans_box_x < 20 ){
					trans_box_x -= 0.015*( xPos - pos_x)/2;
				}
				pos_x = xPos;
			}
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_M_hold) {

			if(pos_y == 0){
				pos_y = yPos;
			}

			if(pos_y > yPos){
				if(trans_box_y > -20){
					trans_box_y -= 0.015*(pos_y - yPos)/2;
				}
				pos_y = yPos;
			}
			else if (pos_y < yPos){
				if(trans_box_y < 20){
					trans_box_y += 0.015*( yPos - pos_y)/2;
				}
				pos_y = yPos;
			}
			//cout << cos(angle_box_x) <<endl;
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_R_hold) {

			if(pos_z == 0){
				pos_z = xPos;
			}

			if(pos_z > xPos){
				if(trans_box_z > -20){
					trans_box_z -= 0.015*(pos_z - xPos)/2;
				}
				pos_z = xPos;
			}
			else if (pos_z < xPos){
				if(trans_box_z < 20){
					trans_box_z += 0.015*( xPos - pos_z)/2;
				}
				pos_z = xPos;
			}
		}
		vec4 tamp = box_rot_sum * vec4(trans_box_x,trans_box_y,trans_box_z,1);
		mat4 tamp2 = {vec4(1,0,0,0),
					  vec4(0,1,0,0),
					  vec4(0,0,1,0),
					  tamp}	;
		trans_box_x = 0;
		trans_box_y = 0;
		trans_box_z = 0;	         
		box_trans = box_trans * tamp2;	         
	}
	if(cur_mode == S_Scale_Model){
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_L_hold) {

			if(pos_x == 0){
				pos_x = xPos;
			}

			if(pos_x > xPos){
				if(sca_box_x > 0){
					sca_box_x -= 0.01 * (pos_x - xPos) / 4;
				}
				pos_x = xPos;
			}
			else if (pos_x < xPos){
				if(sca_box_x < 3 ){
					sca_box_x += 0.01 * ( xPos - pos_x) / 4;
				}
				pos_x = xPos;
			}
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_M_hold) {

			if(pos_y == 0){
				pos_y = yPos;
			}

			if(pos_y > yPos){
				if(sca_box_y > 0){
					sca_box_y -= 0.01*(pos_y - yPos)/4;
				}
				pos_y = yPos;
			}
			else if (pos_y < yPos){
				if(sca_box_y < 3){
					sca_box_y += 0.01*( yPos - pos_y)/4;
				}
				pos_y = yPos;
			}
			//cout << cos(angle_box_x) <<endl;
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_R_hold) {

			if(pos_z == 0 ){
				pos_z = xPos;
			}

			if(pos_z > xPos){
				if(sca_box_z > 0){
					sca_box_z -= 0.01*(pos_z - xPos)/4;
				}
				pos_z = xPos;
			}
			else if (pos_z < xPos){
				if(sca_box_z < 3){
					sca_box_z += 0.01*( xPos - pos_z)/4;
				}
				pos_z = xPos;
			}
		}

		mat4 tamp = {glm::vec4(sca_box_x,0,0,0),
			       glm::vec4(0,sca_box_y,0,0),
			       glm::vec4(0,0,sca_box_z,0),
			       glm::vec4(0,0,0,1)};
		sca_box_x = 1;
		sca_box_y = 1;
		sca_box_z = 1;	       
		box_sca = box_sca * tamp;
	}
	if(cur_mode == O_Rotate_View){
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_L_hold) {

			if(pos_x == 0){
				pos_x = yPos;
			}

			if(pos_x > yPos){
					angle_view_x -= 0.01*(pos_x - yPos)/5;
				pos_x = yPos;
			}
			else if (pos_x < yPos){
					angle_view_x += 0.01*( yPos - pos_x)/5;
				pos_x = yPos;
			}
			mat4 tamp1 = {glm::vec4(1,0,0,0),
						 glm::vec4(0,cos(angle_view_x),-sin(angle_view_x),0),
						 glm::vec4(0,sin(angle_view_x), cos(angle_view_x),0),
						 glm::vec4(0,0,0,1)};
			angle_view_x = 0;			 
			eye_rot_sum = eye_rot_sum * tamp1;			 

		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_M_hold) {

			if(pos_y == 0){
				pos_y = xPos;
			}

			if(pos_y > xPos){
					angle_view_y -= 0.01*(pos_y - xPos)/5;
				pos_y = xPos;
			}
			else if (pos_y < xPos){
					angle_view_y += 0.01*( xPos - pos_y)/5;
				pos_y = xPos;
			}
			mat4 tamp2 = {glm::vec4( cos(angle_view_y), 0, sin(angle_view_y), 0),
						       glm::vec4(0, 1, 0, 0),
						       glm::vec4(-sin(angle_view_y), 0, cos(angle_view_y), 0),
						       glm::vec4(0,0,0,1)};	
			angle_view_y = 0;			       
			eye_rot_sum = eye_rot_sum * tamp2;

		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_R_hold) {

			if(pos_z == 0){
				pos_z = xPos;
			}

			if(pos_z > xPos){
					angle_view_z -= 0.01*(pos_z - xPos)/5;
				pos_z = xPos;
			}
			else if (pos_z < xPos){
					angle_view_z += 0.01*( xPos - pos_z)/5;
				pos_z = xPos;
			}

		    mat4 tamp3 = {glm::vec4(cos(angle_view_z), -sin(angle_view_z),0,0),
						       glm::vec4(sin(angle_view_z),  cos(angle_view_z),0,0),
						       glm::vec4(0,0,1,0),
						       glm::vec4(0,0,0,1)};
			angle_view_z = 0;			       
			eye_rot_sum = eye_rot_sum * tamp3;
		}
	}
	if(cur_mode == N_Translate_View){
		trans_view_x = 0;
		trans_view_y = 0;
		trans_view_z = 0;
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_L_hold) {

			if(pos_x == 0){
				pos_x = xPos;
			}

			if(pos_x < xPos){
				if(trans_view_x < 20){
					trans_view_x += 0.015*(pos_x - xPos)/2;
				}
				pos_x = xPos;
			}
			else if (pos_x > xPos){
				if(trans_view_x > -20){
					trans_view_x -= 0.015*( xPos - pos_x)/2;
				}
				pos_x = xPos;
			}
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_M_hold) {

			if(pos_y == 0){
				pos_y = yPos;
			}

			if(pos_y < yPos){
				if(trans_view_y > -20){
					trans_view_y -= 0.015*(pos_y - yPos)/2;
				}
				pos_y = yPos;
			}
			else if (pos_y > yPos){
				if(trans_view_y < 20){
					trans_view_y += 0.015*( yPos - pos_y)/2;
				}
				pos_y = yPos;
			}
			//cout << cos(angle_box_x) <<endl;
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_R_hold) {

			if(pos_z == 0){
				pos_z = xPos;
			}

			if(pos_z < xPos){
				if(trans_view_z > -20){
					trans_view_z -= 0.015*(pos_z - xPos)/2;
				}
				pos_z = xPos;
			}
			else if (pos_z > xPos){
				if(trans_view_z < 20){
					trans_view_z += 0.015*( xPos - pos_z)/2;
				}
				pos_z = xPos;
			}
		}
  		//print_mat(box_trans);

		vec4 tamp = eye_rot_sum * vec4(trans_view_x,trans_view_y,trans_view_z,1);
		mat4 tamp2 = {vec4(1,0,0,0),
					  vec4(0,1,0,0),
					  vec4(0,0,1,0),
					  tamp}	;

		trans_view_x = 0;
		trans_view_y = 0;
		trans_view_z = 0;
		eye_trans = eye_trans * tamp2;	         
	}
	if(cur_mode == P_Perspective){
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_L_hold) {

			if(pos_x == 0){
				pos_x = xPos;
			}

			if(pos_x > xPos){
				if(fov > M_PI/72){
					fov -= 0.015*(pos_x - xPos)/2;
				}
				pos_x = xPos;
			}
			else if (pos_x < xPos){
				if(fov < M_PI*80/180 ){
					fov += 0.015*( xPos - pos_x)/2;
				}
				pos_x = xPos;
			}
		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_M_hold) {

			if(pos_y == 0){
				pos_y = xPos;
			}

			if(pos_y > xPos){
				if(n > 0){
					n -= 0.015*(pos_y - xPos)/2;
				}
				pos_y = xPos;
			}
			else if (pos_y < xPos){
				if(n < f - 0.015*( xPos - pos_y)/2){
					n += 0.015*( xPos - pos_y)/2;
				}
				pos_y = xPos;
			}
							//cout << n <<endl;

		}
		if (!ImGui::IsMouseHoveringAnyWindow() && mouse_R_hold) {

			if(pos_z == 0){
				pos_z = xPos;
			}

			if(pos_z > xPos){
				if(f > n + 0.015*(pos_z - xPos)/2){
					f -= 0.015*(pos_z - xPos)/2;
				}
				pos_z = xPos;
			}
			else if (pos_z < xPos){
				if(f < 50){
					f += 0.015*(xPos - pos_z)/2;
				}
				pos_z = xPos;
			}
							//cout << f <<endl;

		}
  		//print_mat(box_trans);
		box_with_z = {glm::vec4(1/(tan(fov)*ratio),0,0,0),
					  glm::vec4(0,1/tan(fov),0,0),
					  glm::vec4(0,0,(f+n)/(f-n),-1),
					  glm::vec4(0,0,-2*f*n/(f-n),0)};		  
	}
	if(cur_mode == V_Viewport){

		//cout << "X: " << xPos << " Y: " << yPos << endl;

	

		if(mouse_L_hold && lock_xy_start && !lock_xy_end){
			tempx_pos_begin = ((float)xPos - window_width) * 2 + window_width;
			tempy_pos_begin = -(((float)yPos - window_height) * 2 + window_height);
			lock_xy_start = false;
			lock_xy_end = true;
			//cout << "!x: " << tempx_pos_begin << " !y: " <<tempy_pos_begin <<endl;

		}
		if(mouse_L_hold && !lock_xy_start && lock_xy_end){
			tempx_pos_end = ((float)xPos - window_width) * 2 + window_width;
			tempy_pos_end = -(((float)yPos - window_height) * 2 + window_height);
			if(tempx_pos_end > window_width){
				tempx_pos_end = window_width;
			}
			else if(tempx_pos_end < -window_width){
				tempx_pos_end = -window_width;
			}
			if(tempy_pos_end > window_height){
				tempy_pos_end = window_height;
			}
			else if(tempy_pos_end < -window_height){
				tempy_pos_end = -window_height;
			}
			window_width_viewport = abs(tempx_pos_begin - tempx_pos_end) / 2;
			window_height_viewport = abs(tempy_pos_begin - tempy_pos_end) / 2;
			//cout << "x: " << window_width_viewport << " y: " <<window_height_viewport <<endl;
		}
		if(!mouse_L_hold && !lock_xy_start && lock_xy_end){
			lock_xy_start = true;
			lock_xy_end = false;
			//cout << "x: " << window_width_viewport << " y: " <<window_height_viewport <<endl;
		}
	}


			//vec2(tempx_pos_begin/window_width, tempy_pos_begin/window_height),
			// vec2(tempx_pos_begin/window_width, tempy_pos_end/window_height)






	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (!ImGui::IsMouseHoveringAnyWindow()) {
		// The user clicked in the window.  If it's the left
		// mouse button, initiate a rotation.
		if (actions == GLFW_PRESS){
			if(button == GLFW_MOUSE_BUTTON_LEFT) {
		        mouse_L_hold = 1;
		        //cout << "111" << endl;
		    }
			if(button == GLFW_MOUSE_BUTTON_RIGHT) {
		        mouse_R_hold = 1;
		        //cout << "222" << endl;
		    }
			if(button == GLFW_MOUSE_BUTTON_MIDDLE) {
		        mouse_M_hold = 1;
		        //cout << "333" << endl;
		    }		    
		}
	    else if(actions == GLFW_RELEASE){
			if(button == GLFW_MOUSE_BUTTON_LEFT) {
		        mouse_L_hold = 0;
		    }
		    if(button == GLFW_MOUSE_BUTTON_RIGHT) {
		        mouse_R_hold = 0;
		    }
		    if(button == GLFW_MOUSE_BUTTON_MIDDLE) {
		        mouse_M_hold = 0;
		    }
		    pos_x = 0;
			pos_y = 0;
			pos_z = 0;
	    }
	}


	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);

	window_width = width;
	window_height= height;	
	// Fill in with event handling code...

	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);
	
	if (key == GLFW_KEY_Q) {
		glfwSetWindowShouldClose(m_window, GL_TRUE);
	}
	if (key == GLFW_KEY_A) {
		reset();
	}	
	if (key == GLFW_KEY_O) {
		cur_mode = 0;
	}
	if (key == GLFW_KEY_N) {
		cur_mode = 1;
	}
	if (key == GLFW_KEY_P) {
		cur_mode = 2;
		eventHandled = true;
	}
	if (key == GLFW_KEY_R) {
		cur_mode = 3;
	}
	if (key == GLFW_KEY_T) {
		cur_mode = 4;
	}
	if (key == GLFW_KEY_S) {
		cur_mode = 5;
	}
	if (key == GLFW_KEY_V) {
		cur_mode = 6;
	}
		
	return eventHandled;
}

void A2::print_mat(mat4 mat123){
	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 4; j++){
			cout << mat123[i][j] << " ";
		}
		cout <<endl;
	}
}

void A2::clip_side(vec4 point, vec4 unit_vector){
		for(int i = 0; i < 24;){
//cout << P_A_x << " " << P_A_y  << " " << P_B_x << " " << P_B_y<<endl;
		vec4 pointA = box_points_after_clip[i];
		vec4 pointB = box_points_after_clip[i+1];

		float tempA = dot((pointA - point),
					       unit_vector);
		float tempB = dot((pointB - point),
					       unit_vector);

		if(tempA < 0 && tempB < 0){
			drawline_n[i/2] = false;
		}
		else if(tempA >= 0 && tempB >= 0){

			box_points_after_clip[i] = pointA;
			box_points_after_clip[i+1] = pointB;
		}
		else{

			float t = tempA / (tempA - tempB);
			vec4 new_point = pointA + t * (pointB - pointA);
			//new_point = box_with_z * new_point;
			if(tempA < 0){
				pointA = new_point;		
			}
			else{
				pointB = new_point;	
			}
			box_points_after_clip[i] = pointA;
			box_points_after_clip[i+1] = pointB;
		}
		i += 2;
	}
}

void A2::clip_box_coord(vec4 point, vec4 unit_vector){
		for(int i = 0; i < 6;){
//cout << P_A_x << " " << P_A_y  << " " << P_B_x << " " << P_B_y<<endl;
		vec4 pointA = box_coord_after_clip[i];
		vec4 pointB = box_coord_after_clip[i+1];

		float tempA = dot((pointA - point),
					       unit_vector);
		float tempB = dot((pointB - point),
					       unit_vector);

		if(tempA < 0 && tempB < 0){
			drawline_n2[i/2] = false;
		}
		else if(tempA >= 0 && tempB >= 0){

			box_coord_after_clip[i] = pointA;
			box_coord_after_clip[i+1] = pointB;
		}
		else{

			float t = tempA / (tempA - tempB);
			vec4 new_point = pointA + t * (pointB - pointA);
			//new_point = box_with_z * new_point;
			if(tempA < 0){
				pointA = new_point;		
			}
			else{
				pointB = new_point;	
			}
			box_coord_after_clip[i] = pointA;
			box_coord_after_clip[i+1] = pointB;
		}
		i += 2;
	}
}

void A2::clip_world_coord(vec4 point, vec4 unit_vector){
		for(int i = 0; i < 6;){
//cout << P_A_x << " " << P_A_y  << " " << P_B_x << " " << P_B_y<<endl;
		vec4 pointA = world_coord_after_clip[i];
		vec4 pointB = world_coord_after_clip[i+1];

		float tempA = dot((pointA - point),
					       unit_vector);
		float tempB = dot((pointB - point),
					       unit_vector);

		if(tempA < 0 && tempB < 0){
			drawline_n3[i/2] = false;
		}
		else if(tempA >= 0 && tempB >= 0){

			world_coord_after_clip[i] = pointA;
			world_coord_after_clip[i+1] = pointB;
		}
		else{

			float t = tempA / (tempA - tempB);
			vec4 new_point = pointA + t * (pointB - pointA);
			//new_point = box_with_z * new_point;
			if(tempA < 0){
				pointA = new_point;		
			}
			else{
				pointB = new_point;	
			}
			world_coord_after_clip[i] = pointA;
			world_coord_after_clip[i+1] = pointB;
		}
		i += 2;
	}
}




void A2::point_div_z(vec4 point){
	point[0] = point[0]/point[3];
	point[1] = point[1]/point[3];
	point[2] = point[2]/point[3];
	point[3] = point[3]/point[3];	
}

void A2::reset(){
	cur_mode = 3;
	mouse_L_hold = 0;
	mouse_M_hold = 0;
	mouse_R_hold = 0;	
	window_width_viewport = window_width * 0.9f;
	window_height_viewport = window_height * 0.9f;	
	lock_xy_start = true;
	lock_xy_end = false;
	tempx_pos_begin = -window_width * 0.9f;
	tempy_pos_begin = -window_height * 0.9f;
	tempx_pos_end = window_width * 0.9f;
	tempy_pos_end = window_height * 0.9f;
	eye_rot_sum = mat4(1);
	box_rot_sum = mat4(1);
	box_points[0] = vec4(1,1,1,1);
	box_points[1] = vec4(1,1,-1,1);
	box_points[2] = vec4(1,-1,1,1);
	box_points[3] = vec4(1,-1,-1,1);
	box_points[4] = vec4(-1,1,1,1);
	box_points[5] = vec4(-1,1,-1,1);
	box_points[6] = vec4(-1,-1,1,1);
	box_points[7] = vec4(-1,-1,-1,1);
	box_points_new[0] = vec4(1,1,1,1);
	box_points_new[1] = vec4(1,1,-1,1);
	box_points_new[2] = vec4(1,-1,1,1);
	box_points_new[3] = vec4(1,-1,-1,1);
	box_points_new[4] = vec4(-1,1,1,1);
	box_points_new[5] = vec4(-1,1,-1,1);
	box_points_new[6] = vec4(-1,-1,1,1);
	box_points_new[7] = vec4(-1,-1,-1,1);


	box_coord[0] = vec4(0,0,0,1);
	box_coord[1] = vec4(-0.3,0,0,1);
	box_coord[2] = vec4(0,-0.3,0,1);
	box_coord[3] = vec4(0,0,-0.3,1);
	box_coord_new[0] = vec4(0,0,0,1);
	box_coord_new[1] = vec4(-0.3,0,0,1);
	box_coord_new[2] = vec4(0,-0.3,0,1);
	box_coord_new[3] = vec4(0,0,-0.3,1);
	world_coord[0] = vec4(0,0,0,1);
	world_coord[1] = vec4(-0.3,0,0,1);
	world_coord[2] = vec4(0,-0.3,0,1);
	world_coord[3] = vec4(0,0,-0.3,1);
	world_coord_new[0] = vec4(0,0,0,1);
	world_coord_new[1] = vec4(-0.3,0,0,1);
	world_coord_new[2] = vec4(0,-0.3,0,1);
	world_coord_new[3] = vec4(0,0,-0.3,1);

	box_view = mat4(1);
	box_rot_x = mat4(1);
	box_rot_y = mat4(1);
	box_rot_z = mat4(1);

	trans_box_x = 0.0f;
	trans_box_y = 0.0f;
	trans_box_z = 0.0f;	
	box_trans = mat4(1);

	sca_box_x = 1.0f;
	sca_box_y = 1.0f;
	sca_box_z = 1.0f;

	box_sca	 = mat4(1);

	eye_rot_x = mat4(1);
	eye_rot_y = mat4(1);
	eye_rot_z = mat4(1);

	trans_view_x = 0.0f;
	trans_view_y = 0.0f;
	trans_view_z = -5.0f;
	eye_trans = {glm::vec4(1,0,0,0),
					       glm::vec4(0,1,0,0),
					       glm::vec4(0,0,1,0),
					       glm::vec4(trans_view_x,trans_view_y,trans_view_z,1)};



	n = 3.0f;
	f = 7.0f;
	ratio = 1.0f;
	fov = M_PI/6.0f;

	box_with_z = {glm::vec4(1/(tan(fov)/ratio),0,0,0),
					      	glm::vec4(0,1/tan(fov),0,0),
					      	glm::vec4(0,0,(f+n)/(f-n),-1),
					      	glm::vec4(0,0,-2*f*n/(f-n),0)};	
}