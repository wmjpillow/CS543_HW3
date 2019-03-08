#include "Angel.h"
#define  GLUT_WHEEL_UP 3           
#define  GLUT_WHEEL_DOWN 4

///////////////////////////////////////////////////
typedef Angel::vec4  color4;
typedef Angel::vec4  point4;
typedef Angel::vec3  point3;
struct face3 {
	int face;
	int point1;
	int point2;
	int point3;
};
typedef struct {
	int  facenum;
	int	 vertexnum;
	point4 *flyvertex;
	face3 *flyface;
}Plyfile;
typedef struct {
	char repold;
	char repnew;
}repstr;
typedef struct {
	char ruleold;
	char rulenew[100];
}rulestr;  
typedef struct {
	int  len;
	int	 iter;
	point4 rot;
	repstr rep[10];
	int	repcount;
	char lstart;
	rulestr lrule[10];
	int rulecount;
}Lsysfile; 
typedef struct {
	mat4 cmatrix;
	point4 cpoints;
	point4 corientation;
}cstate;  

using namespace std;

int width = 0;
int height = 0;
float xaxis, yaxis, zaxis;
float yrotate, xrotate, currentrotate;
char *allfiles[45] = { "airplane.ply", "ant.ply", "apple.ply", "balance.ply", "beethoven.ply", "big_atc.ply", "big_dodge.ply", "big_porsche.ply",
	 "big_spider.ply", "canstick.ply", "chopper.ply", "cow.ply", "dolphins.ply", "egret.ply", "f16.ply", "footbones.ply", "fracttree.ply",
	 "galleon.ply", "hammerhead.ply", "helix.ply", "hind.ply", "kerolamp.ply", "ketchup.ply", "mug.ply", "part.ply", "pickup_big.ply",
	 "pump.ply", "pumpa_tb.ply", "sandal.ply", "saratoga.ply","scissors.ply", "shark.ply","steeringweel.ply","stratocaster.ply","street_lamp.ply",
	 "teapot.ply","tennis_shoe.ply","tommygun.ply","trashcan.ply","turbine.ply","urn2.ply","walkman.ply","weathervane.ply","cylinder.ply","sphere.ply" };
int currentmodle;
char *plypath = "ply_files\\\\";
int globaltempnumber, globalcuboidnumber;
int globalmodelmatrix = 0;
mat4 modlematrixstack[1000]; //most 1000 stack state
int globalstate = 0;
cstate statestack[1000]; //most 1000 stack state
char *allsysf[5] = { "lsys1.txt","lsys2.txt","lsys3.txt","lsys4.txt","lsysown" };
char *lsysplypath = "lsys_files\\\\";
float displayglobal = 85;
int drawtarget = 0;
int globalcolor;
char currenttree;

Plyfile readplyfile(char *);
Lsysfile readlsysfile(char *);
void drawtree(char);
void drawplain();
void drawcar();
void drawpark();
int  filldrawing(void);
void drawply(void);
void initGPUAndShader(void);
void displayhw3(void);
void initpro(void); //init state
void transpoint2ordinate(point4 *, int);
point4 translatepoint(float left, float right, float bottom, float top, float znear, float zfar, float x, float y, float z);
char* analyzelsys(Lsysfile);
void PushMatrix(mat4);
mat4 PopMatrix();
void PushState(cstate);
cstate PopState();
void generateGeometry(void);
void keyboard(unsigned char key, int x, int y);

// handle to program
GLuint program;
point4 *pointshw2;
color4 *colorshw2;


color4 vertex_colorshw3[7] = {
	color4(0.0, 0.0, 0.0, 1.0),  
	color4(1.0, 0.0, 0.0, 1.0),  
	color4(1.0, 1.0, 0.0, 1.0),  
	color4(0.0, 1.0, 0.0, 1.0),  
	color4(0.0, 0.0, 1.0, 1.0),  
	color4(1.0, 0.0, 1.0, 1.0), 
	color4(0.0, 1.0, 1.0, 1.0)   
};

color4 vertex_colors[10] = {
	color4(0.0, 0.0, 0.0, 1.0),  
	color4(1.0, 0.0, 0.0, 1.0), 
	color4(1.0, 1.0, 0.0, 1.0),  
	color4(0.0, 1.0, 0.0, 1.0),  
	color4(0.0, 0.0, 1.0, 1.0), 
	color4(1.0, 0.0, 1.0, 1.0), 
	color4(1.0, 1.0, 1.0, 1.0), 
	color4(0.0, 1.0, 1.0, 1.0),   
	color4(0.9, 0.6, 0.1, 1.0), 
	color4(0.5, 0.5, 0.5, 1.0) 
};

