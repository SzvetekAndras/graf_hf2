//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!!
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Szvétek-Müller András
// Neptun : JNDSHB
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const char *const vertexSource = R"(#version 330

in vec4 vertexPosition;

out vec2 texCoord;

void main() {
  gl_Position = vertexPosition;
  texCoord = vertexPosition.xy * 0.5 + 0.5;
}
)";

const char *const fragmentSource = R"(
#version 330 core

const float PI = 3.141592654;
in vec2 texCoord;
uniform float frame;
uniform mat4 viewMat;
out vec4 fragColor;
struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Hit {
    float t;
    vec3 normal;
    vec3 color;
};

Hit noHit() {
    Hit res;
    res.t = -1.0;
    return res;
}

Hit combine(Hit h1, Hit h2) {
    if (h1.t < 0.0 && h2.t < 0.0) {
        return noHit();
    } else if (h1.t < 0.0) {
        return h2;
    } else if (h2.t < 0.0) {
        return h1;
    }
    if (h1.t < h2.t)
    return h1;
    return h2;
}
mat4 perspectiveProjection(float fov, float aspect, float near, float far) {
    float inverseTanFovHalf = 1.0 / tan(fov / 2.0);

    return mat4(
    inverseTanFovHalf / aspect, 0, 0, 0,
    0, inverseTanFovHalf, 0, 0,
    0, 0, -(far + near) / (far - near), -1,
    0, 0, -2.0 * far * near / (far - near), 0
    );
}

mat4 lookatthis(vec3 eye, vec3 target) {
    vec3 forward = normalize(eye - target);
    vec3 right = normalize(cross(vec3(0, 1, 0), forward));
    vec3 up = normalize(cross(forward, right));
    return mat4(
    right, dot(right, eye),
    up, dot(up, eye),
    forward, dot(forward, eye),
    0, 0, 0, 1
    );
}

Hit boxIntersection( in vec3 ro, in vec3 rd, vec3 boxSize, vec3 color)
{
    vec3 m = 1.0/rd; // can precompute if traversing a set of aligned boxes
    vec3 n = m*ro;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(m)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
    if( tN>tF || tF<0.0) return noHit(); // no intersection
    vec3 outNormal = -sign(rd)*step(t1.yzx,t1.xyz)*step(t1.zxy,t1.xyz);
    return Hit(tN, outNormal, color);
}
Hit sphereIntersection(Ray ray, vec3 center, float radius, vec3 color) {
    vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0 * dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;
    float disc = b * b - 4.0 * a * c;
    if (disc < 0.0) {
        return noHit();
    }
    float t = (-b - sqrt(disc)) / (2.0 * a);
    vec3 hitPos = ray.origin + ray.direction * t;
    return Hit(t, normalize(hitPos - center), color);
}
Hit paraboloidIntersection(Ray ray, vec3 center, float radius, float height, vec3 color) {
    float a = dot(ray.direction.xz, ray.direction.xz);
    float b = dot(2.0 *ray.origin.xz, ray.direction.xz)-ray.direction.y;
    float c = dot(ray.origin.xz, ray.origin.xz) - ray.origin.y ;
    float disc = b * b - 4.0 * a * c;
    if (disc < 0.0) {
        return noHit();
    }
    float t1 = (-b - sqrt(disc)) / (2.0 * a);
    float t2 = (-b + sqrt(disc)) / (2.0 * a);
    vec3 hitPos1 = ray.origin + ray.direction * t1;
    vec3 hitPos2 = ray.origin + ray.direction * t2;
    vec3 diff1 = hitPos1;
    vec3 diff2 = hitPos2;
    if (diff1.y < 0.0 || diff1.y > height)
    t1 = -1.0;
    if (diff2.y < 0.0 || diff2.y > height)
    t2 = -1.0;
    diff1.y = 0.0;
    diff2.y = 1.0;
    vec3 n1hp1= vec3(1,2*hitPos1.x,0);
     vec3 n1hp2= vec3(0,2*hitPos1.z,1);

    vec3 n2hp1= vec3(1,2*hitPos2.x,0);
    vec3 n2hp2= vec3(0,2*hitPos2.z,1);
    vec3 n1= cross(n1hp1,n1hp2);
    vec3 n2= cross(n2hp1,n2hp2);
    return combine(
    Hit(t1, normalize(n1), color),
    Hit(t2, normalize(n2), color)
    );
}
Hit cylinderIntersection(Ray ray, vec3 center, float radius, float height, vec3 color) {
    vec2 oc = ray.origin.xz - center.xz;
    float a = dot(ray.direction.xz, ray.direction.xz);
    float b = 2.0 * dot(oc, ray.direction.xz);
    float c = dot(oc, oc) - radius * radius;
    float disc = b * b - 4.0 * a * c;
    if (disc < 0.0) {
        return noHit();
    }
    float t1 = (-b - sqrt(disc)) / (2.0 * a);
    float t2 = (-b + sqrt(disc)) / (2.0 * a);
    vec3 hitPos1 = ray.origin + ray.direction * t1;
    vec3 hitPos2 = ray.origin + ray.direction * t2;
    vec3 diff1 = hitPos1 - center;
    vec3 diff2 = hitPos2 - center;
    if (diff1.y < 0.0 || diff1.y > height)
    t1 = -1.0;
    if (diff2.y < 0.0 || diff2.y > height)
    t2 = -1.0;
    diff1.y = 0.0;
    diff2.y = 0.0;
    return combine(
    Hit(t1, normalize(diff1), color),
    Hit(t2, normalize(diff2), color)
    );
}
Hit cylindertopIntersection(Ray ray, vec3 point, float radius,vec3 normal, vec3 color) {
    float t = dot(point - ray.origin, normal) / dot(ray.direction, normal);
    vec3 hitPos = ray.origin + ray.direction * t;
    if (dot(hitPos - point,hitPos - point) > radius) return noHit();
    return Hit(
        t,
        normal,
        color
    );
}

