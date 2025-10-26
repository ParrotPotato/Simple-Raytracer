#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <math.h>

typedef int32_t s32;
typedef int64_t s64;
typedef int16_t s16;
typedef int8_t s8;

typedef uint32_t u32;
typedef uint16_t u16;
typedef uint64_t u64;
typedef uint8_t u8;

typedef float r32;
typedef double r64;

const r64 INF_NEG = -DBL_MAX;
const r64 INF_POS = DBL_MAX;

const r64 PI = 3.1415926535897932385;


inline r64 degrees_to_radians(r64 degree) {
    return degree * PI / 180.0;
}

inline r64 radians_to_degree(r64 radians) {
    return radians * 180.0 / PI;
}

template<typename T> 
inline T clamp(const T & min, const T & max, const T & value){
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

template<typename T>
inline bool inrange(const T & min, const T & max, const T & value){
    return (value > min && value < max);
}

template<typename T>
T abs(T x) {
    if (x < 0) return -x;
    return x;
}


struct vec3 {
    union {
        struct {r64 x, y, z;};
        struct {r64 r, g, b;};
        r64 data[3];
    };
    vec3() = default;
    vec3(r64 v1, r64 v2, r64 v3);
};

bool near_zero(vec3 a) {
    const r64 epsilon = 10e-5;
    return (abs(a.x) < epsilon && abs(a.y) < epsilon && abs(a.z) < epsilon);
}
vec3 operator - (const vec3 & a);
vec3 operator - (const vec3 & a, const vec3 & b);
vec3 operator + (const vec3 & a, const vec3 & b);
vec3 operator * (const vec3  & a, r64 scale);
vec3 operator * (const vec3  & a, const vec3 & b);
vec3 operator * (const r64 & scale, vec3 a);
vec3 operator / (const vec3  & a, r64 scale);

vec3 cross(const vec3 & a, const vec3 & b);
r64  lengthsq(const vec3 & v);
r64  length(const vec3 & v);
vec3 normalize(const vec3 & val);
r64  dot(const vec3 & a, const vec3 & b);

typedef vec3 color3; // for storing color information
typedef vec3 point3; // for distinction between a direction and a point in space;

color3 get_color_from_hex(u32 hexcolor){
    const u32 MASK = 255;
    color3 result = {};
    result.r = ((hexcolor >> 16) & MASK) / 255.0;
    result.g = ((hexcolor >> 8) & MASK) / 255.0;
    result.b = (hexcolor & MASK) / 255.0;
    return result;
}

struct interval_t  {
    r64 min, max;
    interval_t () = default;
    interval_t (r64 min, r64 max) : min(min), max(max) {}
};
const static interval_t universe(INF_NEG, INF_POS);
const static interval_t empty(INF_POS, INF_NEG);

inline r64 size(interval_t inv);
inline bool contains(interval_t inv, r64 value);
inline bool surrounds(interval_t inv, r64 value);
inline r64 clamp(interval_t inv, r64 x);

struct pixel_t {
    u8 r, g, b;
};

pixel_t color_to_pixel(const color3 & color, bool gamma = false){
    pixel_t result = {};

    color3 gamma_color = {};
    gamma_color.r = color.r > 0.0 ? sqrt(color.r) : color.r;
    gamma_color.g = color.g > 0.0 ? sqrt(color.g) : color.g;
    gamma_color.b = color.b > 0.0 ? sqrt(color.b) : color.b;

    if (gamma){
        result.r = clamp(0.0, 1.0, gamma_color.r) * 255;
        result.g = clamp(0.0, 1.0, gamma_color.g) * 255;
        result.b = clamp(0.0, 1.0, gamma_color.b) * 255;
    }
    else {
        result.r = clamp(-1.0, 1.0, color.r) * 255;
        result.g = clamp(-1.0, 1.0, color.g) * 255;
        result.b = clamp(-1.0, 1.0, color.b) * 255;
    }

    return result;
}

struct image_t {
    s32 width, height;
    pixel_t * pixels;
};

struct ray_t {
    point3 point;
    vec3 dir;
    ray_t() = default;
    ray_t(point3 point, vec3 dir): point(point), dir(dir) {}
};

point3 at(const  ray_t & ray, r64 delta) {
    return (ray.point + (ray.dir * delta));
}

enum material_type {
    Lambertian,
    Metallic,
    Dielectric,
};

struct lambertian_t {
    color3 albedo;
};

struct metallic_t {
    color3 albedo;
    r64 fuzziness;
};

struct dielectric_t {
    color3 albedo;
    r64 refractive;
};

struct material_t {
    material_type type;
    union {
        lambertian_t lambertian;
        metallic_t metallic;
        dielectric_t dielectric;
    };
};

enum entity_type {
    Sphere,
    Cube,
};

struct sphere_t {
    vec3 center;
    material_t mat;
    r64 radius;
};


struct entity_t {
    entity_type type;
    union {
        sphere_t sphere;
    };
};

struct hit_t {
    point3 point;
    vec3   normal;
    r64    delta;

    material_t mat;

    bool front_face;
};

inline hit_t create_hit_info_for_sphere(const ray_t & r, const r64 & t, const sphere_t & sphere){
    hit_t hit = {};

    // normal point out of the surface of the shape
    const vec3 outward_normal = (at(r, t) - sphere.center) / sphere.radius;

    hit.delta = t;
    hit.point = at(r, t);
    hit.front_face = dot(r.dir, outward_normal) <= 0.0;
    hit.normal = hit.front_face ? outward_normal : -outward_normal;
    hit.mat = sphere.mat;
    // we are storing normal opposite to the ray direction : just a design choice
    
    return hit;
}


bool sphere_hit(const ray_t & r, r64 tmin, r64 tmax, const sphere_t &sphere, hit_t * hit){

    vec3 oc = sphere.center - r.point;

    auto a = dot(r.dir, r.dir);
    auto h = dot(r.dir, oc);
    auto c = dot(oc, oc) - sphere.radius* sphere.radius;

    auto discriminant = h*h - a*c;

    if (discriminant < 0) return false;

    r64 dsqrt = sqrt(discriminant);


    r64 t1 = (h + dsqrt) / a;
    r64 t2 = (h - dsqrt) / a;

    if (inrange(tmin, tmax, t2)) {
        *hit = create_hit_info_for_sphere(r, t2, sphere);
        return true;
    } 

    return false;
}

inline r32 random_double(){
    return double(rand()) / double(RAND_MAX + 1.0);
}

inline r32 random_double(r64 min, r64 max){
    return min + (max - min) * random_double();
}

inline point3 random_in_unit_disk(){
    while(1){
        point3 p = point3(random_double(-1, 1), random_double(-1, 1), 0);
        if (lengthsq(p) <= 1){
            return p;
        }
    }
}


inline vec3 random_unit_vector() {
    while (true) {
        vec3 p = vec3(
                r64(rand() % 2000) / 1000.0, 
                r64(rand() % 2000) / 1000.0, 
                r64(rand() % 2000) / 1000.0) - vec3(1.0, 1.0, 1.0);
        r64 lsq = lengthsq(p);
        if (lsq > 10e-60 && lsq <= 1){
            return p / sqrt(lsq);
        }
    }
}

inline vec3 random_unit_in_hemisphere(const vec3 & normal) {
        vec3 result = {};
        while(true){
            result= random_unit_vector();
            r64 dotc = dot(result, normal);
            if (dotc == 0) continue;
            if (dotc < 0.0) {
                result= -result;
            }
            break;
        }
        return result;
}

inline vec3 reflect(const vec3 & inc, const vec3 & normal){
    vec3 result = inc - 2 * dot(inc, normal) * normal;
    return result;
}

inline vec3 refract(const vec3 & normalized_inc, const vec3 & normalized_normal, r64 n1overn2){
    r32 dotp = dot(-normalized_inc, normalized_normal);
    r64 cos_theta = dotp > 1.0 ? 1.0 : dotp;
    vec3 out_prep = n1overn2 * (normalized_inc  + cos_theta * normalized_normal);
    vec3 out_parallel = -sqrt(abs(1.0 - lengthsq(out_prep))) * normalized_normal;
    return out_prep + out_parallel;
}

color3 cast_ray(const ray_t & ray, entity_t entities[], u32 entityCount, u32 bounces_left){
    if (bounces_left == 0){
        return color3(0.0, 0.0, 0.0);
    }

    color3 color;

    vec3 direction = ray.dir;
    r64 delta = 0.0;

    //hit_t hits[5];
    //s32 hitCount = 0;

    bool hitted = false;
    hit_t minhit = {};

    for(s32 idx = 0 ; idx < entityCount ; idx++) {
        delta = 0.0;
        hit_t hit = {};
        if (entities[idx].type == Sphere && sphere_hit(ray, 0.001, INF_POS, entities[idx].sphere, &hit)){
            //hits[hitCount] = hit;
            //hitCount += 1;

            if (hitted == false || minhit.delta > hit.delta) {
                minhit = hit;
                hitted = true;
            }
        }
    }

    //if (hitCount > 0 != hitted) {
    //    printf("failed to execute\n");
    //    exit(-1);
    //}

    if (hitted){

        //r64 minDelta = INF_POS;
        //s32 minDeltaIndex = -1;

        //for (s32 idx = 0 ; idx < hitCount ; idx++) {
        //    if (minDelta > hits[idx].delta) {
        //        minDeltaIndex = idx;
        //        minDelta = hits[idx].delta;
        //    }
        //}
        //// @note: this was for checking if the normals are being displayed correctly or not
        //// color = 0.5 * (hits[minDeltaIndex].normal + color3(1.0, 1.0, 1.0));

        //hit_t hit = hits[minDeltaIndex];

        hit_t hit = minhit;
        //if (hit.delta != minhit.delta) {
        //    printf("failewd to execute\n");
        //    exit(-1);
        //}
        color3 attenuation = vec3(0.5, 0.5, 0.5);

        vec3 newdirection = {};
        // lambertian approximation 
        // light is scattered proportional to cos(phi) where phi is the angle the incident ray 
        // makes with the reflected one 

        // this gives a more sharper circle but make the lighting seems to be vrom above
        // newdirection = random_unit_in_hemisphere(hits[minDeltaIndex].normal)+ hits[minDeltaIndex].normal;

        // this gives a less sharp image but is more accurate to the scattering


        if (hit.mat.type == Lambertian) {
            newdirection = random_unit_vector() + hit.normal;
            if (near_zero(newdirection)){
                newdirection = hit.normal;
            }
            attenuation = hit.mat.lambertian.albedo;
        }
        else if (hit.mat.type == Metallic) {
            vec3 reflected = normalize(reflect(ray.dir, hit.normal));
            newdirection = reflected + random_unit_vector() * hit.mat.metallic.fuzziness;
            if(near_zero(newdirection)) {
                newdirection = reflected;
            }
            attenuation = hit.mat.metallic.albedo;
            
        }
        else if (hit.mat.type == Dielectric) {
            attenuation  = color3(1.0, 1.0, 1.0);
            auto n1overn2 = hit.front_face ? (1.0/hit.mat.dielectric.refractive) : hit.mat.dielectric.refractive;

            vec3 unit_direction = normalize(ray.dir);
            vec3 unit_normal = normalize(hit.normal);
            
            r64 cost = 0, sint = 0;
            {
                r64 d = dot(-unit_direction, unit_normal);
                cost = d > 1.0 ? 1.0 : d;
                sint = sqrt(1 - cost * cost);
            }

            r32 reflectance = 0;
            {
                auto r0 = (1 - n1overn2) / (1 + n1overn2);
                r0 = r0 * r0;
                reflectance = r0 + ( 1 - r0) * pow((1 - cost), 5);

            }

            if (n1overn2 * sint > 1.0 || reflectance > random_double()) {
                newdirection = reflect(unit_direction, unit_normal);
            } else { 
                newdirection = refract(unit_direction, unit_normal, n1overn2);
            }
        }
        
        color = attenuation * cast_ray(ray_t(hit.point, newdirection), entities, entityCount, bounces_left - 1);
    }
    else {
        r64 a = 0.5 * (direction.y + 1.0);
        color = (1.0 - a) * color3(1.0, 1.0, 1.0) + a * color3(0.5, 0.7, 1.0);
    }

    return color;
}

s32 write_image(const char * file, image_t * image);
s32 create_image(image_t * image, s32 width, s32 height, u32 color);

entity_t * create_entities(s32 * count){
    *count = 22 * 22 + 4;
    entity_t * entities = (entity_t *) malloc(sizeof(entity_t) * (*count));

    s32 offset = 0;

    entities[offset].type = Sphere;
    entities[offset].sphere.mat.type = Lambertian;
    entities[offset].sphere.mat.lambertian.albedo = get_color_from_hex(0x888888);
    entities[offset].sphere.center = point3(0, -1000, 0);
    entities[offset].sphere.radius = 1000;

    offset++;

    entities[offset].type = Sphere;
    entities[offset].sphere.mat.type = Dielectric;
    entities[offset].sphere.mat.dielectric.albedo = get_color_from_hex(0xffffff);
    entities[offset].sphere.mat.dielectric.refractive = 1.5;
    entities[offset].sphere.center = point3(0, 1, 0);
    entities[offset].sphere.radius = 1;
    
    offset++;

    entities[offset].type = Sphere;
    entities[offset].sphere.mat.type = Metallic;
    entities[offset].sphere.mat.metallic.albedo = color3(0.7, 0.6, 0.5);
    entities[offset].sphere.mat.metallic.fuzziness = 0.0;
    entities[offset].sphere.center = point3(4, 1, 0);
    entities[offset].sphere.radius = 1;
    

    offset++;

    entities[offset].type = Sphere;
    entities[offset].sphere.mat.type = Lambertian;
    entities[offset].sphere.mat.lambertian.albedo = color3(0.4, 0.2, 0.1);
    entities[offset].sphere.center = point3(-4, 1, 0);
    entities[offset].sphere.radius = 1;

    offset++;
    
    for(s32 i = -11 ; i < 11 ; i++){
        for(s32 j = -11 ; j < 11 ;  j++){
            r64 choose_mat = random_double();
            point3 center(i + 0.9 * random_double(), 0.2,  j + 0.9 * random_double());


            if (length(center - point3(4, 0.2, 9)) > 0.9) {

                if (choose_mat < 0.8 ){
                    entity_t entity = {};
                    entity.type = Sphere;
                    entity.sphere.mat.type = Lambertian;
                    entity.sphere.mat.lambertian.albedo = color3(random_double(), random_double(), random_double()) * color3(random_double(), random_double(), random_double());
                    entity.sphere.radius = 0.2;
                    entity.sphere.center = center;

                    entities[offset] = entity;
                    offset++;
                }

                else if (choose_mat < 0.95){
                    entity_t entity = {};
                    entity.type = Sphere;
                    entity.sphere.mat.type = Metallic;
                    entity.sphere.mat.metallic.albedo = color3(random_double(0.5, 1), random_double(0.5, 1), random_double(0.5, 1));
                    entity.sphere.mat.metallic.fuzziness = random_double(0, 0.5);
                    entity.sphere.radius = 0.2;
                    entity.sphere.center = center;

                    entities[offset] = entity;
                    offset++;
                }

                else {
                    entity_t entity = {};
                    entity.type = Sphere;
                    entity.sphere.mat.type = Dielectric;
                    entity.sphere.mat.dielectric.albedo = get_color_from_hex(0xffffff);
                    entity.sphere.mat.dielectric.refractive= 1.5;
                    entity.sphere.radius = 0.2;
                    entity.sphere.center = center;

                    entities[offset] = entity;
                    offset++;
                }
            }
        }
    }

    *count = offset;
   
    return entities;
}

int main(int argc, char ** argv) {

    srand(3000);

    const s32 MAX_RAY_BOUNCE = 50;
    const s32 RAYS_PER_PIXEL = 500;

    r64 aspect_ratio = 16.0/ 9.0;
    int image_width = 800;
    int image_height = (int)(image_width / aspect_ratio);


    // @note: camera properties

    // distance between the camera and view port
    r64 vfov = 20;
    vec3 camera_up = vec3(0, 1, 0);
    vec3 lookfrom = point3(13, 2, 3);
    vec3 look_at = point3(0, 0, 0);

    r64 defocus_angle = 0.6;
    r64 focus_dist = 10.0;

    vec3 u, v, w;
    w = normalize(lookfrom - look_at);
    u = normalize(cross (camera_up, w));
    v = normalize(cross(w, u));

    point3 camera_center = lookfrom;


    r64 viewport_height = 2 * tan(degrees_to_radians(vfov) / 2) * focus_dist;
    r64 viewport_width = viewport_height * ((r64)(image_width) / (r64)(image_height));

    vec3 viewport_u = viewport_width * u;
    vec3 viewport_v = viewport_height * -v;
    
    vec3 delta_u = viewport_u / image_width;
    vec3 delta_v = viewport_v / image_height;

    point3 viewport_top_left = camera_center - (focus_dist * w) - (viewport_u / 2) - (viewport_v / 2);
    auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2.0));

    vec3 defocus_disk_u = u * defocus_radius;
    vec3 defocus_disk_v = v * defocus_radius;

    point3 pixel00_loc = viewport_top_left + (delta_u + delta_v) * 0.5;

    image_t image;
    create_image(&image, image_width, image_height, 0xffffff);

    s32 entityCount = 0;
    entity_t * entities = create_entities(&entityCount);

    for(s32 i = 0 ; i < image_height ; i++){
        for(s32 ii =0 ; ii < image_width ; ii++){

            color3 colors[RAYS_PER_PIXEL] = {};

            for(s32 iii = 0 ; iii < RAYS_PER_PIXEL ; iii++){

                point3 pixel_center = pixel00_loc + (delta_v * i)  + (delta_u * ii);

                point3 ray_point = pixel_center + delta_u * 0.5 * (r64(rand() % 2000) / 1000.0 - 0.5) + delta_v * 0.5 * (r64(rand() % 2000) / 1000.0 - 0.5);

                point3 ray_origin = camera_center;
                {
                    if (defocus_angle > 0 ){
                        auto temp = random_in_unit_disk();
                        ray_origin = camera_center + (temp.data[0] * defocus_disk_u) + (temp.data[1]  * defocus_disk_v );
                    }
                }

                

                ray_t ray = {ray_origin, ray_point - ray_origin};

                color3 color = cast_ray(ray, entities, entityCount, MAX_RAY_BOUNCE);

                colors[iii] = color;
            }

            color3 avg(0.0, 0.0, 0.0);

            for(s32 iii =0 ; iii < RAYS_PER_PIXEL ; iii ++){
                avg = avg + colors[iii] * 1.0 / RAYS_PER_PIXEL;
            }

            image.pixels[image_width * i + ii] = color_to_pixel(avg, true);
        }
    }

    write_image("output.ppm", &image);
    return 0;
}

