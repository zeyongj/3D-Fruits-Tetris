#include "include/Angel.h"
#include <cstdlib>
#include <iostream>
#include <random>
#include <cstring>
#include <assert.h>
#include <math.h>
// To avoid the error of not finding "unistd.b" .
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;


// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
GLfloat xsize = 720; 
GLfloat ysize = 720;
GLfloat zsize = 720;

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[4][4] = 
	{{vec2(0,0), vec2(-1,0), vec2(1, 0), vec2(-1,-1)},
	{vec2(0,0), vec2(0,-1), vec2(0,1), vec2(1, -1)},     
	{vec2(0,0), vec2(1,0), vec2(-1, 0), vec2(1,  1)},  
	{vec2(0,0), vec2(0,1), vec2(0, -1), vec2(-1, 1)}};

vec2 allRotationsSshape[4][4] =
	{{vec2(0,-1), vec2(1,0), vec2(0,0), vec2(-1,-1)},
	{vec2(1,0), vec2(0,1), vec2(0,0), vec2(1,-1)},
	{vec2(0,0), vec2(-1,-1), vec2(0,-1), vec2(1,0)},
	{vec2(0,0), vec2(1,-1), vec2(1,0), vec2(0,1)}};

vec2 allRotationsIshape[4][4] =
	{{vec2(0,0), vec2(-1,0), vec2(1,0), vec2(-2,0)},
	{vec2(0,0), vec2(0,-1), vec2(0,1), vec2(0,-2)},
	{vec2(-1,0), vec2(0,0), vec2(-2,0), vec2(1,0)},
	{vec2(0,-1), vec2(0,0), vec2(0,-2), vec2(0,1)}};

vec2 shapes[3][4][4] =
	{allRotationsLshape,
	allRotationsSshape,
	allRotationsIshape};

enum orientation
{
	DOWN,
	RIGHT,
	UP,
	LEFT
};

enum tiletypes
{
	L,
	S,
	I
};

enum colors
{
	GREEN,
	PURPLE,
	RED,
	ORANGE,
	YELLOW
};


int current_tile_type;
int current_tile_orientation;

// colors
vec4 darkgrey = vec4(0.5, 0.5, 0.5, 0.5); 
vec4 grey 	= 	vec4(0.8, 0.8, 0.8, 1.0);
vec4 green 	= 	vec4(0.0, 1.0, 0.0, 1.0); 
vec4 purple 	= 	vec4(1.0, 0.0, 1.0, 1.0); 
vec4 red 	=	vec4(1.0, 0.0, 0.0, 1.0); 
vec4 orange = 	vec4(1.0, 0.5, 0.0, 1.0);
vec4 yellow = 	vec4(1.0, 1.0, 0.0, 1.0); 
vec4 white  = 	vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = 	vec4(0.0, 0.0, 0.0, 0.0);

vec4 colors[5] = {green, purple, red, orange, yellow}; 
 
//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20];
bool isDropped = false;

int board_ColorCells[10][20]; //store data about what colors are being used 

//An array containing the colour of each of the 10*20*6 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200];
vec4 newcolours[24*6];
vec4 greycolours[24*6];

// Set location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// Set locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;
GLuint loczsize;

// Set VAO and VBO
GLuint vaoIDs[9]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[18]; // Two Vertex Buffer Objects for each VAO 

bool movetile(vec2 direction);
bool isLegalPos(vec2 cur_tilepos);
//ROTATION TEST
enum { Yaxis = 0, Xaxis = 1, Zaxis = 2, NumAxes = 4 };
int Axis = Xaxis;

GLfloat Theta[NumAxes] = { 0.0, 0.0, 0.0 }; 
GLuint theta;

GLfloat bound = 30;
GLfloat rate = 0.0;
GLfloat Xrate = -3;
GLfloat Zrate = -3;

//CUBOID
const int NumVertices = 36; //(6 faces)*(2 triangles/face)*(3 vertices/triangle)=36

vec4 c_points[NumVertices];
vec4 c_colors[NumVertices];

GLfloat c_a = 0.0;
GLfloat c_b = 1.0;

vec4 vertices[8] = { 
    vec4( c_a, c_a, c_b, 1.0 ),
    vec4( c_a, c_b, c_b, 1.0 ),
    vec4( c_b, c_b, c_b, 1.0 ),
    vec4( c_b, c_a, c_b, 1.0 ),
    vec4( c_a, c_a, c_a, 1.0 ),
    vec4( c_a, c_b, c_a, 1.0 ),
    vec4( c_b, c_b, c_a, 1.0 ),
    vec4( c_b, c_a, c_a, 1.0 )
};

