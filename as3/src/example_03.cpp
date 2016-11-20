#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>
#include <limits>

//include header file for glfw library so that we can use OpenGL
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sstream>
#include "Vec3.h"
#include "Utils.h"

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define CONTROL_POINTS_PER_PATCH 16
#define PI 3.14159265f // Should be used from mathlib

using namespace std;

/*
For UC Berkeley's CS184 Fall 2016 course, assignment 3 (Bezier surfaces)
*/

struct Object {
    bool animate = false;
    bool wire_frame = false;
    bool smooth = true;
    GLfloat scale_co = 1.0f;
    GLfloat translation[3] = {0.0f, 0.0f, 0.0f};
    GLfloat rotation[3] = {0.0f, 0.0f, 0.0f};
    vector<Vec3> triangles, triangle_normals;
};

//****************************************************
// Global Variables
//****************************************************
float p_step = 0.1f;
bool auto_strech = false;
bool adap_subdiv = false;
int Z_buffer_bit_depth = 128;

int Width_global, Height_global;
unsigned long curr_object = 0;
int n_grid_h, n_grid_w;
vector<Object> objects;

//****************************************************
// Simple init function
//****************************************************
void initializeRendering() {
    glfwInit();
}

//****************************************************
// Keyboard inputs. Add things to match the spec! 
//****************************************************
static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (!action) return;

    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case GLFW_KEY_LEFT :
            if (mods == GLFW_MOD_SHIFT)
                objects[curr_object].translation[0] -= 0.01f;
            else
                objects[curr_object].rotation[1] -= 1.0f;
            break;
        case GLFW_KEY_RIGHT:
            if (mods == GLFW_MOD_SHIFT)
                objects[curr_object].translation[0] += 0.01f;
            else
                objects[curr_object].rotation[1] += 1.0f;
            break;
        case GLFW_KEY_UP   :
            if (mods == GLFW_MOD_SHIFT)
                objects[curr_object].translation[1] += 0.01f;
            else
                objects[curr_object].rotation[0] -= 1.0f;
            break;
        case GLFW_KEY_DOWN :
            if (mods == GLFW_MOD_SHIFT)
                objects[curr_object].translation[1] -= 0.01f;
            else
                objects[curr_object].rotation[0] += 1.0f;
            break;
        case GLFW_KEY_KP_ADD:
            objects[curr_object].scale_co += 0.1f;
            break;
        case GLFW_KEY_KP_SUBTRACT:
            if (objects[curr_object].scale_co > 0.1f)
                objects[curr_object].scale_co -= 0.1f;
            break;
        case GLFW_KEY_F:
            if (mods == GLFW_MOD_SHIFT) auto_strech = !auto_strech;
            break;
        case GLFW_KEY_W:
            objects[curr_object].wire_frame = !objects[curr_object].wire_frame;
            break;
        case GLFW_KEY_S:
            objects[curr_object].smooth = !objects[curr_object].smooth;
            break;
        case GLFW_KEY_TAB:
            curr_object = (curr_object + 1) % objects.size();
            break;
        case GLFW_KEY_A:
            objects[curr_object].animate = !objects[curr_object].animate;
            break;

        default:
            break;
    }

}

int is_mouse_down = 0;

void mouse_callback(GLFWwindow *window, int button, int action, int mods) {
    if (action == GLFW_PRESS) {
        is_mouse_down = (button == GLFW_MOUSE_BUTTON_LEFT) + 1;
    } else {
        is_mouse_down = 0;
    }
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    if (xoffset != 0) {
        objects[curr_object].rotation[2] += xoffset;
        return;
    }
    objects[curr_object].scale_co += yoffset * 0.1f;
    if (objects[curr_object].scale_co < 0.1f)
        objects[curr_object].scale_co = 0.1f;
}

