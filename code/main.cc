// including the other required files (unity build system)

#include "raytracer.cc"

// headers which are local to the main file

#include <SDL2/SDL.h>
#include <cstdio>

struct thread_work_queue
{
    SDL_mutex * lock = 0;

    int current_row_index = 0;
    int height = 0;
};

thread_work_queue create_thread_work_queue(int height)
{
    thread_work_queue work_queue;

    work_queue.lock = SDL_CreateMutex();
    assert(work_queue.lock != 0);

    work_queue.height = height;

    return work_queue;
}

int get_next_work(thread_work_queue * work_queue)
{
    int result;

    if(work_queue->current_row_index == work_queue->height) return -1;

    if (SDL_LockMutex(work_queue->lock) == 0)
    {
        result = work_queue->current_row_index;
        work_queue->current_row_index += 1;

        SDL_UnlockMutex(work_queue->lock);
    }
    else
    {
        assert(false);
    }

    return result;
}

struct thread_data
{
    i32 thread_idx;
    thread_work_queue * work_queue;


    u32 * buffer;
    Scene * scene;
    u32 width, height;

    // other scene data
};


static int thread_function(void * data)
{
    i32 thread_idx = ((thread_data *) data)->thread_idx;

    thread_work_queue * work_queue = ((thread_data *) data)->work_queue;

    u32 * buffer  = ((thread_data *) data)->buffer;
    u32 buffer_width = ((thread_data *) data)->width;
    u32 buffer_height = ((thread_data *) data)->height;

    Scene * scene = ((thread_data *) data)->scene;

    printf("[debug-thread-%d] required context for thread execution loaded\n", thread_idx);

    printf("[debug-thread-%d] pointer to scene   : %p\n", thread_idx, scene);
    printf("[debug-thread-%d] scene.sphere_count : %u\n", thread_idx, scene->sphere_count);
    printf("[debug-thread-%d] scene.light_count  : %u\n", thread_idx, scene->directional_light_count);

    int result = get_next_work(work_queue);
    while(result != -1)
    {
//        printf("[thread-%d] processing row - %d\n", thread_idx, result);

        for(u32 i = 0 ; i < buffer_width; i++)
        {
            Color color = scene->get_color_for_pixel(i, result, buffer_width, buffer_height);
            buffer[result * buffer_width + i] = convert_color_to_u32(color);
        }

        result = get_next_work(work_queue);
    }

    printf("[debug-thread-%d] thread execution exited without error\n", thread_idx);

    return 0;
}


int main(void)
{
#if 1
    // window initialisation

    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_Window * window = SDL_CreateWindow("main window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, 0);
    assert(window != 0);

    i32 window_width = 800;
    i32 window_height= 600;

    i32 buffer_width = 16 * window_width;
    i32 buffer_height = 16 * window_height;

    SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    assert(renderer != 0);

    SDL_Texture * main_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, buffer_width, buffer_height);
    assert(main_texture != 0);

    u32 buffer_len = buffer_len * buffer_height;
    u32 * buffer = (u32 *) malloc(sizeof(u32) * buffer_len);
    assert(buffer != 0);

    // TODO(nitesh): Initialise font rendering



    // Initialising scene

    Sphere sphere1;
    sphere1.origin = v3(0.0f, 50.0f, -100.0f);
    sphere1.radius = 60.0f;
    sphere1.color = v4(1, 0.839, 0.839, 1.0f);

    Sphere sphere2;
    sphere2.origin = v3(-20.0f, -80.0f, -100.0f);
    sphere2.radius = 60.0f;
    sphere2.color = v4(0.996, 1, 0.741, 1.0f);

    Sphere sphere3;
    sphere3.origin = v3(540.0f, -15.0f, -2500.0f);
    sphere3.radius = 60.0f;
    sphere3.color = v4(0.839, 0.847, 1, 1.0f);

    DirectionalLight light1;
    light1.position = v3(100.0f, 100.0f, 100.0f);
    light1.color    = v4(1.0f, 1.0f, 1.0f, 1.0f);

    DirectionalLight light2;
    light2.position = v3(-100.0f, -100.0f, 100.0f);
    light2.color    = v4(0.0f, 1.0f, 0.0f, 1.0f);

    DirectionalLight light3;
    light3.position = v3(-100.0f, 100.0f, 100.0f);
    light3.color    = v4(1.0f, 0.0f, 0.0f, 1.0f);

    AmbientLight ambient_light;
    ambient_light.color = v4(0.1f, 0.1, 0.1, 1.0f);

    Scene scene;
    scene.initialise(window_width, window_height, 90);

    scene.add_sphere(sphere1);
    scene.add_sphere(sphere2);
    scene.add_sphere(sphere3);

    scene.add_directional_light(light1);
//    scene.add_directional_light(light2);
    scene.add_directional_light(light3);

    scene.ambient_light = ambient_light;

    printf("[debug] scene zcoordinate of the camera: %f \n", scene.zposition);

    // thread creation
    
    thread_work_queue work_queue = create_thread_work_queue(buffer_height);

    thread_data data;

    data.work_queue = &work_queue;
    data.buffer = buffer;
    data.width = buffer_width;
    data.height = buffer_height;

    data.scene = &scene;

    const int thread_count = 4;

    SDL_Thread * threads[thread_count];

    for(int i = 0 ; i < thread_count ; i++)
    {
        char thread_name_buffer[128];
        sprintf(thread_name_buffer, "%s[%d]", "worker", i);

        data.thread_idx = i;
        threads[i] = SDL_CreateThread(thread_function, thread_name_buffer, (void *) &data);
    }

    bool is_program_running = true;

    while(is_program_running)
    {
        // updating state

        SDL_UpdateTexture(main_texture, 0, buffer, buffer_width * sizeof(u32));

        // process events 

        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
                case SDL_QUIT:
                {
                    is_program_running = false;
                } 
                break;
                
                case SDL_KEYDOWN:
                {
                    is_program_running = (event.key.keysym.sym == SDLK_ESCAPE) ? false : true;
                } 
                break;

                default:
                break;
            }
        }

        // clear display window

        SDL_RenderClear(renderer);

        SDL_RenderCopy(renderer, main_texture, 0, 0);

        SDL_RenderPresent(renderer);
    }

    // TODO(nitesh): Implementing worker thread termination when main thread is requested with
    // SDL_QUIT or Keyboard Interrupt instead of waiting for worker threads to complete executing
    
    // thread count


    for(int i = 0 ; i < thread_count ; i++)
    {
        int returnvalue = 0;
        SDL_WaitThread(threads[i], &returnvalue);
    }

    SDL_Quit();

    return 0;

#else
    // TEST: for checking wether the intersection is working the expected way or not
    Sphere sphere;
    sphere.origin = v3(0.0f, 0.0f, -50.0f);
    sphere.radius = 20.0f;

    Ray ray;
    ray.origin = v3(0.0f, 0.0f, 10.0f);
    ray.direction = normalised(v3(1.0f, 1.0f, -10.0f));

    r32 result = sphere.get_intersection(ray);

    v3 position = ray.origin + (result * ray.direction);

    printf("debug - intersection distance %f\n", result);
    printf("debug - point of intersection %f, %f, %f\n", position.x, position.y, position.z);

    return 0;
#endif
}