Lsysfile readlsysfile(char *filename)
{
	char *name;
	FILE *fp = NULL;
	Lsysfile lsysf;
	int icount;
	int repcount, rulecount;
	char line[256];

	//init
	repcount = 0;
	rulecount = 0;
	icount = 0;

	name = (char *)malloc(sizeof(char) * (strlen(filename) + 5));
	strcpy(name, filename);
	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("can not open the file.\n");
		exit(0);
	}

	while (!feof(fp)) {
		memset(line, 0, 256);
		fscanf(fp, "%s", &line);
		if (strcmp(line, "#") == 0) {
			fgets(line, 255, fp);
		}
		else if (strcmp(line, "len:") == 0) {
			fscanf(fp, "%d", &lsysf.len);
		}
		else if (strcmp(line, "iter:") == 0) {
			fscanf(fp, "%d", &lsysf.iter);
		}
		else if (strcmp(line, "start:") == 0) {
			fgetc(fp);
			fscanf(fp, "%c", &lsysf.lstart);
		}
		else if (strcmp(line, "rot:") == 0) {
			fscanf(fp, "%f %f %f", &lsysf.rot.x, &lsysf.rot.y, &lsysf.rot.z);
			lsysf.rot.w = 1.0f;
		}
		else if (strcmp(line, "rep:") == 0) {
			fgetc(fp); 
			fscanf(fp, "%c", &lsysf.rep[repcount].repold);
			fgetc(fp);
			fscanf(fp, "%c", &lsysf.rep[repcount].repnew);
			if (lsysf.rep[repcount].repnew == '\n') {
				printf("The new rep is empty\n");
			}
			repcount++;
		}
		else { 
			if (line[0] == '\n' || line[0] == '\t' || line[0] == '\0') { //the real last element is \0
				printf("End reading\n");
				break;
			}
			lsysf.lrule[rulecount].ruleold = line[0];
			fscanf(fp, "%s", &lsysf.lrule[rulecount].rulenew);
			rulecount++;
		}
	}

	lsysf.repcount = repcount;
	lsysf.rulecount = rulecount;

	fclose(fp);
	return lsysf;
}

char* analyzelsys(Lsysfile lsysf) { 
	int i, j, k;
	bool changeflag;
	char tempstring[20000] = "\0";
	char finalstring[20000] = "\0";

	for (i = 0; i < lsysf.iter; i++) {
		//init
		k = 0;
		finalstring[0] = lsysf.lstart;
		changeflag = FALSE;
		while (finalstring[k] != NULL) {
			for (j = 0; j < lsysf.rulecount; j++) {
				if (finalstring[k] == lsysf.lrule[j].ruleold) {
					strcat(tempstring, lsysf.lrule[j].rulenew);
					changeflag = TRUE;
					break;
				}
			}
			if (!changeflag) {
				tempstring[strlen(tempstring)] = finalstring[k];
			}
			changeflag = FALSE;
			k++;
		}
		strcpy(finalstring, tempstring);
		memset(tempstring, 0, 20000);
	}
	

	if (lsysf.repcount > 0) {
		//init
		k = 0;
		changeflag = FALSE;
		memset(tempstring, 0, 20000);
		while (finalstring[k] != NULL) {
			for (j = 0; j < lsysf.repcount; j++) {
				if (finalstring[k] == lsysf.rep[j].repold) {
					if (lsysf.rep[j].repnew == '\n') {
						changeflag = TRUE;
						break;
					}
					else {
						tempstring[strlen(tempstring)] = lsysf.rep[j].repnew;
						changeflag = TRUE;
						break;
					}
				}
			}
			if (!changeflag) {
				tempstring[strlen(tempstring)] = finalstring[k];
			}
			changeflag = FALSE;
			k++;
		}
		strcpy(finalstring, tempstring);
	}
	return finalstring;
}