Hit planeIntersect(Ray ray, vec3 point, vec3 normal, vec3 color) {
    return Hit(
    dot(point - ray.origin, normal) / dot(ray.direction, normal),
    normal,
    color
    );
}

//vec3 AMBIENT = vec3(0, 0, 0);
vec3 AMBIENT = vec3(0.2, 0.3, 0.5);
mat3 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;

    return mat3(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c);
}
Ray transformRay(Ray ray, vec3 translate, mat3 transform) {
    ray.origin = transform * (ray.origin + translate);
    ray.direction = transform * ray.direction;
    return ray;
}

Hit sceneHit(Ray ray, float iTime) {
    vec3 normal = vec3(0, 1, 0);
    Hit planeHit = planeIntersect(ray, vec3(0, -4, 0), normal, vec3(0.2, 0.5, 0.2));
    Hit cylHitbase = cylinderIntersection(ray, vec3(0,  -4, 0), 4.0, 1, vec3(0.68627, 0.70, 0.717647));
    Hit sphereHit = sphereIntersection(ray, vec3(0, -3.1, 0), 1.25, vec3(0.28627, 0.40, 0.417647));
    Hit planeHit2 = cylindertopIntersection(ray, vec3(0, -3, 0), 16.0, normal, vec3(0.6, 0.30, 0.6));
   // Hit buttonHit =sphereIntersection(ray, vec3(2, -3.1, 2), 0.25, vec3(0, 1, 0.0));
    float a = iTime*1.5;
    float a2 = iTime*2;
     float a3 = iTime*3;
    mat3 rotMat = rotationMatrix(vec3(0.5,1,0.3),a);
 mat3 rotMat2=rotationMatrix(vec3(1,1,0.5),a2);

    Ray rotatedRay = transformRay(ray, vec3(0, 3, 0),  rotMat);
    Hit cylHit = cylinderIntersection(rotatedRay, vec3(0,  1, 0), 0.35, 3.0,  vec3(0.68627, 0.70, 0.717647));
    cylHit.normal *= rotMat;

    Hit sphere2Hit = sphereIntersection(rotatedRay, vec3(0, 4, 0), 0.5, vec3(0.38627, 0.50, 0.517647));
    sphere2Hit.normal *= rotMat;



    Ray rotatedRay2 = transformRay(rotatedRay, vec3(0, -4, 0), rotMat2);
    Hit cylHit2 = cylinderIntersection(rotatedRay2, vec3(0, 0, 0), 0.25, 4.0,  vec3(0.68627, 0.70, 0.717647));
    cylHit2.normal *= rotMat2 * rotMat;

    Hit sphere3Hit = sphereIntersection(rotatedRay2, vec3(0, 4, 0), 0.75, vec3(0.38627, 0.50, 0.517647));
    sphere3Hit.normal *= rotMat2* rotMat;


        mat3 rotMat3=rotationMatrix(vec3(0,0.5,0.5),a3*2);
         Ray rotatedRay3 = transformRay(rotatedRay2, vec3(0, -4, 0), rotMat3);
        Hit paraboloidHit = paraboloidIntersection(rotatedRay3, vec3(0, -1, 0), 0, 4.0, vec3(0.6, 0.30, 0.6));
        paraboloidHit.normal*= rotMat3*rotMat2* rotMat;

  /*mat3 rotMatlit = rotationMatrix(vec3(0.5,1,0.3),iTime*1.5);
    mat3 rotMatlit2=rotationMatrix(vec3(1,1,0.5),iTime*2);
    mat3 rotMatl = rotationMatrix(vec3(0.0,1,0.0),iTime*2.5);

  //vec3 lightPos2 =  rotMat2*vec3(0, 1, 0)+vec3(0, 1, 0) *rotMat;//itt adddom meg a helyét
    vec3 lightPos2 =(rotMat2*((rotMat*vec3(0,1.5,0))+vec3(0,2.9,0)))+vec3(0, -5.5, 0);
    -rotMatlit*
+vec3(0, -2, 0)
+vec3(0, 3.7, 0)+vec3(0, 1, 0)
  */
mat3 rotMatlit = rotationMatrix(vec3(0.5,1,0.3),iTime*1.5);
mat3 rotMatlit2=rotationMatrix(vec3(1,1,0.5),iTime*2);
mat3 rotMatlit3=rotationMatrix(vec3(0,0.5,0.5),iTime*6);
 Hit sphereHitlight = sphereIntersection(ray,
 ((vec3(0,0.4, 0)*rotMatlit3*rotMatlit2*rotMatlit)+vec3(0, 0, 0))+
                    ((vec3(0,4, 0)*rotMatlit2*rotMatlit)+vec3(0, 0, 0))+
                    ((vec3(0,  4, 0)*rotMatlit)+vec3(0, -3, 0))
 , 0.5, vec3(1, 1, 1));
 sphereHitlight.normal *=rotMatlit3*rotMatlit2*rotMatlit;
  //  return  combine(sphereHitlight, combine(paraboloidHit,combine(sphere3Hit, combine(cylHit2, combine(sphere2Hit, combine(cylHit, combine(sphereHit, combine(planeHit, combine(cylHitbase,combine(planeHit2,buttonHit))))))))));
     return  combine(paraboloidHit,combine(sphere3Hit, combine(cylHit2, combine(sphere2Hit, combine(cylHit, combine(sphereHit, combine(planeHit, combine(cylHitbase,planeHit2))))))));
}