void drawTriangle(const Vec3 &a, const Vec3 &b, const Vec3 &c,
                  const Vec3 &na, const Vec3 &nb, const Vec3 &nc) {
    glBegin(GL_POLYGON);

    glNormal3f(na.x, na.y, na.z);
    glVertex3f(a.x, a.y, a.z);

    glNormal3f(nb.x, nb.y, nb.z);
    glVertex3f(b.x, b.y, b.z);

    glNormal3f(nc.x, nc.y, nc.z);
    glVertex3f(c.x, c.y, c.z);

    glEnd();
}

double last_xpos = -1, last_ypos = -1;

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void display(GLFWwindow *window) {
    if (is_mouse_down) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        xpos /= Width_global;
        ypos /= Height_global;
        if (last_xpos >= 0) {
            if (is_mouse_down == 2)
                objects[curr_object].rotation[1] += (xpos - last_xpos) * 360;
            else
                objects[curr_object].translation[0] += (xpos - last_xpos) * 10;
        }
        if (last_ypos >= 0) {
            if (is_mouse_down == 2)
                objects[curr_object].rotation[0] += (ypos - last_ypos) * 360;
            else
                objects[curr_object].translation[1] -= (ypos - last_ypos) * 10;
        }
        last_xpos = xpos;
        last_ypos = ypos;
    } else {
        last_xpos = -1, last_ypos = -1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // clear background screen to black

    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);                // clear the color buffer (sets everything to black)
    glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations

    for (int o = 1; o < objects.size(); ++o) {
        glPushMatrix();
        glLoadIdentity();

        if (objects[o].animate)
            objects[o].rotation[1] += 1;

        glTranslatef(objects[o].translation[0], objects[o].translation[1], objects[o].translation[2]);
        glRotatef(objects[o].rotation[2], 0, 0, 1);
        glRotatef(objects[o].rotation[1], 0, 1, 0);
        glRotatef(objects[o].rotation[0], 1, 0, 0);
        glScalef(objects[o].scale_co, objects[o].scale_co, objects[o].scale_co);

        float mat[] = { 0, 0, 0, 1.0f };
        (o == curr_object ? Vec3(.1f, .2f, .3f) * 2 : Vec3(.1f, .2f, .3f)).toArray(mat);
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat);
        // glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat);

        glPolygonMode(GL_FRONT_AND_BACK, objects[o].wire_frame ? GL_LINE : GL_FILL);
        glShadeModel(objects[o].smooth ? GL_SMOOTH : GL_FLAT);

        for (int i = 0; i < objects[o].triangles.size(); i += 3)
            drawTriangle(objects[o].triangles[i], objects[o].triangles[i + 1], objects[o].triangles[i + 2],
                         objects[o].triangle_normals[i],
                         objects[o].triangle_normals[i + 1],
                         objects[o].triangle_normals[i + 2]);

        glPopMatrix();
    }

    glfwSwapBuffers(window);
}

//****************************************************
// function that is called when window is resized
//***************************************************
void size_callback(GLFWwindow *window, int, int) {
    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);
    int vp_size_w = min(Width_global, Height_global * n_grid_w / n_grid_h);
    int vp_size_h = vp_size_w * n_grid_h / n_grid_w;
    glViewport((Width_global - vp_size_w) / 2, (Height_global - vp_size_h) / 2, vp_size_w, vp_size_h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-.1f, 2 * n_grid_w + .1f, -.1f, 2 * n_grid_h + .1f, -5.0f, 5.0f);

    display(window);
}

void bezCurveInterp(const Vec3 &a, const Vec3 &b, const Vec3 &c, const Vec3 &d,
                    const float u, Vec3 &p, Vec3 &dPdu) {
    float uu = 1.0f - u;
    Vec3 A = a * uu + b * u;
    Vec3 B = b * uu + c * u;
    Vec3 C = c * uu + d * u;
    Vec3 D = A * uu + B * u;
    Vec3 E = B * uu + C * u;
    p = D * uu + E * u;
    dPdu = 3 * (E - D);
}