void drawcar() {
	float xxx, yyy;
	xxx = float(rand() % 6000 - 2500) / 1000; 
	yyy = float(rand() % 5000 - 2000) / 1000; 

	Angel::mat4 modelMat = Angel::identity();//load 1,1,1,1 unit matrix
	modelMat = modelMat * Angel::Translate(xxx, yyy, -2.0f) * Angel::RotateY(0.0f) * Angel::Scale(0.32);
	float modelMatrixf[16];
	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];

	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];

	// set up projection matricies
	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, modelMatrixf);

	int tempnumber;
	int randomodle = rand() %11;
	if (randomodle == 0) currentmodle = 1;
	else if (randomodle == 1) currentmodle = 4;
	else if (randomodle == 2) currentmodle = 5;
	else if (randomodle == 3) currentmodle = 6;
	else if (randomodle == 4) currentmodle = 7;
	else if (randomodle == 5) currentmodle = 8;
	else if (randomodle == 6) currentmodle = 10;
	else if (randomodle == 7) currentmodle = 11;
	else if (randomodle == 8) currentmodle = 12;
	else if (randomodle == 9) currentmodle = 39;
	else if (randomodle == 10) currentmodle = 35;

	drawtarget = 3;
	tempnumber = filldrawing();
	globaltempnumber = tempnumber;

	transpoint2ordinate(pointshw2, tempnumber);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*globaltempnumber + sizeof(color4)*globaltempnumber, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)*globaltempnumber, pointshw2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)*globaltempnumber, sizeof(color4)*globaltempnumber, colorshw2);
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)*globaltempnumber));
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, globaltempnumber);
	glDisable(GL_DEPTH_TEST);

	glFlush();
	drawtarget = 0;
}

void drawpark() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	char drawchar;
	int randomchar;

	drawplain();
	drawtree('a');
	drawtree('b');
	drawtree('c');
	drawtree('d');
	drawtree('e');
	for (int i = 0; i < 5; i++) {
		randomchar = rand() % 5;
		if (randomchar == 0) drawchar = 'a';
		else if (randomchar == 1) drawchar = 'b';
		else if (randomchar == 2) drawchar = 'c';
		else if (randomchar == 3) drawchar = 'd';
		else if (randomchar == 4) drawchar = 'e';

		drawtree(drawchar);
	}
	for (int i = 0; i < 22; i++) {
		drawcar();
	}
	glFlush();
}

void drawplain() {
	Angel::mat4 modelMat = Angel::identity();//load 1,1,1,1 unit matrix
	modelMat = modelMat * Angel::Translate(0, 0, -2.0f)*Angel::RotateZ(0.0f) * Angel::RotateX(25.0f)*Angel::Scale(1);
	float modelMatrixf[16];
	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];

	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];

	// set up projection matricies
	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, modelMat);

	int sizesize = 50;
	pointshw2 = (point4 *)malloc(sizeof(point4)*sizesize);
	colorshw2 = (color4 *)malloc(sizeof(color4)*sizesize);
	for (int icount = 0; icount < sizesize; icount++) {
		colorshw2[icount] = vertex_colors[8];
	}
	pointshw2[0].x = -2.0f; pointshw2[0].y = -2.0f; pointshw2[0].z = 0.0f; pointshw2[0].w = 1.0f;
	pointshw2[1].x = -1.0f; pointshw2[1].y = 1.0f; pointshw2[1].z = 0.0f; pointshw2[1].w = 1.0f;
	pointshw2[2].x = 1.0f; pointshw2[2].y = 1.0f; pointshw2[2].z = 0.0f; pointshw2[2].w = 1.0f;
	pointshw2[3].x = 2.0f; pointshw2[3].y = -2.0f; pointshw2[3].z = 0.0; pointshw2[3].w = 1.0f;

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	////read from buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*sizesize + sizeof(color4)*sizesize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)*sizesize, pointshw2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)*sizesize, sizeof(color4)*sizesize, colorshw2);
	//reset buffer position
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)*sizesize));  //dynamic allocate memeory
	//glVertexAttrib4f(vColor, 1.0f, 0.0f, 0.0f, 1.0f);
	//glEnable( GL_DEPTH_TEST ); 
	glDrawArrays(GL_QUADS, 0, sizesize);
	//glDisable( GL_DEPTH_TEST ); 
	glFlush(); // force output to graphics hardware

	//recollect memeory
	free(pointshw2);
	free(colorshw2);
}

