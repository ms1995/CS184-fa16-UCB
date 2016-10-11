#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <limits>

//include header file for glfw library so that we can use OpenGL
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// #define IS_DEBUG

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

// #define min(a,b) (a<b?a:b)
// #define max(a,b) (a>b?a:b)
#define bound(x,a,b) (x<a?a:(x>b?b:x))
#define PI 3.14159265 // Should be used from mathlib

using namespace std;

struct Sphere {
    float x, y, z, R, color[3];

    Sphere (const float px,
            const float py,
            const float pz,
            const float pr) {
        x = px; y = py; z = pz; R = pr;
        color[0] = color[1] = color[2] = 1.0f;
    }

    Sphere () {
        x = y = z = 0.0f;
        color[0] = color[1] = color[2] = 1.0f;
    }

    bool operator< (const Sphere &s) const {
        return z < s.z;
    }
};

enum LightType { Directional, Point };

struct Light {
    LightType lt;
    float pos[3], color[3];

    Light (const float px,
            const float py,
            const float pz,
            const float pr,
            const float pg,
            const float pb,
            const LightType plt) {
        pos[0] = px; pos[1] = py; pos[2] = pz;
        color[0] = pr; color[1] = pg; color[2] = pb;
        lt = plt;
    }

    Light (const LightType plt) {
        pos[0] = pos[1] = pos[2] = color[0] = color[1] = color[2] = 0.0f;
        lt = plt;
    }
};

vector<Sphere> spheres;
vector<Light> lights;

float dotProduct3(float a[], float b[]) {
    float ret = 0.0f;
    for (int i = 0; i < 3; ++i)
        ret += a[i] * b[i];
    return ret;
}

void scalarProduct3(float a[], float b, float c[]) {
    for (int i = 0; i < 3; ++i)
        c[i] = a[i] * b;
}

void simpleProduct3(float a[], float b[], float c[]) {
    for (int i = 0; i < 3; ++i)
        c[i] = a[i] * b[i];
}

void addVector3(float a[], float b[], float c[]) {
    for (int i = 0; i < 3; ++i)
        c[i] = a[i] + b[i];
}

void subVector3(float a[], float b[], float c[]) {
    for (int i = 0; i < 3; ++i)
        c[i] = a[i] - b[i];
}

void printVector3(float a[]) {
    for (int i = 0; i < 3; ++i)
        printf("%f, ", a[i]);
    printf("\n");
}

void normalizeVector3(float a[]) {
    float inner_product = dotProduct3(a, a);
    if (inner_product <= 0.0f)
        return;
    scalarProduct3(a, 1/sqrt(inner_product), a);
}

void crossProductVector3(float a[], float b[], float c[]) {
    c[0] = a[1] * b[2] - a[2] * b[1];
    c[1] = a[2] * b[0] - a[0] * b[2];
    c[2] = a[0] * b[1] - a[1] * b[0];
}

void rstVector3(float a[]) {
    a[0] = a[1] = a[2] = 0.0f;
}

//****************************************************
// Global Variables
//****************************************************
GLfloat translation[3] = {0.0f, 0.0f, 0.0f};
bool auto_strech = false;
int Width_global = 400;
int Height_global = 400;

bool use_gl = true, is_asm = false, is_anim = false;
char *save_path;
float *img_f = NULL, color_max = 0.0f;
bool color_max_valid = false;

const float z_viewpoint = 2.0f;

float pu = 1.0f, pv = 1.0f;
float ka[3] = { 0.0f, 0.0f, 0.0f };
float kd[3] = { 0.5f, 0.5f, 0.5f };
float ks[3] = { 0.5f, 0.5f, 0.5f };

inline float sqr(float x) { return x*x; }

