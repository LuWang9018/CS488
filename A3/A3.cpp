#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"
//#include "OGLwin.h"
#include <math.h>
#include <vector>
#include <imgui/imgui.h>
#include <stdio.h>
#include <stdlib.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

short sXReference, sYReference;

int nCurrentDir = DIR_NONE;
static Matrix mRotations    = {{1.0, 0.0, 0.0, 0.0},
                               {0.0, 1.0, 0.0, 0.0},
                               {0.0, 0.0, 1.0, 0.0},
                               {0.0, 0.0, 0.0, 1.0}};
static Matrix mRotations_after    = {{1.0, 0.0, 0.0, 0.0},
	                                 {0.0, 1.0, 0.0, 0.0},
	                                 {0.0, 0.0, 1.0, 0.0},
	                                 {0.0, 0.0, 0.0, 1.0}};                               
static Matrix mTranslations = {{1.0, 0.0, 0.0, 0.0},
                               {0.0, 1.0, 0.0, 0.0},
                               {0.0, 0.0, 1.0, 0.0},
                               {0.0, 0.0, 0.0, 1.0}};
static Matrix mTranslations_after = {{1.0, 0.0, 0.0, 0.0},
                                     {0.0, 1.0, 0.0, 0.0},
                                     {0.0, 0.0, 1.0, 0.0},
                                     {0.0, 0.0, 0.0, 1.0}};                               
Matrix mIdentity     = {{1.0, 0.0, 0.0, 0.0},
                        {0.0, 1.0, 0.0, 0.0},
                        {0.0, 0.0, 1.0, 0.0},
                        {0.0, 0.0, 0.0, 1.0}};

static GLfloat fYellowVec[] = {1.0, 1.0, 0.0};
static GLfloat fWhiteVec[]  = {1.0, 1.0, 1.0};
static GLfloat fBlackVec[]  = {0.0, 0.0, 0.0};
static GLfloat fGreenVec[]  = {0.0, 1.0, 0.0};
static GLfloat fBlueVec[]   = {0.0, 0.0, 1.0};
static GLfloat fGrayVec[]   = {0.5, 0.6, 0.5};
static GLfloat fRedVec[]    = {1.0, 0.0, 0.0};