Plyfile readplyfile(char *filename)
{
	char *name;
	FILE *fp = NULL;
	char filetype[10];
	char temp1[20], temp2[20], temp3[20], temp4[20], temp5[20];
	Plyfile plyf;
	int icount;

	name = (char *)malloc(sizeof(char) * (strlen(filename) + 5));
	strcpy(name, filename);
	if (strlen(name) < 4 || strcmp(name + strlen(name) - 4, ".ply") != 0)
		strcat(name, ".ply");

	fp = fopen(filename, "r");
	if (fp == NULL) {
		printf("can not open the file.\n");
		exit(0);
	}

	fscanf(fp, "%s", &filetype);
	if (strcmp(filetype, "ply") != 0) {
		printf("ERROR! the file type is not existed\n");
		exit(0);
	}
	fscanf(fp, "%s %s %f", &temp1, &temp2, &temp3);
	fscanf(fp, "%s %s %d", &temp1, &temp2, &plyf.vertexnum);
	fscanf(fp, "%s %s %s", &temp1, &temp2, &temp3);
	fscanf(fp, "%s %s %s", &temp1, &temp2, &temp3);
	fscanf(fp, "%s %s %s", &temp1, &temp2, &temp3);
	fscanf(fp, "%s %s %d", &temp1, &temp2, &plyf.facenum);
	fscanf(fp, "%s %s %s %s %s\n", &temp1, &temp2, &temp3, &temp4, &temp5);
	fscanf(fp, "%s", &temp1);

	plyf.flyvertex = (point4 *)calloc(1, sizeof(point4)*plyf.vertexnum);
	for (icount = 0; icount < plyf.vertexnum; icount++) {
		fscanf(fp, "%f %f %f", &plyf.flyvertex[icount].x, &plyf.flyvertex[icount].y, &plyf.flyvertex[icount].z);
	}
	plyf.flyface = (face3 *)calloc(1, sizeof(face3)*plyf.facenum);
	for (icount = 0; icount < plyf.facenum; icount++) {
		fscanf(fp, "%d %d %d %d", &plyf.flyface[icount].face, &plyf.flyface[icount].point1, &plyf.flyface[icount].point2, &plyf.flyface[icount].point3);
	}

	fclose(fp);
	return plyf;
}

int filldrawing()
{
	char tempname[50];
	int i;
	int index = 0;
	i = 0;
	color4 colortarget;

	strcpy(tempname, plypath);
	strcat(tempname, allfiles[currentmodle]);
	Plyfile plyread = readplyfile(tempname);
	int inumber = plyread.facenum * 3;
	pointshw2 = (point4 *)malloc(sizeof(point4)*inumber);
	colorshw2 = (color4 *)malloc(sizeof(color4)*inumber);

	if (drawtarget == 1) {
		colortarget = vertex_colors[3];
	}
	else if (drawtarget == 2) {
		colortarget = vertex_colorshw3[globalcolor];
	}
	else if (drawtarget == 3) {
		colortarget = vertex_colors[5];
	}
	else {
		colortarget = vertex_colors[4];
	}
	while (i < plyread.facenum) {
		pointshw2[index] = plyread.flyvertex[plyread.flyface[i].point1];
		pointshw2[index].w = 1.0;
		colorshw2[index] = colortarget;
		pointshw2[index + 1] = plyread.flyvertex[plyread.flyface[i].point2];
		pointshw2[index + 1].w = 1.0;
		colorshw2[index + 1] = colortarget;
		pointshw2[index + 2] = plyread.flyvertex[plyread.flyface[i].point3];
		pointshw2[index + 2].w = 1.0;
		colorshw2[index + 2] = colortarget;
		index += 3;
		i++;
	}
	return index;
}
void displayhw3(void) {

	Angel::mat4 perspectiveMat = Angel::Perspective((GLfloat)displayglobal, (GLfloat)width / (GLfloat)height, (GLfloat)0.1, (GLfloat) 100.0);

	float viewMatrixf[16];
	viewMatrixf[0] = perspectiveMat[0][0]; viewMatrixf[4] = perspectiveMat[0][1];
	viewMatrixf[1] = perspectiveMat[1][0]; viewMatrixf[5] = perspectiveMat[1][1];
	viewMatrixf[2] = perspectiveMat[2][0]; viewMatrixf[6] = perspectiveMat[2][1];
	viewMatrixf[3] = perspectiveMat[3][0]; viewMatrixf[7] = perspectiveMat[3][1];

	viewMatrixf[8] = perspectiveMat[0][2]; viewMatrixf[12] = perspectiveMat[0][3];
	viewMatrixf[9] = perspectiveMat[1][2]; viewMatrixf[13] = perspectiveMat[1][3];
	viewMatrixf[10] = perspectiveMat[2][2]; viewMatrixf[14] = perspectiveMat[2][3];
	viewMatrixf[11] = perspectiveMat[3][2]; viewMatrixf[15] = perspectiveMat[3][3];

	Angel::mat4 View = Angel::LookAt( 
		Angel::vec3(3, 3, 3), 
		Angel::vec3(0, 1, 0), 
		Angel::vec3(0, -1, 0)  
	);

	float lookMatrixf[16];
	lookMatrixf[0] = View[0][0]; lookMatrixf[4] = View[0][1];
	lookMatrixf[1] = View[1][0]; lookMatrixf[5] = View[1][1];
	lookMatrixf[2] = View[2][0]; lookMatrixf[6] = View[2][1];
	lookMatrixf[3] = View[3][0]; lookMatrixf[7] = View[3][1];

	lookMatrixf[8] = View[0][2]; lookMatrixf[12] = View[0][3];
	lookMatrixf[9] = View[1][2]; lookMatrixf[13] = View[1][3];
	lookMatrixf[10] = View[2][2]; lookMatrixf[14] = View[2][3];
	lookMatrixf[11] = View[3][2]; lookMatrixf[15] = View[3][3];

	GLuint viewMatrix = glGetUniformLocationARB(program, "projection_matrix");
	glUniformMatrix4fv(viewMatrix, 1, GL_FALSE, viewMatrixf);
	GLuint lookMatrix = glGetUniformLocationARB(program, "view_matrix");
	glUniformMatrix4fv(lookMatrix, 1, GL_FALSE, lookMatrixf);

}