void writeToBmp() {
    FILE *f;
    int i, j;
    int w = Width_global, h = Height_global;
    int datasize = 3*w*h;
    int filesize = 54 + datasize;

    unsigned char *img = (unsigned char *) calloc(1, datasize);

    for (i = 0; i < datasize; i += 1) {
        img[i] = (unsigned char)floor(img_f[i] / color_max * 255);
    }

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(w    );
    bmpinfoheader[ 5] = (unsigned char)(w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(w>>16);
    bmpinfoheader[ 7] = (unsigned char)(w>>24);
    bmpinfoheader[ 8] = (unsigned char)(h    );
    bmpinfoheader[ 9] = (unsigned char)(h>> 8);
    bmpinfoheader[10] = (unsigned char)(h>>16);
    bmpinfoheader[11] = (unsigned char)(h>>24);

    f = fopen(save_path, "wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(i=0; i<h; i++)
    {
        fwrite(img+(w*(h-i-1)*3),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }
    fclose(f);

    free(img);
}

//****************************************************
// Simple init function
//****************************************************
void initializeRendering()
{
    glfwInit();
}


//****************************************************
// A routine to set a pixel by drawing a GL point.  This is not a
// general purpose routine as it assumes a lot of stuff specific to
// this example.
//****************************************************
void setPixel(float x, float y, GLfloat r, GLfloat g, GLfloat b) {
    if (r < 0.0f || g < 0.0f || b < 0.0f)
        return;
    if (use_gl) {
        if (color_max_valid) {
            // glColor3f(r / color_max, g / color_max, b / color_max);
            glColor3f(r, g, b);
            glVertex2f(x+0.5, y+0.5);  // The 0.5 is to target pixel centers
            // Note: Need to check for gap bug on inst machines.
        } else {
            color_max = max(color_max, max(r, max(g, b)));
        }
    } else {
        int X = bound((int)x, 0, Width_global - 1);
        int Y = Height_global - 1 - bound((int)y, 0, Height_global - 1);
        img_f[(X+Y*Width_global)*3+2] = r;
        img_f[(X+Y*Width_global)*3+1] = g;
        img_f[(X+Y*Width_global)*3+0] = b;
        color_max = max(color_max, max(r, max(g, b)));
    }
}

//****************************************************
// Keyboard inputs
//****************************************************
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key) {
            
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_Q: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_LEFT :
            if (action) translation[0] -= 0.01f * Width_global; break;
        case GLFW_KEY_RIGHT:
            if (action) translation[0] += 0.01f * Width_global; break;
        case GLFW_KEY_UP   :
            if (action) translation[1] += 0.01f * Height_global; break;
        case GLFW_KEY_DOWN :
            if (action) translation[1] -= 0.01f * Height_global; break;
        case GLFW_KEY_F:
            if (action) auto_strech = !auto_strech; break;
        case GLFW_KEY_SPACE: break;
            
        default: break;
    }
    
}

void render() {
#ifdef IS_DEBUG
static bool printed = false;
#endif
    int i, j;
    float minX = numeric_limits<float>::max();
    float maxX = -minX, maxY = -minX, minY = minX;

    for (vector<Sphere>::iterator it_sp = spheres.begin(); it_sp != spheres.end(); ++it_sp) {
        minX = min(minX, it_sp->x - it_sp->R);
        maxX = max(maxX, it_sp->x + it_sp->R);
        minY = min(minY, it_sp->y - it_sp->R);
        maxY = max(maxY, it_sp->y + it_sp->R);
    }
#ifdef IS_DEBUG
if (!printed)
printf("minX=%f, minY=%f, maxX=%f, maxY=%f\n", minX, minY, maxX, maxY);
#endif
    float window_scale = min((float)Width_global / (maxX - minX),
                            (float)Height_global / (maxY - minY)) * 0.9f;
    int w_adjust = (Width_global - (int)(window_scale * (maxX - minX))) / 2;
    int h_adjust = (Height_global - (int)(window_scale * (maxY - minY))) / 2;

    for (vector<Sphere>::iterator it_sp = spheres.begin(); it_sp != spheres.end(); ++it_sp) {
        if (use_gl) {
            // Draw points
            glBegin(GL_POINTS);
        }
        float center[3] = { it_sp->x, it_sp->y, it_sp->z };
        float x = (it_sp->x - minX) * window_scale;
        float y = (it_sp->y - minY) * window_scale;
        float r = it_sp->R * window_scale; //

        int minI = max(0,(int)ceil(x-r));
        int maxI = min(Width_global-1,(int)floor(x+r));
#ifdef IS_DEBUG
if (!printed)
printf("x=%f, y=%f, r=%f, minI=%d, maxI=%d, Width_global=%d, Height_global=%d\n", x, y, r, minI, maxI, Width_global, Height_global);
#endif
        for (i = minI; i < maxI; ++i) {
            int dj = (int)floor(sqrt(sqr(r) - sqr((float)i - x)));
            int minJ = (int)ceil(y - dj);
            int maxJ = (int)floor(y + dj);
#ifdef IS_DEBUG
if (!printed) printf("\n");
#endif
            for (j = minJ; j < maxJ; ++j) {
                float pos[3], color[3];
                pos[0] = (float)i / window_scale + minX + 0.5f / window_scale;
                pos[1] = (float)j / window_scale + minY + 0.5f / window_scale;
                pos[2] = sqrt(sqr(it_sp->R) - sqr(pos[0] - it_sp->x) - sqr(pos[1] - it_sp->y)) + it_sp->z;

                rstVector3(color);
                
                float viewpoint[3] = {pos[0], pos[1], z_viewpoint};

                for (vector<Light>::iterator it_dl = lights.begin(); it_dl != lights.end(); ++it_dl) {
                    float tmp[3], n[3], L[3], v[3];

                    if (it_dl->lt == Directional) {
                        rstVector3(L);
                        subVector3(L, it_dl->pos, L);
                    } else {
                        subVector3(it_dl->pos, pos, L);
                    }

                    bool blocked = false;
                    for (vector<Sphere>::iterator it_sp2 = spheres.begin(); it_sp2 != spheres.end(); ++it_sp2) {
                        float a = dotProduct3(L, L);
                        subVector3(pos, (float[]){ it_sp2->x, it_sp2->y, it_sp2->z }, tmp);
                        float b = 2 * dotProduct3(L, tmp);
                        float c = dotProduct3(tmp, tmp) - sqr(it_sp2->R);
                        float delta = sqr(b) - 4 * a * c;
                        if (delta < 0)
                            continue;
                        delta = sqrt(delta);
                        if (it_dl->lt == Directional) {
                            if (delta - b > 0.001f) {
                                blocked = true;
                                break;
                            }
                        } else {
                            if ((delta - b > 0.001f && delta - b < 2 * a) ||
                                (-delta - b > 0.001f && -delta - b < 2 * a)) {
                                blocked = true;
                                break;
                            }
                        }
                    }
                    if (blocked)
                        continue;

                    // Normalized light direction
                    normalizeVector3(L);

                    // Normal vector
                    subVector3(pos, center, n);
                    normalizeVector3(n);

                    // Normalized viewpoint direction
                    subVector3(viewpoint, pos, v);
                    normalizeVector3(v);

                    float H[3], U[3], V[3], I[3];
                    float Y[3] = { 0.0f, 1.0f, 0.0f };

                    I[0] = I[1] = I[2] = 1.0f;

                    addVector3(L, v, H);
                    normalizeVector3(H);

                    scalarProduct3(n, dotProduct3(n, Y), V);
                    subVector3(Y, V, V);
                    normalizeVector3(V);

                    crossProductVector3(V, n, U);
                    normalizeVector3(U);

                    float puv = (pu * sqr(dotProduct3(H, U)) + pv * sqr(dotProduct3(H, V))) /
                            (1 - sqr(dotProduct3(n, H)));

                    if (is_asm) {
                        // Ashikhmin-Shirley model
                        subVector3(I, ks, tmp);
                        scalarProduct3(tmp, pow(1 - dotProduct3(H, v), 5), tmp);
                        addVector3(tmp, ks, tmp);

                        scalarProduct3(tmp,
                            sqrt((pu+1) * (pv+1)) *
                            pow(dotProduct3(n, H), puv) /
                            dotProduct3(H, v) /
                            max(dotProduct3(n, v), dotProduct3(n, L)) / 64.0f / M_PI, 
                            tmp);

                        simpleProduct3(tmp, it_dl->color, tmp);
                        addVector3(color, tmp, color);

                        subVector3(I, ks, tmp);
                        simpleProduct3(tmp, kd, tmp);
                        scalarProduct3(tmp,
                            2.0f * 28.0f *
                            (1 - pow(1 - dotProduct3(n, v) / 2, 5)) *
                            (1 - pow(1 - dotProduct3(n, L) / 2, 5)) / 23.0f / M_PI,
                            tmp);
                        simpleProduct3(tmp, it_dl->color, tmp);
                        addVector3(color, tmp, color);
                    } else {
                        // Phong model
                        simpleProduct3(it_dl->color, ka, tmp);
                        addVector3(color, tmp, color);

                        scalarProduct3(kd, max(0.0f, dotProduct3(L, n)), tmp);
                        simpleProduct3(it_dl->color, tmp, tmp);
                        addVector3(color, tmp, color);

                        scalarProduct3(n, 2 * dotProduct3(L, n), tmp);
                        subVector3(tmp, L, tmp);
                        scalarProduct3(ks, pow(max(0.0f, dotProduct3(tmp, v)), puv), tmp);
                        simpleProduct3(it_dl->color, tmp, tmp);
                        addVector3(color, tmp, color);
                    }
                }
#ifdef IS_DEBUG
if (!printed)
printf("%f,%f,%f=%f,%f,%f=%d,%d\n", pos[0], pos[1], pos[2], color[0], color[1], color[2], i, j);
#endif
                setPixel(i + w_adjust, j + h_adjust, color[0], color[1], color[2]);
            }
        }
        if (use_gl) {
            glEnd();
        }
    }
#ifdef IS_DEBUG
printed = true;
#endif
}

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void display( GLFWwindow* window )
{
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f ); //clear background screen to black
    
    glClear(GL_COLOR_BUFFER_BIT);                // clear the color buffer (sets everything to black)
    
    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
    glLoadIdentity();                            // make sure transformation is "zero'd"
    
    //----------------------- code to draw objects --------------------------
    glPushMatrix();
    
    render();
    color_max_valid = true;

    if (is_anim) {
	    float sinx = 0.01f;
	    float cosx = sqrt(1 - sqr(sinx));
	    for (vector<Sphere>::iterator it_sp = spheres.begin(); it_sp != spheres.end(); ++it_sp) {
		    it_sp->x = cosx * it_sp->x + sinx * it_sp->y;
		    it_sp->y = cosx * it_sp->y - sinx * it_sp->x;
		}
	}

    glPopMatrix();
    
    glfwSwapBuffers(window);
    
}