void bezPatchInterp(Vec3 *patch, const float u, const float v,
                    Vec3 &p, Vec3 &n) {
    Vec3 vCurve[4], uCurve[4], t;
    for (int i = 0; i < 4; ++i) {
        bezCurveInterp(patch[i * 4], patch[i * 4 + 1], patch[i * 4 + 2], patch[i * 4 + 3], u, vCurve[i], t);
        bezCurveInterp(patch[i], patch[i + 4], patch[i + 8], patch[i + 12], v, uCurve[i], t);
    }
    bezCurveInterp(vCurve[0], vCurve[1], vCurve[2], vCurve[3], v, p, t);
    bezCurveInterp(uCurve[0], uCurve[1], uCurve[2], uCurve[3], u, p, n);
    n = n.cross(t).normalize();
}

struct eTri {
    float u[3], v[3];
    Vec3 p[3], pn[3];

    void print() {
        for (int i = 0; i < 3; ++i) {
            printf("index = %d\n", i);
            printf("u, v = %f, %f\n", u[i], v[i]);
            printf("p[%d] = ", i);
            p[i].print();
            printf("pn[%d] = ", i);
            pn[i].print();
        }
        printf("\n");
    }
};

void subDividePatchAdaptive(Vec3 *patch, const float step, Object &obj) {
    vector<eTri> *curr = new vector<eTri>();
    vector<eTri> *prev = new vector<eTri>();

    eTri init_1, init_2;

    init_1.u[0] = init_2.u[0] = 0; // p1
    init_1.u[1] = 1; // p2
    init_1.u[2] = init_2.u[1] = 1; // p3
    init_2.u[2] = 0; // p4

    init_1.v[0] = init_2.v[0] = 1; // p1
    init_1.v[1] = 1; // p2
    init_1.v[2] = init_2.v[1] = 0; // p3
    init_2.v[2] = 0; // p4

    for (int i = 0; i < 3; ++i) {
        bezPatchInterp(patch, init_1.u[i], init_1.v[i], init_1.p[i], init_1.pn[i]);
        bezPatchInterp(patch, init_2.u[i], init_2.v[i], init_2.p[i], init_2.pn[i]);
    }

    curr->push_back(init_1);
    curr->push_back(init_2);

    while (true) {
        eTri mp, tmp;
        bool all_clear = true;
        size_t sz = curr->size();

        for (vector<eTri>::iterator i = curr->begin(); i != curr->end(); ++i) {
            bool split[3];
            memset(split, 0, sizeof(split));
            for (int n = 0; n < 3; ++n) {
                mp.u[n] = (i->u[n] + i->u[(n + 1) % 3]) / 2;
                mp.v[n] = (i->v[n] + i->v[(n + 1) % 3]) / 2;
                Vec3 p = (i->p[n] + i->p[(n + 1) % 3]) / 2;
                bezPatchInterp(patch, mp.u[n], mp.v[n], mp.p[n], mp.pn[n]);
                if (mp.p[n].dist(p) > step)
                    split[n] = true;
            }

            bool add_tri[13];
            memset(add_tri, 0, sizeof(add_tri));
            if (split[0]) {
                all_clear = false;
                if (split[1]) {
                    if (split[2]) {
                        add_tri[0] = add_tri[1] = add_tri[2] = add_tri[3] = true;
                    } else {
                        add_tri[2] = add_tri[4] = add_tri[5] = true;
                    }
                } else {
                    if (split[2]) {
                        add_tri[0] = add_tri[6] = add_tri[7] = true;
                    } else {
                        add_tri[7] = add_tri[8] = true;
                    }
                }
            } else {
                if (split[1]) {
                    all_clear = false;
                    if (split[2]) {
                        add_tri[3] = add_tri[9] = add_tri[10] = true;
                    } else {
                        add_tri[11] = add_tri[4] = true;
                    }
                } else {
                    if (split[2]) {
                        all_clear = false;
                        add_tri[10] = add_tri[12] = true;
                    } else {
                        prev->push_back(*i);
                    }
                }
            }

            if (add_tri[0]) {
                // p1, mid1, mid3
                tmp.u[0] = i->u[0]; tmp.v[0] = i->v[0]; tmp.p[0] = i->p[0]; tmp.pn[0] = i->pn[0];
                tmp.u[1] = mp.u[0]; tmp.v[1] = mp.v[0]; tmp.p[1] = mp.p[0]; tmp.pn[1] = mp.pn[0];
                tmp.u[2] = mp.u[2]; tmp.v[2] = mp.v[2]; tmp.p[2] = mp.p[2]; tmp.pn[2] = mp.pn[2];
                prev->push_back(tmp);
            }
            if (add_tri[1]) {
                // mid1, mid2, mid3
                tmp.u[0] = mp.u[0]; tmp.v[0] = mp.v[0]; tmp.p[0] = mp.p[0]; tmp.pn[0] = mp.pn[0];
                tmp.u[1] = mp.u[1]; tmp.v[1] = mp.v[1]; tmp.p[1] = mp.p[1]; tmp.pn[1] = mp.pn[1];
                tmp.u[2] = mp.u[2]; tmp.v[2] = mp.v[2]; tmp.p[2] = mp.p[2]; tmp.pn[2] = mp.pn[2];
                prev->push_back(tmp);
            }
            if (add_tri[2]) {
                // mid1, p2, mid2
                tmp.u[0] = mp.u[0]; tmp.v[0] = mp.v[0]; tmp.p[0] = mp.p[0]; tmp.pn[0] = mp.pn[0];
                tmp.u[1] = i->u[1]; tmp.v[1] = i->v[1]; tmp.p[1] = i->p[1]; tmp.pn[1] = i->pn[1];
                tmp.u[2] = mp.u[1]; tmp.v[2] = mp.v[1]; tmp.p[2] = mp.p[1]; tmp.pn[2] = mp.pn[1];
                prev->push_back(tmp);
            }
            if (add_tri[3]) {
                // mid2, p3, mid3
                tmp.u[0] = mp.u[1]; tmp.v[0] = mp.v[1]; tmp.p[0] = mp.p[1]; tmp.pn[0] = mp.pn[1];
                tmp.u[1] = i->u[2]; tmp.v[1] = i->v[2]; tmp.p[1] = i->p[2]; tmp.pn[1] = i->pn[2];
                tmp.u[2] = mp.u[2]; tmp.v[2] = mp.v[2]; tmp.p[2] = mp.p[2]; tmp.pn[2] = mp.pn[2];
                prev->push_back(tmp);
            }
            if (add_tri[4]) {
                // mid2, p3, p1
                tmp.u[0] = mp.u[1]; tmp.v[0] = mp.v[1]; tmp.p[0] = mp.p[1]; tmp.pn[0] = mp.pn[1];
                tmp.u[1] = i->u[2]; tmp.v[1] = i->v[2]; tmp.p[1] = i->p[2]; tmp.pn[1] = i->pn[2];
                tmp.u[2] = i->u[0]; tmp.v[2] = i->v[0]; tmp.p[2] = i->p[0]; tmp.pn[2] = i->pn[0];
                prev->push_back(tmp);
            }
            if (add_tri[5]) {
                // mid1, mid2, p1
                tmp.u[0] = mp.u[0]; tmp.v[0] = mp.v[0]; tmp.p[0] = mp.p[0]; tmp.pn[0] = mp.pn[0];
                tmp.u[1] = mp.u[1]; tmp.v[1] = mp.v[1]; tmp.p[1] = mp.p[1]; tmp.pn[1] = mp.pn[1];
                tmp.u[2] = i->u[0]; tmp.v[2] = i->v[0]; tmp.p[2] = i->p[0]; tmp.pn[2] = i->pn[0];
                prev->push_back(tmp);
            }
            if (add_tri[6]) {
                // mid1, p3, mid3
                tmp.u[0] = mp.u[0]; tmp.v[0] = mp.v[0]; tmp.p[0] = mp.p[0]; tmp.pn[0] = mp.pn[0];
                tmp.u[1] = i->u[2]; tmp.v[1] = i->v[2]; tmp.p[1] = i->p[2]; tmp.pn[1] = i->pn[2];
                tmp.u[2] = mp.u[2]; tmp.v[2] = mp.v[2]; tmp.p[2] = mp.p[2]; tmp.pn[2] = mp.pn[2];
                prev->push_back(tmp);
            }
            if (add_tri[7]) {
                // mid1, p2, p3
                tmp.u[0] = mp.u[0]; tmp.v[0] = mp.v[0]; tmp.p[0] = mp.p[0]; tmp.pn[0] = mp.pn[0];
                tmp.u[1] = i->u[1]; tmp.v[1] = i->v[1]; tmp.p[1] = i->p[1]; tmp.pn[1] = i->pn[1];
                tmp.u[2] = i->u[2]; tmp.v[2] = i->v[2]; tmp.p[2] = i->p[2]; tmp.pn[2] = i->pn[2];
                prev->push_back(tmp);
            }
            if (add_tri[8]) {
                // p1, mid1, p3
                tmp.u[0] = i->u[0]; tmp.v[0] = i->v[0]; tmp.p[0] = i->p[0]; tmp.pn[0] = i->pn[0];
                tmp.u[1] = mp.u[0]; tmp.v[1] = mp.v[0]; tmp.p[1] = mp.p[0]; tmp.pn[1] = mp.pn[0];
                tmp.u[2] = i->u[2]; tmp.v[2] = i->v[2]; tmp.p[2] = i->p[2]; tmp.pn[2] = i->pn[2];
                prev->push_back(tmp);
            }
            if (add_tri[9]) {
                // p2, mid2, mid3
                tmp.u[0] = i->u[1]; tmp.v[0] = i->v[1]; tmp.p[0] = i->p[1]; tmp.pn[0] = i->pn[1];
                tmp.u[1] = mp.u[1]; tmp.v[1] = mp.v[1]; tmp.p[1] = mp.p[1]; tmp.pn[1] = mp.pn[1];
                tmp.u[2] = mp.u[2]; tmp.v[2] = mp.v[2]; tmp.p[2] = mp.p[2]; tmp.pn[2] = mp.pn[2];
                prev->push_back(tmp);
            }
            if (add_tri[10]) {
                // p1, p2, mid3
                tmp.u[0] = i->u[0]; tmp.v[0] = i->v[0]; tmp.p[0] = i->p[0]; tmp.pn[0] = i->pn[0];
                tmp.u[1] = i->u[1]; tmp.v[1] = i->v[1]; tmp.p[1] = i->p[1]; tmp.pn[1] = i->pn[1];
                tmp.u[2] = mp.u[2]; tmp.v[2] = mp.v[2]; tmp.p[2] = mp.p[2]; tmp.pn[2] = mp.pn[2];
                prev->push_back(tmp);
            }
            if (add_tri[11]) {
                // p1, p2, mid2
                tmp.u[0] = i->u[0]; tmp.v[0] = i->v[0]; tmp.p[0] = i->p[0]; tmp.pn[0] = i->pn[0];
                tmp.u[1] = i->u[1]; tmp.v[1] = i->v[1]; tmp.p[1] = i->p[1]; tmp.pn[1] = i->pn[1];
                tmp.u[2] = mp.u[1]; tmp.v[2] = mp.v[1]; tmp.p[2] = mp.p[1]; tmp.pn[2] = mp.pn[1];
                prev->push_back(tmp);
            }
            if (add_tri[12]) {
                // p2, p3, mid3
                tmp.u[0] = i->u[1]; tmp.v[0] = i->v[1]; tmp.p[0] = i->p[1]; tmp.pn[0] = i->pn[1];
                tmp.u[1] = i->u[2]; tmp.v[1] = i->v[2]; tmp.p[1] = i->p[2]; tmp.pn[1] = i->pn[2];
                tmp.u[2] = mp.u[2]; tmp.v[2] = mp.v[2]; tmp.p[2] = mp.p[2]; tmp.pn[2] = mp.pn[2];
                prev->push_back(tmp);
            }
        }

        if (all_clear)
            break;

        curr->clear();
        vector<eTri> *t = curr;
        curr = prev;
        prev = t;
    }

    for (vector<eTri>::iterator i = curr->begin(); i != curr->end(); ++i) {
        obj.triangles.push_back(i->p[0]);
        obj.triangles.push_back(i->p[1]);
        obj.triangles.push_back(i->p[2]);

        obj.triangle_normals.push_back(i->pn[0]);
        obj.triangle_normals.push_back(i->pn[1]);
        obj.triangle_normals.push_back(i->pn[2]);
    }

    delete curr;
    delete prev;
}

