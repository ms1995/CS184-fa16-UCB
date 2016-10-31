#include <cstdio>
#include <vector>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <sstream>

#include "libjpeg/jpeglib.h"
#include "Vec3.h"
#include "Light.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Utils.h"

#define SAMP_N 2
#define RAYTRACE_TIMES 10

using namespace std;

Vec3 *scene;
int *ray_count;
mutex *scene_mtx;
int SCENE_WIDTH, SCENE_HEIGHT;
bool anti_aliasing = false, use_aabb = false;

vector<Sphere> spheres;
vector<Triangle> triangles;
vector<Light> lights;
vector<Image> textures;

double rlfo = 1.0;
Vec3 E(0, 0, 1), UL(-1, 1, -1), UR(1, 1, -1), LR(1, -1, -1), LL(-1, -1, -1);

inline void pixelAddColor(int i, int j, Vec3 v) {
    i = getbound(i, 0, SCENE_WIDTH - 1);
    j = getbound(j, 0, SCENE_HEIGHT - 1);

    Vec3 *p = scene + j * SCENE_WIDTH + i;
    mutex *mtx = scene_mtx + j * SCENE_WIDTH + i;

    mtx->lock();
    *p += v;
    ++ray_count[j * SCENE_WIDTH + i];
    mtx->unlock();
}

// x, y IN [-1.0, 1.0]
void pixelAddColor(double x, double y, Vec3 v) {
    x = (x + 1.0) / 2 * SCENE_WIDTH;
    y = (y + 1.0) / 2 * SCENE_HEIGHT;

    int i = (int) floor(x);
    int j = (int) floor(y);

    if (false /* anti_aliasing */) {
        int minX = getmax(i - SAMP_N / 2, 0);
        int maxX = getmin(minX + SAMP_N, SCENE_WIDTH);
        int minY = getmax(j - SAMP_N / 2, 0);
        int maxY = getmin(minY + SAMP_N, SCENE_HEIGHT);

        /*
        double vx[SAMP_N * SAMP_N];
        for (i = minX; i < maxX; ++i)
            for (j = minY; j < maxY; ++j)
                vx[(i - minX) * SAMP_N + (j - minY)] = 1.0 / (1.0 + ssqr(x - i - 0.5, y - j - 0.5));

        double sum = 0;
        for (i = 0; i < SAMP_N * SAMP_N; ++i)
            sum += vx[i];
        */

        for (i = minX; i < maxX; ++i)
            for (j = minY; j < maxY; ++j)
                pixelAddColor(i, j, v); // pixelAddColor(i, j, v * (vx[(i - minX) * SAMP_N + (j - minY)] / sum));
    } else {
        pixelAddColor(i, j, v);
    }
}

bool readFromJpeg(const char *read_path, Image &img) {
    int channels;
    unsigned char *rowptr[1];
    struct jpeg_decompress_struct info;
    struct jpeg_error_mgr err;

    FILE *file = fopen(read_path, "rb");

    info.err = jpeg_std_error(&err);
    jpeg_create_decompress(&info);

    if (!file)
        return false;

    jpeg_stdio_src(&info, file);
    jpeg_read_header(&info, TRUE);
    jpeg_start_decompress(&info);

    img.w = info.output_width;
    img.h = info.output_height;
    channels = info.num_components;
    if (channels != 3) // alpha channel not supported yet
        return false;

    int size = info.output_width * info.output_height;
    img.data = new Vec3[size];
    unsigned char *data = new unsigned char[size * 3];
    while (info.output_scanline < info.output_height) {
        rowptr[0] = data + 3 * info.output_width * info.output_scanline;
        jpeg_read_scanlines(&info, rowptr, 1);
    }
    for (int i = 0; i < size; ++i)
        img.data[i] = Vec3(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]) / 255;
    delete[] data;

    jpeg_finish_decompress(&info);
    jpeg_destroy_decompress(&info);
    fclose(file);

    return true;
}