void drawbet2point(point4 ppoint1, point4 ppoint2)
{
	float scalesize = 0.002;

	Angel::mat4 modelMat = Angel::identity();
	if (currenttree == 'a' || currenttree == 'b') {
		scalesize = 0.001;
	}
	modelMat = modelMat * Angel::Translate(ppoint1)* Angel::Scale(scalesize * 5);//pay attention to order
	scalesize = 0.02;

	float modelMatrixf[16];
	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];
	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];
	GLuint modelMatrix = glGetUniformLocationARB(program, "model_matrix");
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, modelMatrixf);

	currentmodle = 44;
	drawtarget = 1;
	drawply(); 

	//caculate target vector 
	vec3 targetvector;
	targetvector.x = ppoint2.x - ppoint1.x;
	targetvector.y = ppoint2.y - ppoint1.y;
	targetvector.z = ppoint2.z - ppoint1.z;

	vec3 axisZStart = vec3(0, 0, -1.0f);
	vec3 axisZEnd = targetvector;
	float moveangle = Angel::dot(normalize(axisZStart), normalize(axisZEnd));
	double moveradius = acos(moveangle);
	vec3 axisRotate = Angel::cross(axisZStart, axisZEnd);
	float veclength = Angel::length(targetvector);
	float veczoom = veclength / 2;
	float axislength = Angel::length(axisRotate);
	
	mat4 rotate_matrix;
	float c = cos(moveradius);
	float s = sin(moveradius);
	float x = axisRotate.x / axislength;
	float y = axisRotate.y / axislength;
	float z = axisRotate.z / axislength;
	rotate_matrix[0][0] = x * x*(1 - c) + c;
	rotate_matrix[0][1] = x * y*(1 - c) - z * s;
	rotate_matrix[0][2] = x * z*(1 - c) + y * s;
	rotate_matrix[0][3] = 0;
	rotate_matrix[1][0] = y * x*(1 - c) + z * s;
	rotate_matrix[1][1] = y * y*(1 - c) + c;
	rotate_matrix[1][2] = y * z*(1 - c) - x * s;
	rotate_matrix[1][3] = 0;
	rotate_matrix[2][0] = x * z*(1 - c) - y * s;
	rotate_matrix[2][1] = y * z*(1 - c) + x * s;
	rotate_matrix[2][2] = z * z*(1 - c) + c;
	rotate_matrix[2][3] = 0;
	rotate_matrix[3][0] = 0;
	rotate_matrix[3][1] = 0;
	rotate_matrix[3][2] = 0;
	rotate_matrix[3][3] = 1;

	scalesize = 0.003;
	if (currenttree != 'b') {
		if (abs(targetvector.x) <= 0.01&&abs(targetvector.z) <= 0.01) {
			scalesize = 0.007;
		}
	}
	if (currenttree == 'e' || currenttree == 'a') {
		scalesize = 0.001;
	}
	modelMat = Angel::identity();//load 1,1,1,1 unit matrix
	modelMat = modelMat * Angel::Translate(ppoint1) * rotate_matrix
		* Angel::Translate(0, 0, -1 * veczoom)*Angel::Scale(scalesize, scalesize, veczoom);
	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];
	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, modelMatrixf);

	currentmodle = 43; 
	drawtarget = 2;
	drawply(); 

	scalesize = 0.002;
	modelMat = Angel::identity();//load 1,1,1,1 unit matrix
	if (currenttree == 'e' || currenttree == 'a') { 
		scalesize = 0.001;
	}
	modelMat = modelMat * Angel::Translate(ppoint2)* Angel::Scale(scalesize * 5);
	scalesize = 0.002;

	modelMatrixf[0] = modelMat[0][0]; modelMatrixf[4] = modelMat[0][1];
	modelMatrixf[1] = modelMat[1][0]; modelMatrixf[5] = modelMat[1][1];
	modelMatrixf[2] = modelMat[2][0]; modelMatrixf[6] = modelMat[2][1];
	modelMatrixf[3] = modelMat[3][0]; modelMatrixf[7] = modelMat[3][1];
	modelMatrixf[8] = modelMat[0][2]; modelMatrixf[12] = modelMat[0][3];
	modelMatrixf[9] = modelMat[1][2]; modelMatrixf[13] = modelMat[1][3];
	modelMatrixf[10] = modelMat[2][2]; modelMatrixf[14] = modelMat[2][3];
	modelMatrixf[11] = modelMat[3][2]; modelMatrixf[15] = modelMat[3][3];
	glUniformMatrix4fv(modelMatrix, 1, GL_FALSE, modelMatrixf);

	currentmodle = 44;
	drawtarget = 1;
	drawply(); 

	//draw
	glFlush();
	//recycle
	modelMat = Angel::identity();
	drawtarget = 0;
}