#define R_BASE_HEIGHT 33.0
#define R_BASE_WIDTH 66.0
#define R_UPPER_ARM_HEIGHT 369.0
#define R_LOWER_ARM_HEIGHT 369.0
#define R_UPPER_ARM_WIDTH  20.0
#define R_LOWER_ARM_WIDTH  40.0

enum {
    Base,
    LowerArm,
    UpperArm,
    NumJointAngles
};

GLfloat ang_gamma[NumJointAngles] = {
  0.0, 	//Base
  10.0, 	//LowerArm
  -60.0 	//UpperArm
};

GLint angle = Base; 

int Index = 0;
void quad( int a, int b, int c, int d )
{
    c_colors[Index] = grey; c_points[Index] = vertices[a]; Index++;
    c_colors[Index] = grey; c_points[Index] = vertices[b]; Index++;
    c_colors[Index] = grey; c_points[Index] = vertices[c]; Index++;
    c_colors[Index] = grey; c_points[Index] = vertices[a]; Index++;
    c_colors[Index] = grey; c_points[Index] = vertices[c]; Index++;
    c_colors[Index] = grey; c_points[Index] = vertices[d]; Index++;
}

void cube( void )
{
    Index = 0;
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}


class MatrixStack {
	int _index;
	int _size;
	mat4* _matrices;

public:
	MatrixStack( int numMatrices = 32):_index(0), _size(numMatrices)
	{ _matrices = new mat4[numMatrices]; }

	~MatrixStack()
	{ delete[]_matrices; }

	mat4& push(const mat4& m ){
		assert( _index +1 < _size);
		_matrices[_index++] = m;
	}

	mat4& pop( void ) {
		assert( _index - 1 >= 0);
		_index--;
		return _matrices[_index];
	}
};

MatrixStack mystack;
mat4 model_view, camera_view, projection_view;
GLuint ModelView;
mat4 Id = mat4(1.0);


int random(int bound){
	random_device rd;
	mt19937 gen(rd());
	uniform_int_distribution<> dist(0,bound);
	int result = dist(gen);
	return result;
}

vec2 getTilePosition() {
	vec2 result;
	result.y += R_BASE_HEIGHT;
	result.x += R_LOWER_ARM_HEIGHT * -sin(M_PI/180.0 * ang_gamma[LowerArm]);
	result.y += R_LOWER_ARM_HEIGHT * cos(M_PI/180.0 * ang_gamma[LowerArm]);
	result.x += R_UPPER_ARM_HEIGHT * -cos(M_PI/180.0 * (90.0 - ang_gamma[LowerArm] - ang_gamma[UpperArm]));
	result.y += R_UPPER_ARM_HEIGHT *  sin(M_PI/180.0 * (90.0 - ang_gamma[LowerArm] - ang_gamma[UpperArm]));

	double val;
	double ceil_v;
	double floor_v;
	for(int i = 0; i<2; i++){
		val = result[i]/33.0;
		ceil_v = ceil(val);
		floor_v = floor(val);
		if((ceil_v - val) < (val - floor_v)) result[i] = ceil_v;
		else result[i] = floor_v;
	}
	return result;
}
// Update the VBO containing its vertex position data
void updatetile()
{
	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 32, 1); 
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 32, 1);
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 32, 1);
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 32, 1);

		vec4 p5 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 1, 1);
		vec4 p6 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 1, 1);
		vec4 p7 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 1, 1);
		vec4 p8 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 1, 1);

		// Two points are used by two triangles each
		vec4 newpoints[36] = {	
								p1, p3, p2, //Front
								p2, p3, p4,

								p1, p2, p5, //left
								p5, p2, p6, 
								
								p5, p6, p7, //back
								p7, p6, p8,

								p7, p4, p3, //right
								p4, p7, p8,

								p2, p4, p6, //top
								p6, p4, p8, 

								p1, p3, p5, //bottom
								p5, p3, p7
							}; 

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*36*sizeof(vec4), 36*sizeof(vec4), newpoints); 
	}

	if(!isLegalPos(getTilePosition()) && !isDropped) { 
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(greycolours), greycolours); // Put the colour data in the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	else if(!isDropped){
		glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	glBindVertexArray(0);
}


// Called at the start of play and every time a tile is placed
void newtile()
{
	tilepos = vec2(5 , 19); // Put the tile at the top of the board
	int current_color = 0;
	int tile_type = random(3);
	current_tile_orientation = DOWN;
	current_tile_type = tile_type;
	// Update the geometry VBO of current tile
	for (int i = 0; i < 4; i++)
		switch(tile_type) {
			case 0:
				tile[i] = allRotationsLshape[current_tile_orientation][i]; // Get the 4 pieces of the new tile for shape L
				break;
			case 1:
				tile[i] = allRotationsSshape[current_tile_orientation][i]; // Get the 4 pieces of the new tile for shape S
				break;
			case 2:
				tile[i] = allRotationsIshape[current_tile_orientation][i]; // Get the 4 pieces of the new tile for shape I
				break;
			default:
				break;
		}
	updatetile(); 

	// Update the color VBO of current tile
	for (int i = 0; i < 144; i++){
		if((i%36)==0){
			current_color = random(4);
		}

		newcolours[i] = colors[current_color]; // Randomlize the color
	}
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	if(board[5][19] == true || board[4][19] == true || board[6][19] == true) {
		printf("Cannot spawn new tile! Game exiting in 1 second...\n");
		sleep(1);
		exit(EXIT_SUCCESS);
	}
}