//****************************************************
// function that is called when window is resized
//***************************************************
void size_callback(GLFWwindow* window, int width, int height)
{
    color_max_valid = false;

    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);
    
    glViewport(0, 0, Width_global, Height_global);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, Width_global, 0, Height_global, 1, -1);
    
    display(window);
}


//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
#ifdef IS_DEBUG
freopen("log", "w", stdout);
#endif
    spheres.push_back(Sphere(0.0f, 0.0f, 0.0f, 1.0f));

    int i, j;
    for (i = 1; i < argc; ) {
        if (!strlen(argv[i]))
            goto success;

        if (argv[i][0] != '-') {
            printf("Parameter key unspecified for '%s'.\n", argv[i]);
            return 1;
        }
        if (!strcmp(argv[i], "-o")) {
            if (++i >= argc)
                goto no_value;
            save_path = argv[i];
            use_gl = false;
            goto success;
        }
        if (!strcmp(argv[i], "-asm")) {
            is_asm = true;
            goto success;
        }
        if (!strcmp(argv[i], "-anim")) {
            is_anim = true;
            goto success;
        }
        if (!strcmp(argv[i], "-spu")) {
            if (++i >= argc)
                goto no_value;
            if (sscanf(argv[i], "%f", &pu) && (pu) >= 0.0f)
                goto success;
            goto wrong_format;
        }
        if (!strcmp(argv[i], "-spv")) {
            if (++i >= argc)
                goto no_value;
            if (sscanf(argv[i], "%f", &pv) && (pv) >= 0.0f)
                goto success;
            goto wrong_format;
        }
        if (!strcmp(argv[i], "-sp")) {
            if (++i >= argc)
                goto no_value;
            if (sscanf(argv[i], "%f", &pv) && (pu=pv) >= 0.0f)
                goto success;
            goto wrong_format;
        }
        if (!strcmp(argv[i], "-s")) {
            if (i + 4 >= argc)
                goto no_value;
            Sphere s;
            if (sscanf(argv[++i], "%f", &s.x) &&
                sscanf(argv[++i], "%f", &s.y) &&
                sscanf(argv[++i], "%f", &s.z) &&
                sscanf(argv[++i], "%f", &s.R) &&
                s.R > 0.0f) {
                spheres.push_back(s);
                goto success;
            } else {
                goto wrong_format;
            }
        }
        if (!strcmp(argv[i], "-ka")) {
            if (i + 3 >= argc)
                goto no_value;
            if (sscanf(argv[++i], "%f", &ka[0]) &&
                ka[0] >= 0.0f && ka[0] <= 1.0f &&
                sscanf(argv[++i], "%f", &ka[1]) &&
                ka[1] >= 0.0f && ka[1] <= 1.0f &&
                sscanf(argv[++i], "%f", &ka[2]) &&
                ka[2] >= 0.0f && ka[2] <= 1.0f) {
                goto success;
            } else {
                goto wrong_format;
            }
        }
        if (!strcmp(argv[i], "-kd")) {
            if (i + 3 >= argc)
                goto no_value;
            if (sscanf(argv[++i], "%f", &kd[0]) &&
                kd[0] >= 0.0f && kd[0] <= 1.0f &&
                sscanf(argv[++i], "%f", &kd[1]) &&
                kd[1] >= 0.0f && kd[1] <= 1.0f &&
                sscanf(argv[++i], "%f", &kd[2]) &&
                kd[2] >= 0.0f && kd[2] <= 1.0f) {
                // printf("-kd read successfully.\n");
                goto success;
            } else {
                goto wrong_format;
            }
        }
        if (!strcmp(argv[i], "-ks")) {
            if (i + 3 >= argc)
                goto no_value;
            if (sscanf(argv[++i], "%f", &ks[0]) &&
                ks[0] >= 0.0f && ks[0] <= 1.0f &&
                sscanf(argv[++i], "%f", &ks[1]) &&
                ks[1] >= 0.0f && ks[1] <= 1.0f &&
                sscanf(argv[++i], "%f", &ks[2]) &&
                ks[2] >= 0.0f && ks[2] <= 1.0f) {
                goto success;
            } else {
                goto wrong_format;
            }
        }
        if (!strcmp(argv[i], "-pl")) {
            if (i + 6 >= argc)
                goto no_value;
            Light l(Point);
            if (sscanf(argv[++i], "%f", &l.pos[0]) &&
                sscanf(argv[++i], "%f", &l.pos[1]) &&
                sscanf(argv[++i], "%f", &l.pos[2]) &&
                sscanf(argv[++i], "%f", &l.color[0]) &&
                l.color[0] >= 0.0f &&
                sscanf(argv[++i], "%f", &l.color[1]) &&
                l.color[1] >= 0.0f &&
                sscanf(argv[++i], "%f", &l.color[2]) &&
                l.color[2] >= 0.0f) {
                lights.push_back(l);
                goto success;
            } else {
                goto wrong_format;
            }
        }
        if (!strcmp(argv[i], "-dl")) {
            if (i + 6 >= argc)
                goto no_value;
            Light l(Directional);
            if (sscanf(argv[++i], "%f", &l.pos[0]) &&
                sscanf(argv[++i], "%f", &l.pos[1]) &&
                sscanf(argv[++i], "%f", &l.pos[2]) &&
                sscanf(argv[++i], "%f", &l.color[0]) &&
                l.color[0] >= 0.0f &&
                sscanf(argv[++i], "%f", &l.color[1]) &&
                l.color[1] >= 0.0f &&
                sscanf(argv[++i], "%f", &l.color[2]) &&
                l.color[2] >= 0.0f) {
                lights.push_back(l);
                goto success;
            } else {
                goto wrong_format;
            }
        }
    success:
        ++i;
        continue;
    wrong_format:
        printf("Wrong input on '%s'.\n", argv[i]);
        return 1;
    no_value:
        printf("Insufficient number of values specified for '%s'.\n", argv[i-1]);
        return 1;
    }

    for (i = 0; i < spheres.size(); ++i)
        for (j = i + 1; j < spheres.size(); ++j)
        {
            if (sqr(spheres[i].x - spheres[j].x) +
                sqr(spheres[i].y - spheres[j].y) +
                sqr(spheres[i].z - spheres[j].z) <
                spheres[i].R + spheres[j].R)
            {
                printf("Overlapping spheres!\n");
                return 2;
            }
        }

    sort(spheres.begin(), spheres.end());

    if (use_gl) {
        //This initializes glfw
        initializeRendering();
        GLFWwindow* window = glfwCreateWindow( Width_global, Height_global, "CS184", NULL, NULL );
        if ( !window )
        {
            cerr << "Error on window creating" << endl;
            glfwTerminate();
            return -1;
        }
        
        const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        if ( !mode )
        {
            cerr << "Error on getting monitor" << endl;
            glfwTerminate();
            return -1;
        }
        
        glfwMakeContextCurrent( window );
        
        // Get the pixel coordinate of the window
        // it returns the size, in pixels, of the framebuffer of the specified window
        glfwGetFramebufferSize(window, &Width_global, &Height_global);
        
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, Width_global, 0, Height_global, 1, -1);
        
        glfwSetWindowTitle(window, "CS184");
        glfwSetWindowSizeCallback(window, size_callback);
        glfwSetKeyCallback(window, key_callback);
        
        while( !glfwWindowShouldClose( window ) ) // infinite loop to draw object again and again
        {   // because once object is draw then window is terminated
            display( window );
            
            if (auto_strech){
                glfwSetWindowSize(window, mode->width, mode->height);
                glfwSetWindowPos(window, 0, 0);
            }
            
            glfwPollEvents();
        }
    } else {
        img_f = (float *) calloc(3 * Width_global * Height_global, sizeof(float));
        render();
        writeToBmp();
        free(img_f);
    }
    return 0;
}