vec3 lighting(vec3 light, vec3 view, vec3 normal, vec3 color, float shininess, vec3 lightIntensity) {
    vec3 diffuse = color / PI;
    vec3 halfway = normalize(light + view);
    float specular = pow(max(dot(halfway, normal), 0.0), shininess);
    return color * AMBIENT + max(dot(normal, light), 0.0) * lightIntensity * (diffuse + specular);
}


void main() {

    float iTime =(frame)/20.0;
   // eye = vec3((eye.x - lookat.x) * cos(dt) + (eye.z - lookat.z) * sin(dt) + lookat.x,eye.y,-(eye.x - lookat.x) * sin(dt) + (eye.z - lookat.z) * cos(dt) + lookat.z);
    vec3 eye = vec3(10, 3, 10);
	vec3 lookat = vec3(0, 1, 0);
float a = iTime*7;
    vec3 rotMat3 = rotationMatrix(-lookat,a)*eye;
  //  vec3 rotMat3 =vec3(15, 3, 3);
    vec3 rayOrigin = vec3(rotMat3);
    //vec3((eye.x - lookat.x) * cos(iTime) + (eye.z - lookat.z) * sin(iTime) + lookat.x,eye.y,-(eye.x - lookat.x) * sin(iTime) + (eye.z - lookat.z) * cos(iTime) + lookat.z);
    float fov = PI / 2.0;
    mat4 proj = perspectiveProjection(fov, 600 / 600, 0.1, 100.0);
    mat4 view = lookatthis(rayOrigin, vec3(0));
    vec4 ndc = vec4(texCoord * 2.0 - 1.0, 1, 1);
    vec4 temp = inverse(proj) * ndc;
    vec3 worldPos = temp.xyz / temp.w;
    vec3 rayDirection = normalize(worldPos);
    rayDirection = mat3(view) * rayDirection;
    Ray ray = Ray(rayOrigin, rayDirection);
    Hit hit = sceneHit(ray, iTime);
    if (dot(hit.normal, ray.direction) > 0.0)
    hit.normal *= -1.0;
vec3 totallight1;
    //vec3 lightPos = vec3(cos(iTime * 5.0), 10.0, cos(3.14 * iTime)) * 5.0;
    vec3 lightPos = vec3(0,15,0);
    if (hit.t > 0.0) {
        vec3 hitPos = ray.origin + ray.direction * hit.t;
        vec3 toLight = normalize(lightPos - hitPos);
        Ray lightRay = Ray(hitPos + hit.normal * 0.01, toLight);
        Hit lightHit = sceneHit(lightRay, iTime);
        float light = 1.0;
        if (lightHit.t > 0.0&& lightHit.t <distance(hitPos, ray.origin)) {
            light = 0.0;
        }
        totallight1 =lighting(toLight, -ray.direction, hit.normal, hit.color, 1000.0, vec3(3.0 * light));

    }
Hit hit2 = sceneHit(ray, iTime);
if (dot(hit2.normal, ray.direction) > 0.0)
    hit2.normal *= -1.0;
 vec3 rotMat4 = rotationMatrix(-lookat,a)*eye;
mat3 rotMatlit = rotationMatrix(vec3(0.5,1,0.3),iTime*1.5);
mat3 rotMatlit2=rotationMatrix(vec3(1,1,0.5),iTime*2);
mat3 rotMatlit3=rotationMatrix(vec3(0,0.5,0.5),iTime*6);


  //vec3 lightPos2 =  rotMat2*vec3(0, 1, 0)+vec3(0, 1, 0) *rotMat;//itt adddom meg a helyét
    vec3 lightPos2 = ((vec3(0,2, 0)*rotMatlit3*rotMatlit2*rotMatlit)+vec3(0, 0, 0))+
                    ((vec3(0,4, 0)*rotMatlit2*rotMatlit)+vec3(0, 0, 0))+
                    ((vec3(0,  4, 0)*rotMatlit)+vec3(0, -3, 0));
    vec3 totallight2;
    if (hit2.t > 0.0) {
        vec3 hitPos2 = ray.origin + ray.direction * hit2.t;
        vec3 toLight2 = normalize(lightPos2 - hitPos2);
        Ray lightRay2 = Ray(hitPos2 + hit2.normal * 0.01, toLight2);
        Hit lightHit2 = sceneHit(lightRay2, iTime);
        float light2 = 1.0;
        if (lightHit2.t > 0.0 && lightHit2.t <distance(hitPos2, lightPos2)) {
            light2 = 0.0;
        }
        totallight2 =lighting(toLight2, -ray.direction, hit2.normal, hit2.color, 1000.0, vec3(10.0 * light2));
         }
    if (hit.t > 0.0 || hit2.t > 0.0)
        fragColor = vec4(totallight2+totallight1, 1);
        //fragColor = vec4(totallight1, 1);
         else {
        fragColor = vec4(vec3(0.5, 3, 1.0), 1);
    }}
)";

GPUProgram gpuProgram(false);
unsigned int vao;
int frame =21750;

void onInitialization() {
    glViewport(0, 0, windowWidth, windowHeight);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float vertices[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

void onDisplay() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
   gpuProgram.setUniform(((float)frame/100), "frame");
    frame++;

   // printf("/n %d",frame);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glutSwapBuffers();

}

void onKeyboard(unsigned char key, int pX, int pY) {
}

void onKeyboardUp(unsigned char key, int pX, int pY) {
}

void onMouseMotion(int pX, int pY) {
}

void onMouse(int button, int state, int pX, int pY) {
}

void onIdle() {
    glutPostRedisplay();
}