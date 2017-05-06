#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>
#define O_Rotate_View 0
#define N_Translate_View 1
#define P_Perspective 2
#define R_Rotate_Model 3
#define T_Translate_Model 4
#define S_Scale_Model 5
#define V_Viewport 6

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
	VertexData();

	std::vector<glm::vec2> positions;
	std::vector<glm::vec3> colours;
	GLuint index;
	GLsizei numVertices;
};


class A2 : public CS488Window {
public:
	A2();
	virtual ~A2();

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
	virtual void clip_side(glm::vec4 point, glm::vec4 unit_vector);
	virtual void clip_box_coord(glm::vec4 point, glm::vec4 unit_vector);
	virtual void clip_world_coord(glm::vec4 point, glm::vec4 unit_vector);
	virtual void point_div_z(glm::vec4 point);
	virtual void reset();	
			void print_mat(glm::mat4 mat123);
	//virtual bool matrix_44x44(float arrayA[4][4], float arrayB[4][4]);

	void createShaderProgram();
	void enableVertexAttribIndices();
	void generateVertexBuffers();
	void mapVboDataToVertexAttributeLocation();
	void uploadVertexDataToVbos();

	void initLineData();

	void setLineColour(const glm::vec3 & colour);

	void drawLine (
			const glm::vec2 & v0,
			const glm::vec2 & v1
	);

	ShaderProgram m_shader;

	GLuint m_vao;            // Vertex Array Object
	GLuint m_vbo_positions;  // Vertex Buffer Object
	GLuint m_vbo_colours;    // Vertex Buffer Object

	VertexData m_vertexData;

	glm::vec3 m_currentLineColour;

//for box

	//box rotate
	float angle_box_x = 0.0f;
	float angle_box_y = 0.0f;
	float angle_box_z = 0.0f;

	glm::mat4 box_view = {glm::vec4(1,0,0,0),
					      glm::vec4(0,1,0,0),
					      glm::vec4(0,0,1,0),
					      glm::vec4(0,0,0,1)};

	glm::mat4 box_rot_x = {glm::vec4(1,0,0,0),
					       glm::vec4(0,cos(angle_box_x),-sin(angle_box_x),0),
					       glm::vec4(0,sin(angle_box_x), cos(angle_box_x),0),
					       glm::vec4(0,0,0,1)};

	glm::mat4 box_rot_y = {glm::vec4( cos(angle_box_y), 0, sin(angle_box_y), 0),
					       glm::vec4(0, 1, 0, 0),
					       glm::vec4(-sin(angle_box_y), 0, cos(angle_box_y), 0),
					       glm::vec4(0,0,0,1)};

	glm::mat4 box_rot_z = {glm::vec4(cos(angle_box_z), -sin(angle_box_z),0,0),
					       glm::vec4(sin(angle_box_z),  cos(angle_box_z),0,0),
					       glm::vec4(0,0,1,0),
					       glm::vec4(0,0,0,1)};

	float trans_box_x = 0.0f;
	float trans_box_y = 0.0f;
	float trans_box_z = 0.0f;

	glm::mat4 box_trans = {glm::vec4(1,0,0,0),
					       glm::vec4(0,1,0,0),
					       glm::vec4(0,0,1,0),
					       glm::vec4(0,0,0,1)};

	float sca_box_x = 1.0f;
	float sca_box_y = 1.0f;
	float sca_box_z = 1.0f;

	glm::mat4 box_sca = {glm::vec4(1,0,0,0),
					       glm::vec4(0,1,0,0),
					       glm::vec4(0,0,1,0),
					       glm::vec4(0,0,0,1)};

	glm::mat4 coord_z = {glm::vec4(1,0,0,0),
					       glm::vec4(0,1,0,0),
					       glm::vec4(0,0,-1,0),
					       glm::vec4(0,0,0,1)};




	float angle_view_x = 0.0f;
	float angle_view_y = 0.0f;
	float angle_view_z = 0.0f;

	glm::mat4 eye_view = {glm::vec4(1,0,0,0),
					      glm::vec4(0,1,0,0),
					      glm::vec4(0,0,1,0),
					      glm::vec4(0,0,0,1)};