void subDividePatchUniform(Vec3 *patch, const float step, Object &obj) {
    float u, v;
    int iu, iv, i = 0;
    int nDiv = (int) ceil((1.0f + EPSILON) / step);

    Vec3 *p = new Vec3[nDiv * nDiv];
    Vec3 *n = new Vec3[nDiv * nDiv];

    for (iu = 0; iu < nDiv; ++iu) {
        u = min(step * iu, 1.0f);
        for (iv = 0; iv < nDiv; ++iv) {
            v = min(step * iv, 1.0f);
            bezPatchInterp(patch, u, v, p[i], n[i]);
            ++i;
        }
    }

    for (iu = 0; iu < nDiv - 1; ++iu) {
        for (iv = 0; iv < nDiv - 1; ++iv) {
            i = iu * nDiv + iv;

            obj.triangles.push_back(p[i + 1]);
            obj.triangles.push_back(p[i + nDiv + 1]);
            obj.triangles.push_back(p[i + nDiv]);
            obj.triangle_normals.push_back(n[i + 1]);
            obj.triangle_normals.push_back(n[i + nDiv + 1]);
            obj.triangle_normals.push_back(n[i + nDiv]);

            obj.triangles.push_back(p[i + 1]);
            obj.triangles.push_back(p[i + nDiv]);
            obj.triangles.push_back(p[i]);
            obj.triangle_normals.push_back(n[i + 1]);
            obj.triangle_normals.push_back(n[i + nDiv]);
            obj.triangle_normals.push_back(n[i]);
        }
    }

    delete[] p;
    delete[] n;
}