void initGrid()
{
	vec4 gridpoints[128]; // Array containing the 64 points of the 32 total lines to be later put in the VBO
	vec4 gridcolours[128]; // One colour per vertex
	// Set vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 33.0, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 33.0, 1);
		
		gridpoints[22+2*i] = vec4((33.0 + (33.0 * i)), 33.0, 0, 1);
		gridpoints[22+2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 0, 1);
		
	}
	// Set horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[44 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 33.0, 1);
		gridpoints[44 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 33.0, 1);

		gridpoints[86+2*i] = vec4(35.0, (33.0 + (33.0 * i)), 0, 1);
		gridpoints[86+2*i + 1] = vec4(365.0, (33.0 + (33.0 * i)), 0, 1);
	}
	// Make all grid lines white
	for (int i = 0; i < 128; i++)
		gridcolours[i] = white;


	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, 128*sizeof(vec4), gridpoints, GL_STATIC_DRAW); // Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, 128*sizeof(vec4), gridcolours, GL_STATIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}


void initBoard()
{
	vec4 boardpoints[1200]; //FRONT 
	vec4 boardpoints_rev[1200]; //BACK 
	vec4 boardpoints_right[1200]; //RIGHT 
	vec4 boardpoints_left[1200]; //LEFT
	vec4 boardpoints_bottom[1200]; //UNDER
	vec4 boardpoints_top[1200]; //TOP


	for (int i = 0; i < 1200; i++) boardcolours[i] = black; 

	GLfloat z_pos1, z_pos2;
	
	z_pos1 = 1.0;
	z_pos2 = 32.0;

	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{	

			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos1, 1);
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos1, 1);
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos1, 1);
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos1, 1);
			// Two points are reused
			boardpoints[6*(10*i + j)    ] = p1;
			boardpoints[6*(10*i + j) + 1] = p2;
			boardpoints[6*(10*i + j) + 2] = p3;
			boardpoints[6*(10*i + j) + 3] = p2;
			boardpoints[6*(10*i + j) + 4] = p3;
			boardpoints[6*(10*i + j) + 5] = p4;

			p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos2, 1);
			p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos2, 1);
			p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos2, 1);
			p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos2, 1);		
			// Set to reverse side
			boardpoints_rev[6*(10*i + j)    ] = p1;
			boardpoints_rev[6*(10*i + j) + 1] = p2;
			boardpoints_rev[6*(10*i + j) + 2] = p3;
			boardpoints_rev[6*(10*i + j) + 3] = p3;
			boardpoints_rev[6*(10*i + j) + 4] = p4;
			boardpoints_rev[6*(10*i + j) + 5] = p2;

			p1 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos1, 1);
			p2 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos1, 1);
			p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos2, 1);
			p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos2, 1);		
			// Set to right side
			boardpoints_right[6*(10*i + j)    ] = p1;
			boardpoints_right[6*(10*i + j) + 1] = p3;
			boardpoints_right[6*(10*i + j) + 2] = p2;
			boardpoints_right[6*(10*i + j) + 3] = p2;
			boardpoints_right[6*(10*i + j) + 4] = p3;
			boardpoints_right[6*(10*i + j) + 5] = p4;

			p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos1, 1);
			p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos1, 1);
			p3 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos2, 1);
			p4 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos2, 1);		
			// Set to left side
			boardpoints_left[6*(10*i + j)    ] = p1;
			boardpoints_left[6*(10*i + j) + 1] = p3;
			boardpoints_left[6*(10*i + j) + 2] = p2;
			boardpoints_left[6*(10*i + j) + 3] = p2;
			boardpoints_left[6*(10*i + j) + 4] = p4;
			boardpoints_left[6*(10*i + j) + 5] = p3;

			p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos1, 1);
			p2 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos2, 1);
			p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos1, 1);
			p4 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), z_pos2, 1);		
			// Set to bottom side
			boardpoints_bottom[6*(10*i + j)    ] = p1;
			boardpoints_bottom[6*(10*i + j) + 1] = p2;
			boardpoints_bottom[6*(10*i + j) + 2] = p3;
			boardpoints_bottom[6*(10*i + j) + 3] = p2;
			boardpoints_bottom[6*(10*i + j) + 4] = p3;
			boardpoints_bottom[6*(10*i + j) + 5] = p4;

			p1 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos1, 1);
			p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos2, 1);
			p3 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos1, 1);
			p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), z_pos2, 1);		
			// Set to top side
			boardpoints_top[6*(10*i + j)    ] = p1;
			boardpoints_top[6*(10*i + j) + 1] = p3;
			boardpoints_top[6*(10*i + j) + 2] = p2;
			boardpoints_top[6*(10*i + j) + 3] = p2;
			boardpoints_top[6*(10*i + j) + 4] = p3;
			boardpoints_top[6*(10*i + j) + 5] = p4;

		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++){
			board[i][j] = false;
			board_ColorCells[i][j] = -1; 
		}
	// BACK: 
	// Set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	// FRONT:
	glBindVertexArray(vaoIDs[3]);
	glGenBuffers(2, &vboIDs[6]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[6]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints_rev, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	// RIGHT:
	glBindVertexArray(vaoIDs[4]);
	glGenBuffers(2, &vboIDs[8]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[8]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints_right, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	// LEFT:
	glBindVertexArray(vaoIDs[5]);
	glGenBuffers(2, &vboIDs[10]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[10]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints_left, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	// BOTTOM:
	glBindVertexArray(vaoIDs[6]);
	glGenBuffers(2, &vboIDs[12]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[12]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints_bottom, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

	// TOP:
	glBindVertexArray(vaoIDs[7]);
	glGenBuffers(2, &vboIDs[14]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[14]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardpoints_top, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);

}

void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 4*36*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 4*36*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void initCube()
{
	cube();

	glBindVertexArray(vaoIDs[8]);
	glGenBuffers(2, &vboIDs[16]);

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[16]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(c_points), c_points, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[17]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(c_colors), c_colors, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	glGenVertexArrays(9, &vaoIDs[0]);

	for(int i = 0; i<144; i++){
		greycolours[i] = darkgrey;
	}

	initCurrentTile();
	initGrid();
	initBoard();
	initCube();

	isDropped = false;
	newtile(); // create new next tile

	// set to default
	ModelView = glGetUniformLocation( program, "ModelView" );
	glEnable( GL_DEPTH_TEST );

	camera_view = LookAt(vec3(0.0, 30.0, 30.0),	vec3(0.0, 10.0, 0.0), vec3(0.0, 1.0, 0.0));

	glBindVertexArray(0);
	glClearColor(0, 0, 0, 0);
}

bool checkRotation(){
	int x_pos = tilepos.x;
	int y_pos = tilepos.y;

	vec2 all_cell_pos[4] = shapes[current_tile_type][current_tile_orientation]; //Set array of cell positions for this tile
	for(int j = 0; j<4; ++j){ //Load positions for all tile cells into an array 
		all_cell_pos[j].x += x_pos;
		all_cell_pos[j].y += y_pos;
	}

	for(int i = 0; i<4; ++i){
		if(board[(int)all_cell_pos[i].x][(int)all_cell_pos[i].y]==true) return false;
	}

	bool move_attempt;

	switch(current_tile_type) {
		case L:	
			if(x_pos == 0) {
				move_attempt = movetile(vec2(1,0));
				return move_attempt;
			}
			else if(x_pos == 9) {
				move_attempt = movetile(vec2(-1,0));
				return move_attempt;
			}
			else if(y_pos == 0) { // Set no floor kick rotations
				return false;
			}
			else if(y_pos==19) {
				move_attempt = movetile(vec2(0,-1));
				return move_attempt;
			}	
			break;
		case S:
			if(x_pos == 0) {
				move_attempt = movetile(vec2(1,0));
				return move_attempt;
			}
			else if(y_pos==19) {
				move_attempt = movetile(vec2(0,-1));
				return move_attempt;
			}
			break;
		case I:
			if(x_pos == 0) {
				move_attempt = movetile(vec2(2,0));
				return move_attempt;
			}
			else if(x_pos == 1){
				move_attempt = movetile(vec2(1,0));
				return move_attempt;
			}
			else if(x_pos == 9) {
				move_attempt = movetile(vec2(-1,0));
				return move_attempt;
			}
			else if(y_pos == 0 || y_pos == 1) { // Set no floor kick rotations
				return false;
			}
			else if(y_pos==19) {
				move_attempt = movetile(vec2(0,-1));
				return move_attempt;
			}
			break;
		default:
			break;
	}
	return true;
}
// Rotates the current tile
void changeOrientation(){
	for (int i = 0; i < 3 ; i++)
		switch(current_tile_type) {
			case L:
				tile[i] = allRotationsLshape[current_tile_orientation][i]; // Get the 4 pieces of the new tile for shape L
				break;
			case S:
				tile[i] = allRotationsSshape[current_tile_orientation][i]; // Get the 4 pieces of the new tile for shape S
				break;
			case I:
				tile[i] = allRotationsIshape[current_tile_orientation][i]; // Get the 4 pieces of the new tile for shape I
				break;
			default:
			break;
		}
}

void rotate()
{      
	switch(current_tile_orientation){
		case DOWN:
			current_tile_orientation = RIGHT;
			changeOrientation();
			if(!checkRotation()){
				current_tile_orientation = DOWN;
				changeOrientation();
			}
			break;
		case RIGHT:
			current_tile_orientation = UP;
			changeOrientation();
			if(!checkRotation()){
				current_tile_orientation = RIGHT;
				changeOrientation();
			}
			break;
		case UP:
			current_tile_orientation = LEFT;
			changeOrientation();
			if(!checkRotation()){
				current_tile_orientation = UP;
				changeOrientation();
			}
			break;
		case LEFT:
			current_tile_orientation = DOWN;
			changeOrientation();
			if(!checkRotation()){
				current_tile_orientation = LEFT;
				changeOrientation();
			}
			break;
		default:
			break;
	}
	updatetile();
}


bool checkForColorMatches(){
	int col_lower, col_higher, row_lower, row_higher, candidate; //Set ranges for potential cell deletion

	col_lower = 0; col_higher = 0; //Initialized to first column entry, remember to reinitialize for next column
	candidate = board_ColorCells[0][0]; //Ditto

	int intsize = sizeof(board_ColorCells[0][0]);

	for(int i = 0; i<10; ++i){ 
		col_lower = 0; col_higher = 0;
		candidate = board_ColorCells[i][0];

		for(int j = 0; j<20; ++j){
			if(board_ColorCells[i][j]==-1 && !(col_higher-col_lower>2)){
				candidate = -1; col_lower = col_higher; col_lower++; col_higher++;
			}
			else if(board_ColorCells[i][j]!=candidate && !(col_higher-col_lower>2)){
				candidate = board_ColorCells[i][j]; col_lower = col_higher; col_higher++;
			}
			else if(board_ColorCells[i][j]!=candidate && (col_higher-col_lower>2)){				
				
				memmove(board[i]+col_lower, board[i]+col_higher, 20-1-col_higher);
				memmove(board_ColorCells[i]+col_lower, board_ColorCells[i]+col_higher, 20*intsize-intsize-intsize*col_higher); 
				board[i][19]=false;
				board_ColorCells[i][19]=-1;

				for(int k = 0; k<(19-col_higher); ++k){					
					memmove(boardcolours+((k+col_lower)*60)+(i*6), boardcolours+((k+col_higher)*60)+(i*6), 96);
				}
				boardcolours[19*60+(i*6)] = black;
				checkForColorMatches();
				return true;
				
				candidate = board_ColorCells[i][j]; col_higher=col_lower; col_higher++;
			}
			else if(board_ColorCells[i][j]==candidate){
				col_higher++;
			}
		}
	}

	for(int j = 0; j<20; ++j){ 
		row_lower = 0; row_higher = 0;
		candidate = board_ColorCells[0][j];

		for(int i = 0; i<10; ++i){
			if(board_ColorCells[i][j]==-1 && !(row_higher-row_lower>2)){
				candidate = -1; row_lower = row_higher; row_lower++; row_higher++;
			}
			else if(board_ColorCells[i][j]!=candidate && !(row_higher-row_lower>2)){
				candidate = board_ColorCells[i][j]; row_lower = row_higher; row_higher++;
			}
			else if(board_ColorCells[i][j]!=candidate && (row_higher-row_lower>2)){ //MEMMOVE HAPPENS HERE
				for(int h = 0; h<(row_higher-row_lower); ++h){
					memmove(board[row_lower+h]+j, board[row_lower+h]+j+1, 20-j-1);
					memmove(board_ColorCells[row_lower+h]+j, board_ColorCells[row_lower+h]+j+1, 20*intsize-intsize-intsize*j); 
					board[row_lower+h][19]=false;
					board_ColorCells[row_lower+h][19]=-1;
				}

				for(int k = 0; k<(19-j); ++k){					
					memmove(boardcolours+((k+j)*60)+(row_lower*6), boardcolours+((k+1+j)*60)+(row_lower*6), 96*(row_higher-row_lower));
				}
				for(int h = row_lower; h<row_higher; ++h){
					boardcolours[19*60+(h*6)] = black;
				}
				checkForColorMatches();

				return true;

			}
			else if(board_ColorCells[i][j]==candidate){
				row_higher++;
			}	
		}

	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);

	return true;	
}

// Checks if the specified row (0 is the bottom 19 the top) is full
void checkfullrow(int row) 
{
	for(int k = 0; k<10; ++k){
		if(board[k][row]==false) {
			return;
		}
	}

	int intsize = sizeof(board_ColorCells[0][0]);

	for(int i = 0; i<10; ++i){ 
		memmove(board[i]+row, board[i]+row+1, 20-1-row);
		memmove(board_ColorCells[i]+row, board_ColorCells[i]+row+1, 20*intsize-intsize-intsize*row); 
		board[i][19]=false;
		board_ColorCells[i][19]=-1;
	}
	
	memmove(boardcolours+(row*60), boardcolours+((row+1)*60), (18240)-((row)*60*16)); 
	for(int j = 1140; j< 1200; ++j){
		boardcolours[j] = black;
	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);

	checkForColorMatches();
}



// Places the current tile
void settile()
{

	int low = 20; int high = -1;
	int x_pos = tilepos.x;
	int y_pos = tilepos.y;

	vec2 all_cell_pos[4] = shapes[current_tile_type][current_tile_orientation]; // Set array of cell positions for coloring

	for(int j = 0; j<4; ++j){
		all_cell_pos[j].x += x_pos;
		all_cell_pos[j].y += y_pos;
		if((int)all_cell_pos[j].y > high) high = (int)all_cell_pos[j].y;
		if((int)all_cell_pos[j].y < low) low = (int)all_cell_pos[j].y;
	}


	for(int i = 0; i < 4; ++i){ 
		for(int k = 0; k < 6; ++k){ 
			boardcolours[(60*(int)all_cell_pos[i].y + 6*(int)all_cell_pos[i].x) + k] = {newcolours[k+(i*36)][0], newcolours[k+(i*36)][1], newcolours[k+(i*36)][2], newcolours[k+(i*36)][3]};
		}

		if((newcolours[i*36][0] == 0.0f) && (newcolours[i*36][1] == 1.0f) && (newcolours[i*36][2] == 0.0f) && (newcolours[i*36][3] == 1.0f)){ //GREEN
			board_ColorCells [(int)all_cell_pos[i].x][(int)all_cell_pos[i].y] = GREEN;
		}
		else if((newcolours[i*36][0] == 0.0f) && (newcolours[i*36][1] == 0.0f) && (newcolours[i*36][2] == 1.0f) && (newcolours[i*36][3] == 1.0f)){ //BLUE
			board_ColorCells [(int)all_cell_pos[i].x][(int)all_cell_pos[i].y] = PURPLE;
		}
		else if((newcolours[i*36][0] == 1.0f) && (newcolours[i*36][1] == 0.0f) && (newcolours[i*36][2] == 0.0f) && (newcolours[i*36][3] == 1.0f)){ //RED
			board_ColorCells [(int)all_cell_pos[i].x][(int)all_cell_pos[i].y] = RED;
		}
		else if((newcolours[i*36][0] == 1.0f) && (newcolours[i*36][1] == 0.5f) && (newcolours[i*36][2] == 0.0f) && (newcolours[i*36][3] == 1.0f)){ //ORANGE
			board_ColorCells [(int)all_cell_pos[i].x][(int)all_cell_pos[i].y] = ORANGE;
		}
		else if((newcolours[i*36][0] == 1.0f) && (newcolours[i*36][1] == 1.0f) && (newcolours[i*36][2] == 0.0f) && (newcolours[i*36][3] == 1.0f)){ //YELLOW
			board_ColorCells [(int)all_cell_pos[i].x][(int)all_cell_pos[i].y] = YELLOW;
		}



		board[(int)all_cell_pos[i].x][(int)all_cell_pos[i].y] = true;

	}

	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);


	for(int l = high; l >=low; --l){ 
		checkfullrow(l);
	}

	checkForColorMatches();

	newtile();
	isDropped = false;
}

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	int left_border, right_border, bottom_border;
	left_border = 0; right_border = 0; bottom_border = 0;

	switch(current_tile_type) {
		case L: //0
			switch(current_tile_orientation) {
				case DOWN:
					left_border = 1; right_border = 8; bottom_border = 1;
					break;
				case RIGHT:
					left_border = 0; right_border = 8; bottom_border = 1;
					break;
				case UP:
					left_border = 1; right_border = 8; bottom_border = 0;
					break;
				case LEFT:
					left_border = 1; right_border = 9; bottom_border = 1;
					break;
				default:
					break;
			}
			break;
		case S: //1
			switch(current_tile_orientation) {
				case DOWN:
					left_border = 1; right_border = 8; bottom_border = 1;
					break;
				case RIGHT:
					left_border = 0; right_border = 8; bottom_border = 1;
					break;
				case UP:
					left_border = 1; right_border = 8; bottom_border = 1;
					break;
				case LEFT:
					left_border = 0; right_border = 8; bottom_border = 1;
					break;
				default:
					break;
			}
			break;
		case I: //2
			switch(current_tile_orientation) {
				case DOWN:
					left_border = 2; right_border = 8; bottom_border = 0;
					break;
				case RIGHT:
					left_border = 0; right_border = 9; bottom_border = 2;
					break;
				case UP:
					left_border = 2; right_border = 8; bottom_border = 0;
					break;
				case LEFT:
					left_border = 0; right_border = 9; bottom_border = 2;
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}

	int x_pos = tilepos.x;
	int y_pos = tilepos.y;

	vec2 all_cell_pos[4] = shapes[current_tile_type][current_tile_orientation]; // Set array of cell positions for this tile
	for(int j = 0; j<4; ++j){ //Load positions for all tile cells into an array 
		all_cell_pos[j].x += x_pos;
		all_cell_pos[j].y += y_pos;
	}

	for(int k = 0; k<4; ++k){
		if((board[(int)all_cell_pos[k].x][(int)all_cell_pos[k].y-1]==true) && (direction.y!=0)){
			settile();
			return false;
		} 
	}

	if(tilepos.y==bottom_border && direction.y!=0) {
		settile();
		return false;
	}

	if(tilepos.y+direction.y >= bottom_border) tilepos.y += direction.y;
	if((tilepos.x+direction.x >= left_border) && 
		(tilepos.x+direction.x <= right_border)){ 
		for(int k = 0; k<4; ++k){
			if(board[(int)all_cell_pos[k].x + (int)direction.x][(int)all_cell_pos[k].y]==true){
				return false;
			}
		}
		tilepos.x += direction.x;
	}
	return true;
}

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	init();
}


void base(){
	mystack.push(model_view);
	glBindVertexArray(vaoIDs[8]);
	mat4 instance = (Translate(-33.0, 0.0, 33.0)*Scale(99.0, 33.0, 99.0));
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	model_view = mystack.pop();
}

void lower(){
	mystack.push(model_view);
	glBindVertexArray(vaoIDs[8]);
	mat4 instance = (
		Translate(0.0, 0.0, 38.0)
		*Scale(R_LOWER_ARM_WIDTH, R_LOWER_ARM_HEIGHT, R_LOWER_ARM_WIDTH)
		);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	model_view = mystack.pop();
}

void upper(){
	mystack.push(model_view);
	glBindVertexArray(vaoIDs[8]);
	mat4 instance = (
		Translate(10.0, 0.0, 38.0)
		*Scale(R_UPPER_ARM_WIDTH, R_UPPER_ARM_HEIGHT, R_UPPER_ARM_WIDTH)
		);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	model_view = mystack.pop();
}

void tiledropper(){
	mystack.push(model_view);
	glBindVertexArray(vaoIDs[2]);
	mat4 instance = (
		Scale(1.0, 1.0, 1.0) *
		Translate(-198.0, -660.0, 33.0)
		);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view * instance);
	glDrawArrays(GL_TRIANGLES, 0, 4*36);
	model_view = mystack.pop();
}