static bool show_gui = true;
int window_width = 1024;
int window_height = 768;
int mouse_x;
int mouse_y;
const size_t CIRCLE_PTS = 48;
bool mouse_L_hold = 0;
bool mouse_M_hold = 0;
bool mouse_R_hold = 0;
double pos_x = 0;
double pos_y = 0;
double pos_z = 0;
float cur_x = 0;
float cur_y = 0;
float cur_z = 0 ;
int picking_mode = 0;
int current_mode = 0;
bool lock_xy_start = true;
bool lock_xy_end = false;
float tempx_pos_begin = 0;
float tempy_pos_begin = 0;
float tempx_pos_end = 0;
float tempy_pos_end = 0;
vector<bool> node_seleced;
int current_pointed_node = -1;
//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
	: m_luaSceneFile(luaSceneFile),
	  m_positionAttribLocation(0),
	  m_normalAttribLocation(0),
	  m_vao_meshData(0),
	  m_vbo_vertexPositions(0),
	  m_vbo_vertexNormals(0),
	  m_vao_arcCircle(0),
	  m_vbo_arcCircle(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
	// Set the background colour.
	glClearColor(0.35, 0.35, 0.35, 1.0);

	createShaderProgram();

	glGenVertexArrays(1, &m_vao_arcCircle);
	glGenVertexArrays(1, &m_vao_meshData);
	enableVertexShaderInputSlots();

	processLuaSceneFile(m_luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			getAssetFilePath("cube.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("suzanne.obj"),
			getAssetFilePath("head.obj"),
			getAssetFilePath("face_left.obj"),
			getAssetFilePath("face_right.obj"),			
			getAssetFilePath("body.obj"),
			getAssetFilePath("body_up.obj"),			
			getAssetFilePath("arm_left.obj"),
			getAssetFilePath("arm_right.obj"),			
			getAssetFilePath("leg_left.obj"),
			getAssetFilePath("leg_right.obj"),			
			getAssetFilePath("tail.obj"),
			getAssetFilePath("ear_left_out.obj"),
			getAssetFilePath("ear_left_in.obj"),
			getAssetFilePath("ear_right_out.obj"),
			getAssetFilePath("ear_right_in.obj"),
			getAssetFilePath("eye_left_out.obj"),
			getAssetFilePath("eye_left_in.obj"),
			getAssetFilePath("eye_right_out.obj"),
			getAssetFilePath("eye_right_in.obj"),			
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(m_batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);

	mapVboDataToVertexShaderInputLocations();

	initPerspectiveMatrix();

	initViewMatrix();

	initLightSources();


	// Exiting the current scope calls delete automatically on meshConsolidator freeing
	// all vertex data resources.  This is fine since we already copied this data to
	// VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
	// this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	// std::string assetFilePath = getAssetFilePath(filename.c_str());
	// m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!m_rootNode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
	for (int i = 0; i < (*m_rootNode).totalSceneNodes(); i++){
		node_seleced.push_back(0);
	}
}

//----------------------------------------------------------------------------------------
void A3::createShaderProgram()
{
	m_shader.generateProgramObject();
	m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
	m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
	m_shader.link();

	m_shader_arcCircle.generateProgramObject();
	m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
	m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
	m_shader_arcCircle.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
{
	//-- Enable input slots for m_vao_meshData:
	{
		glBindVertexArray(m_vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_positionAttribLocation = m_shader.getAttribLocation("position");
		glEnableVertexAttribArray(m_positionAttribLocation);

		// Enable the vertex shader attribute location for "normal" when rendering.
		m_normalAttribLocation = m_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(m_normalAttribLocation);

		CHECK_GL_ERRORS;
	}


	//-- Enable input slots for m_vao_arcCircle:
	{
		glBindVertexArray(m_vao_arcCircle);

		// Enable the vertex shader attribute location for "position" when rendering.
		m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
		glEnableVertexAttribArray(m_arc_positionAttribLocation);

		CHECK_GL_ERRORS;
	}

	// Restore defaults
	glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	//for picking 


	// Generate VBO to store all vertex position data
	{
		glGenBuffers(1, &m_vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
				meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store all vertex normal data
	{
		glGenBuffers(1, &m_vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
				meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}

	// Generate VBO to store the trackball circle.
	{
		glGenBuffers( 1, &m_vbo_arcCircle );
		glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

		float *pts = new float[ 2 * CIRCLE_PTS ];
		for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
			float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
			pts[2*idx] = cos( ang );
			pts[2*idx+1] = sin( ang );
		}

		glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
{
	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_meshData);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
	glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
	// "normal" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
	glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;

	// Bind VAO in order to record the data mapping.
	glBindVertexArray(m_vao_arcCircle);

	// Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
	// "position" vertex attribute location for any bound vertex shader program.
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
	glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//-- Unbind target, and restore default values:
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}


//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
	m_view = glm::lookAt(vec3(0.0f, 0.0f, -2.0f), vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
	// World-space position
	m_light.position = vec3(-2.0f, 5.0f, 0.5f);
	m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
	m_shader.enable();
	{
		//-- Set Perpsective matrix uniform for the scene:
		GLint location = m_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = m_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(m_light.position));
			location = m_shader.getUniformLocation("light.rgbIntensity");
			glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
			CHECK_GL_ERRORS;
		}

		//-- Set background light ambient intensity
		{
			location = m_shader.getUniformLocation("ambientIntensity");
			vec3 ambientIntensity(0.05f);
			glUniform3fv(location, 1, value_ptr(ambientIntensity));
			CHECK_GL_ERRORS;
		}
	}
	m_shader.disable();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
	// Place per frame, application logic here ...

	uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);
	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
	float opacity(0.5f);



	ImGui::BeginMainMenuBar();

	if (ImGui::BeginMenu("Application"))
        {
        	ImGui::MenuItem("Reset Position", NULL);
        	ImGui::MenuItem("Reset Orientation", NULL);
        	ImGui::MenuItem("Reset Joints", NULL);
        	ImGui::MenuItem("Reset Joints", NULL);
        	ImGui::MenuItem("Quit", NULL);
			ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
        	ImGui::MenuItem("Undo", NULL);
        	ImGui::MenuItem("Redo", NULL);
			ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Options"))
        {
        	ImGui::MenuItem("Circle", NULL);
        	ImGui::MenuItem("Z-buffer", NULL);
        	ImGui::MenuItem("Backface culling", NULL);
        	ImGui::MenuItem("Frontface culling", NULL);
        	if(ImGui::MenuItem("Position/Orientation", NULL)){
        		current_mode = 1; 
        		cout << "current mode : " << current_mode <<endl;   
        	}     	
        	if(ImGui::MenuItem("Joints", NULL)){ 
        		current_mode = 2; 
        		cout << "current mode : " << current_mode <<endl;   

        	} 
			ImGui::EndMenu();
        }

	ImGui::EndMainMenuBar();



	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);


		// Add more gui elements here here ...


		// Create Button, and check if it was clicked:
		if( ImGui::Button( "Quit Application" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
		}

		ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & viewMatrix
) {

	shader.enable();
	{
		//-- Set ModelView matrix:
		GLint location = shader.getUniformLocation("ModelView");
		mat4 modelView = viewMatrix * node.trans;
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
		CHECK_GL_ERRORS;

		//-- Set NormMatrix:
		location = shader.getUniformLocation("NormalMatrix");
		mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
		glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
		CHECK_GL_ERRORS;






		//-- Set Material values:
		location = shader.getUniformLocation("material.kd");
		vec3 kd = node.material.kd;
		glUniform3fv(location, 1, value_ptr(kd));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.ks");
		int temp_node_num = node.m_nodeId;
		vec3 ks;
		if(node_seleced[temp_node_num] == 0){
			ks = node.material.ks;
		}
		else{
			ks = vec3(1) + node.material.ks;
		}
		glUniform3fv(location, 1, value_ptr(ks));
		CHECK_GL_ERRORS;
		location = shader.getUniformLocation("material.shininess");
		glUniform1f(location, node.material.shininess);
		CHECK_GL_ERRORS;


		//ADDED
		location = shader.getUniformLocation("picking_mode");
		glUniform1i(location, picking_mode);
		CHECK_GL_ERRORS;

		//picking_mode = 0;
		location = shader.getUniformLocation("color");
		glUniform1f(location, node.m_nodeId/255.0);
		CHECK_GL_ERRORS;
	}
	shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {



	picking_mode = 0;
	glEnable( GL_DEPTH_TEST );
	renderSceneGraph(*m_rootNode);


	glDisable( GL_DEPTH_TEST );
	renderArcCircle();
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	glBindVertexArray(m_vao_meshData);

	// This is emphatically *not* how you should be drawing the scene graph in
	// your final implementation.  This is a non-hierarchical demonstration
	// in which we assume that there is a list of GeometryNodes living directly
	// underneath the root node, and that we can draw them in a loop.  It's
	// just enough to demonstrate how to get geometry and materials out of
	// a GeometryNode and onto the screen.

	// You'll want to turn this into recursive code that walks over the tree.
	// You can do that by putting a method in SceneNode, overridden in its
	// subclasses, that renders the subtree rooted at every node.  Or you
	// could put a set of mutually recursive functions in this class, which
	// walk down the tree from nodes of different types.
	glDisable(GL_CULL_FACE);
	//cout << mRotations[0][0] << " " << mRotations[0][1] << " " << mRotations[0][2] << " " << mRotations[0][3] << endl;
	//cout << mRotations[1][0] << " " << mRotations[1][1] << " " << mRotations[1][2] << " " << mRotations[1][3] << endl << endl;
	mat4 rot_tamp = {{mRotations[0][0],mRotations[1][0],mRotations[2][0],mRotations[3][0]},
					 {mRotations[0][1],mRotations[1][1],mRotations[2][1],mRotations[3][1]},
					 {mRotations[0][2],mRotations[1][2],mRotations[2][2],mRotations[3][2]}, 
					 {mRotations[0][3],mRotations[1][3],mRotations[2][3],mRotations[3][3]} };
	mat4 trans_tamp = {{mTranslations[0][0],mTranslations[1][0],mTranslations[2][0],mTranslations[3][0]},
					 {mTranslations[0][1],mTranslations[1][1],mTranslations[2][1],mTranslations[3][1]},
					 {mTranslations[0][2],mTranslations[1][2],mTranslations[2][2],mTranslations[3][2]}, 
					 {mTranslations[0][3],mTranslations[1][3],mTranslations[2][3],mTranslations[3][3]}  };	
	renderSceneGraph_helper( m_view * transpose(trans_tamp) * root.trans * transpose(rot_tamp) * inverse(root.trans), &root);

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

void A3::renderSceneGraph_helper(mat4 trans, const SceneNode * node1){

	mat4 tamp_trans = trans * node1->get_transform();

	if(node1 != NULL){
		for (const SceneNode * node : node1->children) {

			if (node->m_nodeType != NodeType::GeometryNode){
				if(node->children.size() != 0){
					renderSceneGraph_helper(tamp_trans, node);
				}
			}

			else{
				const GeometryNode * geometryNode = static_cast<const GeometryNode *>(node);

				updateShaderUniforms(m_shader, *geometryNode, tamp_trans);


				// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
				BatchInfo batchInfo = m_batchInfoMap[geometryNode->meshId];

				//-- Now render the mesh:
				m_shader.enable();

				glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
				m_shader.disable(); 
				if(node->children.size() != 0){
					renderSceneGraph_helper(tamp_trans, node);
				}
			}
		}
	}

}


//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
	glBindVertexArray(m_vao_arcCircle);

	m_shader_arcCircle.enable();
		GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
		float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
		glm::mat4 M;
		if( aspect > 1.0 ) {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
		} else {
			M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
		}
		glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
		glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
	m_shader_arcCircle.disable();

	glBindVertexArray(0);
	CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	mouse_x = xPos;
	mouse_y = yPos;
	if(current_mode == 1){
		if(tempx_pos_begin == -1 && tempy_pos_begin == -1){
			if (mouse_L_hold || mouse_R_hold || mouse_M_hold){
				tempx_pos_begin = xPos;
				tempy_pos_begin = window_height - yPos;
			}	
		}		
		else if(mouse_L_hold || mouse_R_hold || mouse_M_hold ){
			tempx_pos_end = xPos;
			tempy_pos_end = window_height - yPos;
			vPerformTransfo(tempx_pos_begin, tempx_pos_end, tempy_pos_begin,  tempy_pos_end);
			//cout << "x: " << window_width_viewport << " y: " <<window_height_viewport <<endl;
			tempx_pos_begin = tempx_pos_end;
			tempy_pos_begin = tempy_pos_end;			
		}
	}
	else if (current_mode == 2){
		//cout << "here2 " <<endl;
		
			//cout << "here1 " <<endl;
			picking_mode = 1;
			glGenRenderbuffers(1, &color_renderbuffer);
			glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)color_renderbuffer );
			glRenderbufferStorage( GL_RENDERBUFFER, GL_RGBA8, window_width, window_height );
			glBindRenderbuffer( GL_RENDERBUFFER, 0 );

			glGenRenderbuffers(1, &depth_renderbuffer);
			glBindRenderbuffer( GL_RENDERBUFFER, (GLuint)depth_renderbuffer );
			glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT, window_width, window_height );
			glBindRenderbuffer( GL_RENDERBUFFER, 0 );

			glGenFramebuffers(1, &framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, (GLuint)framebuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, color_renderbuffer);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glEnable(GL_DEPTH_TEST);
			renderSceneGraph(*m_rootNode);
			glDisable(GL_DEPTH_TEST);
			//glClear
			glReadBuffer(GL_COLOR_ATTACHMENT0);

			vector< unsigned char > pixels( 1 * 1 * 4 );
			glReadPixels( 	mouse_x,
						  	window_height - mouse_y,
						  	1,
						  	1,
						  	GL_RGB,
						  	GL_UNSIGNED_BYTE,
						  	&pixels[0]);

		    cout <<"x: " << mouse_x << " y: " << mouse_y << " r: " << (float)pixels[0] << " g: " << (float)pixels[1] <<
		    " b: " << (float)pixels[2] << endl;
		    for(int i = 0 ; i < node_seleced.size() ; i++){
		    	cout << node_seleced[i] << " ";
		    }
		    cout << endl;
		    current_pointed_node = pixels[0];
		    cout << current_pointed_node <<endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			glDeleteRenderbuffers(1, &color_renderbuffer);
			glDeleteRenderbuffers(1, &depth_renderbuffer);
			glDeleteFramebuffers(1, &framebuffer);
		

	}


	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
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
		        if(current_mode = 2){
				    if (current_pointed_node < 80 && current_pointed_node >= 0){
						if (node_seleced[current_pointed_node] == 1){

							node_seleced[current_pointed_node] = 0;
							cout << "here2" <<endl;							
						}
						else{
							node_seleced[current_pointed_node] = 1;
							cout << "here1" <<endl;
						}
					}
		        }
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
	    	tempx_pos_begin = -1;
	    	tempy_pos_begin = -1;
			if(button == GLFW_MOUSE_BUTTON_LEFT) {
		        mouse_L_hold = 0;
		        current_pointed_node = -1;
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
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
		int width,
		int height
) {
    window_width = width;
	window_height = height;
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS ) {
		if( key == GLFW_KEY_M ) {
			show_gui = !show_gui;
			eventHandled = true;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}


void vTransposeMatrix(Matrix mSrcDst) {
    GLdouble temp;
    int i,j;

    // Transpose matrix
    for ( i=0; i<4; ++i ) {
        for ( j=i+1; j<4; ++j ) {
            temp = mSrcDst[i][j];
            mSrcDst[i][j] = mSrcDst[j][i];
            mSrcDst[j][i] = temp;
        }
    }
}

void vCopyMatrix(Matrix mSource, Matrix mDestination) 
{
    int i, j;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            mDestination[i][j] = mSource[i][j];
        }
    }
}

void vRightMultiply(Matrix mMat1, Matrix mMat2) 
{
    int    i, j;
    Matrix mMat3;

    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            mMat3[i][j] = mMat1[i][0]*mMat2[0][j] + mMat1[i][1]*mMat2[1][j] +
                mMat1[i][2]*mMat2[2][j] + mMat1[i][3]*mMat2[3][j];
        }
    }
    for(i = 0; i < 4; i++) {
        for(j = 0; j < 4; j++) {
            mMat1[i][j] = mMat3[i][j];
        }
    }
}

void vTranslate(float fTrans, char cAxis, Matrix mMat)
{
    vCopyMatrix(mIdentity, mMat);
    switch(cAxis) {
    case 'x':
        mMat[3][0] = fTrans;
        break;

    case 'y':
        mMat[3][1] = fTrans;
        break;

    case 'z':
        mMat[3][2] = fTrans;
        break;
    }
}

void vToggleDir(int nDir) 
{
    nCurrentDir ^= nDir;
}

void A3::vPerformTransfo(float fOldX, float fNewX, float fOldY, float fNewY) 
{
    float  fRotVecX, fRotVecY, fRotVecZ;
    Matrix mNewMat;
  
    /*
     * Track ball rotations are being used.
     */
    if (mouse_R_hold) {
        float fDiameter;
        int iCenterX, iCenterY;
        float fNewModX, fNewModY, fOldModX, fOldModY;
	
        /* vCalcRotVec expects new and old positions in relation to the center of the
         * trackball circle which is centered in the middle of the window and
         * half the smaller of nWinWidth or nWinHeight.
         */
        fDiameter = (window_width < window_height) ? window_width * 0.5 : window_height * 0.5;
        iCenterX = window_width / 2;
        iCenterY = window_height / 2;
        fOldModX = fOldX - iCenterX;
        fOldModY = fOldY - iCenterY;
        fNewModX = fNewX - iCenterX;
        fNewModY = fNewY - iCenterY;

        vCalcRotVec(fNewModX, fNewModY,
                        fOldModX, fOldModY,
                        fDiameter,
                        &fRotVecX, &fRotVecY, &fRotVecZ);
        /* Negate Y component since Y axis increases downwards
         * in screen space and upwards in OpenGL.
         */
        vAxisRotMatrix(fRotVecX, -fRotVecY, fRotVecZ, mNewMat);

        // Since all these matrices are meant to be loaded
        // into the OpenGL matrix stack, we need to transpose the
        // rotation matrix (since OpenGL wants rows stored
        // in columns)
        vTransposeMatrix(mNewMat);
        vRightMultiply(mRotations, mNewMat);

    }

    /*
     * Pan translations are being used.
     */
    if (mouse_L_hold) {
        vTranslate((fNewX - fOldX) / ((float)SENS_PANX), 'x', mNewMat);
        vRightMultiply(mTranslations, mNewMat);
        vTranslate(-(fNewY - fOldY) / ((float)SENS_PANY), 'y', mNewMat);
        vRightMultiply(mTranslations, mNewMat);
    }
   
    /*
     * Zoom translations are being used.
     */
    if (mouse_M_hold) {
        vTranslate((fNewY - fOldY) / ((float)SENS_ZOOM), 'z', mNewMat);
        vRightMultiply(mTranslations, mNewMat);
    }

}

void A3::vCalcRotVec(float fNewX, float fNewY,
                 float fOldX, float fOldY,
                 float fDiameter,
                 float *fVecX, float *fVecY, float *fVecZ) {
   long  nXOrigin, nYOrigin;
   float fNewVecX, fNewVecY, fNewVecZ,        /* Vector corresponding to new mouse location */
         fOldVecX, fOldVecY, fOldVecZ,        /* Vector corresponding to old mouse location */
         fLength;

   /* Vector pointing from center of virtual trackball to
    * new mouse position
    */
   fNewVecX    = fNewX * 2.0 / fDiameter;
   fNewVecY    = fNewY * 2.0 / fDiameter;
   fNewVecZ    = (1.0 - fNewVecX * fNewVecX - fNewVecY * fNewVecY);

   /* If the Z component is less than 0, the mouse point
    * falls outside of the trackball which is interpreted
    * as rotation about the Z axis.
    */
   if (fNewVecZ < 0.0) {
      fLength = sqrt(1.0 - fNewVecZ);
      fNewVecZ  = 0.0;
      fNewVecX /= fLength;
      fNewVecY /= fLength;
   } else {
      fNewVecZ = sqrt(fNewVecZ);
   }

   /* Vector pointing from center of virtual trackball to
    * old mouse position
    */
   fOldVecX    = fOldX * 2.0 / fDiameter;
   fOldVecY    = fOldY * 2.0 / fDiameter;
   fOldVecZ    = (1.0 - fOldVecX * fOldVecX - fOldVecY * fOldVecY);
 
   /* If the Z component is less than 0, the mouse point
    * falls outside of the trackball which is interpreted
    * as rotation about the Z axis.
    */
   if (fOldVecZ < 0.0) {
      fLength = sqrt(1.0 - fOldVecZ);
      fOldVecZ  = 0.0;
      fOldVecX /= fLength;
      fOldVecY /= fLength;
   } else {
      fOldVecZ = sqrt(fOldVecZ);
   }

   /* Generate rotation vector by calculating cross product:
    * 
    * fOldVec x fNewVec.
    * 
    * The rotation vector is the axis of rotation
    * and is non-unit length since the length of a crossproduct
    * is related to the angle between fOldVec and fNewVec which we need
    * in order to perform the rotation.
    */
   *fVecX = (fOldVecY * fNewVecZ - fNewVecY * fOldVecZ);
   *fVecY = (fOldVecZ * fNewVecX - fNewVecZ * fOldVecX);
   *fVecZ = (fOldVecX * fNewVecY - fNewVecX * fOldVecY);
}

void A3::vAxisRotMatrix(float fVecX, float fVecY, float fVecZ, Matrix mNewMat) {
    float fRadians, fInvLength, fNewVecX, fNewVecY, fNewVecZ;

    /* Find the length of the vector which is the angle of rotation
     * (in radians)
     */
    fRadians = sqrt(fVecX * fVecX + fVecY * fVecY + fVecZ * fVecZ);

    /* If the vector has zero length - return the identity matrix */
    if (fRadians > -0.000001 && fRadians < 0.000001) {
        vCopyMatrix(mIdentity, mNewMat);
        return;
    }

    /* Normalize the rotation vector now in preparation for making
     * rotation matrix. 
     */
    fInvLength = 1 / fRadians;
    fNewVecX   = fVecX * fInvLength;
    fNewVecY   = fVecY * fInvLength;
    fNewVecZ   = fVecZ * fInvLength;

    /* Create the arbitrary axis rotation matrix */
    double dSinAlpha = sin(fRadians);
    double dCosAlpha = cos(fRadians);
    double dT = 1 - dCosAlpha;

    mNewMat[0][0] = dCosAlpha + fNewVecX*fNewVecX*dT;
    mNewMat[0][1] = fNewVecX*fNewVecY*dT + fNewVecZ*dSinAlpha;
    mNewMat[0][2] = fNewVecX*fNewVecZ*dT - fNewVecY*dSinAlpha;
    mNewMat[0][3] = 0;

    mNewMat[1][0] = fNewVecX*fNewVecY*dT - dSinAlpha*fNewVecZ;
    mNewMat[1][1] = dCosAlpha + fNewVecY*fNewVecY*dT;
    mNewMat[1][2] = fNewVecY*fNewVecZ*dT + dSinAlpha*fNewVecX;
    mNewMat[1][3] = 0;

    mNewMat[2][0] = fNewVecZ*fNewVecX*dT + dSinAlpha*fNewVecY;
    mNewMat[2][1] = fNewVecZ*fNewVecY*dT - dSinAlpha*fNewVecX;
    mNewMat[2][2] = dCosAlpha + fNewVecZ*fNewVecZ*dT;
    mNewMat[2][3] = 0;

    mNewMat[3][0] = 0;
    mNewMat[3][1] = 0;
    mNewMat[3][2] = 0;
    mNewMat[3][3] = 1;
}

SceneNode find_parents(int id, ScenNode * root){
	if(node1 != NULL){
		for (const SceneNode * node : root->children) {
			if (node->m_nodeId = id){
				return root;
			}

			else{ 
				if(node->children.size() != 0){
					find_parents(int id, ScenNode root);
				}
			}
		}
	}	

}