	glm::mat4 eye_rot_x = {glm::vec4(1,0,0,0),
					       glm::vec4(0,cos(angle_view_x),sin(angle_view_x),0),
					       glm::vec4(0,-sin(angle_view_x), cos(angle_view_x),0),
					       glm::vec4(0,0,0,1)};

	glm::mat4 eye_rot_y = {glm::vec4( cos(angle_view_y), 0, -sin(angle_view_y), 0),
					       glm::vec4(0, 1, 0, 0),
					       glm::vec4(sin(angle_view_y), 0, cos(angle_view_y), 0),
					       glm::vec4(0,0,0,1)};

	glm::mat4 eye_rot_z = {glm::vec4(cos(angle_view_z), sin(angle_view_z),0,0),
					       glm::vec4(-sin(angle_view_z),  cos(angle_view_z),0,0),
					       glm::vec4(0,0,1,0),
					       glm::vec4(0,0,0,1)};

	float trans_view_x = 0.0f;
	float trans_view_y = 0.0f;
	float trans_view_z = -5.0f;

	glm::mat4 eye_trans = {glm::vec4(1,0,0,0),
					       glm::vec4(0,1,0,0),
					       glm::vec4(0,0,1,0),
					       glm::vec4(trans_view_x,trans_view_y,trans_view_z,1)};





float n = 3.0f;
float f = 7.0f;
float ratio = 1.0f;
float fov = M_PI/6.0f;



	glm::mat4 box_with_z = {glm::vec4(1/(tan(fov)/ratio),0,0,0),
					      	glm::vec4(0,1/tan(fov),0,0),
					      	glm::vec4(0,0,(f+n)/(f-n),-1),
					      	glm::vec4(0,0,-2*f*n/(f-n),0)};




	glm::mat4 box_half = {glm::vec4(0.5,0,0,0),
					      glm::vec4(0,0.5,0,0),
					      glm::vec4(0,0,0.5,0),
					      glm::vec4(0,0,0,1)};
	glm::mat4 eye_view_init = {glm::vec4(1,0,0,0),
					           glm::vec4(0,1,0,0),
					           glm::vec4(0,0,1,0),
					           glm::vec4(0,0,5,1)};					      

	glm::mat4 world_view = {glm::vec4(1,0,0,0),
					       glm::vec4(0,1,0,0),
					       glm::vec4(0,0,1,0),
					       glm::vec4(0,0,0,1)};

	glm::mat4 eye_view_sum = {glm::vec4(1,0,0,0),
					      glm::vec4(0,1,0,0),
					      glm::vec4(0,0,1,0),
					      glm::vec4(0,0,0,1)};

	glm::mat4 flat_M = {glm::vec4(1,0,0,0),
					    glm::vec4(0,1,0,0),
					    glm::vec4(0,0,0,0),
					    glm::vec4(0,0,0,0)};	

	glm::mat4 screen_view = {glm::vec4(1,0,0,0),
					      	glm::vec4(0,1,0,0),
					      	glm::vec4(0,0,0,0),
					      	glm::vec4(0,0,0,0)};

	std::vector<glm::vec4> box_points;
	std::vector<glm::vec4> box_points_new;

	std::vector<glm::vec4> box_coord;
	std::vector<glm::vec4> box_coord_new;

	std::vector<glm::vec4> world_coord;
	std::vector<glm::vec4> world_coord_new;

	int link_arr[12][2] = {{5,6},
							{6,2},
							{2,1},
							{1,5},

							{7,8},
							{8,4},
							{4,3},
							{3,7},

							{5,7},
							{6,8},
							{2,4},
							{1,3}
							};
	int link_arr2[3][2] = {{0,1},
							{0,2},
							{0,3}							
							};
	int link_arr3[3][2] = {{0,1},
							{0,2},
							{0,3}							
							};															
	std::vector<glm::vec4> box_points_after_clip;
	std::vector<glm::vec4> box_coord_after_clip;
	std::vector<glm::vec4> world_coord_after_clip;	
	bool drawline_n[12] = {};
	bool drawline_n2[3] = {};	
	bool drawline_n3[3] = {};					  
};