void writeToJpeg(const char *save_path) {
    int i;
    double color_max = 0.0;
    unsigned char *img = (unsigned char *) calloc(1, SCENE_WIDTH * SCENE_HEIGHT * 3);
/*
    for (i = 0; i < SCENE_WIDTH * SCENE_HEIGHT; ++i) {
        color_max = getmax(color_max, scene[i].x);
        color_max = getmax(color_max, scene[i].y);
        color_max = getmax(color_max, scene[i].z);
    }

    for (i = 0; i < SCENE_WIDTH * SCENE_HEIGHT; ++i) {
        img[i * 3] = (unsigned char) floor(scene[i].x / color_max * 255);
        img[i * 3 + 1] = (unsigned char) floor(scene[i].y / color_max * 255);
        img[i * 3 + 2] = (unsigned char) floor(scene[i].z / color_max * 255);
    }
*/

    for (i = 0; i < SCENE_WIDTH * SCENE_HEIGHT; ++i) {
        color_max = getmax(scene[i].x, scene[i].y);
        color_max = getmax(color_max, scene[i].z);
        if (color_max < 1.0)
            color_max = 1.0;
        img[i * 3] = getmin((unsigned char) floor(scene[i].x * 255 / color_max), (unsigned char) 255);
        img[i * 3 + 1] = getmin((unsigned char) floor(scene[i].y * 255 / color_max), (unsigned char) 255);
        img[i * 3 + 2] = getmin((unsigned char) floor(scene[i].z * 255 / color_max), (unsigned char) 255);
    }

    FILE *outfile = fopen(save_path, "wb");
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    cinfo.image_width = SCENE_WIDTH;
    cinfo.image_height = SCENE_HEIGHT;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, 100, TRUE);
    jpeg_start_compress(&cinfo, TRUE);

    JSAMPROW row_pointer;
    while (cinfo.next_scanline < cinfo.image_height) {
        row_pointer = (JSAMPROW) (img + 3 * cinfo.image_width * cinfo.next_scanline);
        jpeg_write_scanlines(&cinfo, &row_pointer, 1);
    }
    jpeg_finish_compress(&cinfo);

    fclose(outfile);
    free(img);
}

bool testAABB(Vec3 min_v, Vec3 max_v, const Vec3 &pE, const Vec3 &pP) {
    min_v -= pE;
    max_v -= pE;
    // Should just be fine when divided by zero?
    return (isGTZero(min_v.x / pP.x) || isGTZero(max_v.x / pP.x)) &&
           (isGTZero(min_v.y / pP.y) || isGTZero(max_v.y / pP.y)) &&
           (isGTZero(min_v.z / pP.z) || isGTZero(max_v.z / pP.z));
}

bool getFirstHit(const Vec3 &pE, const Vec3 &pP, double &min_t, Vec3 &normal, Vec3 &newE,
                 Material &material, const bool stopFirst = false) {
    int i;
    double t;
    Vec3 v, e, txc;
    bool is_hit = false;

    min_t = numeric_limits<double>::max();

    for (i = 0; i < spheres.size(); ++i)
        if ((!use_aabb || testAABB(spheres[i].min_v, spheres[i].max_v, pE, pP)) &&
            spheres[i].isHit(pE, pP, t, v, e) && t < min_t) {
            min_t = t;
            normal = v;
            newE = e;
            is_hit = true;
            material = spheres[i].material;
            if (stopFirst)
                goto success;
        }

    for (i = 0; i < triangles.size(); ++i)
        if ((!use_aabb || testAABB(triangles[i].min_v, triangles[i].max_v, pE, pP)) &&
            triangles[i].isHit(pE, pP, t, v, e, txc) && t < min_t) {
            min_t = t;
            normal = v;
            newE = e;
            is_hit = true;
            material = triangles[i].material;
            if (triangles[i].texture.data != NULL)
                material.ka = txc;
            if (stopFirst)
                goto success;
        }

    success:
    return is_hit;
}

bool isHitByAnyObject(const Vec3 &pE, const Vec3 &pP, double &t) {
    Material material;
    Vec3 normal, newE;
    return getFirstHit(pE, pP, t, normal, newE, material, true);
}