void boardGridTileDraw(){
	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the front of the board 
	glDrawArrays(GL_TRIANGLES, 0, 1200); //Draw the board front 

	if(isDropped){
		glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile
		glDrawArrays(GL_TRIANGLES, 0, 4*36); // Draw the current tile
	}

	glBindVertexArray(vaoIDs[3]); // Bind the VAO representing the back of the board
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board back

	glBindVertexArray(vaoIDs[4]); // Bind the VAO representing left of the board
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board left

	glBindVertexArray(vaoIDs[5]); // Bind the VAO representing right of the board
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board right

	glBindVertexArray(vaoIDs[6]); // Bind the VAO representing the board bottom
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board bottom

	glBindVertexArray(vaoIDs[7]); // Bind the VAO representing top of the board
	glDrawArrays(GL_TRIANGLES, 0, 1200); // Draw the board top	

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 128); // Draw the grid lines
}

//vec4 t_point = {198.0, 660.0, 1.0, 1.0};
// Draws the game
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	projection_view = Perspective(45.0, 1.0*xsize/ysize, 5.0, 300.0); //setting viewing space

	mat4 M = mat4();
	M *= Translate(0.0, 10.0, 0);
	M *= Scale(1.0/33.0, 1.0/33.0, 1.0/33.0);
	M *= Translate(-200.0, -360.0, 0.0);

	model_view = projection_view * camera_view * M;

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);

	boardGridTileDraw();
	
	base();

	mystack.push(model_view);
	model_view *= ( 
		Translate(0.0, R_BASE_HEIGHT, 0.0) *
		RotateZ(ang_gamma[LowerArm])
		);

	lower();

	model_view *= (
		Translate(0.0, R_LOWER_ARM_HEIGHT, 0.0) *
		RotateZ(ang_gamma[UpperArm])
		);

	upper();
	
	model_view *= (
		Translate(0.0, R_UPPER_ARM_HEIGHT, 0.0) * 
		RotateZ(-ang_gamma[UpperArm]) *
		RotateZ(-ang_gamma[LowerArm])
		);

	if(!isDropped) tiledropper();
	
	model_view = mystack.pop();

	glutSwapBuffers();
}


// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	zsize = w;	
	glViewport(0, 0, w, h);
}

void shuffleColors(){
	vec4 temp[6];
	memcpy(temp, newcolours, 96);
	memmove(newcolours, newcolours+6, 288);
	memcpy(newcolours+18, temp, 96);
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO

}

void dropTile(){ //assume tile is in a legal position
	vec2 cur_tilepos = getTilePosition();
	int x_pos = cur_tilepos.x;
	int y_pos = cur_tilepos.y;

	tilepos.x = x_pos-1;
	tilepos.y = y_pos-1;

	updatetile();
	isDropped = true;
	
}

bool isLegalPos(vec2 cur_tilepos){
	bool result = true;
	int x_pos = cur_tilepos.x;
	int y_pos = cur_tilepos.y;

	vec2 all_cell_pos[4] = shapes[current_tile_type][current_tile_orientation]; // Set array of cell positions for this tile
	for(int j = 0; j<4; ++j){ //Load positions for all tile cells into an array 
		all_cell_pos[j].x += x_pos;
		all_cell_pos[j].y += y_pos;
	}


	for(int i = 0; i<4; i++){
		if( all_cell_pos[i].x < 1 
			|| all_cell_pos[i].x > 10 
			|| all_cell_pos[i].y < 1 
			|| all_cell_pos[i].y > 20) result = false;

		if(board[(int)all_cell_pos[i].x-1][(int)all_cell_pos[i].y-1] == true) result = false;
	}

	return result;	
}