void drawtree(char treeflag) {
	float suitsize;
	char *sstring;
	Lsysfile lsysf;

	if (treeflag == 'a') {
		lsysf = readlsysfile("lsys_files\\\\lsys1.txt");
		sstring = analyzelsys(lsysf);
		suitsize = 0.02f; 
		currenttree = 'a';
	}
	else if (treeflag == 'b') {
		lsysf = readlsysfile("lsys_files\\\\lsys2.txt");
		sstring = analyzelsys(lsysf);
		suitsize = 0.04f;
		currenttree = 'b';
	}
	else if (treeflag == 'c') {
		lsysf = readlsysfile("lsys_files\\\\lsys3.txt");
		sstring = analyzelsys(lsysf);
		suitsize = 0.06f;
		currenttree = 'c';
	}
	else if (treeflag == 'd') {
		lsysf = readlsysfile("lsys_files\\\\lsys4.txt");
		sstring = analyzelsys(lsysf);
		suitsize = 0.08f;
		currenttree = 'd';
	}
	else if (treeflag == 'e') {
		lsysf = readlsysfile("lsys_files\\\\lsysown.txt");
		sstring = analyzelsys(lsysf);
		suitsize = 0.1f;
		currenttree = 'e';
	}

	Angel::mat4 modelMat2 = Angel::identity();//load 1,1,1,1 unit matrix
	modelMat2 = modelMat2 * Angel::RotateY(0.5f)* Angel::RotateX(0.5f) * Angel::RotateZ(0.1f); //matrix for caculate the position

	//init
	int scount = 0;

	point4 startpoint;
	startpoint.x = (float)(rand() % 2500 - 1500) / 1000; 
	startpoint.y = (float)(rand() % 2000 - 1500) / 1000;   
	startpoint.z = (float)(rand() % 1000 - 1000) / 1000; ; 
	startpoint.w = 1.0f;

	vec4 orientation; //start vertex
	orientation.x = 0.0f;
	orientation.y = 1.0f;  
	orientation.z = 0.0f;
	orientation.w = 0.0f;


	int sizesize = strlen(sstring);
	pointshw2 = (point4 *)malloc(sizeof(point4)*sizesize);
	colorshw2 = (color4 *)malloc(sizeof(color4)*sizesize);
	globalcolor = rand() % 7;

	int index = 0;
	point4 currentpoint = startpoint;
	point4 temppp[2];
	cstate currentstate;
	while (sstring[scount] != NULL) { 
		if (sstring[scount] == 'F') { 
		
			temppp[0] = currentpoint;
			orientation = modelMat2 * orientation;
			float unito = Angel::length(orientation);
		
			temppp[1].x = currentpoint.x + orientation.x / unito * lsysf.len*suitsize; //tempority use 0.1
			temppp[1].y = currentpoint.y + orientation.y / unito * lsysf.len*suitsize;
			temppp[1].z = currentpoint.z + orientation.z / unito * lsysf.len*suitsize;
			temppp[1].w = 1.0f;
			drawbet2point(temppp[0], temppp[1]);

			modelMat2 = Angel::identity();
			currentpoint = temppp[1];
			currentstate.corientation = orientation;
			currentstate.cpoints = currentpoint;
			currentstate.cmatrix = modelMat2;

		}
		else if (sstring[scount] == '+') {//Apply a positive rotatation about the X-axis of xrot degrees.
			modelMat2 = modelMat2 * Angel::RotateZ(lsysf.rot.x);
			currentstate.cmatrix = modelMat2;
		}
		else if (sstring[scount] == '-') {//Apply a negative rotatation about the X-axis of xrot degrees.
			modelMat2 = modelMat2 * Angel::RotateZ(-lsysf.rot.x);
			currentstate.cmatrix = modelMat2;
		}
		else if (sstring[scount] == '&') {//Apply a positive rotatation about the Y-axis of yrot degrees.
			modelMat2 = modelMat2 * Angel::RotateY(lsysf.rot.y);
			currentstate.cmatrix = modelMat2;
		}
		else if (sstring[scount] == '^') {//Apply a negative rotatation about the Y-axis of yrot degrees.
			modelMat2 = modelMat2 * Angel::RotateY(-lsysf.rot.y);
			currentstate.cmatrix = modelMat2;
		}
		else if (sstring[scount] == '\\') {//Apply a positive rotatation about the Z-axis of zrot degrees.
			modelMat2 = modelMat2 * Angel::RotateX(lsysf.rot.z);
			currentstate.cmatrix = modelMat2;
		}
		else if (sstring[scount] == '/') {//Apply a negative rotatation about the Z-axis of zrot degrees.
			modelMat2 = modelMat2 * Angel::RotateX(-lsysf.rot.z);
			currentstate.cmatrix = modelMat2;
		}
		else if (sstring[scount] == '|') {//Turn around 180 degrees.
			modelMat2 = modelMat2 * Angel::RotateX(180.0f);
			currentstate.cmatrix = modelMat2;
		}
		else if (sstring[scount] == '[') {//Push the current state of the turtle onto a pushdown stack.
			//PushMatrix(modelMat);
			PushState(currentstate); //store the current point, matrix and orientation as well
			//modelMat = Angel::identity();
		}
		else if (sstring[scount] == ']') {//Pop the state from the top of the turtle stack, and make it the current turtle stack.
			//modelMat = PopMatrix();
			currentstate = PopState();
			currentpoint = currentstate.cpoints;
			orientation = currentstate.corientation;
			modelMat2 = currentstate.cmatrix;
		}
		scount++;
	}


	glFlush(); // force output to graphics hardware

	//recyle
	free(pointshw2);
	free(colorshw2);
	globalcolor = 0;
}