inline r64 size(interval_t inv) {
    return inv.max - inv.min;
}

inline bool contains(interval_t inv, r64 value) {
    return inv.max >= value && inv.min <= value;
}

inline bool surrounds(interval_t inv, r64 value)  {
    return inv.max > value && inv.min < value;
}

inline r64 clamp(interval_t inv, r64 x) {
    return clamp(inv.min, inv.max, x);
}


vec3::vec3(r64 x1, r64 x2, r64 x3){
    x = x1;
    y = x2;
    z = x3;
}

vec3 operator - (const vec3 & a){
    vec3 result = {
        -a.x,
        -a.y,
        -a.z,
    };
    return result;
}

vec3 operator - (const vec3 & a, const vec3 & b){
    vec3 result = {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
    return result;
}

vec3 operator + (const vec3 & a, const vec3 & b) {
    vec3 result = {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
    return result;
}

vec3 operator * (const vec3  & a, r64 scale){
    vec3 result = {
        a.x * scale,
        a.y * scale,
        a.z * scale,
    };
    return result;
}

vec3 operator * (const vec3 & b, const vec3 & a){
    vec3 result = {
        a.x * b.x,
        a.y * b.y,
        a.z * b.z,
    };
    return result;
}


vec3 operator * (const r64 & scale, vec3 a){
    vec3 result = {
        a.x * scale,
        a.y * scale,
        a.z * scale,
    };
    return result;
}

vec3 operator / (const vec3  & a, r64 scale){
    vec3 result = {
        a.x / scale,
        a.y / scale,
        a.z / scale,
    };
    return result;
}

vec3 cross(const vec3 & a, const vec3 & b) {
    vec3 result = {};
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

r64 lengthsq(const vec3 & v) {
    return (v.x * v.x + v.y * v.y + v.z * v.z);
}

r64 length(const vec3 & v){
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3 normalize(const vec3 & val){
    if (length(val) == 0) {
        return vec3(0.0, 0.0, 0.0);
    }
    return val/ length(val);
}


r64 dot(const vec3 & a, const vec3 & b) {
    return (a.x * b.x + a.y * b.y + a.z * b.z);
}


s32 create_image(image_t * image, s32 width, s32 height, u32 color){
    const u64 MASK = 255;
    s32 result = 0;
    image->pixels = (pixel_t *) malloc(sizeof(pixel_t) * width * height);
    for(s64 i = 0 ; i < width * height ; i++) {
        image->pixels[i].r = (color >> 16) & MASK ;
        image->pixels[i].g = (color >> 8) &  MASK ;
        image->pixels[i].b = (color) & MASK ;
    }
    image->width = width;
    image->height = height;
    return result;
}

s32 write_image(const char * file, image_t * image) {
    s32 result = 0;
    FILE * fp = fopen(file, "w");
    if (!fp) {
        result = -1;
        goto return_image_write_result;
    }
    fprintf(fp, "P3\n%d %d\n255\n", image->width, image->height);
    for(s64 i = 0 ; i < image->width * image->height; i++){
        fprintf(fp, "%u %u %u\n", image->pixels[i].r, image->pixels[i].g, image->pixels[i].b);
    }
    fclose(fp);

return_image_write_result:
    return result;
}