void special_up(int key, int x, int y){
	switch(key){
		case GLUT_KEY_LEFT:
			rate = 0.0f;
			break;
		case GLUT_KEY_RIGHT:
			rate = 0.0f;
			break;
	}
}
// Handle arrow key keypresses
void special(int key, int x, int y)
{
	switch(key)
	{
		case GLUT_KEY_UP:
			if(isDropped){
				rotate();
				updatetile();
			}
			break;
		case GLUT_KEY_DOWN:
			if(isDropped){
				movetile(vec2(0,-1));
				updatetile();
			}
			break;
		case GLUT_KEY_LEFT:
			if(glutGetModifiers() & GLUT_ACTIVE_CTRL) {
				Axis = Yaxis;
				rate = 2.0f;
				camera_view *= RotateY(rate);
				break;
			}
			else if(isDropped){
				movetile(vec2(-1,0));	
				updatetile();
			}
			break;
		case GLUT_KEY_RIGHT:
			if(glutGetModifiers() & GLUT_ACTIVE_CTRL) {
				Axis = Yaxis;
				rate = -2.0f;
				camera_view *= RotateY(rate);
				break;
			}
			else if(isDropped){
				movetile(vec2(1,0));
				updatetile();
			}
			break;
	}
}

// Handles standard keypresses