void PushState(cstate currentstate) {
	if (globalmodelmatrix >= 999) {
		printf("ERROR! the state limited is 1000\n");
		exit(0);
	}
	else {
		statestack[globalstate] = currentstate;
		globalstate++;
	}
}

cstate PopState() {
	cstate returnstate;
	if (globalstate <= 0) {
		printf("ERROR! can't pop cause there are no state in stack\n");
		exit(0);
	}
	else {
		returnstate = statestack[globalstate - 1];
		globalstate--;
	}
	return returnstate;
}

void PushMatrix(mat4 modlematrix) {
	if (globalmodelmatrix >= 999) {
		printf("ERROR! the matrix limited is 1000\n");
		exit(0);
	}
	else {
		modlematrixstack[globalmodelmatrix] = modlematrix;
		globalmodelmatrix++;
	}
}

mat4 PopMatrix() {
	mat4 returnmatrix;
	if (globalmodelmatrix <= 0) {
		printf("ERROR! can't pop cause there are no matrix in stack\n");
		exit(0);
	}
	else {
		returnmatrix = modlematrixstack[globalmodelmatrix - 1];
		globalmodelmatrix--;
	}
	return returnmatrix;
}



void initGPUAndShader(void)
{
	//GPU init
	// Create a vertex array object
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	//Shader Init
	// Load shaders and use the resulting shader program
	program = InitShader("vshader1.glsl", "fshader1.glsl");
	glUseProgram(program);

	// sets the default color to clear screen
	glClearColor(1.0, 1.0, 1.0, 1.0); // white background
}

void drawply(void)
{
	int tempnumber;
	tempnumber = filldrawing();
	globaltempnumber = tempnumber;

	//transpoint2ordinate(pointshw2,tempnumber);

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBufferData(GL_ARRAY_BUFFER, sizeof(point4)*globaltempnumber + sizeof(color4)*globaltempnumber, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)*globaltempnumber, pointshw2);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)*globaltempnumber, sizeof(color4)*globaltempnumber, colorshw2);
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)*globaltempnumber));
	glEnable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, globaltempnumber);
	glDisable(GL_DEPTH_TEST);
}