bool readVec3(Vec3 &v, istream &fin, const string &cmd) {
    if (v.read(fin) == 3)
        return false;
    printf("Missing parameter(s) near command %s.\n", cmd.c_str());
    return true;
}

bool readObjFile(const char *obj_path, Object &obj) {
    Vec3 minV = Vec3().fill(numeric_limits<float>::max());
    Vec3 maxV = -minV;

    ifstream fin(obj_path);
    if (!fin.is_open()) {
        printf("Cannot open %s.\n", obj_path);
        return false;
    }
    vector<Vec3> vertices;
    vector<Vec3> normals;
    string line;
    while (getline(fin, line)) {
        string cmd;
        // printf("Processing %s.\n", line.c_str());
        stringstream line_ss(line);
        line_ss >> cmd;
        if (cmd == "" || cmd[0] == '#' || cmd == "g")
            continue;
        if (cmd == "v") {
            Vec3 v;
            if (readVec3(v, line_ss, cmd + "(" + obj_path + ")"))
                return false;
            vertices.push_back(v);
            minV.keepMin(v);
            maxV.keepMax(v);
        } else if (cmd == "vn") {
            Vec3 v;
            if (readVec3(v, line_ss, cmd + "(" + obj_path + ")"))
                return false;
            normals.push_back(v.normalize());
        } else if (cmd == "f") {
            string s;
            vector<int> vs, vns;
            while (line_ss >> s) {
                stringstream ss(s);
                int i, j = 0;
                if (!(ss >> i) || i < 1 || i > vertices.size())
                    return false;
                char c;
                if (ss >> c && c == '/' && ss >> c && c == '/') {
                    ss >> j;
                    if (j < 1 || j > normals.size())
                        return false;
                }
                vs.push_back(i - 1);
                vns.push_back(j - 1);
            }
            // we assume it's always a convex
            if (vs.size() < 3)
                return false; // hmm it's not a surface
            for (int i = 0; i < vs.size() - 2; ++i) {
                obj.triangles.push_back(vertices[vs[0]]);
                obj.triangles.push_back(vertices[vs[i + 1]]);
                obj.triangles.push_back(vertices[vs[i + 2]]);

                Vec3 n_def = (vertices[vs[i + 2]] - vertices[vs[0]]).cross(vertices[vs[i + 1]] - vertices[vs[0]]).normalize();

                if (vns[0] >= 0)
                    obj.triangle_normals.push_back(normals[vns[0]]);
                else
                    obj.triangle_normals.push_back(n_def);

                if (vns[i + 1] >= 0)
                    obj.triangle_normals.push_back(normals[vns[i + 1]]);
                else
                    obj.triangle_normals.push_back(n_def);

                if (vns[i + 2] >= 0)
                    obj.triangle_normals.push_back(normals[vns[i + 2]]);
                else
                    obj.triangle_normals.push_back(n_def);
            }
        } else {
            // printf("Unrecognized command %s (%s).\n", cmd.c_str(), obj_path);
            // return false;
        }
    }

    Vec3 midV = (minV + maxV) / 2;
    float max_diff = (maxV - minV).getMax();
    if (max_diff > 0) {
        for (vector<Vec3>::iterator tri = obj.triangles.begin(); tri != obj.triangles.end(); ++tri) {
            *tri -= midV;
            *tri *= 2.0f / max_diff;
        }
    }

    return true;
}