void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 'w':
			angle = UpperArm;
			if(ang_gamma[angle] < 0) ang_gamma[angle] += 1;			
			if(!isDropped) updatetile();
			break;
		case 's':
			angle = UpperArm;
			if(ang_gamma[angle] > -180) ang_gamma[angle] -= 1;
			if(!isDropped) updatetile();
			break;
		case 'a':
			angle = LowerArm;
			if(ang_gamma[angle] < 90) ang_gamma[angle] += 1;
			if(!isDropped) updatetile();
			break;
		case 'd':
			angle = LowerArm;
			if(ang_gamma[angle] > -90) ang_gamma[angle] -= 1;
			if(!isDropped) updatetile();
			break;
		case ' ':
			if(!isDropped && isLegalPos(getTilePosition())){
				dropTile();
			} 

			break;
		case 033: 
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': 
			restart();
			break;
	}	
	glutPostRedisplay();
}


void idle(void)
{
	glutPostRedisplay();
}

void timedMove(int t){ 
	if(isDropped){
		movetile(vec2(0, -1));
		updatetile();
	}
	glutTimerFunc(1000, timedMove, 0);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); 
	glutCreateWindow("Fruit Tetris 3D");
	glewInit();
	init();
	// Callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutSpecialUpFunc(special_up);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);
	glutTimerFunc(1000, timedMove, 0);

	glutMainLoop(); // Start main loop
	return 0;
}