Vec3 PhoneShading(Vec3 v, Vec3 n, Vec3 p, Material &material, Vec3 xl, Vec3 xlp) {
    Vec3 ret;

    for (int i = 0; i < lights.size(); ++i) {
        Vec3 l, color;
        double t, max_t = 0;

        if (lights[i].type == Directional) {
            l = -lights[i].pos;
            max_t = numeric_limits<double>::max();
            color = lights[i].color;
        } else if (lights[i].type == Point) {
            l = lights[i].pos - p;
            max_t = 1.0;
            color = lights[i].color / (1.0 + (lights[i].falloff == 0 ? 0 : pow(l * l, lights[i].falloff / 2)));
        } else if (lights[i].type == Ambient) {
            ret += lights[i].color.scale(material.ka);
            continue;
        } else {
            // l = xlp;
            // color = xl;
            continue;
        }

        if (isHitByAnyObject(p, l, t) && t < max_t)
            continue;

        l.normalize();

        Vec3 r = (2 * l * n) * n - l;
        r.normalize();

        Vec3 Y(0, 1.0, 0);
        Vec3 H = (v + l).normalize();
        Vec3 V = Y - (n * Y) * n;
        V.normalize();
        Vec3 U = V.cross(n).normalize();

        double a = material.spu * sqr(H * U) + material.spv * sqr(H * V), b = 1 - sqr(n * H), puv = 0;
        if (a > 0 && b > 0) puv = a / b;
        double LdotN = l * n, RdotV = r * v;
        ret += color.scale(
                material.ka +
                material.kd * getmax(LdotN, 0) +
                material.ks * (LdotN >= 0 && RdotV > 0 ? pow(getmax(RdotV, 0), puv) : 0)
        );
    }

    return ret + xl.scale(material.kr * (xlp * n > 0)); // Only consider reflections on the positive side.
}

Vec3 raytracer(Vec3 pE, Vec3 pP, int step) {
    if (step <= 0)
        return Vec3();

    double min_t;
    Vec3 normal, newE;
    Material material;

    if (getFirstHit(pE, pP, min_t, normal, newE, material)) {
        Vec3 newP = pP - 2 * (pP * normal) * normal;
        return PhoneShading(-pP, normal, newE, material,
                            material.kr.allZeros() ? Vec3() : raytracer(newE, newP.normalize(), step - 1),
                            newP) / (1.0 + (rlfo == 0 ? 0 : pow((newE - pE) * (newE - pE), rlfo / 2)));
    }

    return Vec3();
}

void render(double x, double y) {
    if (x < -1.0 || y < -1.0 || x >= 1.0 || y >= 1.0)
        return;

    double u = (x + 1.0) / 2, v = (y + 1.0) / 2;
    Vec3 P = (1 - u) * (v * LL + (1 - v) * UL) + u * (v * LR + (1 - v) * UR);
    Vec3 d = P - E;
    d.normalize();

    pixelAddColor(x, y, raytracer(P, d, RAYTRACE_TIMES));
}

struct samplerTask {
    double minX, maxX, Xstep, Ystep;
};

void *sampleWorker(void *p) {
    samplerTask *task = (samplerTask *) p;
    double x, y;
    for (x = task->minX; x < task->maxX; x += task->Xstep)
        for (y = -1.0 + task->Ystep / 2; y < 1.0; y += task->Ystep)
            render(x + task->Ystep * (rand() % 101 - 100) / 100,
                   y + task->Ystep * (rand() % 101 - 100) / 100);
    return NULL;
}

void uniformSampler(double step, int thread_count) {
    if (thread_count < 1)
        thread_count = 1;
    pthread_t *worker_threads = new pthread_t[thread_count];
    samplerTask *worker_tasks = new samplerTask[thread_count];

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    int i;
    for (i = 0; i < thread_count; ++i) {
        worker_tasks[i].Xstep = step * thread_count;
        worker_tasks[i].Ystep = step;
        worker_tasks[i].minX = i * step + step / 2 - 1.0;
        worker_tasks[i].maxX = 1.0;
        if (pthread_create(worker_threads + i, &attr, sampleWorker, (void *) (worker_tasks + i))) {
            printf("Cannot create worker thread #%d.\n", i);
            goto done;
        }
    }
    pthread_attr_destroy(&attr);

    void *status;
    for (i = 0; i < thread_count; ++i)
        if (pthread_join(worker_threads[i], &status))
            printf("Cannot join worker thread #%d.\n", i);

    done:
    delete[] worker_threads;
    delete[] worker_tasks;

    for (i = 0; i < SCENE_WIDTH * SCENE_HEIGHT; ++i)
        if (ray_count[i])
            scene[i] = scene[i] / ray_count[i];
}

