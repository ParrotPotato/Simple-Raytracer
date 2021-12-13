#ifndef RAYTRACER_CC
#define RAYTRACER_CC

#include "types.hh"
#include "vector.hh"

#include <cassert>

// NOTE(nitesh): For debigging purpose only, do not printf from a hot path
#include <cstdio>

typedef v4 Color;

Color multiply_color(const Color & color, const Color & light)
{
    v4 result;
    result.x = color.x * light.x;
    result.y = color.y * light.y;
    result.z = color.z * light.z;
    result.w = color.w * light.w;
    return result;
}

Color add_color(const Color & light1, const Color & light2)
{
    return light1 + light2;
}

u32 convert_color_to_u32(const Color & color)
{
    // gamma correction can be added here later
    u32 result = 0;
    result |= ((u32) (color.x * 255)) << 24;
    result |= ((u32) (color.y * 255)) << 16;
    result |= ((u32) (color.z * 255)) <<  8;
    result |= ((u32) (color.w * 255)) <<  0;
    return result;
}

void clip_color_value(Color & color)
{
    color.x = ((color.x > 1.0)? 1.0f : ((color.x < 0.0)? 0.0f : color.x));
    color.y = ((color.y > 1.0)? 1.0f : ((color.y < 0.0)? 0.0f : color.y));
    color.z = ((color.z > 1.0)? 1.0f : ((color.z < 0.0)? 0.0f : color.z));
    color.w = ((color.w > 1.0)? 1.0f : ((color.w < 0.0)? 0.0f : color.w));
}


struct Ray
{
    v3 origin;
    v3 direction;
};

struct Sphere
{
    v3 origin;
    r32 radius;

    Color color;

    r32 get_intersection(const Ray & ray)
    {
        v3 od = origin - ray.origin;
        r32 b = dot(od, ray.direction);
        r32 delta = b * b - dot(od, od) + radius * radius;
        if(delta < 0) return 0; 
        delta = sqrt(delta);
        return (((b - delta) > 0) ? (b - delta) : (((b + delta) > 0) ? (b + delta) : 0));
    }
};

struct DirectionalLight
{
    v3 position;
    Color color;
};

struct AmbientLight
{
    Color color;
};

// Scene assumes that the scene is set up in the negative z direction and the
// camera is places at the positive z direction ( the display being at the origin)
// with (width/2.0f, buffer_height/2.0f) as the exact origin location

#define MAX_SPHERE_COUNT 25
#define MAX_LIGHT_COUNT 25

struct Scene 
{
    Sphere spheres[MAX_SPHERE_COUNT];
    u32 sphere_count = 0;

    DirectionalLight directional_light[MAX_LIGHT_COUNT];
    u32 directional_light_count = 0;

    AmbientLight ambient_light;

    r32 render_width = 0;
    r32 render_height = 0;

    r32 zposition = 0;

    void initialise(r32 render_width_, r32 render_height_, r32 fov_)
    {
        render_width = render_width_;
        render_height = render_height_;

        zposition = render_width / (2.0f * tan(fov_ / 2.0f * M_PI / 180.0f));
    }

    void add_sphere(const Sphere & sphere)
    {
        spheres[sphere_count] = sphere;
        sphere_count += 1;
    }

    void add_directional_light(const DirectionalLight & light)
    {
        directional_light[directional_light_count] = light;
        directional_light_count += 1;
    }

    Color get_color_for_pixel(
            const u32 & xoffset,
            const u32 & yoffset,
            const u32 & buffer_width,
            const u32 & buffer_height);

};

Color Scene::get_color_for_pixel(
        const u32 & xoffset,
        const u32 & yoffset,
        const u32 & buffer_width,
        const u32 & buffer_height)
{
    r32 xpos = - render_width / 2.0f  + xoffset * render_width / (r32) buffer_width;
    r32 ypos =   render_height / 2.0f - yoffset * render_height / (r32) buffer_height;

    v3 camera_position = v3(0.0f, 0.0f, zposition);

    Ray ray;
    ray.origin = camera_position;
    ray.direction = normalised(v3(xpos, ypos, 0.0f) - ray.origin);

    char buffer[128];

    /*

    ray.origin.to_string(buffer, sizeof(buffer));
    printf("[debug]: ray origin : %s\n", buffer);

    ray.direction.to_string(buffer, sizeof(buffer));
    printf("[debug]: ray direction : %s\n", buffer);

    */

    Color result = v4(0.0f);

    const r32 diffusion_coefficient = 0.7f;
    const r32 specular_coefficient = 0.7f;

    const r32 specular_cuttoff = 0.95f;

    for(u32 i = 0 ; i  < sphere_count ; i++)
    {
        r32 delta = spheres[i].get_intersection(ray);
        if(delta <= 0) continue;

        v3 collision_point = ray.origin + delta * ray.direction;
        v3 normal = normalised(collision_point - spheres[i].origin); 

        Color diffused_light_color = v4(0.0f);
        Color specular_light_color = v4(0.0f);
        Color ambient_light_color = v4(0.0f);

        Color light_color;

        // calculating color based on directional lights in the secne
        for(u32 j = 0 ; j < directional_light_count ; j++)
        {
            DirectionalLight & light = directional_light[j];

            { // diffused lighting calculation 

                r32 dot_product = dot(normalised(light.position - collision_point), normal);

                if(dot_product <= 0) dot_product = 0.0f;

                diffused_light_color = add_color(diffused_light_color, dot_product * light.color);
            }

            { // specular lighting calculation

                v3 incidence_ray = normalised(light.position - collision_point);
                // calculating incidence ray ...
                v3 reflected_ray = normalised((2 * dot(normal, incidence_ray)) * normal - incidence_ray);

                v3 to_camera = normalised(camera_position - collision_point);
                r32 dot_product = dot(to_camera, reflected_ray);

                if(dot_product <= specular_cuttoff) dot_product = 0.0f;

                specular_light_color = add_color(specular_light_color, dot_product * light.color);
            }

        }

        // adding ambient light

        ambient_light_color = multiply_color(spheres[i].color, ambient_light.color);
        diffused_light_color = multiply_color(spheres[i].color, diffused_light_color);

        light_color = ambient_light_color + diffusion_coefficient * diffused_light_color + specular_coefficient * specular_light_color;
        clip_color_value(light_color);

        result = light_color;
        break;
    }

    return result;
}

#endif