bool readBezFile(const char *bez_path, Object &obj) {
    int nPatches = 0;
    Vec3 *control_points = NULL;

    Vec3 minV = Vec3().fill(numeric_limits<float>::max());
    Vec3 maxV = -minV;

    ifstream fin(bez_path);
    if (!fin.is_open()) {
        printf("Cannot open %s.\n", bez_path);
        return false;
    }

    if (!(fin >> nPatches) || nPatches < 1) {
        printf("Wrong number of nPatches in %s.\n", bez_path);
        return false;
    }

    control_points = new Vec3[CONTROL_POINTS_PER_PATCH * nPatches];

    for (int i = 0; i < CONTROL_POINTS_PER_PATCH * nPatches; ++i) {
        if (control_points[i].read(fin) != 3) {
            printf("Cannot read control points in %s.\n", bez_path);
            delete[] control_points;
            return false;
        }
        minV.keepMin(control_points[i]);
        maxV.keepMax(control_points[i]);
    }

    Vec3 midV = (minV + maxV) / 2;
    float max_diff = (maxV - minV).getMax();
    if (max_diff > 0) {
        for (int i = 0; i < CONTROL_POINTS_PER_PATCH * nPatches; ++i) {
            control_points[i] -= midV;
            control_points[i] *= 2.0f / max_diff;
        }
    }

    if (adap_subdiv) {
        for (int i = 0; i < nPatches; ++i) {
            // printf("patch id = %d\n", i);
            subDividePatchAdaptive(control_points + i * CONTROL_POINTS_PER_PATCH, p_step, obj);
        }
    } else {
        for (int i = 0; i < nPatches; ++i)
            subDividePatchUniform(control_points + i * CONTROL_POINTS_PER_PATCH, p_step, obj);
    }

    delete[] control_points;
    return true;
}