bool readVec3(Vec3 &v, istream &fin, const string &cmd) {
    if (v.read(fin) == 3)
        return false;
    printf("Missing parameter(s) near command %s.\n", cmd.c_str());
    return true;
}

bool readObjFile(const char *obj_path, const Material &mat, const Mat4 &trans) {
    ifstream fin(obj_path);
    if (!fin.is_open())
        return false;
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
        } else if (cmd == "vn") {
            Vec3 v;
            if (readVec3(v, line_ss, cmd + "(" + obj_path + ")"))
                return false;
            normals.push_back(v);
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
                Triangle t(vertices[vs[0]], vertices[vs[i+1]], vertices[vs[i+2]], mat);
                t.setTransMat(trans);
                if (vns[0] >= 0)
                    t.na = normals[vns[0]];
                if (vns[i+1] >= 0)
                    t.nb = normals[vns[i+1]];
                if (vns[i+2] >= 0)
                    t.nc = normals[vns[i+2]];
                triangles.push_back(t);
            }
        } else {
            printf("Unrecognized command %s (%s).\n", cmd.c_str(), obj_path);
            // return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    srand((unsigned int) time(NULL));

    // Scene file start
    Mat4 curr_trans;
    Material curr_mat;
    int thread_num = 1, quality = 1;
    string output_fn = "output.jpg";

    if (argc < 2 || !argv[1]) {
        printf("Please specify the scene file.\n");
        return -1;
    }

    ifstream fin(argv[1]);
    if (!fin.is_open()) {
        printf("Cannot read scene file.\n");
        return -2;
    }

    // ifstream fin("scene");
    string line;
    while (getline(fin, line)) {
        string cmd;
        stringstream line_ss(line);
        line_ss >> cmd;
        // printf("cmd = %s\n", cmd.c_str());
        if (cmd == "")
            continue;
        if (cmd == "cam") {
            if (readVec3(E, line_ss, cmd)  ||
                readVec3(LL, line_ss, cmd) ||
                readVec3(LR, line_ss, cmd) ||
                readVec3(UL, line_ss, cmd) ||
                readVec3(UR, line_ss, cmd))
                return -3;
            E = curr_trans * E;
            LL = curr_trans * LL;
            LR = curr_trans * LR;
            UL = curr_trans * UL;
            UR = curr_trans * UR;
        } else if (cmd == "sph") {
            Vec3 r;
            if (readVec3(r, line_ss, cmd))
                return -4;
            double R;
            if (line_ss >> R)
                spheres.push_back(Sphere(r, R, curr_mat).setTransMat(curr_trans));
            else {
                printf("Missing radius near command %s.\n", cmd.c_str());
                return -5;
            }
        } else if (cmd == "tri") {
            Vec3 a, b, c;
            if (readVec3(a, line_ss, cmd) ||
                readVec3(b, line_ss, cmd) ||
                readVec3(c, line_ss, cmd))
                return -6;
            Triangle t = Triangle(a, b, c, curr_mat).setTransMat(curr_trans);
            Vec3 txt_x, txt_y;
            if (txt_x.read(line_ss) == 3 && txt_y.read(line_ss) == 3) {
                if (textures.size())
                    t.setTexture(textures[textures.size() - 1], txt_x, txt_y);
                else {
                    printf("No usable texture available near command %s.\n", cmd.c_str());
                    return -6;
                }
            }
            triangles.push_back(t);
        } else if (cmd == "obj") {
            string obj_path;
            if (line_ss >> obj_path) {
                if (!readObjFile(obj_path.c_str(), curr_mat, curr_trans))
                    return -7;
            } else {
                printf("Missing .obj file path near command %s.\n", cmd.c_str());
                return -7;
            }
        } else if (cmd == "ltp") {
            Vec3 l, c;
            if (readVec3(l, line_ss, cmd) || readVec3(c, line_ss, cmd))
                return -8;
            int fo = 0;
            line_ss >> fo;
            lights.push_back(Light(l, c, Point, fo));
        } else if (cmd == "ltd") {
            Vec3 l, c;
            if (readVec3(l, line_ss, cmd) || readVec3(c, line_ss, cmd))
                return -9;
            lights.push_back(Light(l, c, Directional));
        } else if (cmd == "lta") {
            Vec3 c;
            if (readVec3(c, line_ss, cmd))
                return -10;
            lights.push_back(Light(c));
        } else if (cmd == "mat") {
            if (readVec3(curr_mat.ka, line_ss, cmd) ||
                    readVec3(curr_mat.kd, line_ss, cmd) ||
                    readVec3(curr_mat.ks, line_ss, cmd))
                return -11;
            string s;
            if (!(line_ss >> s)) {
                printf("Missing sp near command %s.\n", cmd.c_str());
                return -12;
            }
            stringstream ss(s);
            double sp;
            if (!(ss >> sp) || sp < 0) {
                printf("Invalid sp near command %s.\n", cmd.c_str());
                return -12;
            }
            char c;
            if (ss >> c && c == ',') {
                curr_mat.spu = sp;
                if (!(ss >> curr_mat.spv) || curr_mat.spv < 0) {
                    printf("Invalid spv near command %s.\n", cmd.c_str());
                    return -12;
                }
            } else {
                curr_mat.spu = curr_mat.spv = sp;
            }
            if (readVec3(curr_mat.kr, line_ss, cmd))
                return -13;
        } else if (cmd == "xft") {
            Vec3 x;
            if (readVec3(x, line_ss, cmd))
                return -14;
            curr_trans = curr_trans * getTranslationMatrix(x);
        } else if (cmd == "xfr") {
            Vec3 x;
            if (readVec3(x, line_ss, cmd))
                return -14;
            curr_trans = curr_trans * getRotationMatrix(x);
        } else if (cmd == "xfs") {
            Vec3 x;
            if (readVec3(x, line_ss, cmd))
                return -14;
            curr_trans = curr_trans * getScalingMatrix(x);
        } else if (cmd == "xfz") {
            curr_trans = Mat4();
        } else if (cmd == "thread") {
            if (!(line_ss >> thread_num) || thread_num < 1) {
                printf("Invalid thread number near command %s.\n", cmd.c_str());
                return -15;
            }
        } else if (cmd == "quality") {
            if (!(line_ss >> quality) || quality < 1) {
                printf("Invalid quality value near command %s.\n", cmd.c_str());
                return -19;
            }
        } else if (cmd == "rlfo") {
            if (!(line_ss >> rlfo) || rlfo < 0) {
                printf("Invalid reflection light falloff coefficient near command %s.\n", cmd.c_str());
                return -16;
            }
        } else if (cmd == "anti-aliasing") {
            anti_aliasing = true;
        } else if (cmd == "use-aabb") {
            use_aabb = true;
        } else if (cmd == "texture") {
            string texture_input;
            if (line_ss >> texture_input) {
                Image txt;
                if (readFromJpeg(texture_input.c_str(), txt)) {
                    textures.push_back(txt);
                    // printf("Texture added, w = %d, h = %d\n", txt.w, txt.h);
                } else {
                    printf("Cannot read texture file near command %s.\n", cmd.c_str());
                    return -18;
                }
            } else {
                printf("Missing texture file path near command %s.\n", cmd.c_str());
                return -17;
            }
        } else if (cmd == "fn") {
            if (!(line_ss >> output_fn)) {
                printf("Missing output file path near command %s.\n", cmd.c_str());
                return -18;
            }
        } else {
            printf("Unrecognized command %s.\n", cmd.c_str());
            // return -100;
        }
    }

    // Scene file stop
    SCENE_WIDTH = SCENE_HEIGHT = 800 * quality;
    ray_count = new int[SCENE_WIDTH * SCENE_HEIGHT]();
    scene = new Vec3[SCENE_WIDTH * SCENE_HEIGHT]();
    scene_mtx = new mutex[SCENE_WIDTH * SCENE_HEIGHT]();

    uniformSampler(1.0 / 800 / quality / (anti_aliasing ? SAMP_N : 1.0), thread_num);
    writeToJpeg(output_fn.c_str());

    delete[] ray_count;
    delete[] scene;
    delete[] scene_mtx;
    for (int i = 0; i < textures.size(); ++i)
        delete[] textures[i].data;

    return 0;
}