point4 translatepoint(float left, float right, float bottom, float top, float znear, float zfar, float x, float y, float z) {
	int tranrate = 2;

	float xzoom = tranrate / (right - left);
	float yzoom = tranrate / (top - bottom);
	float zzoom = tranrate / (znear - zfar);
	float xoffset = (right + left) / (right - left);
	float yoffset = (top + bottom) / (top - bottom);
	float zoffset = (zfar + znear) / (zfar - znear);
	float newx = x * xzoom - xoffset;
	float newy = y * xzoom - yoffset;
	float newz = z * xzoom - zoffset;
	//float newy = y * yzoom- yoffset;
	//float newz = z * zzoom- zoffset;
	return point4(newx, newy, newz, 1.0);
}

void transpoint2ordinate(point4 ppoint[], int pointnumber) {
	int tranrate = 2;
	float ratiox, ratioy, ratioz;
	float xmin, xmax, ymin, ymax, zmax, zmin;
	float xoffset, yoffset, zoffset;

	xmin = ppoint[0].x;
	xmax = ppoint[0].x;
	ymin = ppoint[0].y;
	ymax = ppoint[0].y;
	zmin = ppoint[0].z;
	zmax = ppoint[0].z;

	for (int i = 0; i < pointnumber; i++) {
		if (ppoint[i].x > xmax) xmax = ppoint[i].x;
		if (ppoint[i].x < xmin) xmin = ppoint[i].x;
		if (ppoint[i].y > ymax) ymax = ppoint[i].y;
		if (ppoint[i].y < ymin) ymin = ppoint[i].y;
		if (ppoint[i].z > zmax) zmax = ppoint[i].z;
		if (ppoint[i].z < zmin) zmin = ppoint[i].z;
	}

	ratiox = tranrate / (xmax - xmin);
	ratioy = tranrate / (ymax - ymin);
	ratioz = tranrate / (zmax - zmin);
	xoffset = xmax - (xmax - xmin) / tranrate;
	yoffset = ymax - (ymax - ymin) / tranrate;
	zoffset = zmax - (zmax - zmin) / tranrate;

	if (ratiox < ratioy) {
		for (int i = 0; i < pointnumber; i++) {
			ppoint[i].x = (ppoint[i].x - xoffset)*ratiox;
			ppoint[i].y = (ppoint[i].y - yoffset)*ratiox;
			ppoint[i].z = (ppoint[i].z)*ratiox;
			//ppoint[i].z = (ppoint[i].z-zoffset)*ratiox; 
		}
	}

	else {
		for (int i = 0; i < pointnumber; i++) {
			ppoint[i].x = (ppoint[i].x - xoffset)*ratioy;
			ppoint[i].y = (ppoint[i].y - yoffset)*ratioy;
			ppoint[i].z = (ppoint[i].z)*ratioy;
			//ppoint[i].z = (ppoint[i].z-zoffset)*ratioy;
		}
	}
	/*else if((ratioz<ratiox)&&(ratioz<ratioy)){
		for(int i=0;i<pointnumber;i++){
			ppoint[i].x = (ppoint[i].x-xoffset)*ratioz;
			ppoint[i].y = (ppoint[i].y-yoffset)*ratioz;
			ppoint[i].z = (ppoint[i].z-zoffset)*ratioz;
		}
	}*/
}

// keyboard handler
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a':
		drawtree('a');
		break;
	case 'b':
		drawtree('b');
		break;
	case 'c':
		drawtree('c');
		break;
	case 'd':
		drawtree('d');
		break;
	case 'e':
		drawtree('e');
		break;
	case 'f':
		drawpark();
		break;
	case 'p':
		drawplain();
		break;
	case 'o':
		drawcar();
		break;
	case 033:
		exit(EXIT_SUCCESS);
		break;
	case 'w':
		initpro();
		break;
	}
}

void initpro() {
	yaxis = 0.0f;
	xaxis = 0.0f;
	zaxis = -2.0f;
	yrotate = 0.0f;
	xrotate = 45.0f;
	currentmodle = 11;  
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	width = 800;
	height = 600;

	glutInitContextVersion(3, 1);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("Color Cube");

	glewInit();

	initGPUAndShader();

	initpro();

	glutDisplayFunc(displayhw3);
	glutKeyboardFunc(keyboard);

	glutMainLoop();
	return 0;
}