//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
    char *output_path = NULL;
    objects.push_back(Object()); // dummy object

    vector<char*> bez_path, obj_path;

    for (int i = 1; i < argc; ++i) {
        // printf("cmd = %s\n", argv[i]);

        if (!strcmp(argv[i], "-a")) {
            adap_subdiv = true;
            continue;
        }

        if (!strcmp(argv[i], "-o")) {
            if (++i < argc)
                output_path = argv[i];
            continue;
        }

        size_t len = strlen(argv[i]);
        if (len >= 4) {
            if (!strcmp(argv[i] + len - 4, ".bez")) {
                bez_path.push_back(argv[i]);
                continue;
            } else if (!strcmp(argv[i] + len - 4, ".obj")) {
                obj_path.push_back(argv[i]);
                continue;
            }
        }

        float n_value;
        if (sscanf(argv[i], "%f", &n_value)) {
            p_step = n_value;
            continue;
        }

        printf("Unrecognized command: %s\n", argv[i]);
    }

    for (int i = 0; i < bez_path.size(); ++i) {
    	Object obj;
        if (readBezFile(bez_path[i], obj))
            objects.push_back(obj);
    }

    for (int i = 0; i < obj_path.size(); ++i) {
    	Object obj;
        if (readObjFile(obj_path[i], obj))
            objects.push_back(obj);
    }

    if (objects.size() < 2) {
        printf("Nothing to render.\n");
        return -1;
    }

    if (output_path != NULL) {
        ofstream fout(output_path);
        if (!fout.is_open()) {
            printf("Cannot open %s for output.\n", output_path);
            return -1;
        }

        Object *curr = &objects[1];
        for (int i = 0; i < curr->triangles.size(); ++i) {
            fout << "v " << curr->triangles[i] << endl;
            fout << "vn " << curr->triangle_normals[i] << endl;
        }

        for (int i = 0; i < curr->triangles.size(); i += 3)
            fout << "f " << i + 1 << " " << i + 2 << " " << i + 3 << endl;

        return 0;
    }

    n_grid_w = (int) ceil(sqrt(objects.size() - 1));
    n_grid_h = (int) ceil((double) (objects.size() - 1) / n_grid_w);

    float left_extra = 2.0f * (n_grid_w - (objects.size() - 1) % n_grid_w) / ((objects.size() - 1) % n_grid_w + 1);
    for (int i = 1; i < objects.size(); ++i) {
        objects[i].translation[0] = 1 + (i - 1) % n_grid_w * 2;
        if (objects.size() <= ceil((float) i / n_grid_w) * n_grid_w)
            objects[i].translation[0] += left_extra * ((i - 1) % n_grid_w + 1);
        objects[i].translation[1] = 1 + (n_grid_h - 1 - (i - 1) / n_grid_w) * 2;
    }

    //This initializes glfw
    initializeRendering();

    Width_global = 400 * n_grid_w;
    Height_global = 400 * n_grid_h;
    GLFWwindow *window = glfwCreateWindow(Width_global, Height_global, "CS184", NULL, NULL);
    if (!window) {
        cerr << "Error on window creating" << endl;
        glfwTerminate();
        return -1;
    }

    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if (!mode) {
        cerr << "Error on getting monitor" << endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Get the pixel coordinate of the window
    // it returns the size, in pixels, of the framebuffer of the specified window
    glfwGetFramebufferSize(window, &Width_global, &Height_global);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // glOrtho(-3.5, 3.5, -3.5, 3.5, 5, -5);
    glOrtho(-.1f, 2 * n_grid_w + .1f, -.1f, 2 * n_grid_h + .1f, -5.0f, 5.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_DEPTH_TEST);    // enable z-buffering
    glDepthFunc(GL_LESS);

    float light_position[] = { 0, 0, -10, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    glfwSetWindowTitle(window, "Yet Another 3D Object Viewer");
    glfwSetWindowSizeCallback(window, size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    while (!glfwWindowShouldClose(window)) // infinite loop to draw object again and again
    {   // because once object is draw then window is terminated
        display(window);

        if (auto_strech) {
            glfwSetWindowSize(window, mode->width, mode->height);
            glfwSetWindowPos(window, 0, 0);
        }

        glfwPollEvents();
    }

    return 0;
}
